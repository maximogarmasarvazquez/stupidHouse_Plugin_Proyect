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

    static float softClip(float x)
    {
        float safeDrive = std::max(gDrive, 0.01f);
        float y = x * safeDrive;
        return std::tanh(y);
    }

    static float hardClip(float x)
    {
        float y = juce::jlimit(-1.f, 1.f, x * gDrive);
        return y;
    }

    static float tapeSat(float x)
    {
        float safeDrive = std::max(gDrive, 0.01f);
        float y = x * safeDrive;
        return std::atan(y) / std::atan(safeDrive);
    }

    static float foldback(float x)
    {
        float safeDrive = std::max(gDrive, 0.01f);
        float y = x * safeDrive;

        if (y > 1.0f || y < -1.0f)
        {
            // Foldback: pliega la señal pero con suavizado
            y = std::abs(std::fmod(y - 1.0f, 4.0f) - 2.0f) - 1.0f;
        }

        // Normalizamos la salida para que no se exceda [-1,1]
        y = juce::jlimit(-1.f, 1.f, y);

        return y / safeDrive;
    }
}
class ShapeModule
{
public:
    ShapeModule()
        : os(2, 2, juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple)
    {
    }

    void prepare(double sampleRate, int samplesPerBlock, int numChannels);
    void setParameters(ShapePreset preset, float driveAmount);
    void process(juce::AudioBuffer<float>& buffer);

private:
    void rebuildFunction();   // actualiza la curva del WaveShaper

    int   curveType = 0;
    float drive = 0.5f;

    juce::dsp::Oversampling<float> os{ 2, 4, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR };
    juce::dsp::WaveShaper<float>   shaper;
};