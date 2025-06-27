#pragma once
#include <JuceHeader.h>

class DelayModule
{
public:
    void prepare(double sampleRate, int maxDelaySamples = 96000)
    {
        sr = sampleRate;
        delay.reset();
        delay.setMaximumDelayInSamples(maxDelaySamples);

        juce::dsp::ProcessSpec spec{ sampleRate,
                                      static_cast<juce::uint32>(maxDelaySamples),
                                      2 }; // stereo
        delay.prepare(spec);

        // Rampa de 20 ms para suavizar cambios de tiempo
        smoothTime.reset(sr, 0.02);
    }

    void setTime(float timeSec)  // 0 - 2 s
    {
        targetTimeSec = juce::jlimit(0.f, maxDelaySeconds, timeSec);
        smoothTime.setTargetValue(targetTimeSec);
    }

    void setFeedback(float feedback)
    {
        fb = juce::jlimit(0.0f, 0.99f, feedback);
    }

    void setDryWet(float dryWet)
    {
        mix = juce::jlimit(0.0f, 1.0f, dryWet);
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        const int numSamples = buffer.getNumSamples();
        const int numCh = buffer.getNumChannels();

        for (int ch = 0; ch < numCh; ++ch)
        {
            auto* data = buffer.getWritePointer(ch);

            for (int i = 0; i < numSamples; ++i)
            {
                const float curTimeSec = juce::jlimit(0.f, maxDelaySeconds, smoothTime.getNextValue());
                delay.setDelay(curTimeSec * static_cast<float>(sr));

                const float in = data[i];
                const float del = delay.popSample(ch);

                const float fbSafe = juce::jlimit(0.0f, 0.95f, fb);  // Evita saturación por acumulación
                delay.pushSample(ch, in + del * fbSafe);

                const float wet = del;
                data[i] = mix * wet + (1.0f - mix) * in;
            }
        }
    }

private:
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delay{ 96000 };
    float targetTimeSec{ 0.5f };
    juce::SmoothedValue<float> smoothTime;
    double sr{ 44100.0 };
    float fb{ 0.5f };
    float mix{ 0.5f };

    static constexpr float maxDelaySeconds = 2.0f;
};