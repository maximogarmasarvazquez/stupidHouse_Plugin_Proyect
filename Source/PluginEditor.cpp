// P// PluginEditor.cpp
#include "PluginEditor.h"
#include "IDs.h"

// ── Aliases para ahorrar tipeo ──────────────────────────────────────────────
using ChoiceAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

// ─────────────────────────────────────────────────────────────────────────────
StupidHouseAudioProcessorEditor::StupidHouseAudioProcessorEditor(StupidHouseAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    /* ---------- ComboBoxes ---------- */
    juce::StringArray presetOptions{ "Default", "Soft", "Hard", "Tape" };

    auto setupBox = [this, &presetOptions](juce::ComboBox& box)
        {
            box.addItemList(presetOptions, 1);   // IDs empiezan en 1
            addAndMakeVisible(box);
        };

    setupBox(shapeBox);  setupBox(heatBox);
    setupBox(spiceBox);  setupBox(depthBox);

    /* ---------- Sliders básicos (Rotary) ---------- */
    auto rotary = [this](juce::Slider& s)
        {
            s.setSliderStyle(juce::Slider::Rotary);
            s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
            addAndMakeVisible(s);
        };

    rotary(shapeSlider);  rotary(heatSlider);
    rotary(spiceSlider);  rotary(depthSlider);
    rotary(overallSlider); rotary(outputSlider);

    /* ---------- Sliders horizontales (Rotary HV) ---------- */
    auto rotaryHV = [this](juce::Slider& s)
        {
            s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
            addAndMakeVisible(s);
        };

    // timeSlider ES UN DelaySlider (definido en PluginEditor.h)
    rotaryHV(timeSlider);       rotaryHV(feedbackSlider);    rotaryHV(dryWetDelaySlider);
    rotaryHV(speedSlider);      rotaryHV(highShelfSlider);   rotaryHV(dryWetModSlider);

    /* ---------- Labels ---------- */
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

    /* ---------- Attachments ---------- */
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
    timeAttach = std::make_unique<SliderAttachment>(audioProcessor.parameters, IDs::time, timeSlider);
    feedbackAttach = std::make_unique<SliderAttachment>(audioProcessor.parameters, IDs::feedback, feedbackSlider);
    dryWetDelayAttach = std::make_unique<SliderAttachment>(audioProcessor.parameters, IDs::dryWetDelay, dryWetDelaySlider);
    speedAttach = std::make_unique<SliderAttachment>(audioProcessor.parameters, IDs::speed, speedSlider);
    highShelfAttach = std::make_unique<SliderAttachment>(audioProcessor.parameters, IDs::highShelf, highShelfSlider);
    dryWetModAttach = std::make_unique<SliderAttachment>(audioProcessor.parameters, IDs::dryWetMod, dryWetModSlider);

    /* ---------- timeSlider configuración específica ---------- */
    timeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    timeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    // El texto ya lo devuelve DelaySlider::getTextFromValue()

    setSize(900, 600);
}

StupidHouseAudioProcessorEditor::~StupidHouseAudioProcessorEditor() {}

/* ========================================================================== */
/* ========================== PAINT ========================================= */
/* ========================================================================== */
void StupidHouseAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Fondo con leve degradado
    juce::Colour dark = juce::Colour::fromRGB(30, 30, 30);
    juce::Colour mid = juce::Colour::fromRGB(50, 50, 50);

    g.setGradientFill(juce::ColourGradient(dark, 0, 0, mid, 0, (float)getHeight(), false));
    g.fillAll();

    // Título opcional
    g.setColour(juce::Colours::lightgrey);
    g.setFont(18.0f);
    g.drawFittedText("StupidHouse FX", 0, 4, getWidth(), 24, juce::Justification::centred, 1);
}

/* ========================================================================== */
/* ========================== RESIZED ======================================= */
/* ========================================================================== */
void StupidHouseAudioProcessorEditor::resized()
{
    const int margin = 30;
    const int gap = 20;

    // Tamaños de knobs
    const int topKnob = 110;
    const int smallK = 90;
    const int overallK = 130;
    const int outputK = 80;

    // Combo‑box
    const int comboH = 20;
    const int comboW = 80;

    /* ---------- Fila superior: 4 knobs + combos ---------- */
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

    /* ---------- Fila inferior ---------- */
    const int bottomY = getHeight() - margin - smallK;
    const int groupW = 3 * smallK + 2 * gap;

    // — Central pair (Overall + Output) —
    const int pairGap = 16;
    const int pairW = overallK + pairGap + outputK;
    const int pairX = (getWidth() - pairW) / 2;

    overallSlider.setBounds(pairX,
        bottomY - (overallK - smallK) / 2,
        overallK, overallK);

    outputSlider.setBounds(pairX + overallK + pairGap,
        bottomY - (outputK - smallK) / 2,
        outputK, outputK);

    // — Grupos laterales —
    auto place3 = [&](int x, juce::Slider& a, juce::Slider& b, juce::Slider& c)
        {
            a.setBounds(x, bottomY, smallK, smallK);
            b.setBounds(x + smallK + gap, bottomY, smallK, smallK);
            c.setBounds(x + 2 * (smallK + gap), bottomY, smallK, smallK);
        };

    place3(margin, timeSlider, feedbackSlider, dryWetDelaySlider);
    place3(getWidth() - margin - groupW, speedSlider, highShelfSlider, dryWetModSlider);
}
