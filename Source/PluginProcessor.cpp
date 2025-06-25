#include "PluginProcessor.h"
#include "PluginEditor.h"

StupidHouseAudioProcessor::StupidHouseAudioProcessor()
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    parameters(*this, nullptr)
{
    // 1) Array con los nombres de sub-presets
    const juce::StringArray subNames{ "Soft", "Hard", "Tape" };

    // 2) Un parámetro Choice por módulo
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterChoice>(
        IDs::shapePreset, "Shape Preset", subNames, 0));

    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterChoice>(
        IDs::heatPreset, "Heat Preset", subNames, 0));

    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterChoice>(
        IDs::spicePreset, "Spice Preset", subNames, 0));

    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterChoice>(
        IDs::depthPreset, "Depth Preset", subNames, 0));

    // Aquí agregas tus parámetros, ejemplo simple:
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>(IDs::shape, "Shape", 0.0f, 1.0f, 0.5f));
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>(IDs::overall, "Overall", 0.0f, 1.0f, 0.5f));
    
    parameters.state = juce::ValueTree("parameters");
}

const juce::String StupidHouseAudioProcessor::getName() const { return JucePlugin_Name; }

StupidHouseAudioProcessor::~StupidHouseAudioProcessor() {}

bool StupidHouseAudioProcessor::acceptsMidi() const { return false; }
bool StupidHouseAudioProcessor::producesMidi() const { return false; }
bool StupidHouseAudioProcessor::isMidiEffect() const { return false; }
double StupidHouseAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int StupidHouseAudioProcessor::getNumPrograms() { return 1; }
int StupidHouseAudioProcessor::getCurrentProgram() { return 0; }
void StupidHouseAudioProcessor::setCurrentProgram(int) {}
const juce::String StupidHouseAudioProcessor::getProgramName(int) { return {}; }
void StupidHouseAudioProcessor::changeProgramName(int, const juce::String&) {}

void StupidHouseAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {}
void StupidHouseAudioProcessor::releaseResources() {}

bool StupidHouseAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void StupidHouseAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Aquí tu procesamiento de audio
}


juce::AudioProcessorEditor* StupidHouseAudioProcessor::createEditor()
{
    return new StupidHouseAudioProcessorEditor(*this);
}

bool StupidHouseAudioProcessor::hasEditor() const { return true; }


//==============================================================================
void StupidHouseAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // 1. Copiá el ValueTree actual que contiene todos los parámetros
    auto state = parameters.copyState();

    // 2. Convertilo a XML
    std::unique_ptr<juce::XmlElement> xml(state.createXml());

    // 3. Guardalo en el bloque binario que el host escribirá en la sesión/preset
    copyXmlToBinary(*xml, destData);
}

//==============================================================================
void StupidHouseAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // 1. El host devuelve el blob binario: lo convertimos a XML
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    // 2. Si el XML es válido y el tag coincide con el tipo de nuestro ValueTree...
    if (xmlState != nullptr && xmlState->hasTagName(parameters.state.getType()))
    {
        // 3. …lo transformamos en ValueTree y reemplazamos todo el estado.
        parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
}


// ** IMPORTANTE: Esta función es la que faltaba y genera el error LNK2001 **
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StupidHouseAudioProcessor();
}
