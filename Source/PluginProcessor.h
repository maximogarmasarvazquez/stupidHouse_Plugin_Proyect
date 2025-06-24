#pragma once

#include <JuceHeader.h>

namespace IDs
{
    constexpr auto shape = "shape";
    constexpr auto heat = "heat";
    constexpr auto spice = "spice";
    constexpr auto depth = "depth";

    constexpr auto overall = "overall";
    constexpr auto time = "time";
    constexpr auto feedback = "feedback";
    constexpr auto dryWetDelay = "dryWetDelay";
    constexpr auto speed = "speed";
    constexpr auto highShift = "highShift";
    constexpr auto dryWetMod = "dryWetMod";

}

class StupidHouseAudioProcessor : public juce::AudioProcessor
{
public:
    StupidHouseAudioProcessor();
    ~StupidHouseAudioProcessor() override;


    void prepareToPlay(double, int) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;


    juce::AudioProcessorValueTreeState parameters;




private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StupidHouseAudioProcessor)
};
