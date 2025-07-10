#include "ShapeModule.h"
using namespace ShapeIntern;

// ─────────────────────────────────────────────────────────────────────────────
void ShapeModule::prepare(double sampleRate,
    int samplesPerBlock,
    int numChannels)
{
    juce::dsp::ProcessSpec spec{ sampleRate,
                                 static_cast<juce::uint32>(samplesPerBlock),
                                 static_cast<juce::uint32>(numChannels) };

    os.reset();
    os.initProcessing(samplesPerBlock);  // JUCE 8: necesario

    shaper.prepare(spec);
    rebuildFunction();
}

// ─────────────────────────────────────────────────────────────────────────────
void ShapeModule::setParameters(ShapePreset preset, float driveAmount, bool agc)
{
    enableAGC = agc;

    const int presetIndex = static_cast<int>(preset);

    if (presetIndex != curveType || driveAmount != drive)
    {
        curveType = presetIndex;
        drive = driveAmount;
        rebuildFunction();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void ShapeModule::process(juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    if (curveType == 0 || drive <= 0.001f) // 0 = Clean
        return;

    float inputRMS = 0.0f;
    if (enableAGC)
        inputRMS = buffer.getRMSLevel(0, 0, numSamples); // canal 0

    // Aplicar distorsión por muestra
    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* data = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i)
            data[i] = processSample(data[i]);
    }

    // Compensación de ganancia
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

// ─────────────────────────────────────────────────────────────────────────────
void ShapeModule::rebuildFunction()
{
    gType = curveType;
    gDrive = drive;

    shaper.functionToUse = [](float x) -> float
        {
            switch (gType)
            {
            case 0:  return x;              // Clean
            case 1:  return softClip(x);
            case 2:  return hardClip(x);
            case 3:  return tapeSat(x);
            case 4:  return foldback(x);
            default: return x;
            }
        };
}

// ─────────────────────────────────────────────────────────────────────────────
float ShapeModule::processSample(float x)
{
    return shaper.functionToUse(x);
}