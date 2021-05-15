/*
  ==============================================================================

    Analyser.cpp
    Created: 12 Apr 2021 8:01:02pm
    Author:  James Kelly

  ==============================================================================
*/

#include "Analyser.h"
#include "Simulators/Simulator.h"
#include "PL_SYSTEM.h"
#include "PL_SCENE.h"
#include "FreeGrid.h"
#include <sstream>

void Analyser::Encode(Simulator* Simulator, PLVector EncodingPosition, int* OutVoxelIndex)
{
    if (!Simulator || !OutVoxelIndex)
    {
        return;
    }
    
    int EncodingIndex;
    Simulator->GetScene()->GetVoxelIndexOfPosition(EncodingPosition, &EncodingIndex);
    
    *OutVoxelIndex = EncodingIndex;
    
    std::string EncodingIndexString = std::to_string(EncodingIndex) + ".wav";
    
    juce::File DesktopDirectory = juce::File::getSpecialLocation(juce::File::userDesktopDirectory);
    juce::File OutputFile = DesktopDirectory.getChildFile(EncodingIndexString);
    OutputFile.deleteFile();    // Delete so we can override ???

    float SamplingRate = Simulator->GetSamplingRate();

    if (std::unique_ptr<juce::FileOutputStream> FileStream = std::unique_ptr<juce::FileOutputStream>(OutputFile.createOutputStream()))
    {
        juce::WavAudioFormat WavFormat;

        if (auto Writer = WavFormat.createWriterFor(FileStream.get(), SamplingRate, 1, 16, {}, 0))
        {
            FileStream.release(); // (passes responsibility for deleting the stream to the writer object that is now using it)

            juce::AudioBuffer<float> AudioBuffer (1, SamplingRate);

            const std::vector<std::vector<PLVoxel>>& SimulatedLattice = Simulator->GetSimulatedLattice();

            const std::vector<PLVoxel>& Response = SimulatedLattice[EncodingIndex];

            for(int i = 0; i < Response.size(); ++i)
            {
                AudioBuffer.addSample(0, i, static_cast<float>(Response[i].AirPressure));
            }

            Writer->writeFromAudioSampleBuffer(AudioBuffer, 0, static_cast<int>(Response.size()));

            delete Writer;
        }
    }
    /**
     AudioBuffer<float> buffer;
     WavAudioFormat format;
     std::unique_ptr<AudioFormatWriter> writer;
     writer.reset (format.createWriterFor (new FileOutputStream (file),
                                           48000.0,
                                           buffer.getNumChannels(),
                                           24,
                                           {},
                                           0));
     if (writer != nullptr)
         writer->writeFromAudioSampleBuffer (buffer, 0, buffer.getNumSamples());
     */
}

void Analyser::GetOcclusion(Simulator* Simulator, PLVector EncodingPosition, float* OutOcclusion)
{
    const int NumSamples = Simulator->GetTimeSteps();
    const float SamplingRate = Simulator->GetSamplingRate();
    
    const PL_SCENE* Scene = Simulator->GetScene();
    
    // It can be assumed the encoding position in the emitter location
    int EmitterIndex;   // The array index
    int EmitterX, EmitterY, EmitterZ;   // The 3D world indexes of the emitter
    Scene->GetVoxelIndexOfPosition(EncodingPosition, &EmitterIndex);
    Scene->GetThreeDimensionalIndexOfIndex(EmitterIndex, EmitterX, EmitterY, EmitterZ);
    
    const std::vector<std::vector<PLVoxel>>& SimulatedLattice = Simulator->GetSimulatedLattice();
    const std::vector<PLVoxel> Response = SimulatedLattice[EmitterIndex];   // Response at the emitter location
    
    PLVector ListenerLocation;
    PL_SYSTEM* System;
    Scene->GetSystem(&System);
    System->GetListenerPosition(ListenerLocation);
    int ListenerIndex;
    int ListenerX, ListenerY, ListenerZ;
    Scene->GetVoxelIndexOfPosition(ListenerLocation, &ListenerIndex);
    Scene->GetThreeDimensionalIndexOfIndex(ListenerIndex, ListenerX, ListenerY, ListenerZ);

    //
    // ONSET DELAY
    //
    int OnsetSample = 0; // first sample that meets a certain threshold
    for (; OnsetSample < NumSamples; ++OnsetSample)
    {
        double Next = Response[OnsetSample].AirPressure;
        if (std::abs(Next) > 0.00000316f)   // precomputed value for -110db
        {
            break;
        }
    }
    
     const int DirectGainSamples = static_cast<int>(0.01f * SamplingRate);    // go forward 10ms
     const int DirectEnd = OnsetSample + DirectGainSamples;

     float ObstructionGain = 0.0f;
     {
         float Edry = 0;

         int j = 0;
         for (; j < DirectEnd; ++j)
         {
             const double AirPressure = Response[j].AirPressure;
             Edry += AirPressure * AirPressure;
         }

         // Normalize dry energy by free-space energy to obtain geometry-based
         // obstruction gain with distance attenuation factored out
         double EfreePr = 0.0f;
         {
             FreeGrid* FreeGrid;
             Scene->GetFreeGrid(&FreeGrid);
             EfreePr = FreeGrid->GetFreeEnergy(ListenerX, ListenerZ, EmitterX, EmitterZ);
         }

         float E = (Edry / EfreePr);
         ObstructionGain = std::sqrt(E);
         
         std::ostringstream StringStream;
         StringStream << "Occlusion: " << ObstructionGain << ". Final Energy: " << E << ". Energy From Geometry: " << Edry << ". Energy From Grid: " << EfreePr;
         DebugLog(StringStream.str().c_str());
     }
    
//    double r = 1.0f / std::max(0.001f, ObstructionGain);
     
    *OutOcclusion = ObstructionGain;
}
