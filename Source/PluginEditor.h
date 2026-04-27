/*
  ==============================================================================

    Editor del plugin basado en juce::WebBrowserComponent (WebView UI).
    El DSP vive en PluginProcessor; la UI est� en Resources/ (HTML/CSS/JS)
    y se sirve desde BinaryData a trav�s de un ResourceProvider.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

using SliderAttachment = juce::WebSliderParameterAttachment;
using ComboAttachment  = juce::WebComboBoxParameterAttachment;

//==============================================================================
class DistressorCloneAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                             private juce::Timer
{
public:
    DistressorCloneAudioProcessorEditor (DistressorCloneAudioProcessor&);
    ~DistressorCloneAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    std::optional<juce::WebBrowserComponent::Resource> provideResource (const juce::String& url);

    DistressorCloneAudioProcessor& audioProcessor;

    // Relays expuestos a la UI web (deben construirse antes que la WebBrowserComponent).
    juce::WebSliderRelay   inputGainRelay  { "inputGain"  };
    juce::WebSliderRelay   outputGainRelay { "outputGain" };
    juce::WebSliderRelay   attackRelay     { "attack"     };
    juce::WebSliderRelay   releaseRelay    { "release"    };
    juce::WebSliderRelay   ratioRelay      { "ratio"      };
    juce::WebSliderRelay   dryWetRelay     { "dryWet"     };
    juce::WebComboBoxRelay modeRelay       { "mode"       };
    juce::WebComboBoxRelay distModeRelay   { "distortionMode" };

    juce::WebBrowserComponent webView;

    // Attachments para sincronizar los relays con los par�metros del processor.
    std::unique_ptr<SliderAttachment> inputGainAttach;
    std::unique_ptr<SliderAttachment> outputGainAttach;
    std::unique_ptr<SliderAttachment> attackAttach;
    std::unique_ptr<SliderAttachment> releaseAttach;
    std::unique_ptr<SliderAttachment> ratioAttach;
    std::unique_ptr<SliderAttachment> dryWetAttach;
    std::unique_ptr<ComboAttachment>  modeAttach;
    std::unique_ptr<ComboAttachment>  distModeAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DistressorCloneAudioProcessorEditor)
};
