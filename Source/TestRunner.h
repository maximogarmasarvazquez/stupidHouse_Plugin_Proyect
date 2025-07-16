#pragma once
#include "PluginProcessor.h"

#include "ShapeModule.h"
#include "DelayModule.h"
#include "FilterModule.h"
#include "ModModule.h"
#include <JuceHeader.h>

void printPerceptualLoudness(const juce::AudioBuffer<float>& buffer, const juce::String& label, float appliedGain = 0.0f);
