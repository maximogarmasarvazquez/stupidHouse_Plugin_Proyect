// ShapeModule.cpp
#include "ShapeModule.h"
using namespace ShapeIntern;

void ShapeModule::prepare(double sampleRate, int samplesPerBlock, int numChannels)
{
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(numChannels);

    shaper.prepare(spec);
    rebuildFunction();
}

void ShapeModule::setParameters(ShapePreset preset, float driveAmount, float dryWetAmount, bool agc)
{
    enableAGC = agc;
    curveType = static_cast<int>(preset);
    drive = juce::jlimit(0.f, 5.f, driveAmount); // drive limitado para evitar crujidos fuertes
    dryWet = juce::jlimit(0.f, 1.f, dryWetAmount);

    rebuildFunction();
}

void ShapeModule::process(juce::AudioBuffer<float>& buffer)
{
    if (curveType == 0 || drive <= 0.001f || dryWet <= 0.001f)
        return;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    juce::AudioBuffer<float> dryBuffer(numChannels, numSamples);
    dryBuffer.makeCopyOf(buffer);

    float inputRMS = 0.f;
    if (enableAGC)
        inputRMS = buffer.getRMSLevel(0, 0, numSamples);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* data = buffer.getWritePointer(ch);
        const float* dryData = dryBuffer.getReadPointer(ch);

        for (int i = 0; i < numSamples; ++i)
        {
            float distortedSample = processSample(data[i]);
            data[i] = dryData[i] * (1.0f - dryWet) + distortedSample * dryWet;
        }
    }

    if (enableAGC)
    {
        float outputRMS = buffer.getRMSLevel(0, 0, numSamples);
        float comp = (outputRMS > 0.0f) ? (inputRMS / outputRMS) : 1.0f;

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float* data = buffer.getWritePointer(ch);
            for (int i = 0; i < numSamples; ++i)
                data[i] *= comp;
        }
    }
}

void ShapeModule::rebuildFunction()
{
    gType = curveType;
    gDrive = drive;

    shaper.functionToUse = [](float x) -> float
        {
            switch (gType)
            {
            case 0: return x;
            case 1: return softClip(x);
            case 2: return hardClip(x);
            case 3: return tapeSat(x);
            case 4: return foldback(x);
            default: return x;
            }
        };
}

float ShapeModule::processSample(float x)
{
    return shaper.functionToUse(x);
}
