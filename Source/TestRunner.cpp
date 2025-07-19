// TestRunner.cpp
#include <JuceHeader.h> // ajustá según tu proyecto
#include "ShapeModule.h"


// Imprime RMS, Peak, Crest y Loudness de un buffer, con opcional gain aplicado
void printPerceptualLoudness(const juce::AudioBuffer<float>& buffer, const juce::String& label, float appliedGain = 1.f)
{
    float rms = 0.f;
    float peak = 0.f;
    int numChannels = buffer.getNumChannels();
    int numSamples = buffer.getNumSamples();

    for (int ch = 0; ch < numChannels; ++ch)
    {
        rms += buffer.getRMSLevel(ch, 0, numSamples);
        peak = std::max(peak, buffer.getMagnitude(ch, 0, numSamples));
    }
    rms /= static_cast<float>(numChannels);

    float crestFactor = (rms > 0.0f) ? (peak / rms) : 0.0f;
    float loudness = 20.0f * std::log10(rms + 1e-12f); // evitamos log(0)

    juce::String message;
    message << label
        << " | RMS: " << juce::String(rms, 6)
        << " | PEAK: " << juce::String(peak, 6)
        << " | Crest: " << juce::String(crestFactor, 3)
        << " | Loudness (dB): " << juce::String(loudness, 3);

    if (appliedGain != 1.0f)
        message << " | Gain Applied: " << juce::String(appliedGain, 6);

    DBG(message);
}
