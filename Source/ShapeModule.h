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

    inline float softClip(float x)
    {
        float safeDrive = std::max(gDrive, 0.01f);
        float y = x * safeDrive;
        return std::tanh(y);
    }

    inline float hardClip(float x)
    {
        float y = juce::jlimit(-1.f, 1.f, x * gDrive);
        return y;
    }

    inline float tapeSat(float x)
    {
        float safeDrive = std::max(gDrive, 0.01f);
        float y = x * safeDrive;
        return std::atan(y) / std::atan(safeDrive);
    }

    inline float foldback(float x)
    {
        float safeDrive = std::max(gDrive, 0.01f);
        float shaped = x * safeDrive;

        // Nuevo algoritmo suave de tipo wavefold
        return std::tanh(shaped * std::sin(shaped)) / std::tanh(safeDrive);
    }
}
class ShapeModule
{
public:
    void prepare(double sampleRate, int samplesPerBlock, int numChannels);
    void setParameters(ShapePreset preset, float driveAmount, bool agc); 
    void process(juce::AudioBuffer<float>& buffer);


    float processSample(float x);

    ShapeModule()
        : os(2, 2, juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple)
    {
    }



private:
    void rebuildFunction();   // actualiza la curva del WaveShaper

    bool enableAGC = true; // ✅ solo una vez
    int   curveType = 0;
    float drive = 0.5f;

    juce::dsp::Oversampling<float> os{ 2, 4, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR };
    juce::dsp::WaveShaper<float>   shaper;
};