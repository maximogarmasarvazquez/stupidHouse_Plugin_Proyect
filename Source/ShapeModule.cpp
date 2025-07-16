#include "ShapeModule.h"
#include <cmath>

void ShapeModule::prepare(double sampleRate, int /*samplesPerBlock*/, int /*numChannels*/)
{
    smoothedAGC.reset(sampleRate, 0.05); // 50 ms
    smoothedAGC.setCurrentAndTargetValue(1.0f);

    silenceCounter = 0;
}

void ShapeModule::setParameters(ShapePreset preset, float drive, float /*outputGain*/, bool applySoftClip)
{
    currentPreset = preset;
    driveAmount = drive;
    softClipEnabled = applySoftClip;

    // Compensación automática para mantener volumen estable
    // Ajusta el factor 0.8 según lo que suene mejor en tu plugin
    outputGain = 1.0f / (1.0f + driveAmount * 0.8f);
}

// Curva para distorsión sin subir ganancia brutalmente
float ShapeModule::driveCurve(float x, float drive)
{
    // Podés cambiar a std::pow(drive, 2.0f) si querés más curvatura perceptual
    float perceptualDrive = std::pow(drive, 1.0f);
    float k = juce::jlimit(0.01f, 10.0f, perceptualDrive * 5.0f);
    return (1.0f - std::exp(-k * std::abs(x))) * std::copysign(1.0f, x);
}

float ShapeModule::saturateSoft(float x)
{
    return driveCurve(x, driveAmount);
}

float ShapeModule::saturateHard(float x)
{
    float clean = driveCurve(x, driveAmount);
    if (clean > 0.7f) return 0.7f;
    if (clean < -0.7f) return -0.7f;
    return clean;
}

float ShapeModule::saturateTape(float x)
{
    float clean = driveCurve(x, driveAmount);
    if (clean > 0.0f)
        return 1.0f - std::exp(-clean);
    else
        return -1.0f + std::exp(clean);
}

float ShapeModule::saturateFoldback(float x)
{
    const float threshold = 0.5f;
    float clean = driveCurve(x, driveAmount);
    if (clean > threshold || clean < -threshold)
    {
        return std::fabs(std::fmod(std::fabs(clean - threshold), threshold * 4.f) - threshold * 2.f) - threshold;
    }
    return clean;
}

void ShapeModule::process(juce::AudioBuffer<float>& buffer)
{
    if (currentPreset == ShapePreset::Clean)
        return;

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* data = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i)
        {
            float x = data[i];

            switch (currentPreset)
            {
            case ShapePreset::Soft:      data[i] = saturateSoft(x); break;
            case ShapePreset::Hard:      data[i] = saturateHard(x); break;
            case ShapePreset::Tape:      data[i] = saturateTape(x); break;
            case ShapePreset::Foldback:  data[i] = saturateFoldback(x); break;
            default: break;
            }

            if (softClipEnabled)
                data[i] = std::tanh(data[i]);

            data[i] *= outputGain;
        }
    }
}