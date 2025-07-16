#pragma once
#include "PluginProcessor.h"

void runAllTests(StupidHouseAudioProcessor& processor);
void printPerceptualLoudness(const juce::AudioBuffer<float>& buffer, const juce::String& label, float appliedGain = 0.0f);
