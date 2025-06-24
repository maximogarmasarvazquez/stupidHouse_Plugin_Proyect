#include "PluginProcessor.h"
#include "PluginEditor.h"

StupidHouseAudioProcessorEditor::StupidHouseAudioProcessorEditor(StupidHouseAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Sliders config (igual que antes)
    shapeSlider.setSliderStyle(juce::Slider::Rotary);
    shapeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(shapeSlider);

    heatSlider.setSliderStyle(juce::Slider::Rotary);
    heatSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(heatSlider);

    spiceSlider.setSliderStyle(juce::Slider::Rotary);
    spiceSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(spiceSlider);

    depthSlider.setSliderStyle(juce::Slider::Rotary);
    depthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(depthSlider);

    overallSlider.setSliderStyle(juce::Slider::Rotary);
    overallSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(overallSlider);

    timeSlider.setSliderStyle(juce::Slider::Rotary);
    timeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(timeSlider);

    feedbackSlider.setSliderStyle(juce::Slider::Rotary);
    feedbackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(feedbackSlider);

    dryWetDelaySlider.setSliderStyle(juce::Slider::Rotary);
    dryWetDelaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(dryWetDelaySlider);

    speedSlider.setSliderStyle(juce::Slider::Rotary);
    speedSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(speedSlider);

    highShiftSlider.setSliderStyle(juce::Slider::Rotary);
    highShiftSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(highShiftSlider);

    dryWetModSlider.setSliderStyle(juce::Slider::Rotary);
    dryWetModSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(dryWetModSlider);



    // Labels correctamente adjuntados a cada slider
    shapeLabel.setText("Shape", juce::dontSendNotification);
    shapeLabel.attachToComponent(&shapeSlider, false);  // label a la izquierda del slider

    heatLabel.setText("Heat", juce::dontSendNotification);
    heatLabel.attachToComponent(&heatSlider, false);  // label a la izquierda del slider

    spiceLabel.setText("Spice", juce::dontSendNotification);
    spiceLabel.attachToComponent(&spiceSlider, false);  // label a la izquierda del slider

    depthLabel.setText("Depth", juce::dontSendNotification);
    depthLabel.attachToComponent(&depthSlider, false);  // label a la izquierda del slider


    overallLabel.setText("Overall", juce::dontSendNotification);
    overallLabel.attachToComponent(&overallSlider, false);

    timeLabel.setText("Time", juce::dontSendNotification);
    timeLabel.attachToComponent(&timeSlider, false);

    feedbackLabel.setText("Feedback", juce::dontSendNotification);
    feedbackLabel.attachToComponent(&feedbackSlider, false);

    dryWetDelayLabel.setText("Dry/Wet Delay", juce::dontSendNotification);
    dryWetDelayLabel.attachToComponent(&dryWetDelaySlider, false);

    speedLabel.setText("Speed", juce::dontSendNotification);
    speedLabel.attachToComponent(&speedSlider, false);

    highShiftLabel.setText("High Shift", juce::dontSendNotification);
    highShiftLabel.attachToComponent(&highShiftSlider, false);

    dryWetModLabel.setText("Dry/Wet Mod", juce::dontSendNotification);
    dryWetModLabel.attachToComponent(&dryWetModSlider, false);

    // No necesitas hacer addAndMakeVisible para labels con attachToComponent()

    // Attachments de sliders a parámetros (igual que antes)
    shapeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, IDs::shape, shapeSlider);
    heatAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, IDs::heat, heatSlider);
    spiceAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, IDs::spice, spiceSlider);
    depthAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, IDs::depth, depthSlider);

    overallAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, IDs::overall, overallSlider);
    timeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, IDs::time, timeSlider);
    feedbackAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, IDs::feedback, feedbackSlider);
    dryWetDelayAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, IDs::dryWetDelay, dryWetDelaySlider);
    speedAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, IDs::speed, speedSlider);
    highShiftAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, IDs::highShift, highShiftSlider);
    dryWetModAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, IDs::dryWetMod, dryWetModSlider);

    setSize(1000, 600);
}


StupidHouseAudioProcessorEditor::~StupidHouseAudioProcessorEditor() {}

void StupidHouseAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::grey);
    g.setColour(juce::Colours::white);
    g.setFont(15.0f);
}

void StupidHouseAudioProcessorEditor::resized()
{
    const int margin = 20;   // borde exterior
    const int gap = 10;   // espacio entre sliders
    const int topRowH = 120;  // alto de la fila superior (A)
    const int bigSliderSz = 200;  // ancho = alto del slider grande (B)
    const int smallSz = 80;   // tamaño de los sliders pequeños (C)

    auto bounds = getLocalBounds().reduced(margin);

    // ── Fila (A) ──
    auto topRow = bounds.removeFromTop(topRowH);

    // ── Fila (C) inferior ──
    auto bottomRowH = smallSz + gap;          // alto = slider + margen
    auto bottomRow = bounds.removeFromBottom(bottomRowH);

    // ── Cuadrado central para el grande (B) ──
    auto centre = bounds.withSizeKeepingCentre(bigSliderSz, bigSliderSz);

    int filterW = (topRow.getWidth() - 3 * gap) / 4;   // 4 sliders
    shapeSlider.setBounds(topRow.removeFromLeft(filterW).reduced(5));
    topRow.removeFromLeft(gap);
    heatSlider.setBounds(topRow.removeFromLeft(filterW).reduced(5));
    topRow.removeFromLeft(gap);
    spiceSlider.setBounds(topRow.removeFromLeft(filterW).reduced(5));
    topRow.removeFromLeft(gap);
    depthSlider.setBounds(topRow.removeFromLeft(filterW).reduced(5));

    overallSlider.setBounds(centre.reduced(5));

    // Izquierda (Time, Feedback, DryWetDelay)
    auto leftCol = bottomRow.removeFromLeft((getWidth() / 2) - (bigSliderSz / 2) - margin);
    timeSlider.setBounds(leftCol.removeFromLeft(smallSz).withHeight(smallSz));
    leftCol.removeFromLeft(gap);
    feedbackSlider.setBounds(leftCol.removeFromLeft(smallSz).withHeight(smallSz));
    leftCol.removeFromLeft(gap);
    dryWetDelaySlider.setBounds(leftCol.removeFromLeft(smallSz).withHeight(smallSz));

    // Derecha (Speed, HighShift, DryWetMod)
    auto rightCol = bottomRow.removeFromRight((getWidth() / 2) - (bigSliderSz / 2) - margin);
    speedSlider.setBounds(rightCol.removeFromLeft(smallSz).withHeight(smallSz));
    rightCol.removeFromLeft(gap);
    highShiftSlider.setBounds(rightCol.removeFromLeft(smallSz).withHeight(smallSz));
    rightCol.removeFromLeft(gap);
    dryWetModSlider.setBounds(rightCol.removeFromLeft(smallSz).withHeight(smallSz));

}
