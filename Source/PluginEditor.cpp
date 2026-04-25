/*
  ==============================================================================

    Editor basado en juce::WebBrowserComponent.
    Los recursos (HTML/CSS/JS) se sirven desde BinaryData v�a un ResourceProvider
    para que el plugin sea autocontenido (no depende de archivos externos).

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <unordered_map>

namespace
{
    //  Mapea una URL pedida por la WebView a la entrada correspondiente en BinaryData.
    //  La URL viene siempre con una "/" inicial; "/" sin path equivale a index.html.
    struct ResourceEntry { const char* binaryName; const char* mime; };

    static const std::unordered_map<juce::String, ResourceEntry>& getResourceTable()
    {
        static const std::unordered_map<juce::String, ResourceEntry> table = {
            { "/",                                { "index_html",                 "text/html" } },
            { "/index.html",                      { "index_html",                 "text/html" } },
            { "/styles.css",                      { "styles_css",                 "text/css" } },
            { "/main.js",                         { "main_js",                    "text/javascript" } },
            { "/juce/index.js",                   { "index_js",                   "text/javascript" } },
            { "/juce/check_native_interop.js",    { "check_native_interop_js",    "text/javascript" } },
        };
        return table;
    }
}

//==============================================================================
DistressorCloneAudioProcessorEditor::DistressorCloneAudioProcessorEditor (DistressorCloneAudioProcessor& p)
    : AudioProcessorEditor (&p),
      audioProcessor (p),
      webView (juce::WebBrowserComponent::Options{}
                   .withBackend (juce::WebBrowserComponent::Options::Backend::webview2)
                   .withWinWebView2Options (juce::WebBrowserComponent::Options::WinWebView2{}
                                                .withUserDataFolder (juce::File::getSpecialLocation (juce::File::tempDirectory))
                                                .withBackgroundColour (juce::Colours::black))
                   .withNativeIntegrationEnabled()
                   .withResourceProvider ([this] (const juce::String& url)
                                          { return provideResource (url); })
                   .withOptionsFrom (inputGainRelay)
                   .withOptionsFrom (outputGainRelay)
                   .withOptionsFrom (attackRelay)
                   .withOptionsFrom (releaseRelay)
                   .withOptionsFrom (ratioRelay)
                   .withOptionsFrom (modeRelay)
                   .withOptionsFrom (distModeRelay))
{
    // Enlazamos cada relay a su RangedAudioParameter correspondiente.
    auto attachSlider = [this] (juce::WebSliderRelay& relay, const juce::String& paramID)
        -> std::unique_ptr<SliderAttachment>
    {
        if (auto* param = audioProcessor.getParamByID (paramID))
            return std::make_unique<SliderAttachment> (*param, relay, nullptr);
        jassertfalse; // id mal escrito o par�metro no registrado
        return nullptr;
    };

    auto attachCombo = [this] (juce::WebComboBoxRelay& relay, const juce::String& paramID)
        -> std::unique_ptr<ComboAttachment>
    {
        if (auto* param = audioProcessor.getParamByID (paramID))
            return std::make_unique<ComboAttachment> (*param, relay, nullptr);
        jassertfalse;
        return nullptr;
    };

    inputGainAttach  = attachSlider (inputGainRelay,  "inputGain");
    outputGainAttach = attachSlider (outputGainRelay, "outputGain");
    attackAttach     = attachSlider (attackRelay,     "attack");
    releaseAttach    = attachSlider (releaseRelay,    "release");
    ratioAttach      = attachSlider (ratioRelay,      "ratio");
    modeAttach       = attachCombo  (modeRelay,       "mode");
    distModeAttach   = attachCombo  (distModeRelay,   "distortionMode");

    addAndMakeVisible (webView);
    webView.goToURL (juce::WebBrowserComponent::getResourceProviderRoot());

    setSize (600, 600);

    startTimerHz (30); // refresh de los VU meters
}

DistressorCloneAudioProcessorEditor::~DistressorCloneAudioProcessorEditor() = default;

//==============================================================================
void DistressorCloneAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void DistressorCloneAudioProcessorEditor::resized()
{
    webView.setBounds (getLocalBounds());
}

//==============================================================================
void DistressorCloneAudioProcessorEditor::timerCallback()
{
    juce::DynamicObject::Ptr payload = new juce::DynamicObject();
    payload->setProperty ("inL",  (double) audioProcessor.inputPeakDbL.load());
    payload->setProperty ("inR",  (double) audioProcessor.inputPeakDbR.load());
    payload->setProperty ("outL", (double) audioProcessor.outputPeakDbL.load());
    payload->setProperty ("outR", (double) audioProcessor.outputPeakDbR.load());
    payload->setProperty ("gr",   (double) audioProcessor.gainReductionDb.load());

    webView.emitEventIfBrowserIsVisible ("meters", juce::var (payload.get()));
}

//==============================================================================
std::optional<juce::WebBrowserComponent::Resource>
DistressorCloneAudioProcessorEditor::provideResource (const juce::String& url)
{
    const auto& table = getResourceTable();
    const auto it = table.find (url);
    if (it == table.end())
        return std::nullopt;

    int size = 0;
    const char* data = BinaryData::getNamedResource (it->second.binaryName, size);
    if (data == nullptr || size <= 0)
        return std::nullopt;

    juce::WebBrowserComponent::Resource resource;
    resource.data.reserve ((size_t) size);
    resource.data.insert (resource.data.end(),
                          reinterpret_cast<const std::byte*> (data),
                          reinterpret_cast<const std::byte*> (data) + size);
    resource.mimeType = it->second.mime;
    return resource;
}
