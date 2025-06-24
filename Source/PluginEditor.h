#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class StupidHouseAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit StupidHouseAudioProcessorEditor(StupidHouseAudioProcessor&);
    ~StupidHouseAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    StupidHouseAudioProcessor& audioProcessor;
    juce::Label shapeLabel, heatLabel, spiceLabel, depthLabel,overallLabel, timeLabel, feedbackLabel;
    juce::Label dryWetDelayLabel, speedLabel, highShiftLabel, dryWetModLabel;
    // Sliders
    juce::Slider shapeSlider;
    juce::Slider heatSlider;
    juce::Slider spiceSlider;
    juce::Slider depthSlider;

    juce::Slider overallSlider;
    juce::Slider timeSlider;
    juce::Slider feedbackSlider;

    juce::Slider dryWetDelaySlider;
    juce::Slider speedSlider;
    juce::Slider highShiftSlider;
    juce::Slider dryWetModSlider;

    // Slider attachments for parameters
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> shapeAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> heatAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> spiceAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> depthAttach;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> overallAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> timeAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> feedbackAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dryWetDelayAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> speedAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highShiftAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dryWetModAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StupidHouseAudioProcessorEditor)
};