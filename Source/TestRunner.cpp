#include "TestRunner.h"
#include <cmath>

// Función para imprimir métricas RMS, Peak y Loudness de un buffer
void printPerceptualLoudness(const juce::AudioBuffer<float>& buffer, const juce::String& label, float appliedGain)
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
    float loudness = 20.0f * std::log10(rms + 1e-6f); // evitar log(0)

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

// Testea el ShapeModule con un preset y drive específico
void testShapeModule(ShapeModule& shape, juce::AudioBuffer<float>& buffer, ShapePreset preset, float drive)
{
    shape.setParameters(preset, drive, 1.0f, false);
    juce::AudioBuffer<float> copy(buffer);
    shape.process(copy);
    printPerceptualLoudness(copy, "Shape [" + juce::String((int)preset) + "]", drive);
}

// Testea DelayModule con parámetros específicos
void testDelayModule(DelayModule& delay, juce::AudioBuffer<float>& buffer, float time, float fb, float dw)
{
    delay.setTime(time);
    delay.setFeedback(fb);
    delay.setDryWet(dw);

    juce::AudioBuffer<float> copy(buffer);
    delay.process(copy);
    printPerceptualLoudness(copy, "Delay T=" + juce::String(time, 2) + "s FB=" + juce::String(fb) + " DW=" + juce::String(dw));
}

// Testea FilterModule con ganancia en dB
void testFilterModule(FilterModule& filter, juce::AudioBuffer<float>& buffer, float gainDb)
{
    filter.setGain(gainDb);
    juce::AudioBuffer<float> copy(buffer);
    filter.process(copy);
    printPerceptualLoudness(copy, "Filter Gain=" + juce::String(gainDb) + " dB");
}

// Testea ModModule (tremolo) con velocidad y mezcla dry/wet
void testModModule(ModModule& mod, juce::AudioBuffer<float>& buffer, float speedHz, float dryWet)
{
    mod.setParameters(speedHz, dryWet);
    juce::AudioBuffer<float> copy(buffer);
    mod.process(copy);
    printPerceptualLoudness(copy, "Tremolo F=" + juce::String(speedHz, 2) + "Hz DW=" + juce::String(dryWet));
}

// Función para correr todos los tests, crea un buffer con seno a 440Hz para probar
void runAllTests(StupidHouseAudioProcessor& processor)
{
    juce::AudioBuffer<float> testBuffer(2, 512);
    testBuffer.clear();

    for (int ch = 0; ch < 2; ++ch)
    {
        for (int i = 0; i < testBuffer.getNumSamples(); ++i)
        {
            float sample = std::sin(2.0f * juce::MathConstants<float>::pi * 440.0f * i / 44100.0f);
            testBuffer.setSample(ch, i, sample * 0.3f); // Volumen moderado para no saturar
        }
    }

    DBG("==================== INICIO DE TEST ====================");

    DBG("=== SHAPE MODULE ===");
    for (int i = 0; i <= 4; ++i)
        testShapeModule(processor.shape, testBuffer, static_cast<ShapePreset>(i), 1.0f);

    DBG("=== DELAY MODULE ===");
    testDelayModule(processor.delay, testBuffer, 0.5f, 0.5f, 0.5f);

    DBG("=== FILTER MODULE ===");
    testFilterModule(processor.shelf, testBuffer, 6.0f);

    DBG("=== MOD MODULE ===");
    testModModule(processor.mod, testBuffer, 5.0f, 0.5f);

    DBG("==================== FIN DE TEST ======================");
}
