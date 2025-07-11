// ShapeModule.h
#pragma once
#include <JuceHeader.h>

enum class ShapePreset
{
    Clean = 0,
    Soft,
    Hard,
    Tape,
    Foldback
};

namespace ShapeIntern
{
    inline int   gType = 0;
    inline float gDrive = 0.5f;
    inline float gBias = 0.0f; // por ahora sin bias

    inline float softClip(float x)
    {
        float safeDrive = std::max(gDrive, 0.01f);
        float shaped = x * safeDrive;
        return std::tanh(shaped);
    }

    inline float hardClip(float x)
    {
        float safeDrive = std::max(gDrive, 0.01f);
        float shaped = x * safeDrive;
        float limit = 0.95f;
        return juce::jlimit(-limit, limit, shaped);
    }

    inline float tapeSat(float x)
    {
        float safeDrive = std::max(gDrive, 0.01f);
        float shaped = (x + gBias) * (safeDrive * 1.5f);  
        float saturated = (std::tanh(shaped * 2.0f) + shaped) * 0.5f;
        return juce::jlimit(-1.f, 1.f, saturated);
    }

    inline float foldback(float x)
    {
        float safeDrive = std::max(gDrive, 0.01f);
        float shaped = x * safeDrive;
        float softness = 0.7f;
        float folded = std::sin(shaped * softness) * std::tanh(shaped * softness);
        return juce::jlimit(-1.f, 1.f, folded);
    }
}

class ShapeModule
{
public:
    ShapeModule() {}

    void prepare(double sampleRate, int samplesPerBlock, int numChannels);
    void setParameters(ShapePreset preset, float driveAmount, float dryWetAmount, bool agc);
    void process(juce::AudioBuffer<float>& buffer);
    float processSample(float x);

private:
    void rebuildFunction();

    bool enableAGC = true;
    int curveType = 0;
    float drive = 0.5f;
    float dryWet = 1.0f;

    juce::dsp::WaveShaper<float> shaper;
    juce::dsp::ProcessSpec spec;
};
