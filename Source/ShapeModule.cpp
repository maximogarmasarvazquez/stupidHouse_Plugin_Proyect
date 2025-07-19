#include "ShapeModule.h"
#include <cmath>

void ShapeModule::prepare(double sampleRate, int /*samplesPerBlock*/, int /*numChannels*/)
{
    smoothedAGC.reset(sampleRate, 0.05); // Suavizado 50 ms para ganancia automática
    smoothedAGC.setCurrentAndTargetValue(1.0f);
    silenceCounter = 0;
}

void ShapeModule::setParameters(ShapePreset preset, float drive, float outGain, bool applySoftClip)
{
    currentPreset = preset;
    driveAmount = juce::jlimit(0.f, 1.f, drive);
    outputGain = outGain;
    softClipEnabled = applySoftClip;
}


float ShapeModule::saturateTape(float x)
{
    float gain = 1.0f + driveAmount * 3.5f;
    float bias = 0.08f * driveAmount;
    float driven = gain * x + bias;

    float compressed = driven / (1.0f + 0.6f * std::abs(driven));
    float softened = std::tanh(compressed * 0.9f);

    float mix = juce::jlimit(0.4f, 0.8f, 0.4f + 0.5f * driveAmount);

    return ((1.0f - mix) * x + mix * softened) * 1.1f;
}


float ShapeModule::saturateSoft(float x)
{
    float k = driveAmount * 6.0f + 0.1f;
    float shaped = std::tanh(k * x);
    return shaped * (0.9f - 0.2f * driveAmount);
}

float ShapeModule::saturateHard(float x)
{
    float gain = 1.0f + driveAmount * 4.0f;
    float driven = gain * x;

    float clipped = juce::jlimit(-0.9f, 0.9f, driven);

    if (clipped > 0.75f)
        clipped = 0.75f + 0.25f * std::tanh((clipped - 0.75f) * 5.0f);
    else if (clipped < -0.75f)
        clipped = -0.75f + 0.25f * std::tanh((clipped + 0.75f) * 5.0f);

    return clipped;
}


float foldbackFunc(float input, float threshold)
{
    if (input > threshold)
        return threshold - (input - threshold);
    else if (input < -threshold)
        return -threshold - (input + threshold);
    else
        return input;
}

float ShapeModule::saturateFoldback(float x)
{
    float gain = 1.0f + driveAmount * 4.0f;
    float shaped = gain * x;
    constexpr float threshold = 1.0f;

    // Aplicar foldback una o más veces para suavizar
    for (int i = 0; i < 2; ++i)
        shaped = foldbackFunc(shaped, threshold);

    shaped = std::tanh(shaped * 1.5f);

    float mix = juce::jlimit(0.f, 1.f, driveAmount);
    return (1.0f - mix) * x + mix * shaped;
}

float ShapeModule::mapDriveValue(float input)
{
    constexpr float maxDrive = 3.5f;
    constexpr float exponent = 2.0f;
    return std::pow(input, exponent) * maxDrive;
}

void ShapeModule::updateParametersFromPreset(int rawPreset, float drive, bool applySoftClip)
{
    ShapePreset safePreset = static_cast<ShapePreset>(juce::jlimit(0, 4, rawPreset));
    setParameters(safePreset, drive, 1.0f, applySoftClip);
}

float ShapeModule::getPresetGainCompensation() const
{
    switch (currentPreset)
    {
    case ShapePreset::Soft:     return 1.0f / (1.0f + 0.2f * driveAmount);
    case ShapePreset::Hard:     return 1.0f / (1.0f + 0.35f * driveAmount);
    case ShapePreset::Tape:     return 1.0f / (1.0f + 0.08f * driveAmount); // menos compensación
    case ShapePreset::Foldback: return 1.0f / (1.0f + 0.5f * driveAmount);
    case ShapePreset::Clean:
    default:                   return 1.0f;
    }
}

void ShapeModule::processWithCompensation(juce::AudioBuffer<float>& buffer, float dryWet, juce::SmoothedValue<float>& smoothedGain)
{
    if (currentPreset == ShapePreset::Clean)
        return;

    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);

    float baseTarget = 0.5f;
    if (currentPreset == ShapePreset::Tape)
        baseTarget = 0.7f;  // menos reducción para Tape

    float inputLevel = buffer.getRMSLevel(0, 0, buffer.getNumSamples());
    float targetGain = (inputLevel > 0.0001f) ? (baseTarget / inputLevel) : 1.0f;

    smoothedGain.setTargetValue(targetGain);
    float smoothGain = smoothedGain.getNextValue();

    buffer.applyGain(smoothGain);

    process(buffer);

    if (dryWet < 1.0f)
    {
        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float* wetData = buffer.getWritePointer(ch);
            const float* dryData = dryBuffer.getReadPointer(ch);

            for (int i = 0; i < numSamples; ++i)
                wetData[i] = dryData[i] * (1.0f - dryWet) + wetData[i] * dryWet;
        }
    }
}

void ShapeModule::process(juce::AudioBuffer<float>& buffer)
{
    if (currentPreset == ShapePreset::Clean)
        return;

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    float gainCompensation = getPresetGainCompensation();

    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* data = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i)
        {
            float x = data[i];
            float y = 0.0f;

            switch (currentPreset)
            {
            case ShapePreset::Soft:     y = saturateSoft(x); break;
            case ShapePreset::Hard:     y = saturateHard(x); break;
            case ShapePreset::Tape:     y = saturateTape(x); break;
            case ShapePreset::Foldback: y = saturateFoldback(x); break;
            default:                   y = x; break;
            }

            if (softClipEnabled)
                y = std::tanh(y);

            data[i] = y * gainCompensation * outputGain;
        }
    }
}
