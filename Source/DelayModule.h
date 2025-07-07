
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
                                    2 };
        delay.prepare(spec);

        smoothTime.reset(sr, 0.05);
        smoothTime.setCurrentAndTargetValue(targetTimeSec);

        smoothFeedback.reset(sr, 0.05);
        smoothFeedback.setCurrentAndTargetValue(feedback);

        smoothDryWet.reset(sr, 0.05);
        smoothDryWet.setCurrentAndTargetValue(dryWet);
    }

    void setTime(float seconds)
    {
        targetTimeSec = juce::jlimit(0.f, maxDelaySeconds, seconds);
        smoothTime.setTargetValue(targetTimeSec);
    }

    void setFeedback(float fb)
    {
        feedback = juce::jlimit(0.f, 0.9f, fb);
        smoothFeedback.setTargetValue(feedback);
    }

    void setDryWet(float dw)
    {
        dryWet = juce::jlimit(0.f, 1.f, dw);
        smoothDryWet.setTargetValue(dryWet);
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        const int numSamples = buffer.getNumSamples();
        const int numChannels = buffer.getNumChannels();

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float* channelData = buffer.getWritePointer(ch);

            for (int i = 0; i < numSamples; ++i)
            {
                // Suavizados
                float delayTimeSamples = smoothTime.getNextValue() * (float)sr;
                delay.setDelay(delayTimeSamples);

                float currentFeedback = smoothFeedback.getNextValue();
                float currentDryWet = smoothDryWet.getNextValue();

                float inputSample = channelData[i];
                float delayedSample = delay.popSample(ch);

                // Feedback sumado con suavizado
                float fbSample = delayedSample * currentFeedback;

                // Insertamos en delay la suma de input + feedback
                delay.pushSample(ch, inputSample + fbSample);

                // Mezcla dry/wet
                channelData[i] = inputSample * (1.f - currentDryWet) + delayedSample * currentDryWet;
            }
        }
    }

private:
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delay{ 96000 };

    double sr{ 44100.0 };

    float targetTimeSec{ 0.5f };
    float feedback{ 0.5f };
    float dryWet{ 0.5f };

    juce::SmoothedValue<float> smoothTime;
    juce::SmoothedValue<float> smoothFeedback;
    juce::SmoothedValue<float> smoothDryWet;

    static constexpr float maxDelaySeconds = 2.0f;
};