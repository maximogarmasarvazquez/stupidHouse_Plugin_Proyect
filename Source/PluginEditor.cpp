#include "PluginEditor.h"
#include "IDs.h"

// Aliases
using ChoiceAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

// Constructor
StupidHouseAudioProcessorEditor::StupidHouseAudioProcessorEditor(StupidHouseAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
    fileChooser("Seleccionar archivo de audio...", juce::File{}, "*.wav;*.mp3;*.aiff")
{
    addAndMakeVisible(delayStatusLight);
    startTimerHz(30); // Actualiza la luz y bloqueo cada ~33ms

    // Presets
    juce::StringArray presetOptions{ "Default", "Soft", "Hard", "Tape" };
    auto setupBox = [this, &presetOptions](juce::ComboBox& box)
        {
            box.addItemList(presetOptions, 1);
            addAndMakeVisible(box);
        };
    setupBox(shapeBox);  setupBox(heatBox);
    setupBox(spiceBox);  setupBox(depthBox);

    // Sliders
    auto rotary = [this](juce::Slider& s)
        {
            s.setSliderStyle(juce::Slider::Rotary);
            s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
            addAndMakeVisible(s);
        };
    rotary(shapeSlider);  rotary(heatSlider);
    rotary(spiceSlider);  rotary(depthSlider);
    rotary(overallSlider); rotary(outputSlider);

    auto rotaryHV = [this](juce::Slider& s)
        {
            s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
            addAndMakeVisible(s);
        };
    rotaryHV(dryWetDelaySlider);
    rotaryHV(speedSlider); rotaryHV(highShelfSlider); rotaryHV(dryWetModSlider);
    rotaryHV(timeSlider); timeSlider.setChangeNotificationOnlyOnRelease(true);   // ← NUEVO
    rotaryHV(feedbackSlider);
    rotaryHV(timeSlider);
    rotaryHV(feedbackSlider);

    //  ⬇ Añadí esta línea:
    timeAttach = std::make_unique<SliderAttachment>(
        audioProcessor.parameters, IDs::time, timeSlider);
    timeSlider.setNumDecimalPlacesToDisplay(2);
    timeSlider.setTextValueSuffix(" s");
    feedbackSlider.setNumDecimalPlacesToDisplay(2);
    speedSlider.setNumDecimalPlacesToDisplay(2);
    speedSlider.setTextValueSuffix(" Hz");

    // Labels
    auto label = [](juce::Label& l, juce::Slider& s, const juce::String& t)
        {
            l.setText(t, juce::dontSendNotification);
            l.attachToComponent(&s, false);
        };
    label(shapeLabel, shapeSlider, "Shape");
    label(heatLabel, heatSlider, "Heat");
    label(spiceLabel, spiceSlider, "Spice");
    label(depthLabel, depthSlider, "Depth");
    label(overallLabel, overallSlider, "Overall");
    label(outputLabel, outputSlider, "Output");
    label(timeLabel, timeSlider, "Time");
    label(feedbackLabel, feedbackSlider, "Feedback");
    label(dryWetDelayLabel, dryWetDelaySlider, "Dry/Wet Delay");
    label(speedLabel, speedSlider, "Speed");
    label(highShelfLabel, highShelfSlider, "High Shelf");
    label(dryWetModLabel, dryWetModSlider, "Dry/Wet Mod");

    // Attachments
    shapePresetAttach = std::make_unique<ChoiceAttachment>(audioProcessor.parameters, IDs::shapePreset, shapeBox);
    heatPresetAttach = std::make_unique<ChoiceAttachment>(audioProcessor.parameters, IDs::heatPreset, heatBox);
    spicePresetAttach = std::make_unique<ChoiceAttachment>(audioProcessor.parameters, IDs::spicePreset, spiceBox);
    depthPresetAttach = std::make_unique<ChoiceAttachment>(audioProcessor.parameters, IDs::depthPreset, depthBox);

    shapeAttach = std::make_unique<SliderAttachment>(audioProcessor.parameters, IDs::shape, shapeSlider);
    heatAttach = std::make_unique<SliderAttachment>(audioProcessor.parameters, IDs::heat, heatSlider);
    spiceAttach = std::make_unique<SliderAttachment>(audioProcessor.parameters, IDs::spice, spiceSlider);
    depthAttach = std::make_unique<SliderAttachment>(audioProcessor.parameters, IDs::depth, depthSlider);
    overallAttach = std::make_unique<SliderAttachment>(audioProcessor.parameters, IDs::overall, overallSlider);
    outputAttach = std::make_unique<SliderAttachment>(audioProcessor.parameters, IDs::outputGain, outputSlider);
    dryWetDelayAttach = std::make_unique<SliderAttachment>(audioProcessor.parameters, IDs::dryWetDelay, dryWetDelaySlider);
    speedAttach = std::make_unique<SliderAttachment>(audioProcessor.parameters, IDs::speed, speedSlider);
    highShelfAttach = std::make_unique<SliderAttachment>(audioProcessor.parameters, IDs::highShelf, highShelfSlider);
    dryWetModAttach = std::make_unique<SliderAttachment>(audioProcessor.parameters, IDs::dryWetMod, dryWetModSlider);
    feedbackAttach = std::make_unique<SliderAttachment>(audioProcessor.parameters, IDs::feedback, feedbackSlider);

    // Botón Cargar
    addAndMakeVisible(loadButton);
    loadButton.setButtonText("Cargar Audio");
    loadButton.onClick = [this]()
        {
            fileChooser.launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                [this](const juce::FileChooser& fc)
                {
                    auto file = fc.getResult();
                    if (file.existsAsFile())
                        audioProcessor.loadTestFile(file);
                });
        };

    setSize(900, 600);
}

StupidHouseAudioProcessorEditor::~StupidHouseAudioProcessorEditor() {}

void StupidHouseAudioProcessorEditor::paint(juce::Graphics& g)
{
    juce::Colour dark = juce::Colour::fromRGB(20, 20, 20);
    juce::Colour mid = juce::Colour::fromRGB(50, 50, 50);

    g.setGradientFill(juce::ColourGradient(dark, 0, 0, mid, 0, (float)getHeight(), false));
    g.fillAll();

    g.setColour(juce::Colours::lightgrey);
    g.setFont(18.0f);
    g.drawFittedText("StupidHouse FX", 0, 4, getWidth(), 24, juce::Justification::centred, 1);
}

void StupidHouseAudioProcessorEditor::resized()
{
    const int margin = 30;
    const int gap = 20;

    const int topKnob = 110;
    const int smallK = 90;
    const int overallK = 130;
    const int outputK = 80;

    const int comboH = 20;
    const int comboW = 80;

    auto area = getLocalBounds().reduced(margin);
    auto top = area.removeFromTop(topKnob + comboH + 10);
    int colW = (top.getWidth() - 3 * gap) / 4;

    auto placeTop = [&](juce::Slider& s, juce::ComboBox& cb)
        {
            juce::Rectangle<int> col = top.removeFromLeft(colW);
            s.setBounds(col.withSizeKeepingCentre(topKnob, topKnob));
            int cx = s.getX() + (s.getWidth() - comboW) / 2;
            cb.setBounds(cx, s.getBottom() + 2, comboW, comboH);
            top.removeFromLeft(gap);
        };

    placeTop(shapeSlider, shapeBox);
    placeTop(heatSlider, heatBox);
    placeTop(spiceSlider, spiceBox);
    placeTop(depthSlider, depthBox);

    const int bottomY = getHeight() - margin - smallK;
    const int groupW = 3 * smallK + 2 * gap;
    const int pairGap = 16;
    const int pairW = overallK + pairGap + outputK;
    const int pairX = (getWidth() - pairW) / 2;

    overallSlider.setBounds(pairX, bottomY - (overallK - smallK) / 2, overallK, overallK);
    outputSlider.setBounds(pairX + overallK + pairGap, bottomY - (outputK - smallK) / 2, outputK, outputK);

    auto place3 = [&](int x, juce::Slider& a, juce::Slider& b, juce::Slider& c)
        {
            a.setBounds(x, bottomY, smallK, smallK);
            b.setBounds(x + smallK + gap, bottomY, smallK, smallK);
            c.setBounds(x + 2 * (smallK + gap), bottomY, smallK, smallK);
        };

    place3(margin, feedbackSlider, timeSlider, dryWetDelaySlider);
    place3(getWidth() - margin - groupW, speedSlider, highShelfSlider, dryWetModSlider);

    const int lightSize = 10;
    int lightX = timeSlider.getX() + (timeSlider.getWidth() / 2) - (lightSize / 2);
    int lightY = timeSlider.getBottom() + 5;
    delayStatusLight.setBounds(lightX, lightY, lightSize, lightSize);

    loadButton.setBounds(margin, getHeight() - margin - 150, 100, 35);
}

// Timer: actualiza la luz y el enable/disable de sliders
void StupidHouseAudioProcessorEditor::timerCallback()
{
    auto timeValue = audioProcessor.parameters.getRawParameterValue(IDs::time)->load();
    bool delayIsOn = (timeValue > 0.001f);

    delayStatusLight.setActive(delayIsOn);
    feedbackSlider.setEnabled(delayIsOn);
    dryWetDelaySlider.setEnabled(delayIsOn);
}

void StupidHouseAudioProcessorEditor::setDelayLight(bool isActive)
{
    delayStatusLight.setActive(isActive);
}

void StupidHouseAudioProcessorEditor::resetDelaySliders()
{
    auto* fbParam = audioProcessor.parameters.getParameter(IDs::feedback);
    if (fbParam)
    {
        fbParam->beginChangeGesture();
        fbParam->setValueNotifyingHost(0.0f);
        fbParam->endChangeGesture();
    }

    auto* dwParam = audioProcessor.parameters.getParameter(IDs::dryWetDelay);
    if (dwParam)
    {
        dwParam->beginChangeGesture();
        dwParam->setValueNotifyingHost(0.0f);
        dwParam->endChangeGesture();
    }
}
