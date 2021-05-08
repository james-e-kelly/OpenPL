/*
  ==============================================================================

    Analyser.cpp
    Created: 12 Apr 2021 8:01:02pm
    Author:  James Kelly

  ==============================================================================
*/

#include "Analyser.h"
#include "Simulators/Simulator.h"
#include "PL_SCENE.h"
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
    
    int EncodingIndex;
    Simulator->GetScene()->GetVoxelIndexOfPosition(EncodingPosition, &EncodingIndex);
    
    const std::vector<std::vector<PLVoxel>>& SimulatedLattice = Simulator->GetSimulatedLattice();
    const std::vector<PLVoxel> Response = SimulatedLattice[EncodingIndex];

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
//         Real EfreePr = 0.0f;
//         {
//             const int listenerX = (int)(listenerPos.x * (1.f / m_dx));
//             const int listenerY = (int)(listenerPos.z * (1.f / m_dx));
//             const int emitterX = gridIndex.x;
//             const int emitterY = gridIndex.y;
//
//             EfreePr = m_freeGrid->GetEFreePerR(listenerX, listenerY, emitterX, emitterY);
//         }

//         float E = (Edry / EfreePr);
//         ObstructionGain = std::sqrt(E);
     }
     
    *OutOcclusion = ObstructionGain;
}
