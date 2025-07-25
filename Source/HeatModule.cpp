#include "HeatModule.h"

void HeatModule::prepare(double, int blockSize, int numChannels)
{
    dryBuffer.setSize(numChannels, blockSize, false, false, true);
}

void HeatModule::updateParametersFromPreset(int presetIndex, float amount)
{
    currentPreset = static_cast<HeatPreset>(presetIndex);

    // Presets base
    switch (currentPreset)
    {
    case HeatPreset::Clean:
        drive = 1.0f;
        mix = 0.0f;
        bias = 0.0f;
        break;
    case HeatPreset::Warm:
        drive = 1.5f;
        mix = 0.25f;
        bias = 0.05f;
        break;
    case HeatPreset::Analog:
        drive = 2.2f;
        mix = 0.5f;
        bias = 0.1f;
        break;
    case HeatPreset::Dirty:
        drive = 3.0f;
        mix = 0.75f;
        bias = 0.15f;
        break;
    case HeatPreset::Crunch:
        drive = 4.0f;
        mix = 1.0f;
        bias = 0.2f;
        break;
    }

    drive *= juce::jmap(amount, 0.f, 1.f, 1.f, 4.f);  // escalado suave
    mix *= amount; // proporcional al parámetro "heat"
}

void HeatModule::process(juce::AudioBuffer<float>& buffer, float amount)
{
    if (amount < 0.01f || currentPreset == HeatPreset::Clean)
        return;

    dryBuffer.makeCopyOf(buffer);
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* data = buffer.getWritePointer(ch);
        const float* dry = dryBuffer.getReadPointer(ch);

        for (int i = 0; i < numSamples; ++i)
        {
            float input = data[i] * drive + bias;
            float saturated = saturate(input);
            data[i] = juce::jmap(mix, 0.f, 1.f, dry[i], saturated);
        }
    }
}
float HeatModule::saturate(float x) const
{
    // Alternativa: Añadir armónicos suaves tipo válvula sin saturar

    // Agrega contenido armónico: armónicos pares (x^2), impares (x^3)
    float harmonicContent = 0.6f * x + 0.3f * (x * x * ((x >= 0.0f) ? 1.0f : -1.0f)) + 0.1f * x * x * x;

    // Simula leve compresión de válvula
    float compressed = harmonicContent * (1.0f - 0.2f * std::exp(-std::abs(x)));

    // Agrega un leve énfasis en los medios-bajos (frecuencia ficticia)
    return compressed * (1.0f + 0.05f * std::sin(2.0f * juce::MathConstants<float>::pi * 150.0f * x));
}
