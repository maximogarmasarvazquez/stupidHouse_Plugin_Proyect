#pragma once
#include <JuceHeader.h>

namespace ShapeIntern
{
    inline int   gType = 0;
    inline float gDrive = 0.5f;

    static float softClip(float x)
    {
        float y = x * gDrive;
        return std::tanh(y) / std::tanh(gDrive);   // máx = 1
    }
    static float hardClip(float x)
    {
        float y = x * gDrive;
        y = juce::jlimit(-1.f, 1.f, y);
        return y / gDrive;            // divide por drive para bajar RMS
    }
    static float tapeSat(float x)
    {
        float y = x * gDrive;
        float norm = juce::MathConstants<float>::pi * 0.5f / std::atan(gDrive);
        return std::atan(y) * norm;  // salida máx ≈ 1
    }
}

class ShapeModule
{
public:
    ShapeModule()
        : os(2, 2,
            juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple) {
    }

    void prepare(double sampleRate, int samplesPerBlock, int numChannels);
    void setParameters(int presetIndex, float driveAmount);
    void process(juce::AudioBuffer<float>& buffer);

private:
    void rebuildFunction();   // actualiza la curva del WaveShaper

    int   curveType = 0;
    float drive = 0.5f;
    juce::dsp::Oversampling<float> os{ 2 /*canales*/, 4 /*factor*/,
    juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR };
    juce::dsp::WaveShaper<float>   shaper;
};