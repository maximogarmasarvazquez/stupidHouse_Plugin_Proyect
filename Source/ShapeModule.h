#pragma once
#include <JuceHeader.h>

/*------------  Preset struct ------------*/
struct ShapePreset
{
    float drive = 1.0f;        // ganancia previa
    int   curveType = -1;      // -1 = passthrough, 0 soft, 1 hard, 2 tape
};

/*------------  Shape DSP module ----------*/
class ShapeModule
{
public:

   
    ShapeModule() = default;

    void prepare(double sampleRate, int samplesPerBlock, int numChannels);

    void setPreset(const ShapePreset& p) { current = p; rebuildFunction(); }


    void process(juce::AudioBuffer<float>& buffer);

private:

    static float softClip(float x) { return std::tanh(x); }
    static float hardClip(float x) { return juce::jlimit(-1.0f, 1.0f, x); }
    static float tapeSat(float x) { return 0.5f * std::tanh(2.0f * x + 0.3f * x * x * x); }

    void rebuildFunction();

    ShapePreset current;

    juce::dsp::WaveShaper<float> shaper;
    juce::dsp::Oversampling<float> os{
        2, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR
    };
};
