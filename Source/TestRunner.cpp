// TestRunner.cpp
#include <JuceHeader.h> // ajustá según tu proyecto
#include "ShapeModule.h"


// Imprime RMS, Peak, Crest y Loudness de un buffer, con opcional gain aplicado
void printPerceptualLoudness(const juce::AudioBuffer<float>& buffer, const juce::String& label, float appliedGain = 0.f)
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
    float loudness = 20.0f * std::log10(rms + 1e-6f);

    juce::String message;
    message << label
        << " | RMS: " << juce::String(rms, 3)
        << " | PEAK: " << juce::String(peak, 3)
        << " | Crest: " << juce::String(crestFactor, 2)
        << " | Est. Loudness: " << juce::String(loudness, 2) << " dB";

    if (appliedGain > 0.0f)
        message += " | GainComp: " + juce::String(appliedGain, 3);

    DBG(message);
}

