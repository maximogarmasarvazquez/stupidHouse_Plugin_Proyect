#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "IDs.h"

class StupidHouseAudioProcessorEditor : public juce::AudioProcessorEditor,
    private juce::Timer
{
public:
    explicit StupidHouseAudioProcessorEditor(StupidHouseAudioProcessor&);
    ~StupidHouseAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    juce::TextButton loadButton{ "Cargar Audio" };
    juce::FileChooser fileChooser;

    void timerCallback() override;  // <-- método override correcto

    StupidHouseAudioProcessor& audioProcessor;

    // Luz indicadora delay
    class DelayStatusLight : public juce::Component
    {
    public:
        void setActive(bool isActive)
        {
            active = isActive;
            repaint();
        }
        void paint(juce::Graphics& g) override
        {
            auto color = active ? juce::Colours::green : juce::Colours::red;
            g.setColour(color);
            auto bounds = getLocalBounds().toFloat();
            g.fillEllipse(bounds);
        }
    private:
        bool active = false;
    };

    DelayStatusLight delayStatusLight;

    // ComboBoxes para presets
    juce::ComboBox shapeBox, heatBox, spiceBox, depthBox;

    // Sliders principales
    juce::Slider shapeSlider, heatSlider, spiceSlider, depthSlider, overallSlider, outputSlider;

    // Sliders secundarios (con debounce en timeSlider y feedbackSlider)
    juce::Slider timeSlider, feedbackSlider, dryWetDelaySlider;
    juce::Slider speedSlider, highShelfSlider, dryWetModSlider;

    // Labels para sliders
    juce::Label shapeLabel, heatLabel, spiceLabel, depthLabel, overallLabel, outputLabel;
    juce::Label timeLabel, feedbackLabel, dryWetDelayLabel;
    juce::Label speedLabel, highShelfLabel, dryWetModLabel;

    // Attachments para sliders y combos (smart pointers)
    using ChoiceAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<ChoiceAttachment> shapePresetAttach, heatPresetAttach, spicePresetAttach, depthPresetAttach;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> shapeAttach, heatAttach, spiceAttach, depthAttach;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> overallAttach, outputAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dryWetDelayAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> speedAttach, highShelfAttach, dryWetModAttach;

    // ** NO attachment para timeSlider y feedbackSlider, manejados con debounce **

    // Variables para debounce slider time
    bool timeSliderIsBeingDragged = false;
    double timeSliderLastMoveTime = 0.0;

    // Variables para debounce slider feedback
    bool feedbackSliderIsBeingDragged = false;
    double feedbackSliderLastMoveTime = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StupidHouseAudioProcessorEditor)
};
