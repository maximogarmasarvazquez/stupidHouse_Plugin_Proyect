#pragma once
#include <JuceHeader.h>

class ModModule
{
public:
    void prepare(double sampleRate)
    {
        sr = sampleRate;
        phase = 0.0f;
        mix.reset(sampleRate, 0.005);   // 5 ms de rampa para cambio rápido pero suave
    }

    // speedHz: 0‑10 Hz   dryWet: 0‑1
    void setParameters(float speedHz, float dryWet)
    {
        speedHz = juce::jlimit(0.f, 10.f, speedHz);
        phaseInc = (speedHz == 0.f ? 0.f
            : juce::MathConstants<float>::twoPi * speedHz / (float)sr);

        mix.setTargetValue(juce::jlimit(0.f, 1.f, dryWet));  // cambio rápido pero con rampa
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        const int numCh = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();

        if (phaseInc == 0.f && mix.getCurrentValue() < 0.001f)
            return; // bypass si speed 0 y drywet casi 0

        for (int ch = 0; ch < numCh; ++ch)
        {
            float* data = buffer.getWritePointer(ch);

            float localPhase = phase;
            float localMix = mix.getCurrentValue();
            float mixTarget = mix.getTargetValue();
            float mixStep = (mixTarget - localMix) / (float)numSamples;

            for (int i = 0; i < numSamples; ++i)
            {
                float mod = 0.5f * (1.f + std::sin(localPhase)); // 0-1
                localPhase += phaseInc;
                if (localPhase > juce::MathConstants<float>::twoPi)
                    localPhase -= juce::MathConstants<float>::twoPi;

                float dryMix = 1.f - localMix;
                data[i] *= dryMix + localMix * mod;

                localMix += mixStep;
            }

            if (ch == numCh - 1)
            {
                phase = localPhase;
                mix.setCurrentAndTargetValue(localMix);
            }
        }
    }

private:
    double sr{ 44100.0 };
    float  phase{ 0.f };
    float  phaseInc{ 0.f };

    juce::SmoothedValue<float> mix;   // dry/wet del tremolo
};