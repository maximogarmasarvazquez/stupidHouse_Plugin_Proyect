#include "PluginProcessor.h"
#include "PluginEditor.h"

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
    pSpeed = parameters.getRawParameterValue(IDs::speed);
    pDryWetMod = parameters.getRawParameterValue(IDs::dryWetMod);
    pTime = parameters.getRawParameterValue(IDs::time);
    pFeedback = parameters.getRawParameterValue(IDs::feedback);
    pDryWetDelay = parameters.getRawParameterValue(IDs::dryWetDelay);
    pHighShelf = parameters.getRawParameterValue(IDs::highShelf);
    pOverall = parameters.getRawParameterValue(IDs::overall);
    pOutputGain = parameters.getRawParameterValue(IDs::outputGain);
    pShape = parameters.getRawParameterValue(IDs::shape);

    parameters.addParameterListener(IDs::time, this);

    jassert(pShape && pShapePreset && pSpeed && pDryWetMod && pTime &&
        pFeedback && pDryWetDelay && pHighShelf && pOverall && pOutputGain);
}

StupidHouseAudioProcessor::~StupidHouseAudioProcessor()
{
    parameters.removeParameterListener(IDs::time, this);
}

juce::AudioProcessorValueTreeState::ParameterLayout StupidHouseAudioProcessor::createParameterLayout()
{
    using namespace juce;
    using APF = AudioParameterFloat;

    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    StringArray shapePresetNames{ "Default", "Soft", "Hard", "Tape", "Foldback" };
    StringArray heatPresetNames{ "Default", "Mild", "Medium", "Extreme" };
    StringArray spicePresetNames{ "Default", "Low", "Medium", "High" };
    StringArray depthPresetNames{ "Default", "Shallow", "Medium", "Deep" };

    params.push_back(std::make_unique<AudioParameterChoice>(IDs::shapePreset, "Shape Preset", shapePresetNames, 0));
    params.push_back(std::make_unique<AudioParameterFloat>(IDs::shape, "Shape Amount", 0.f, 2.f, 0.f));
    params.push_back(std::make_unique<AudioParameterChoice>(IDs::heatPreset, "Heat Preset", heatPresetNames, 0));
    params.push_back(std::make_unique<APF>(IDs::heat, "Heat Amount", 0.f, 1.f, 0.f));
    params.push_back(std::make_unique<AudioParameterChoice>(IDs::spicePreset, "Spice Preset", spicePresetNames, 0));
    params.push_back(std::make_unique<APF>(IDs::spice, "Spice Amount", 0.f, 1.f, 0.f));
    params.push_back(std::make_unique<AudioParameterChoice>(IDs::depthPreset, "Depth Preset", depthPresetNames, 0));
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

void StupidHouseAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    shape.prepare(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
    delay.prepare(sampleRate, static_cast<int>(sampleRate * 2.0));
    mod.prepare(sampleRate);
    shelf.prepare(sampleRate);
    transportSource.prepareToPlay(samplesPerBlock, sampleRate);

    smoothedGainCompensation.reset(sampleRate, 0.05); // suavizado 50ms
    smoothedGainCompensation.setCurrentAndTargetValue(1.0f); // ganancia neutra inicial
    smoothedGainComp.reset(sampleRate, 0.05); // 50ms
    smoothedGainComp.setCurrentAndTargetValue(1.0f);

    wetBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock, false, false, true);

    smoothedOutput.reset(sampleRate, 0.002);
    smoothedFeedback.reset(sampleRate, 0.01);
    smoothedSpeed.reset(sampleRate, 0.005);
    smoothedDryWetMod.reset(sampleRate, 0.005);
    smoothedShelfDb.reset(sampleRate, 0.01);

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

    const bool mute = juce::jmap(newValue, 0.f, 1.f, 0.f, 2.f) < 0.001f;

    if (mute && !delayMuted.load())
    {
        juce::MessageManager::callAsync([this]
            {
                parameters.getParameter(IDs::dryWetDelay)->setValueNotifyingHost(0.f);
                parameters.getParameter(IDs::feedback)->setValueNotifyingHost(0.f);

                if (auto* ed = dynamic_cast<StupidHouseAudioProcessorEditor*>(getActiveEditor()))
                {
                    ed->setDelayLight(false);  // luz roja
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
                    ed->setDelayLight(true);  // luz verde
            });
        delayMuted = false;
    }
}

void StupidHouseAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    constexpr float maxDrive = 3.5f;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    {
        const juce::ScopedLock sl(transportLock);
        juce::AudioSourceChannelInfo info(&buffer, 0, numSamples);
        transportSource.getNextAudioBlock(info);
    }

    const float overallK = juce::jlimit(0.f, 1.f, pOverall ? pOverall->load() : 0.1f);

    // ───── Shape (Distorsión con compensación RMS y bypass para valores bajos) ───────────────────────────────
    {
        int shapePresetRaw = static_cast<int>(*parameters.getRawParameterValue(IDs::shapePreset));
        ShapePreset safePreset = static_cast<ShapePreset>(juce::jlimit(0, 4, shapePresetRaw));

        float shapeAmount = juce::jlimit(0.f, 2.5f, currentDriveAmount);
        float normalizedDrive = juce::jlimit(0.f, 1.f, (shapeAmount * overallK) / maxDrive);
        float driveAmount = juce::jmap(normalizedDrive, 0.f, 1.f, 0.f, maxDrive);

        shape.setParameters(safePreset, shapeAmount, 1.0f, true);

        // Si el drive es muy bajo, pasamos el buffer limpio (sin distorsión ni compensación)
        if (safePreset != ShapePreset::Clean && normalizedDrive > 0.02f)
        {
            // 1. Guardamos la señal original
            juce::AudioBuffer<float> dryBuffer;
            dryBuffer.makeCopyOf(buffer);

            // 2. Calculamos RMS de entrada
            float rmsIn = dryBuffer.getRMSLevel(0, 0, buffer.getNumSamples()); // canal 0

            // 3. Procesamos shape (distorsión)
            shape.process(buffer);

            // 4. Calculamos RMS de salida
            float rmsOut = buffer.getRMSLevel(0, 0, buffer.getNumSamples());

            // 5. Compensación con límite
            float minGainComp = juce::jmap(normalizedDrive, 0.f, 1.f, 1.f, 0.7f); // sin bajar volumen en bajos drives
            float gainComp = (rmsOut > 0.0001f) ? rmsIn / rmsOut : 1.0f;
            gainComp = juce::jlimit(minGainComp, 1.6f, gainComp); // límites seguros

            // 6. Suavizado
            smoothedGainCompensation.setTargetValue(gainComp);
            float smoothGain = smoothedGainCompensation.getNextValue();
            buffer.applyGain(smoothGain);

            // 7. Dry/wet mix
            float dryWet = pDryWetDistortion ? juce::jlimit(0.f, 1.f, pDryWetDistortion->load()) : 1.f;

            for (int ch = 0; ch < numChannels; ++ch)
            {
                float* dryData = dryBuffer.getWritePointer(ch);
                float* wetData = buffer.getWritePointer(ch);
                for (int i = 0; i < numSamples; ++i)
                    wetData[i] = dryData[i] * (1.0f - dryWet) + wetData[i] * dryWet;
            }
        }
        else
        {
            // Bypass distorsión, dejamos buffer limpio y ganancia neutra
            smoothedGainCompensation.setTargetValue(1.0f);
            float smoothGain = smoothedGainCompensation.getNextValue();
            buffer.applyGain(smoothGain);
        }
    }

    // ───── Parámetros modulados por overallK ───────────────────────
    smoothedShelfDb.setTargetValue((pHighShelf ? pHighShelf->load() : 0.f) * overallK);
    smoothedSpeed.setTargetValue((pSpeed ? pSpeed->load() : 0.f) * overallK);
    smoothedDryWetMod.setTargetValue((pDryWetMod ? pDryWetMod->load() : 0.f) * overallK);
    smoothedFeedback.setTargetValue((pFeedback ? pFeedback->load() : 0.f) * overallK);

    const float speedHz = juce::jmap(smoothedSpeed.getNextValue(), 0.f, 1.f, 0.f, 10.f);
    const float modMix = smoothedDryWetMod.getNextValue();
    const float shelfDb = smoothedShelfDb.getNextValue();

    const float uiTime = pTime ? pTime->load() : 0.f;
    const float timeSec = juce::jmap(uiTime, 0.f, 1.f, 0.f, 10.f);

    float dryWetDelay = pDryWetDelay ? juce::jlimit(0.f, 1.f, pDryWetDelay->load()) * overallK : 0.f;
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

    // ───── Output Gain final ──────────────────────────
    float outGain = juce::jmap(pOutputGain ? pOutputGain->load() : 0.5f, 0.f, 1.f, 0.f, 2.f);
    smoothedOutput.setTargetValue(outGain);

    outGain = smoothedOutput.getNextValue();
    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* data = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i)
            data[i] *= outGain;
    }
}


void StupidHouseAudioProcessor::setDriveAmountFromEditor(float newValue)
{
    // Limitar al rango 0-5 para que coincida con el parámetro
    currentDriveAmount = juce::jlimit(0.f, 2.f, newValue);

    if (auto* param = parameters.getParameter(IDs::shape))
    {
        param->beginChangeGesture();
        param->setValueNotifyingHost(currentDriveAmount / 2.f);
        param->endChangeGesture();
    }
}
// Métodos restantes (Editor, estado, fábrica)
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
