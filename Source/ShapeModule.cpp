#include "ShapeModule.h"
using namespace ShapeIntern;

// ─────────────────────────────────────────────────────────────────────────────
void ShapeModule::prepare(double sampleRate,
    int    samplesPerBlock,
    int    numChannels)
{
    juce::dsp::ProcessSpec spec{ sampleRate,
                                  static_cast<juce::uint32> (samplesPerBlock),
                                  static_cast<juce::uint32> (numChannels) };

    os.reset();
    os.initProcessing(samplesPerBlock);   // en JUCE 8 basta con esto

    shaper.prepare(spec);
    rebuildFunction();
}

// ─────────────────────────────────────────────────────────────────────────────
void ShapeModule::setParameters(int presetIndex, float driveAmount)
{
    if (presetIndex != curveType || driveAmount != drive)
    {
        curveType = presetIndex;
        drive = driveAmount;
        rebuildFunction();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void ShapeModule::process(juce::AudioBuffer<float>& buffer)
{
    auto block = juce::dsp::AudioBlock<float>(buffer);
    auto upsampled = os.processSamplesUp(block);            // ↑ x2

    juce::dsp::ProcessContextReplacing<float> ctx(upsampled);
    shaper.process(ctx);                                     // distorsión

    os.processSamplesDown(block);                            // ↓ x2
}

// ─────────────────────────────────────────────────────────────────────────────
void ShapeModule::rebuildFunction()
{
    // Copiamos a las variables globales para que la lambda sin capturas funcione
    gType = curveType;
    gDrive = drive;

    shaper.functionToUse = [](float x) -> float
        {
            switch (gType)
            {
            case 0:  return x;          // Default: sin distorsión, señal limpia
            case 1:  return softClip(x);
            case 2:  return hardClip(x);
            case 3:  return tapeSat(x);
            default: return x;          // En caso de valor fuera de rango, también sin distorsión
            }
        };
}
