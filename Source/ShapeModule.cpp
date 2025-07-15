#include "ShapeModule.h"
#include <cmath>

void ShapeModule::prepare(double sampleRate, int samplesPerBlock, int /*numChannels*/)
{
    smoothedAGC.reset(sampleRate, 0.05); // 50 ms
    smoothedAGC.setCurrentAndTargetValue(1.0f);

    smoothedInputRMS.reset(sampleRate, 0.05);
    smoothedOutputRMS.reset(sampleRate, 0.05);

    oversampling.reset();
    oversampling.initProcessing(static_cast<size_t>(samplesPerBlock));

    silenceCounter = 0;
}

void ShapeModule::setParameters(ShapePreset preset, float drive, float outGain, bool applySoftClip, bool agc)
{
    enableAGC = agc;
    currentPreset = preset;
    driveAmount = drive;
    outputGain = outGain;
    softClipEnabled = applySoftClip;
}

float ShapeModule::saturateSoft(float x)
{
    // Saturación suave tipo tanh aproximado
    return std::tanh(driveAmount * x);
}

float ShapeModule::saturateHard(float x)
{
    // Saturación hard clipping
    float clean = driveAmount * x;
    if (clean > 1.0f) return 1.0f;
    if (clean < -1.0f) return -1.0f;
    return clean;
}

float ShapeModule::saturateTape(float x)
{
    // Saturación estilo cinta (tape saturation)
    float clean = driveAmount * x;
    if (clean > 0.0f)
        return 1.0f - std::exp(-clean);
    else
        return -1.0f + std::exp(clean);
}

float ShapeModule::saturateFoldback(float x)
{
    const float threshold = 0.5f; // Umbral de foldback
    if (x > threshold || x < -threshold)
    {
        return std::fabs(std::fmod(std::fabs(x - threshold), threshold * 4.f) - threshold * 2.f) - threshold;
    }
    return x;
}

void ShapeModule::process(juce::AudioBuffer<float>& buffer)
{
    if (currentPreset == ShapePreset::Clean)
        return; // No hacer nada en Clean

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    float inputRMS = 0.f;
    if (enableAGC)
        inputRMS = buffer.getRMSLevel(0, 0, numSamples);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* data = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i)
        {
            float x = data[i];

            // Aplicar saturación según preset
            switch (currentPreset)
            {
            case ShapePreset::Soft:
                data[i] = saturateSoft(x);
                break;
            case ShapePreset::Hard:
                data[i] = saturateHard(x);
                break;
            case ShapePreset::Tape:
                data[i] = saturateTape(x);
                break;
            case ShapePreset::Foldback:
                data[i] = saturateFoldback(x);
                break;
            default:
                break;
            }

            // Aplicar ganancia de salida
            data[i] *= outputGain;

            // Soft clip opcional
            if (softClipEnabled)
                data[i] = std::tanh(data[i]);
        }
    }

    // ─── AGC adaptativo ───────────────────────────────
    if (enableAGC)
    {
        const float minSignalThreshold = 0.001f;
        const int silenceThresholdBlocks = 5;

        float inRMS = buffer.getRMSLevel(0, 0, numSamples);
        float outRMS = buffer.getRMSLevel(0, 0, numSamples);

        // Bloqueo de AGC durante silencios prolongados
        if (inRMS < minSignalThreshold)
        {
            silenceCounter++;
            if (silenceCounter >= silenceThresholdBlocks)
            {
                smoothedAGC.setTargetValue(1.0f); // valor neutro
                return; // salimos sin aplicar AGC
            }
        }
        else
        {
            silenceCounter = 0;
        }

        smoothedInputRMS.setTargetValue(inRMS);
        smoothedOutputRMS.setTargetValue(outRMS);

        float input = smoothedInputRMS.getNextValue();
        float output = smoothedOutputRMS.getNextValue();

        if (input > minSignalThreshold && output > minSignalThreshold)
        {
            float targetComp = juce::jlimit(0.1f, 4.0f, input / output);
            smoothedAGC.setTargetValue(targetComp);
        }

        // Aplicar compensación sample a sample
        for (int ch = 0; ch < numChannels; ++ch)
        {
            float* data = buffer.getWritePointer(ch);
            for (int i = 0; i < numSamples; ++i)
            {
                float gain = smoothedAGC.getNextValue();
                data[i] *= gain;
            }
        }
    }
}