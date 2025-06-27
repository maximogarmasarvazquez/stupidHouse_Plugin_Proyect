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

    /* ---------- Shape / Heat / Spice / Depth ---------- */
    params.push_back(std::make_unique<AudioParameterChoice>(IDs::shapePreset, "Shape Preset", presetNames, 0));
    params.push_back(std::make_unique<APF>(IDs::shape, "Shape Amount", 0.f, 1.f, 0.5f));

    params.push_back(std::make_unique<AudioParameterChoice>(IDs::heatPreset, "Heat Preset", presetNames, 0));
    params.push_back(std::make_unique<APF>(IDs::heat, "Heat Amount", 0.f, 1.f, 0.5f));

    params.push_back(std::make_unique<AudioParameterChoice>(IDs::spicePreset, "Spice Preset", presetNames, 0));
    params.push_back(std::make_unique<APF>(IDs::spice, "Spice Amount", 0.f, 1.f, 0.5f));

    params.push_back(std::make_unique<AudioParameterChoice>(IDs::depthPreset, "Depth Preset", presetNames, 0));
    params.push_back(std::make_unique<APF>(IDs::depth, "Depth Amount", 0.f, 1.f, 0.5f));

    /* ---------- Macro & Salida ---------- */
    params.push_back(std::make_unique<APF>(IDs::overall, "Overall Amount", 0.f, 1.f, 0.5f)); // centro
    params.push_back(std::make_unique<APF>(IDs::outputGain, "Output Gain", 0.f, 1.f, 0.5f)); // unity

    // Delay
    params.push_back(std::make_unique<APF>(IDs::time, "Delay Time", 0.0f, 2.0f, 0.0f)); // ← 0 s
    params.push_back(std::make_unique<APF>(IDs::feedback, "Feedback", 0.0f, 1.0f, 0.0f)); // ← 0
    params.push_back(std::make_unique<APF>(IDs::dryWetDelay, "Dry/Wet Delay", 0.0f, 1.0f, 0.0f)); // ← dry

    // Mod
    params.push_back(std::make_unique<APF>(IDs::speed, "Mod Speed", 0.0f, 1.0f, 0.0f)); // ← 0 Hz
    params.push_back(std::make_unique<APF>(IDs::dryWetMod, "Dry/Wet Mod", 0.0f, 1.0f, 0.0f)); // ← dry

    // EQ
    params.push_back(std::make_unique<APF>(IDs::highShelf, "High-Shelf", -24.f, 24.f, 0.0f)); // ← 0 dB


    return { params.begin(), params.end() };
}

// ─────────────────────────────────────────────────────────────────────────────
// Métodos básicos
// ─────────────────────────────────────────────────────────────────────────────
const juce::String StupidHouseAudioProcessor::getName() const { return JucePlugin_Name; }
bool   StupidHouseAudioProcessor::acceptsMidi()  const { return false; }
bool   StupidHouseAudioProcessor::producesMidi() const { return false; }
bool   StupidHouseAudioProcessor::isMidiEffect() const { return false; }
double StupidHouseAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int    StupidHouseAudioProcessor::getNumPrograms() { return 1; }
int    StupidHouseAudioProcessor::getCurrentProgram() { return 0; }
void   StupidHouseAudioProcessor::setCurrentProgram(int) {}
const juce::String StupidHouseAudioProcessor::getProgramName(int) { return {}; }
void   StupidHouseAudioProcessor::changeProgramName(int, const juce::String&) {}

// ─────────────────────────────────────────────────────────────────────────────
// prepareToPlay / releaseResources
// ─────────────────────────────────────────────────────────────────────────────
void StupidHouseAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    delay.prepare(sampleRate, static_cast<int> (sampleRate * 2.0));
    mod.prepare(sampleRate);
    shelf.prepare(sampleRate);

    lfo.prepare(sampleRate);
    lfo.setFrequency(0.5f);
    lfo.setWaveform(0);
    lfo.reset();
}

void StupidHouseAudioProcessor::releaseResources() {}

bool StupidHouseAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

// ─────────────────────────────────────────────────────────────────────────────
// processBlock
// ─────────────────────────────────────────────────────────────────────────────
void StupidHouseAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer&)
{
    juce::ScopedNoDenormals _;
    const int totalIn = getTotalNumInputChannels();
    const int totalOut = getTotalNumOutputChannels();

    // Silenciar salidas sobrantes
    for (int ch = totalIn; ch < totalOut; ++ch)
        buffer.clear(ch, 0, buffer.getNumSamples());

    /* ---- leer y mapear parámetros ---- */

    // Delay time 1 ms … 2000 ms → segundos
    float uiTime = pTime ? pTime->load() : 0.f;
    float timeSec = (std::pow(uiTime, 4.f) * 1999.f + 1.f) / 1000.f;
    delay.setTime(timeSec);

    // Velocidad de modulación 0–10 Hz
    float speedUI = pSpeed ? pSpeed->load() : 0.f;
    float speedHz = juce::jmap(speedUI, 0.f, 1.f, 0.f, 10.f);

    float fb = pFeedback ? pFeedback->load() : 0.f;
    float mixDelay = pDryWetDelay ? pDryWetDelay->load() : 0.f;
    float mixMod = pDryWetMod ? pDryWetMod->load() : 0.f;
    float shelfDb = pHighShelf ? pHighShelf->load() : 0.f;
    float overallK = pOverall ? pOverall->load() : 0.5f;  // macro 0‑1

    /* ---- enviar a los módulos (escalados por Overall) ---- */
    mod.setParameters(speedHz * overallK, mixMod * overallK);
    delay.setFeedback(fb * overallK);
    delay.setDryWet(mixDelay * overallK);
    shelf.setGainDecibels(shelfDb * overallK);

    /* ---- procesar audio ---- */
    mod.process(buffer);   // si tu clase mod contiene un process()
    delay.process(buffer);
    shelf.process(buffer);

    /* ---- ganancia de salida ---- */
    float outUI = pOutputGain ? pOutputGain->load() : 0.5f;
    float outGain = juce::jmap(outUI, 0.f, 1.f, 0.f, 2.f);
    buffer.applyGain(outGain);
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

// ─────────────────────────────────────────────────────────────────────────────
// Fábrica
// ─────────────────────────────────────────────────────────────────────────────
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StupidHouseAudioProcessor();
}
