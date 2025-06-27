#pragma once
#include <JuceHeader.h>

class LFO
{
public:
    LFO() = default;

    //––– Renombramos el parámetro para no “ocultar” al miembro –––
    void prepare(double newSampleRate)
    {
        sampleRate = newSampleRate;       // ← asigna al miembro
        updateDelta();
    }

    void setFrequency(float newFrequency)
    {
        frequency = newFrequency;
        updateDelta();
    }

    void reset() { phase = 0.f; }
    void setWaveform(int type) { waveformType = type; } // 0 Sine · 1 Tri · 2 Saw · 3 Sq

    float getNextSample()
    {
        float output{};
        switch (waveformType)
        {
        case 0:  output = std::sin(juce::MathConstants<float>::twoPi * phase);   break;
        case 1:  output = 2.f * std::abs(2.f * (phase - 0.5f)) - 1.f;           break;
        case 2:  output = 2.f * phase - 1.f;                                     break;
        case 3:  output = (phase < 0.5f) ? 1.f : -1.f;                           break;
        default: output = 0.f;
        }

        phase += phaseDelta;
        if (phase >= 1.f) phase -= 1.f;
        return output;
    }

private:
    void updateDelta() { phaseDelta = frequency / static_cast<float>(sampleRate); }

    double sampleRate = 44100.0;
    float  frequency = 1.0f;
    float  phase = 0.0f;
    float  phaseDelta = 0.0f;
    int    waveformType = 0;
};
