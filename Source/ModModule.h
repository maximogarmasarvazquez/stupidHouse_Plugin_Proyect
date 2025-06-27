#pragma once
#include <JuceHeader.h>

class ModModule
{
public:
    void prepare(double sampleRate)
    {
        sr = sampleRate;
        phase = 0.0f;
    }

    void setParameters(float speedHz, float dryWet)
    {
        speedHz = juce::jlimit(0.1f, 10.0f, speedHz);
        mix = juce::jlimit(0.0f, 1.0f, dryWet);
        phaseInc = juce::MathConstants<float>::twoPi * speedHz / static_cast<float> (sr);
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        const int numCh = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();
        const float dryMix = 1.0f - mix;

        for (int i = 0; i < numSamples; ++i)
        {
            float mod = 0.5f * (1.0f + std::sin(phase));
            phase += phaseInc;
            if (phase > juce::MathConstants<float>::twoPi)
                phase -= juce::MathConstants<float>::twoPi;

            for (int ch = 0; ch < numCh; ++ch)
            {
                float* data = buffer.getWritePointer(ch);
                float in = data[i];
                float out = mod * in;
                data[i] = dryMix * in + mix * out;
            }
        }
    }

private:
    double sr{ 44100.0 };
    float  mix{ 0.5f };
    float  phase{ 0.0f };
    float  phaseInc{ 0.0f };
};
