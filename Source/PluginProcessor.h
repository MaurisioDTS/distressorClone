/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class DistressorCloneAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    DistressorCloneAudioProcessor();
    ~DistressorCloneAudioProcessor() override;

    //juce::AudioProcessorValueTreeState parameters;

    //void setDeltaMode(bool enabled) { deltaMode = enabled; }
    //bool isDeltaMode() const { return deltaMode.load(); }

    std::atomic<float> gainReductionDb { 0.0f };
    std::atomic<float> gainReductionDbR{ 0.0f }; // canal R (o Side en M/S)

    // Peaks por bloque para los VU meters de la UI. En dBFS (-inf..0, aprox).
    // Se actualizan al final de cada processBlock y se leen desde el editor.
    std::atomic<float> inputPeakDbL  { -100.0f };
    std::atomic<float> inputPeakDbR  { -100.0f };
    std::atomic<float> outputPeakDbL { -100.0f };
    std::atomic<float> outputPeakDbR { -100.0f };

    int getDistMode() { return distModeParam->getIndex(); }
    int getCompMode() { return compModeParam->getIndex(); }

    juce::RangedAudioParameter* getParamByID(const juce::String& id) const
    {
        for (auto* p : getParameters())
            if (auto* withID = dynamic_cast<juce::AudioProcessorParameterWithID*>(p))
                if (withID->paramID == id)
                    return dynamic_cast<juce::RangedAudioParameter*>(p);
        return nullptr;
    }

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int index) override {}
    const juce::String getProgramName(int index) { return {}; }
    void changeProgramName(int index, const juce::String& newName) override {}

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //std::atomic<bool> deltaMode{ false };

private:

    // Par�metros del compresor
    std::vector<float> envelopeDb;   // detector de envolvente por canal (en dB)

    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;

    juce::dsp::Oversampling<float> oversampling{
    2, // n�mero de etapas (x2, x4, etc). Aqu� x4 = 2^2
    2, // factor de oversampling (2 = polyphase IIR, mejor calidad)
    juce::dsp::Oversampling<float>::FilterType::filterHalfBandPolyphaseIIR,
    true // mantener DC
    };

    //  umbral fijo (de momento)
    float threshold = -24.0f;

    juce::AudioParameterFloat* attack;
    juce::AudioParameterFloat* release;
    juce::AudioParameterFloat* inputGain;
    juce::AudioParameterFloat* outputGain;

    juce::AudioParameterFloat* ratio;

    juce::AudioParameterChoice* compModeParam;
    juce::AudioParameterChoice* distModeParam;

    float linkedEnvelopeDb = 0.0f;  // detector com�n para modo "Link"
    float midEnvDb = 0.0f;          // detector Mid
    float sideEnvDb = 0.0f;         // detector Side

    float currentGainDb = 0.0f; // En dB
    float sampleRate = 44100.0f;

    // funciones

    float computeGainFromLevel(float inputLevelDb, float threshDb, float ratio);
    float Compressor(float inputSample, int channel);
    float LinkComp(float inputSample);

    float postCompFxChain(float inputSample);
    float thDistortion(float inputSample);
    float hiPassFilter(float inputSample);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DistressorCloneAudioProcessor)
};
