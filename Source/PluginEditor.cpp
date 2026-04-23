/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DistressorCloneAudioProcessorEditor::DistressorCloneAudioProcessorEditor (DistressorCloneAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    //grMeter = std::make_unique<GainReductionMeter>(audioProcessor.gainReductionDb);
    //addAndMakeVisible(*grMeter);

    //// Ratio
    //ratioAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
    //    audioProcessor.getParameters(), "ratio", ratioSlider);
    //addAndMakeVisible(ratioSlider);

    //// Attack
    //attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
    //    audioProcessor.getParameters(), "attack", attackSlider);
    //attackSlider.setSliderStyle(juce::Slider::Rotary);
    //addAndMakeVisible(attackSlider);

    //// Release
    //releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
    //    audioProcessor.getParameters(), "release", releaseSlider);
    //addAndMakeVisible(releaseSlider);

    // Input Gain
    // enlazamos el slider directamente con el RangedAudioParameter (sin APVTS).
    if (auto* inputGainParam = audioProcessor.getParamByID("inputGain"))
    {
        inputGainAttachment = std::make_unique<SliderAttachment>(
            *inputGainParam, inputGainSlider, nullptr);
        addAndMakeVisible(inputGainSlider);
    }

    ////// Output
    //outputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
    //    audioProcessor.getParameters(), "outputGainParam", outputGainSlider);
    //addAndMakeVisible(outputGainSlider);

    //// DistModeBTN
    //distorsionModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
    //    audioProcessor.getParameters(), "distortionModeParam", distorsionModeButton);
    //addAndMakeVisible(distorsionModeButton);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 400);
    //grMeter->setBounds(20, 20, 20, 20); // barra vertical
}

DistressorCloneAudioProcessorEditor::~DistressorCloneAudioProcessorEditor()
{
}

//==============================================================================
void DistressorCloneAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    //g.setFont (juce::FontOptions (15.0f));
    //g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);

}

void DistressorCloneAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..


    //auto area = getLocalBounds().reduced(20);   // margen general
    auto sliderWidth = 300;
    auto sliderHeight = 50;
    auto spacing = 10;

    //int x = area.getX();
    //int y = area.getY() + 40;

    //thresholdSlider.setBounds(x, y, sliderWidth, sliderHeight);
    //x += sliderWidth + spacing;

    //ratioSlider.setBounds(y, x, sliderWidth, sliderHeight);
    //x += sliderHeight + spacing;

    //attackSlider.setBounds(y, x, sliderWidth, sliderHeight);
    //x += sliderHeight + spacing;

    //releaseSlider.setBounds(y, x, sliderWidth, sliderHeight);
    //x += sliderHeight + spacing;

    //inputGainSlider.setBounds(y, x, sliderWidth, sliderHeight);
    //x += sliderHeight + spacing;

    //outputGainSlider.setBounds(y, x, sliderWidth, sliderHeight);
    //x += sliderHeight + spacing;

    //distorsionModeButton.setBounds(y, x, sliderWidth, sliderHeight);
    //distorsionModeButton.setButtonText("" + audioProcessor.getDistMode());




}