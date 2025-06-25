#include "PluginProcessor.h"
#include "PluginEditor.h"

StupidHouseAudioProcessorEditor::StupidHouseAudioProcessorEditor(StupidHouseAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // ComboBoxes visibles
    addAndMakeVisible(shapeBox);
    addAndMakeVisible(heatBox);
    addAndMakeVisible(spiceBox);
    addAndMakeVisible(depthBox);

    // Opcional: agregás manualmente las opciones visibles si no querés que vengan del parámetro (sólo visual, no funcional):
    shapeBox.addItemList({ "Soft", "Hard", "Tape" }, 1);  // si querés que se vean incluso si no están conectados aún
    heatBox.addItemList({ "Soft", "Hard", "Tape" }, 1);  // si querés que se vean incluso si no están conectados aún
    spiceBox.addItemList({ "Soft", "Hard", "Tape" }, 1);  // si querés que se vean incluso si no están conectados aún
    depthBox.addItemList({ "Soft", "Hard", "Tape" }, 1);  // si querés que se vean incluso si no están conectados aún


    // Attachments
    using ChoiceAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

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


    shapePresetAttach = std::make_unique<ChoiceAttachment>(audioProcessor.parameters, IDs::shapePreset, shapeBox);
    heatPresetAttach = std::make_unique<ChoiceAttachment>(audioProcessor.parameters, IDs::heatPreset, heatBox);
    spicePresetAttach = std::make_unique<ChoiceAttachment>(audioProcessor.parameters, IDs::spicePreset, spiceBox);
    depthPresetAttach = std::make_unique<ChoiceAttachment>(audioProcessor.parameters, IDs::depthPreset, depthBox);

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

    setSize(900, 600);
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
    /* ── Parámetros básicos ─────────────────────────────── */
    const int margin = 30;
    const int gapCols = 20;
    const int topKnobSz = 120;
    const int comboH = 18;
    const int comboW = 80;

    const int smallKnob = 90;   // diámetros knobs inferiores
    const int gapBottom = 15;   // espacio entre knobs inferiores
    const int overallSz = 140;  // tamaño knob Overall (más grande)

    /* ── Sección superior: 4 knobs + combos ─────────────── */
    auto bounds = getLocalBounds().reduced(margin);
    auto topArea = bounds.removeFromTop(topKnobSz + comboH + 5);
    int colW = (topArea.getWidth() - 3 * gapCols) / 4;

    auto placeTop = [&](juce::Slider& sl, juce::ComboBox& cb)
        {
            juce::Rectangle<int> col = topArea.removeFromLeft(colW);
            juce::Rectangle<int> knobR = col.removeFromTop(topKnobSz);
            sl.setBounds(knobR.withSizeKeepingCentre(topKnobSz, topKnobSz));

            int boxX = knobR.getCentreX() - comboW / 2;
            cb.setBounds(boxX, knobR.getBottom() + 2, comboW, comboH);

            topArea.removeFromLeft(gapCols);
        };

    placeTop(shapeSlider, shapeBox);
    placeTop(heatSlider, heatBox);
    placeTop(spiceSlider, spiceBox);
    placeTop(depthSlider, depthBox);

    /* ── Fila inferior – dos grupos de 3 + Overall central ─────────*/

    // Coordenadas básicas
    const int lowRowY = getHeight() - margin - smallKnob;   // top de los knobs pequeños
    const int grpW = 3 * smallKnob + 2 * gapBottom;      // ancho de cada grupo de 3
    const int leftX = margin;
    const int rightX = getWidth() - margin - grpW;
    const int centreX = getWidth() / 2;

    /* --- Colocar grupos de 3 pequeños --- */
    auto placeSmallGrp = [&](int startX,
        juce::Slider& k1, juce::Slider& k2, juce::Slider& k3)
        {
            k1.setBounds(startX, lowRowY, smallKnob, smallKnob);
            k2.setBounds(startX + smallKnob + gapBottom, lowRowY, smallKnob, smallKnob);
            k3.setBounds(startX + 2 * (smallKnob + gapBottom), lowRowY, smallKnob, smallKnob);
        };

    placeSmallGrp(leftX, timeSlider, feedbackSlider, dryWetDelaySlider);
    placeSmallGrp(rightX, speedSlider, highShiftSlider, dryWetModSlider);

    /* --- Knob OVERALL centrado entre los dos grupos --- */
    // Queremos que verticalmente se alinee (centro) con los pequeños
    int overallTop = lowRowY - (overallSz - smallKnob) / 2;
    overallSlider.setBounds(
        centreX - overallSz / 2,   // X centrado
        overallTop,              // Y ajustado
        overallSz, overallSz);
}