#pragma once
 #include <juce_dsp/juce_dsp.h>
#include <JuceHeader.h>

enum class ShapePreset
{
    Clean = 0,
    Soft,
    Hard,
    Tape,
    Foldback
};

class ShapeModule
{
public:
    ShapeModule() = default;

    void prepare(double sampleRate, int samplesPerBlock, int numChannels);
    void process(juce::AudioBuffer<float>& buffer);
    void setParameters(ShapePreset preset, float drive, float outputGain, bool applySoftClip, bool agc);
private:

    int silenceCounter = 0;

    juce::SmoothedValue<float> smoothedAGC;
    juce::SmoothedValue<float> smoothedInputRMS;
    juce::SmoothedValue<float> smoothedOutputRMS;
    ShapePreset currentPreset = ShapePreset::Clean;

    float driveAmount = 1.0f;
    float outputGain = 1.0f;
    bool softClipEnabled = true;
    bool enableAGC = true;
    // Saturation functions
    float saturateSoft(float x);
    float saturateHard(float x);
    float saturateTape(float x);
    float saturateFoldback(float x);

    juce::dsp::Oversampling<float> oversampling{ 2, 2, juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShapeModule)
};
