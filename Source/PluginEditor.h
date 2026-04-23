/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

class GainReductionMeter : public juce::Component, private juce::Timer
{
public:
    GainReductionMeter(std::atomic<float>& grValue) : gainReduction(grValue)
    {
        startTimerHz(30); // refrescar 30 fps
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        g.fillAll(juce::Colours::black);

        float gr = gainReduction.load(); // en dB positivos (0 = nada, 12 = mucha reducción)
        float maxReduction = 24.0f;      // escala máxima en dB

        float proportion = juce::jlimit(0.0f, 1.0f, gr / maxReduction);

        auto barHeight = bounds.getHeight() * proportion;
        g.setColour(juce::Colours::red);
        g.fillRect(bounds.removeFromBottom((int)barHeight));
    }

private:
    void timerCallback() override { repaint(); }

    std::atomic<float>& gainReduction;
};

//==============================================================================
/**
*/
class DistressorCloneAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    DistressorCloneAudioProcessorEditor (DistressorCloneAudioProcessor&);
    ~DistressorCloneAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    DistressorCloneAudioProcessor& audioProcessor;


    // Knobs (sliders)
    //juce::Slider thresholdSlider;
    //juce::Slider ratioSlider;
    //juce::Slider attackSlider;
    //juce::Slider releaseSlider;
    ////juce::Slider makeupSlider;
    //juce::Slider inputGainSlider;
    //juce::Slider outputGainSlider;

    // Labels opcionales
    //juce::Label thresholdLabel, ratioLabel, attackLabel, releaseLabel, outputGainLabel;

    //juce::ToggleButton deltaButton{ "delta" };


    std::unique_ptr<GainReductionMeter> grMeter;

    juce::Slider ratioSlider;
    std::unique_ptr<SliderAttachment> ratioAttachment;

    juce::Slider attackSlider;
    std::unique_ptr<SliderAttachment> attackAttachment;

    juce::Slider releaseSlider;
    std::unique_ptr<SliderAttachment> releaseAttachment;

    juce::Slider inputGainSlider;
    std::unique_ptr<SliderAttachment> inputGainAttachment;

    juce::Slider outputGainSlider;
    std::unique_ptr<SliderAttachment> outputAttachment;

    //juce::TextButton distorsionModeButton;
    //std::unique_ptr<ButtonAttachment> distorsionModeAttachment;

    // funciones

    void handleDistModeButton() {  }
    void handleCompModeButton() {  }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DistressorCloneAudioProcessorEditor)
};


