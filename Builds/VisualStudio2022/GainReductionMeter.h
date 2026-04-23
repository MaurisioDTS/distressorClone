#pragma once

#include <JuceHeader.h>

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

