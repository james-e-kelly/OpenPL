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

void Analyser::Encode(Simulator* Simulator, PLVector EncodingPosition)
{
    juce::File DesktopDirectory = juce::File::getSpecialLocation(juce::File::userDesktopDirectory);
    juce::File OutputFile = DesktopDirectory.getNonexistentChildFile("TestImpulseResponse", ".wav");
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

            int EncodingIndex;
            Simulator->GetScene()->GetVoxelIndexOfPosition(EncodingPosition, &EncodingIndex);

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
