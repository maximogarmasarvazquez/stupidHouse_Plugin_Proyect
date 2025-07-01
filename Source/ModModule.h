#pragma once
#include <JuceHeader.h>

class ModModule
{
public:
    void prepare(double sampleRate)
    {
        sr = sampleRate;
        phase = 0.0f;
        mix.reset(sampleRate, 0.02);   // 20 ms de rampa para evitar clicks
    }

    // speedHz: 0‑10 Hz   dryWet: 0‑1
    void setParameters(float speedHz, float dryWet)
    {
        // si speed==0 => bypass tremolo
        speedHz = juce::jlimit(0.f, 10.f, speedHz);
        phaseInc = (speedHz == 0.f ? 0.f
            : juce::MathConstants<float>::twoPi * speedHz / (float)sr);

        mix.setTargetValue(juce::jlimit(0.f, 1.f, dryWet));
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        const int numCh = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();

        if (phaseInc == 0.f || mix.getCurrentValue() < 0.0001f)
            return;                         // tremolo bypass; nada que hacer

        for (int ch = 0; ch < numCh; ++ch)
        {
            float* data = buffer.getWritePointer(ch);

            float localPhase = phase;
            float localMix = mix.getCurrentValue();
            float mixStep = (mix.getNextValue() - localMix) / numSamples;

            for (int i = 0; i < numSamples; ++i)
            {
                float mod = 0.5f * (1.f + std::sin(localPhase));   // 0‑1
                localPhase += phaseInc;
                if (localPhase > juce::MathConstants<float>::twoPi)
                    localPhase -= juce::MathConstants<float>::twoPi;

                float dryMix = 1.f - localMix;          // mezcla lineal
                data[i] *= dryMix + localMix * mod;      // aplica tremolo

                localMix += mixStep;                     // rampa suavizada
            }

            // conservar fase final por bloque (una sola vez)
            if (ch == numCh - 1)
            {
                phase = localPhase;
                mix.setCurrentAndTargetValue(localMix); // deja listo para el próximo bloque
            }
        }
    }

private:
    double sr{ 44100.0 };
    float  phase{ 0.f };
    float  phaseInc{ 0.f };

    juce::SmoothedValue<float> mix;   // dry/wet del tremolo
};
