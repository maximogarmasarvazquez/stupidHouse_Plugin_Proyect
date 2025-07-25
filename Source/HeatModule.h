#pragma once
#include <JuceHeader.h>

enum class HeatPreset
{
    Clean = 0,
    Warm,
    Analog,
    Dirty,
    Crunch
};

class HeatModule
{
public:
    void prepare(double sampleRate, int blockSize, int numChannels);

    void updateParametersFromPreset(int presetIndex, float amount);
    void process(juce::AudioBuffer<float>& buffer, float amount);

private:
    float drive = 1.0f;
    float mix = 0.5f;
    float bias = 0.0f;
    HeatPreset currentPreset = HeatPreset::Clean;

    juce::AudioBuffer<float> dryBuffer;

    float saturate(float x) const;
};
