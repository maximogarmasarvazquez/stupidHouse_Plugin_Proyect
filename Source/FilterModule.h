#pragma once
#include <JuceHeader.h>

class FilterModule
{
public:
    void prepare(double sampleRate)
    {
        sr = sampleRate;
        updateCoeffs();
        for (auto& f : filters)
            f.prepare({ sampleRate, 512, 1 });
    }

    void setGainDecibels(float newGainDB)
    {
        newGainDB = juce::jlimit(minGain, maxGain, newGainDB);
        if (juce::approximatelyEqual(newGainDB, gainDB))
            return;
        gainDB = newGainDB;
        updateCoeffs();
    }

    // Alias para facilitar el uso
    void setGain(float newGainDB)
    {
        setGainDecibels(newGainDB);
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        juce::dsp::AudioBlock<float> block(buffer);
        for (size_t ch = 0; ch < block.getNumChannels(); ++ch)
        {
            auto singleChannelBlock = block.getSingleChannelBlock(ch);
            juce::dsp::ProcessContextReplacing<float> context(singleChannelBlock);
            filters[ch].process(context);
        }
    }

private:
    void updateCoeffs()
    {
        auto coeff = juce::dsp::IIR::Coefficients<float>::makeHighShelf(sr, Fc, Q, juce::Decibels::decibelsToGain(gainDB));
        *filters[0].coefficients = *coeff;
        *filters[1].coefficients = *coeff;
    }

    static constexpr float Fc = 6000.0f;
    static constexpr float Q = 0.707f;
    static constexpr float minGain = -24.0f;
    static constexpr float maxGain = 24.0f;

    double sr{ 44100.0 };
    float  gainDB{ 0.0f };

    std::array<juce::dsp::IIR::Filter<float>, 2> filters;
};