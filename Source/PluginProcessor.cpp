#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "TestRunner.h"
#include "ShapeModule.h"
// Constructor
StupidHouseAudioProcessor::StupidHouseAudioProcessor()
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    parameters(*this, nullptr, juce::Identifier("StupidHouseParams"), createParameterLayout())
{
    formatManager.registerBasicFormats();

    pShapePreset = parameters.getRawParameterValue(IDs::shapePreset);
    pHeatPreset = parameters.getRawParameterValue(IDs::heatPreset);

    pHeat = parameters.getRawParameterValue(IDs::heat);
    pShape = parameters.getRawParameterValue(IDs::shape);

    pSpeed = parameters.getRawParameterValue(IDs::speed);
    pDryWetMod = parameters.getRawParameterValue(IDs::dryWetMod);
    pTime = parameters.getRawParameterValue(IDs::time);
    pFeedback = parameters.getRawParameterValue(IDs::feedback);
    pDryWetDelay = parameters.getRawParameterValue(IDs::dryWetDelay);
    pHighShelf = parameters.getRawParameterValue(IDs::highShelf);
    pOverall = parameters.getRawParameterValue(IDs::overall);
    pOutputGain = parameters.getRawParameterValue(IDs::outputGain);

    parameters.addParameterListener(IDs::time, this);

    jassert(pShape && pShapePreset && pSpeed && pDryWetMod && pTime &&
        pFeedback && pDryWetDelay && pHighShelf && pOverall && pOutputGain && pHeat && pHeatPreset);
}

// Destructor
StupidHouseAudioProcessor::~StupidHouseAudioProcessor()
{
    parameters.removeParameterListener(IDs::time, this);
}

// Parámetros del plugin
juce::AudioProcessorValueTreeState::ParameterLayout StupidHouseAudioProcessor::createParameterLayout()
{
    using namespace juce;
    using APF = AudioParameterFloat;

    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    params.push_back(std::make_unique<AudioParameterChoice>(IDs::shapePreset, "Shape Preset",
        StringArray{ "Default", "Soft", "Hard", "Tape", "Foldback" }, 0));
    params.push_back(std::make_unique<APF>(IDs::shape, "Shape Amount", 0.f, 1.f, 0.f));

    params.push_back(std::make_unique<AudioParameterChoice>(IDs::heatPreset, "Heat Preset",
        StringArray{ "Clean", "Warm", "Analog", "Dirty", "Crunch" }, 0));
    params.push_back(std::make_unique<APF>(IDs::heat, "Heat Amount", 0.f, 1.f, 0.f));

    params.push_back(std::make_unique<AudioParameterChoice>(IDs::spicePreset, "Spice Preset",
        StringArray{ "Default", "Low", "Medium", "High" }, 0));
    params.push_back(std::make_unique<APF>(IDs::spice, "Spice Amount", 0.f, 1.f, 0.f));
    params.push_back(std::make_unique<AudioParameterChoice>(IDs::depthPreset, "Depth Preset",
        StringArray{ "Default", "Shallow", "Medium", "Deep" }, 0));
    params.push_back(std::make_unique<APF>(IDs::depth, "Depth Amount", 0.f, 1.f, 0.f));

    params.push_back(std::make_unique<APF>(IDs::overall, "Overall Amount", 0.f, 1.f, 0.5f));
    params.push_back(std::make_unique<APF>(IDs::outputGain, "Output Gain", 0.f, 1.f, 0.5f));

    params.push_back(std::make_unique<APF>(IDs::time, "Delay Time", 0.f, 10.f, 0.f));
    params.push_back(std::make_unique<APF>(IDs::feedback, "Feedback", 0.f, 1.f, 0.f));
    params.push_back(std::make_unique<APF>(IDs::dryWetDelay, "Dry/Wet Delay", 0.f, 1.f, 0.f));
    params.push_back(std::make_unique<APF>(IDs::speed, "Mod Speed", 0.f, 1.f, 0.f));
    params.push_back(std::make_unique<APF>(IDs::dryWetMod, "Dry/Wet Mod", 0.f, 1.f, 0.f));
    params.push_back(std::make_unique<APF>(IDs::highShelf, "High/Shelf", -24.f, 24.f, 0.f));

    return { params.begin(), params.end() };
}

// Preparación del plugin para reproducir
void StupidHouseAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    shape.prepare(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
    delay.prepare(sampleRate, static_cast<int>(sampleRate * 2.0));
    mod.prepare(sampleRate);
    shelf.prepare(sampleRate);
    transportSource.prepareToPlay(samplesPerBlock, sampleRate);
    heat.prepare(sampleRate, samplesPerBlock, getTotalNumOutputChannels());

    smoothedGainCompensation.reset(sampleRate, 0.05);  // suavizado más lento, 50 ms
    smoothedGainCompensation.setCurrentAndTargetValue(1.0f);

    wetBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock, false, false, true);

    smoothedOutput.reset(sampleRate, 0.005);
    smoothedFeedback.reset(sampleRate, 0.005);
    smoothedSpeed.reset(sampleRate, 0.005);
    smoothedDryWetMod.reset(sampleRate, 0.005);
    smoothedShelfDb.reset(sampleRate, 0.005);

    smoothedOutput.setCurrentAndTargetValue(0.0f);
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

void StupidHouseAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID != IDs::time)
        return;

    bool mute = juce::jmap(newValue, 0.f, 1.f, 0.f, 2.f) < 0.001f;

    if (mute && !delayMuted.load())
    {
        juce::MessageManager::callAsync([this]
            {
                parameters.getParameter(IDs::dryWetDelay)->setValueNotifyingHost(0.f);
                parameters.getParameter(IDs::feedback)->setValueNotifyingHost(0.f);

                if (auto* ed = dynamic_cast<StupidHouseAudioProcessorEditor*>(getActiveEditor()))
                {
                    ed->setDelayLight(false);
                    ed->resetDelaySliders();
                }
            });
        delayMuted = true;
    }
    else if (!mute && delayMuted.load())
    {
        juce::MessageManager::callAsync([this]
            {
                if (auto* ed = dynamic_cast<StupidHouseAudioProcessorEditor*>(getActiveEditor()))
                    ed->setDelayLight(true);
            });
        delayMuted = false;
    }
}



void StupidHouseAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    constexpr float maxDrive = 3.5f;
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Playback seguro (no tocar)
    {
        const juce::ScopedLock sl(transportLock);
        juce::AudioSourceChannelInfo info(&buffer, 0, numSamples);
        transportSource.getNextAudioBlock(info);
    }

    const float overallK = juce::jlimit(0.f, 1.f, pOverall ? pOverall->load() : 0.1f);

    // === SHAPE DISTORTION CON COMPENSACIÓN ===

    float shapeAmount = pShape ? juce::jlimit(0.f, 0.7f, pShape->load() * overallK) : 0.f;
    float mappedDrive = ShapeModule::mapDriveValue(shapeAmount);
    float normalizedDrive = juce::jlimit(0.f, 1.f, mappedDrive / maxDrive);

    int shapePresetRaw = static_cast<int>(*parameters.getRawParameterValue(IDs::shapePreset));
    shape.updateParametersFromPreset(shapePresetRaw, shapeAmount);

    float inputRMSBefore = buffer.getRMSLevel(0, 0, numSamples); // Nivel antes del shape
    DBG("Input RMS Before Shape: " << inputRMSBefore);

    if (shapePresetRaw != static_cast<int>(ShapePreset::Clean) && normalizedDrive > 0.01f)
    {
        shape.processWithCompensation(buffer, shapeAmount, smoothedGainCompensation);
        lastAppliedGain = smoothedGainCompensation.getCurrentValue();

        DBG("Gain Applied (smoothed): " << lastAppliedGain);
        printPerceptualLoudness(buffer, "Salida del plugin", lastAppliedGain);
    }
    else
    {
        smoothedGainCompensation.setTargetValue(1.f);
        float smoothGain = smoothedGainCompensation.getNextValue();
        buffer.applyGain(smoothGain);

        lastAppliedGain = smoothGain;
        DBG("Gain Applied (no shape): " << lastAppliedGain);
        printPerceptualLoudness(buffer, "Salida del plugin", lastAppliedGain);
    }

    float inputRMSAfter = buffer.getRMSLevel(0, 0, numSamples);
    DBG("RMS después del Shape: " << inputRMSAfter);

    // === HEAT == 

    float heatAmount = pHeat ? juce::jlimit(0.f, 1.f, pHeat->load() * overallK) : 0.f;
    int heatPreset = static_cast<int>(*parameters.getRawParameterValue(IDs::heatPreset));

    if (heatPreset != static_cast<int>(HeatPreset::Clean) && heatAmount > 0.001f)
    {
        heat.updateParametersFromPreset(heatPreset, heatAmount);
        heat.process(buffer, heatAmount);
    }


    // === RESTO DEL PROCESAMIENTO ===

    smoothedShelfDb.setTargetValue((pHighShelf ? pHighShelf->load() : 0.f) * overallK);
    smoothedSpeed.setTargetValue((pSpeed ? pSpeed->load() : 0.f) * overallK);
    smoothedDryWetMod.setTargetValue((pDryWetMod ? pDryWetMod->load() : 0.f) * overallK);
    smoothedFeedback.setTargetValue((pFeedback ? pFeedback->load() : 0.f) * overallK);

    const float speedHz = juce::jmap(smoothedSpeed.getNextValue(), 0.f, 1.f, 0.f, 10.f);
    const float modMix = smoothedDryWetMod.getNextValue();
    const float shelfDb = smoothedShelfDb.getNextValue();

    const float uiTime = pTime ? pTime->load() : 0.f;
    const float timeSec = juce::jmap(uiTime, 0.f, 1.f, 0.f, 10.f);

    float dryWetDelay = (pDryWetDelay ? juce::jlimit(0.f, 1.f, pDryWetDelay->load()) : 0.f) * overallK;
    float fb = juce::jlimit(0.f, 0.8f, smoothedFeedback.getNextValue());

    if (timeSec < 0.001f)
    {
        dryWetDelay = 0.f;
        fb = 0.f;
    }

    mod.setParameters(speedHz, modMix);
    delay.setTime(timeSec);
    delay.setFeedback(fb);
    delay.setDryWet(dryWetDelay);
    shelf.setGainDecibels(shelfDb);

    delay.process(buffer);
    shelf.process(buffer);
    mod.process(buffer);

    // === GANANCIA FINAL ===

    float shapeGain = lastAppliedGain;
    float outGain = juce::jmap(pOutputGain ? pOutputGain->load() : 0.5f, 0.f, 1.f, 0.f, 2.f);
    smoothedOutput.setTargetValue(outGain);
    outGain = smoothedOutput.getNextValue();

    float combinedGain = shapeGain * outGain;

    const float maxCombinedGain = (shapePresetRaw == static_cast<int>(ShapePreset::Tape)) ? 1.7f : 1.5f;
    combinedGain = juce::jlimit(0.f, maxCombinedGain, combinedGain);
    DBG("Combined Gain after limit: " << combinedGain);

    // Aplicar tanh limitador
    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* data = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i)
            data[i] = std::tanh(data[i] * combinedGain);
    }
}

void StupidHouseAudioProcessor::setDriveAmountFromEditor(float newValue)
{
    currentDriveAmount = juce::jlimit(0.f, 1.f, newValue);

    if (auto* param = parameters.getParameter(IDs::shape))
    {
        param->beginChangeGesture();
        param->setValueNotifyingHost(currentDriveAmount);
        param->endChangeGesture();
    }
}

// Métodos básicos del plugin
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
bool StupidHouseAudioProcessor::acceptsMidi() const { return false; }
bool StupidHouseAudioProcessor::producesMidi() const { return false; }
bool StupidHouseAudioProcessor::isMidiEffect() const { return false; }
double StupidHouseAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int StupidHouseAudioProcessor::getNumPrograms() { return 1; }
int StupidHouseAudioProcessor::getCurrentProgram() { return 0; }
void StupidHouseAudioProcessor::setCurrentProgram(int) {}
const juce::String StupidHouseAudioProcessor::getProgramName(int) { return {}; }
void StupidHouseAudioProcessor::changeProgramName(int, const juce::String&) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StupidHouseAudioProcessor();
}
