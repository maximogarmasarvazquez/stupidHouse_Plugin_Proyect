#include "PluginProcessor.h"
#include "PluginEditor.h"

// ────────────────────────────────────────────────────────────────
// Constructor
// ────────────────────────────────────────────────────────────────
StupidHouseAudioProcessor::StupidHouseAudioProcessor()
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    parameters(*this, nullptr, juce::Identifier("StupidHouseParams"),
        createParameterLayout())
{
    formatManager.registerBasicFormats();

    pShapePreset = parameters.getRawParameterValue(IDs::shapePreset);
    pSpeed = parameters.getRawParameterValue(IDs::speed);
    pDryWetMod = parameters.getRawParameterValue(IDs::dryWetMod);
    pTime = parameters.getRawParameterValue(IDs::time);
    pFeedback = parameters.getRawParameterValue(IDs::feedback);
    pDryWetDelay = parameters.getRawParameterValue(IDs::dryWetDelay);
    pHighShelf = parameters.getRawParameterValue(IDs::highShelf);
    pOverall = parameters.getRawParameterValue(IDs::overall);
    pOutputGain = parameters.getRawParameterValue(IDs::outputGain);
    pShape = parameters.getRawParameterValue(IDs::shape);

    jassert(pShape && pShapePreset && pSpeed && pDryWetMod && pTime &&
        pFeedback && pDryWetDelay && pHighShelf && pOverall &&
        pOutputGain);
}

// ────────────────────────────────────────────────────────────────
// Parameter layout
// ────────────────────────────────────────────────────────────────
juce::AudioProcessorValueTreeState::ParameterLayout
StupidHouseAudioProcessor::createParameterLayout()
{
    using namespace juce;
    using APF = AudioParameterFloat;

    std::vector<std::unique_ptr<RangedAudioParameter>> params;
    StringArray presetNames{ "Default", "Soft", "Hard", "Tape" };

    params.push_back(std::make_unique<AudioParameterChoice>(IDs::shapePreset, "Shape Preset", presetNames, 0));
    params.push_back(std::make_unique<APF>(IDs::shape, "Shape Amount", 0.f, 1.f, 0.f));
    params.push_back(std::make_unique<AudioParameterChoice>(IDs::heatPreset, "Heat Preset", presetNames, 0));
    params.push_back(std::make_unique<APF>(IDs::heat, "Heat Amount", 0.f, 1.f, 0.f));
    params.push_back(std::make_unique<AudioParameterChoice>(IDs::spicePreset, "Spice Preset", presetNames, 0));
    params.push_back(std::make_unique<APF>(IDs::spice, "Spice Amount", 0.f, 1.f, 0.f));
    params.push_back(std::make_unique<AudioParameterChoice>(IDs::depthPreset, "Depth Preset", presetNames, 0));
    params.push_back(std::make_unique<APF>(IDs::depth, "Depth Amount", 0.f, 1.f, 0.f));

    params.push_back(std::make_unique<APF>(IDs::overall, "Overall Amount", 0.f, 1.f, 0.5f));
    params.push_back(std::make_unique<APF>(IDs::outputGain, "Output Gain", 0.f, 1.f, 0.5f));

    params.push_back(std::make_unique<APF>(IDs::time, "Delay Time", 0.f, 2.f, 0.f));
    params.push_back(std::make_unique<APF>(IDs::feedback, "Feedback", 0.f, 1.f, 0.f));
    params.push_back(std::make_unique<APF>(IDs::dryWetDelay, "Dry/Wet Delay", 0.f, 1.f, 0.f));

    params.push_back(std::make_unique<APF>(IDs::speed, "Mod Speed", 0.f, 1.f, 0.f));
    params.push_back(std::make_unique<APF>(IDs::dryWetMod, "Dry/Wet Mod", 0.f, 1.f, 0.f));

    params.push_back(std::make_unique<APF>(IDs::highShelf, "High/Shelf", -24.f, 24.f, 0.f));

    return { params.begin(), params.end() };
}

// ────────────────────────────────────────────────────────────────
// prepareToPlay / releaseResources
// ────────────────────────────────────────────────────────────────
void StupidHouseAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    shape.prepare(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
    delay.prepare(sampleRate, static_cast<int> (sampleRate * 2.0));
    mod.prepare(sampleRate);
    shelf.prepare(sampleRate);

    transportSource.prepareToPlay(samplesPerBlock, sampleRate);

    wetBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock, false, false, true);

    smoothedOutput.reset(sampleRate, 0.02);       // 20 ms
    smoothedFeedback.reset(sampleRate, 0.05);     // 50 ms
    smoothedSpeed.reset(sampleRate, 0.02);        // 20 ms
    smoothedDryWetMod.reset(sampleRate, 0.02);    // 20 ms
    smoothedShelfDb.reset(sampleRate, 0.02);      // 20 ms

    smoothedOutput.setCurrentAndTargetValue(1.0f);
    smoothedFeedback.setCurrentAndTargetValue(0.0f);
    smoothedSpeed.setCurrentAndTargetValue(0.0f);
    smoothedDryWetMod.setCurrentAndTargetValue(0.0f);
    smoothedShelfDb.setCurrentAndTargetValue(0.0f);

    lfo.prepare(sampleRate);
    lfo.setFrequency(0.5f);
    lfo.setWaveform(0);
    lfo.reset();
}

void StupidHouseAudioProcessor::releaseResources()
{
    transportSource.releaseResources();
}

bool StupidHouseAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

// ────────────────────────────────────────────────────────────────
// loadTestFile
// ────────────────────────────────────────────────────────────────
void StupidHouseAudioProcessor::loadTestFile(const juce::File& file)
{
    if (!file.existsAsFile())
        return;

    const juce::ScopedLock sl(transportLock);

    transportSource.stop();
    transportSource.setSource(nullptr);
    readerSource.reset();

    if (auto* reader = formatManager.createReaderFor(file))
    {
        readerSource.reset(new juce::AudioFormatReaderSource(reader, true));
        transportSource.setSource(readerSource.get(), 0, nullptr, reader->sampleRate);
        transportSource.setPosition(0.0);
        transportSource.start();
    }
}

// ────────────────────────────────────────────────────────────────
// processBlock  (única definición)
// ────────────────────────────────────────────────────────────────
void StupidHouseAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer&)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    /* 0. Audio de prueba desde transportSource (opcional) */
    {
        const juce::ScopedLock sl(transportLock);
        juce::AudioSourceChannelInfo info(&buffer, 0, numSamples);
        transportSource.getNextAudioBlock(info);
    }

    /* 1. Distorsión (Shape) */
    const float overallK = pOverall ? pOverall->load() : 0.5f;
    const int   shapePreset = (int)(pShapePreset ? pShapePreset->load() : 0);
    const float shapeAmt = pShape ? pShape->load() : 0.f;
    const float intensity = shapeAmt * overallK;

    shape.setParameters(intensity <= 0.001f ? 0 : shapePreset,
        intensity <= 0.001f ? 0.f
        : juce::jmap(intensity, 0.f, 1.f, 0.f, 6.f));
    shape.process(buffer);

    /* 2. Smoothed parameters */
    smoothedOutput.setTargetValue(juce::jmap(pOutputGain ? pOutputGain->load() : 0.5f,
        0.f, 1.f, 0.f, 2.f));
    smoothedShelfDb.setTargetValue((pHighShelf ? pHighShelf->load() : 0.f) * overallK);
    smoothedSpeed.setTargetValue((pSpeed ? pSpeed->load() : 0.f) * overallK);
    smoothedDryWetMod.setTargetValue((pDryWetMod ? pDryWetMod->load() : 0.f) * overallK);
    smoothedFeedback.setTargetValue((pFeedback ? pFeedback->load() : 0.f) * overallK);

    const float speedHz = juce::jmap(smoothedSpeed.getNextValue(), 0.f, 1.f, 0.f, 10.f);
    const float modMix = smoothedDryWetMod.getNextValue();
    const float shelfDb = smoothedShelfDb.getNextValue();

    /* 3. Parámetros Delay */
    const float uiTime = pTime ? pTime->load() : 0.f;   // 0‑1, valor directo del control

    // --- NUEVO: aplicar Overall al tiempo ---
    // (si prefieres que Overall no afecte al tiempo, salta esta línea)
    const float timeSec = juce::jmap(uiTime , 0.f, 1.f, 0.f, 2.f);

    // dry/wet y feedback también escalados:
    float dryWet = (pDryWetDelay ? pDryWetDelay->load() : 0.f) * overallK;
    float fb = juce::jlimit(0.f, 0.8f,
        smoothedFeedback.getNextValue()) * overallK;

    // --- NUEVO: si el tiempo real < 1 ms lo tratamos como “delay OFF”
    if (timeSec < 0.001f)
    {
        dryWet = 0.f;   // no mezcla
        fb = 0.f;   // sin realimentación
    }
    /* 4. Procesar módulos */
    mod.setParameters(speedHz, modMix);

    delay.setTime(timeSec);   // ← ahora escalado por overallK
    delay.setFeedback(fb);    // ← ya escalado / forzado a 0 si delay OFF
    delay.setDryWet(dryWet);  // ← idem

    shelf.setGainDecibels(shelfDb);

    mod.process(buffer);
    delay.process(buffer);
    shelf.process(buffer);

    /* 5. Ganancia de salida */
    const float outGain = smoothedOutput.getNextValue();
    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* data = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i)
            data[i] *= outGain;
    }
}

// ────────────────────────────────────────────────────────────────
// Editor / State / Factory (sin cambios)
// ────────────────────────────────────────────────────────────────
bool StupidHouseAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* StupidHouseAudioProcessor::createEditor() { return new StupidHouseAudioProcessorEditor(*this); }

void StupidHouseAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    copyXmlToBinary(*state.createXml(), destData);
}

void StupidHouseAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

const juce::String StupidHouseAudioProcessor::getName() const { return JucePlugin_Name; }
bool   StupidHouseAudioProcessor::acceptsMidi()  const { return false; }
bool   StupidHouseAudioProcessor::producesMidi() const { return false; }
bool   StupidHouseAudioProcessor::isMidiEffect() const { return false; }
double StupidHouseAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int  StupidHouseAudioProcessor::getNumPrograms() { return 1; }
int  StupidHouseAudioProcessor::getCurrentProgram() { return 0; }
void StupidHouseAudioProcessor::setCurrentProgram(int) {}
const juce::String StupidHouseAudioProcessor::getProgramName(int) { return {}; }
void StupidHouseAudioProcessor::changeProgramName(int, const juce::String&) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StupidHouseAudioProcessor();
}