#include "ShapeModule.h"
#include <cmath>

void ShapeModule::prepare(double sampleRate, int /*samplesPerBlock*/, int /*numChannels*/)
{
    smoothedAGC.reset(sampleRate, 0.05); // 50 ms smoothing time
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

float ShapeModule::getPresetGainCompensation() const
{
    switch (currentPreset)
    {
    case ShapePreset::Soft:
        return 1.0f / (1.0f + 0.2f * driveAmount);
    case ShapePreset::Hard:
        return 1.0f / (1.0f + 0.35f * driveAmount);
    case ShapePreset::Tape:
        return 1.0f / (1.0f + 0.15f * driveAmount);
    case ShapePreset::Foldback:
        return 1.0f / (1.0f + 0.5f * driveAmount);
    case ShapePreset::Clean:
    default:
        return 1.0f;
    }
}

float ShapeModule::driveCurve(float x, float drive)
{
    float perceptualDrive = std::pow(drive, 1.0f);
    float k = juce::jlimit(0.01f, 10.0f, perceptualDrive * 5.0f);
    return (1.0f - std::exp(-k * std::abs(x))) * std::copysign(1.0f, x);
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

float ShapeModule::saturateTape(float x)
{
    float gain = 1.0f + driveAmount * 3.5f;
    float bias = 0.08f * driveAmount;
    float driven = gain * x + bias;

    float compressed = driven / (1.0f + 0.6f * std::abs(driven));
    float softened = std::tanh(compressed * 0.9f);

    float mix = 0.4f + 0.5f * driveAmount;

    return (1.0f - mix) * x + mix * softened;
}

float ShapeModule::saturateFoldback(float x)
{
    float gain = 1.0f + driveAmount * 4.0f;
    float shaped = gain * x;
    const float threshold = 1.0f;

    if (std::abs(shaped) > threshold)
    {
        float excess = std::fabs(shaped) - threshold;
        float foldValue = threshold - std::sin(excess * (juce::MathConstants<float>::pi * 0.5f));

        shaped = (shaped > 0) ? foldValue : -foldValue;

        shaped = std::tanh(shaped * 2.0f);
    }
    else
    {
        shaped = std::tanh(shaped * 1.3f);
    }

    float mix = juce::jlimit(0.f, 1.f, driveAmount);
    return (1.0f - mix) * x + mix * shaped;
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
