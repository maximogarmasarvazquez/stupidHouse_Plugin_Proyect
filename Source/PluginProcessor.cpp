// ===============================
// PluginProcessor.cpp
// ===============================
#include "PluginProcessor.h"
#include "PluginEditor.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor / Destructor
// ─────────────────────────────────────────────────────────────────────────────
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
    parameters.state = juce::ValueTree("StupidHouseParams");

    // Cache de punteros a parámetros
    pSpeed = parameters.getRawParameterValue(IDs::speed);
    pDryWetMod = parameters.getRawParameterValue(IDs::dryWetMod);
    pTime = parameters.getRawParameterValue(IDs::time);
    pFeedback = parameters.getRawParameterValue(IDs::feedback);
    pDryWetDelay = parameters.getRawParameterValue(IDs::dryWetDelay);
    pHighShelf = parameters.getRawParameterValue(IDs::highShelf);
    pOverall = parameters.getRawParameterValue(IDs::overall);
    pOutputGain = parameters.getRawParameterValue(IDs::outputGain);
}

StupidHouseAudioProcessor::~StupidHouseAudioProcessor() {}

// ─────────────────────────────────────────────────────────────────────────────
// Parámetros
// ─────────────────────────────────────────────────────────────────────────────
juce::AudioProcessorValueTreeState::ParameterLayout
StupidHouseAudioProcessor::createParameterLayout()
{
    using namespace juce;
    using APF = AudioParameterFloat;

    std::vector<std::unique_ptr<RangedAudioParameter>> params;
    const StringArray presetNames{ "Default", "Soft", "Hard", "Tape" };

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

    params.push_back(std::make_unique<APF>(IDs::time, "Delay Time", 0.0f, 2.0f, 0.0f));
    params.push_back(std::make_unique<APF>(IDs::feedback, "Feedback", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<APF>(IDs::dryWetDelay, "Dry/Wet Delay", 0.0f, 1.0f, 0.0f));

    params.push_back(std::make_unique<APF>(IDs::speed, "Mod Speed", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<APF>(IDs::dryWetMod, "Dry/Wet Mod", 0.0f, 1.0f, 0.0f));

    params.push_back(std::make_unique<APF>(IDs::highShelf, "High-Shelf", -24.f, 24.f, 0.0f));

    return { params.begin(), params.end() };
}

// ─────────────────────────────────────────────────────────────────────────────
// prepareToPlay / releaseResources
// ─────────────────────────────────────────────────────────────────────────────
void StupidHouseAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    transportSource.prepareToPlay(samplesPerBlock, sampleRate);
    delay.prepare(sampleRate, static_cast<int> (sampleRate * 2.0));
    mod.prepare(sampleRate);
    shelf.prepare(sampleRate);


    smoothedOutput.reset(sampleRate, 0.01);
    smoothedOutput.setCurrentAndTargetValue(1.0f);

    smoothedFeedback.reset(sampleRate, 0.01);
    smoothedFeedback.setCurrentAndTargetValue(0.0f);

    smoothedSpeed.reset(sampleRate, 0.01);
    smoothedSpeed.setCurrentAndTargetValue(0.0f);

    smoothedDryWetMod.reset(sampleRate, 0.01);
    smoothedDryWetMod.setCurrentAndTargetValue(0.0f);

    smoothedDryWetDelay.reset(sampleRate, 0.01);
    smoothedDryWetDelay.setCurrentAndTargetValue(0.0f);

    smoothedShelfDb.reset(sampleRate, 0.01);
    smoothedShelfDb.setCurrentAndTargetValue(0.0f);

    lfo.prepare(sampleRate);
    lfo.setFrequency(0.5f);
    lfo.setWaveform(0);
    lfo.reset();
}

void StupidHouseAudioProcessor::releaseResources() {
    transportSource.releaseResources();
}

bool StupidHouseAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

//──────────────────────────────────────────────────────────────────────────────
// Carga de archivo de prueba
//──────────────────────────────────────────────────────────────────────────────
void StupidHouseAudioProcessor::loadTestFile(const juce::File& file)
{
    if (!file.existsAsFile())
        return;

    const juce::ScopedLock sl(transportLock); // bloquea mientras reconfiguramos

    transportSource.stop();
    transportSource.setSource(nullptr);   // suelta cualquier fuente previa
    readerSource.reset();

    if (auto* reader = formatManager.createReaderFor(file))
    {
        readerSource.reset(new juce::AudioFormatReaderSource(reader, true)); // true = owns reader
        transportSource.setSource(readerSource.get(),
            0,                       // read ahead (0 → automático)
            nullptr,                 // no own thread
            reader->sampleRate);
        transportSource.setPosition(0.0);
        transportSource.start();        // reproduce inmediatamente (puedes quitarlo si prefieres play manual)
    }
}
// ─────────────────────────────────────────────────────────────────────────────
// processBlock
// ─────────────────────────────────────────────────────────────────────────────
void StupidHouseAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer&)
{

    const juce::ScopedLock sl(transportLock);

    // Reservamos un bloque que apunte al mismo buffer que ya recibió el host
    juce::AudioSourceChannelInfo info(&buffer, 0, buffer.getNumSamples());

    transportSource.getNextAudioBlock(info);

    juce::ScopedNoDenormals noDenormals;

    // --- Limpiar salidas sobrantes ----------------------------------------
    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear(ch, 0, buffer.getNumSamples());

    const int totalIn = getTotalNumInputChannels();
    const int totalOut = getTotalNumOutputChannels();
    for (int ch = totalIn; ch < totalOut; ++ch)
        buffer.clear(ch, 0, buffer.getNumSamples());

    // Cargar parámetros y aplicar suavizado
    float overallK = pOverall ? pOverall->load() : 0.5f;

    float uiTime = pTime ? pTime->load() : 0.f;
    float timeSec = (std::pow(uiTime, 4.f) * 1999.f + 1.f) / 1000.f;
    delay.setTime(timeSec);

    smoothedFeedback.setTargetValue((pFeedback ? pFeedback->load() : 0.f) * overallK);
    smoothedSpeed.setTargetValue((pSpeed ? pSpeed->load() : 0.f) * overallK);
    smoothedDryWetMod.setTargetValue((pDryWetMod ? pDryWetMod->load() : 0.f) * overallK);
    smoothedDryWetDelay.setTargetValue((pDryWetDelay ? pDryWetDelay->load() : 0.f) * overallK);
    smoothedShelfDb.setTargetValue((pHighShelf ? pHighShelf->load() : 0.f) * overallK);

    float fb = smoothedFeedback.getNextValue();
    float speedHz = juce::jmap(smoothedSpeed.getNextValue(), 0.f, 1.f, 0.f, 10.f);
    float mixMod = smoothedDryWetMod.getNextValue();
    float mixDelay = smoothedDryWetDelay.getNextValue();
    float shelfDb = smoothedShelfDb.getNextValue();

    mod.setParameters(speedHz, mixMod);
    delay.setFeedback(fb);
    delay.setDryWet(mixDelay);
    shelf.setGainDecibels(shelfDb);

    mod.process(buffer);
    delay.process(buffer);
    shelf.process(buffer);

    // --- Output gain suave, sin desfase entre canales ---
    float targetOut = juce::jmap(pOutputGain ? pOutputGain->load() : 0.5f,
        0.f, 1.f, 0.f, 2.f);
    smoothedOutput.setTargetValue(targetOut);

    const int numSamples = buffer.getNumSamples();
    const int numCh = buffer.getNumChannels();

    for (int i = 0; i < numSamples; ++i)
    {
        const float g = smoothedOutput.getNextValue();          // 1 avance por muestra

        for (int ch = 0; ch < numCh; ++ch)
            buffer.getWritePointer(ch)[i] *= g;
    }


}

// ─────────────────────────────────────────────────────────────────────────────
// Editor
// ─────────────────────────────────────────────────────────────────────────────
bool StupidHouseAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* StupidHouseAudioProcessor::createEditor()
{
    return new StupidHouseAudioProcessorEditor(*this);
}

// ─────────────────────────────────────────────────────────────────────────────
// Estado
// ─────────────────────────────────────────────────────────────────────────────
void StupidHouseAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    auto xml = state.createXml();
    copyXmlToBinary(*xml, destData);
}

void StupidHouseAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState && xmlState->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

const juce::String StupidHouseAudioProcessor::getName() const { return JucePlugin_Name; }
bool StupidHouseAudioProcessor::acceptsMidi() const { return false; }
bool StupidHouseAudioProcessor::producesMidi() const { return false; }
bool StupidHouseAudioProcessor::isMidiEffect() const { return false; }
double StupidHouseAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int StupidHouseAudioProcessor::getNumPrograms() { return 1; }
int StupidHouseAudioProcessor::getCurrentProgram() { return 0; }
void StupidHouseAudioProcessor::setCurrentProgram(int) {}
const juce::String StupidHouseAudioProcessor::getProgramName(int) { return {}; }
void StupidHouseAudioProcessor::changeProgramName(int, const juce::String&) {}

// ─────────────────────────────────────────────────────────────────────────────
// Fábrica
// ─────────────────────────────────────────────────────────────────────────────
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StupidHouseAudioProcessor();
}
