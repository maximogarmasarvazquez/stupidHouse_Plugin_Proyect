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
    void setParameters(ShapePreset preset, float drive, float outGain, bool applySoftClip);
    void updateParametersFromPreset(int rawPreset, float drive, bool applySoftClip = true); // <-- parámetro opcional aquí
    void process(juce::AudioBuffer<float>& buffer);
    void processWithCompensation(juce::AudioBuffer<float>& buffer, float dryWet, juce::SmoothedValue<float>& gainSmoother);

    static float mapDriveValue(float input);
    float getPresetGainCompensation() const;

private:
    int silenceCounter = 0;
    juce::SmoothedValue<float> smoothedAGC;

    ShapePreset currentPreset = ShapePreset::Clean;

    float driveAmount = 0.0f;
    float outputGain = 1.0f;
    bool softClipEnabled = false;

    // Saturation functions
    float saturateSoft(float x);
    float saturateHard(float x);
    float saturateTape(float x);
    float saturateFoldback(float x);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShapeModule)
};
