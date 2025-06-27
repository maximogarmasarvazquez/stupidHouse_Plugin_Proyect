// ===============================
// PluginProcessor.h
// ===============================
#pragma once

#include <JuceHeader.h>
#include "DelayModule.h"
#include "ModModule.h"
#include "FilterModule.h"
#include "LFO.h"
#include "IDs.h"

class StupidHouseAudioProcessor : public juce::AudioProcessor
{
public:
    StupidHouseAudioProcessor();
    ~StupidHouseAudioProcessor() override;

    // AudioProcessor overrides
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;

    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int) override;
    const juce::String getProgramName(int) override;
    void changeProgramName(int, const juce::String&) override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout&) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    bool hasEditor() const override;
    juce::AudioProcessorEditor* createEditor() override;

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    // Parámetros expuestos a la UI
    juce::AudioProcessorValueTreeState parameters;

private:
    // Módulos de procesamiento
    DelayModule delay;
    ModModule mod;
    FilterModule shelf;
    LFO lfo;

    // Punteros rápidos a parámetros
    std::atomic<float>* pSpeed{ nullptr };
    std::atomic<float>* pDryWetMod{ nullptr };
    std::atomic<float>* pTime{ nullptr };
    std::atomic<float>* pFeedback{ nullptr };
    std::atomic<float>* pHighShelf{ nullptr };
    std::atomic<float>* pOverall{ nullptr };
    std::atomic<float>* pDryWetDelay{ nullptr };
    std::atomic<float>* pOutputGain{ nullptr };
    // Factoría de parámetros
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StupidHouseAudioProcessor)
};