#pragma once

#include <JuceHeader.h>
#include "DelayModule.h"
#include "ModModule.h"
#include "FilterModule.h"
#include "LFO.h"
#include "IDs.h"
#include "ShapeModule.h"
#include "HeatModule.h"

class StupidHouseAudioProcessor : public juce::AudioProcessor,
    public juce::AudioProcessorValueTreeState::Listener

{
public:
    StupidHouseAudioProcessor();
    ~StupidHouseAudioProcessor() override;

    void parameterChanged(const juce::String& parameterID,
        float newValue) override;
    void setDriveAmountFromEditor(float newDrive);

    // ───────────── AudioProcessor overrides ─────────────
    const juce::String getName() const override;
    bool   acceptsMidi()  const override;
    bool   producesMidi() const override;
    bool   isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int  getNumPrograms() override;
    int  getCurrentProgram() override;
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
    // Cargar un archivo de audio de prueba
    void loadTestFile(const juce::File& file);

    // ValueTree con todos los parámetros
    juce::AudioProcessorValueTreeState parameters;
private:
    float lastAppliedGain = 1.0f;
    std::atomic<bool> delayMuted{ false };
     float currentDriveAmount = 0.f;  // Guarda el valor enviado desde el editor
public:
    /*─────────── Módulos DSP ───────────*/
    DelayModule  delay;   // Feedback + dry/wet internos
    ModModule    mod;
    FilterModule shelf;
    LFO          lfo;
    ShapeModule  shape;

    /*─────────── Smoothed values ───────*/
    juce::SmoothedValue<float> smoothedFeedback;
    juce::SmoothedValue<float> smoothedSpeed;
    juce::SmoothedValue<float> smoothedDryWetMod;
    juce::SmoothedValue<float> smoothedShelfDb;
    juce::SmoothedValue<float> smoothedOutput;
    juce::SmoothedValue<float> smoothedGainCompensation;

    /*─────────── Buffer auxiliar ───────*/
    juce::AudioBuffer<float> wetBuffer;

    /*─────────── Punteros a parámetro ──*/
    std::atomic<float>* pShapePreset{ nullptr };
    std::atomic<float>* pShape{ nullptr };

    //std::atomic<float>* pHeatPreset{ nullptr };
    //std::atomic<float>* pHeat{ nullptr };


    std::atomic<float>* pSpeed{ nullptr };
    std::atomic<float>* pDryWetMod{ nullptr };
    std::atomic<float>* pTime{ nullptr };
    std::atomic<float>* pFeedback{ nullptr };
    std::atomic<float>* pHighShelf{ nullptr };
    std::atomic<float>* pOverall{ nullptr };
    std::atomic<float>* pDryWetDelay{ nullptr };
    std::atomic<float>* pOutputGain{ nullptr };

    /*─────────── Helpers archivo prueba ─*/
    juce::AudioFormatManager                       formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource                     transportSource;
    juce::CriticalSection                          transportLock;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StupidHouseAudioProcessor)
};
