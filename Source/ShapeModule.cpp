#include "ShapeModule.h"
#include <cmath>

void ShapeModule::prepare(double sampleRate, int /*samplesPerBlock*/, int /*numChannels*/)
{
    smoothedAGC.reset(sampleRate, 0.05); // 50 ms smoothing time
    smoothedAGC.setCurrentAndTargetValue(1.0f);
    silenceCounter = 0;
}

void ShapeModule::setParameters(ShapePreset preset, float drive, float /*outputGain*/, bool applySoftClip)
{
    currentPreset = preset;
    driveAmount = juce::jlimit(0.f, 1.f, drive);
    softClipEnabled = applySoftClip;
}

float ShapeModule::driveCurve(float x, float drive)
{
    float perceptualDrive = std::pow(drive, 1.0f);
    float k = juce::jlimit(0.01f, 10.0f, perceptualDrive * 5.0f);
    return (1.0f - std::exp(-k * std::abs(x))) * std::copysign(1.0f, x);
}

float ShapeModule::saturateSoft(float x)
{
    float k = driveAmount * 6.0f + 0.1f;        // Más ganancia para hacerlo sentir más
    float shaped = std::tanh(k * x);            // Saturación suave
    return shaped * (0.9f - 0.2f * driveAmount); // Controla el nivel final
}

float ShapeModule::saturateHard(float x)
{
    float gain = 1.0f + driveAmount * 4.0f;
    float driven = gain * x;

    // Clip más permisivo: más rango, más redondeado
    float clipped = juce::jlimit(-0.9f, 0.9f, driven);

    // Suavizado en los bordes
    if (clipped > 0.75f)
        clipped = 0.75f + 0.25f * std::tanh((clipped - 0.75f) * 5.0f);
    else if (clipped < -0.75f)
        clipped = -0.75f + 0.25f * std::tanh((clipped + 0.75f) * 5.0f);

    return clipped;
}


float ShapeModule::saturateTape(float x)
{
    float gain = 1.0f + driveAmount * 3.5f;
    float bias = 0.08f * driveAmount;              // Leve desalineación
    float driven = gain * x + bias;

    // Curva de saturación con compresión dinámica tipo cinta
    float compressed = driven / (1.0f + 0.6f * std::abs(driven));
    float softened = std::tanh(compressed * 0.9f);

    // Crossfade más fuerte
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
        // Foldback clásico: reflejo del exceso, pero suavizado con seno para transiciones suaves
        float excess = std::fabs(shaped) - threshold;

        // Foldback con función seno para suavizar (más orgánico)
        float foldValue = threshold - std::sin(excess * (juce::MathConstants<float>::pi * 0.5f));

        shaped = (shaped > 0) ? foldValue : -foldValue;

        // Aplicamos saturación suave extra para suavizar picos
        shaped = std::tanh(shaped * 2.0f);
    }
    else
    {
        // Saturación leve para señal dentro de rango
        shaped = std::tanh(shaped * 1.3f);
    }

    // Mezcla señal limpia y foldback según driveAmount
    float mix = juce::jlimit(0.f, 1.f, driveAmount);
    return (1.0f - mix) * x + mix * shaped;
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

            data[i] = y * outputGain;
        }
    }
}
