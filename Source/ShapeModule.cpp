#include "ShapeModule.h"

void ShapeModule::prepare(double sr, int spb, int chans)
{
    juce::dsp::ProcessSpec spec{ sr,
                                   static_cast<juce::uint32>(spb),
                                   static_cast<juce::uint32>(chans) };

    os.reset();
    os.initProcessing(spb);        // <-- único llamado necesario al Oversampling

    shaper.prepare(spec);          // WaveShaper sí necesita prepare()
    rebuildFunction();
}

void ShapeModule::rebuildFunction()
{
    switch (current.curveType)
    {
    case 0:  shaper.functionToUse = &softClip; break;
    case 1:  shaper.functionToUse = &hardClip; break;
    case 2:  shaper.functionToUse = &tapeSat;  break;
    default: // passthrough
        shaper.functionToUse = [](float x) { return x; };
        break;
    }
}

void ShapeModule::process(juce::AudioBuffer<float>& buffer)
{
    auto block = juce::dsp::AudioBlock<float>(buffer);
    auto upBlock = os.processSamplesUp(block);

    upBlock.multiplyBy(current.drive);          // aplica el drive antes de la curva

    juce::dsp::ProcessContextReplacing<float> ctx(upBlock);
    shaper.process(ctx);

    os.processSamplesDown(block);
}
