/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DistressorCloneAudioProcessor::DistressorCloneAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))/*,
    parameters(*this, nullptr, "PARAMETERS", {
std::make_unique<juce::AudioParameterFloat>("inputGainParam", "Input Gain", -24.0f, 24.0f, 0.0f),
std::make_unique<juce::AudioParameterFloat>("ratioParam", "Ratio", 1.0f, 25.0f, 2.0f),
std::make_unique<juce::AudioParameterFloat>("attackParam", "Attack", 0.5f, 300.0f, 10.0f),
std::make_unique<juce::AudioParameterFloat>("releaseParam", "Release", 500.0f, 35000.0f, 100.0f),
std::make_unique<juce::AudioParameterFloat>("outputGainParam", "Output Gain", -24.0f, 24.0f, 0.0f),

std::make_unique<juce::AudioParameterChoice>("compModeParam", "Compressor mode",
    juce::StringArray{ "Unlink", "Link", "Mid/Side" },
    1),
std::make_unique<juce::AudioParameterChoice>("distortionModeParam","Distortion Mode",
    juce::StringArray{ "Normal", "Dist 2", "Dist 3" },
    0)
        })*/
{
    addParameter(inputGain = new juce::AudioParameterFloat("inputGain", "input Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
        0.0f));
    //addParameter(threshold = new juce::AudioParameterFloat("threshold", "Threshold", -60.0f, 0.0f, -24.0f));
    addParameter(ratio = new juce::AudioParameterFloat("ratio", "Ratio", 1.0f, 25.0f, 4.0f));
    addParameter(attack = new juce::AudioParameterFloat("attack", "Attack", 0.5f, 300.0f, 10.0f));
    addParameter(release = new juce::AudioParameterFloat("release", "Release", 500.0f, 35000.0f, 100.0f));              
    addParameter(compModeParam = new juce::AudioParameterChoice("mode", "Compressor mode",
        juce::StringArray{ "Unlink", "Link", "Mid/Side" },
        1));
    addParameter(distModeParam = new juce::AudioParameterChoice("distortionMode","Distortion Mode",
        juce::StringArray{ "Normal", "Dist 2", "Dist 3" },
        0));
    addParameter(outputGain = new juce::AudioParameterFloat("outputGain", "Output Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
        0.0f));
}

DistressorCloneAudioProcessor::~DistressorCloneAudioProcessor()
{
}

//==============================================================================
void DistressorCloneAudioProcessor::prepareToPlay (double newSampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate;

    // Reservamos espacio para el n�mero m�ximo de canales
    envelopeDb.assign(getTotalNumOutputChannels(), 0.0f);

    // Calcular coeficientes a partir de los par�metros iniciales
    float atkMs = attack->get();
    float relMs = release->get();

    attackCoeff = std::exp(-1.0f / (0.001f * atkMs * sampleRate));
    releaseCoeff = std::exp(-1.0f / (0.001f * relMs * sampleRate));

    oversampling.reset();
    oversampling.initProcessing(samplesPerBlock);
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DistressorCloneAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void DistressorCloneAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    int numChannels = buffer.getNumChannels();
    int numSamples = buffer.getNumSamples();

    //float inputGainDb = parameters.getRawParameterValue("ratio")->load();
    float inputGainDb = inputGain->get();
    int mode = compModeParam->getIndex(); //  0=Unlink, 1=Link, 2=Mid/Side. 
    //  (no me gustan los numeros m�gicos pero hay que sacar esto adelante)

    float inputGainLin = juce::Decibels::decibelsToGain(inputGainDb);
    float outGain = juce::Decibels::decibelsToGain(outputGain->get());

    buffer.applyGain(inputGainLin);

    // TODO arreglar esta condicional
    if (false)
    {
        for (int sample = 0; sample < numSamples; ++sample)
        {
            for (int ch = 0; ch < numChannels; ++ch)
            {
                float in = buffer.getSample(ch, sample);
                float out = in - Compressor(in, ch);
                buffer.setSample(ch, sample, out);
            }
        }
    }
    else // Procesamiento normal
    {
        if (mode == 1) {
            for (int sample = 0; sample < numSamples; ++sample)
            {
                // detector com�n: max entre todos los canales
                float maxIn = 0.0f;
                for (int ch = 0; ch < numChannels; ++ch)
                    maxIn = std::max(maxIn, std::fabs(buffer.getSample(ch, sample)));

                // aplicar reducci�n com�n
                float gain = LinkComp(maxIn);

                for (int ch = 0; ch < numChannels; ++ch)
                    buffer.setSample(ch, sample, buffer.getSample(ch, sample) * gain);
            }
        }
        if (mode == 2 && numChannels >= 2) // el mid side SOLO funciona en stereo, 2 canales.
        {
            for (int sample = 0; sample < numSamples; ++sample)
            {
                float L = buffer.getSample(0, sample);
                float R = buffer.getSample(1, sample);

                // convertir a Mid/Side
                float Mid = (L + R) * 0.7071f;
                float Side = (L - R) * 0.7071f;

                // procesar por separado
                float MidOut = Compressor(Mid, 0);
                float SideOut = Compressor(Side, 1);

                // volver a L/R
                float newL = (MidOut + SideOut) * 0.7071f;
                float newR = (MidOut - SideOut) * 0.7071f;

                buffer.setSample(0, sample, newL);
                buffer.setSample(1, sample, newR);
            }
        }
        else { // mode 0 es el default
            for (int sample = 0; sample < numSamples; ++sample)
            {
                for (int ch = 0; ch < numChannels; ++ch)
                {
                    float in = buffer.getSample(ch, sample);
                    float out = postCompFxChain(Compressor(in, ch)); // aqui est�n los efectos en serie
                    buffer.setSample(ch, sample, out);
                }
            }
        }
    }

    // ========================================
    //  APLICAR EL OUT GAIN SIEMPRE AL FINAL.
    //  UNICA Y EXCLUSIVAMENTE 1 VEZ, SINO VAS A PICAR LA MIXER A +INF.
    //      never forget!
    buffer.applyGain(outGain);
}

float DistressorCloneAudioProcessor::computeGainFromLevel(float inputLevelDb, float threshDb, float ratio)
{
    // par�metros del knee mapping
    const float maxRatio = 20.0f;
    const float minRatioForMaxKnee = 2.0f;
    const float maxKneeDb = 16.0f;

    // calcular knee en dB seg�n ratio
    float kneeDb;
    if (ratio <= minRatioForMaxKnee)
        kneeDb = maxKneeDb;
    else if (ratio >= maxRatio)
        kneeDb = 0.0f;
    else
        kneeDb = maxKneeDb * ((maxRatio - ratio) / (maxRatio - minRatioForMaxKnee));

    // separaci�n entre nivel y threshold
    float x = inputLevelDb - threshDb; // x en dB
    float gainReductionDb = 0.0f;

    if (kneeDb <= 0.0001f) // hard knee (brickwall-like)
    {
        if (x > 0.0f)
            gainReductionDb = (threshDb + x / ratio) - (threshDb + x); // x*(1/ratio - 1)
        else
            gainReductionDb = 0.0f;
    }
    else
    {
        float halfK = 0.5f * kneeDb;

        if (x <= -halfK)
        {
            // below knee - no compression
            gainReductionDb = 0.0f;
        }
        else if (x >= halfK)
        {
            // above knee - normal compression (hard)
            gainReductionDb = (threshDb + x / ratio) - (threshDb + x); // = x*(1/ratio - 1)
        }
        else
        {
            // inside knee - quadratic smoothing
            // a = x + k/2
            float a = x + halfK;
            // factor (1/ratio - 1)
            float kRatioTerm = (1.0f / ratio) - 1.0f;
            // gain reduction dB
            gainReductionDb = (kRatioTerm * a * a) / (2.0f * kneeDb);
        }
    }
    // gainReductionDb es <= 0 (0 = sin reducci�n). sumamos makeup m�s adelante si corresponde.
    return gainReductionDb; // normalmente negativo o 0
}

float DistressorCloneAudioProcessor::Compressor(float inputSample, int channel)
{;
    float thresh = this->threshold; // -24dB
    float ratio = this->ratio->get();
    float gainReductionDb = 0.0f;

    // 1. Convertir a dBFS
    float inputLevelDb = juce::Decibels::gainToDecibels(std::fabs(inputSample) + 1.0e-10f);
    //float inputLevelDbLink = juce::Decibels::gainToDecibels(inputSample + 1.0e-10f);

    // 2. La compresi�n en si misma.
        
    if (inputLevelDb > thresh)
    {
       //gainReductionDb = (thresh + (inputLevelDb - thresh) / compRatio) - inputLevelDb
        gainReductionDb = computeGainFromLevel(inputLevelDb,thresh,ratio);
    }

    // 3. Suavizado por canal
    if (gainReductionDb < envelopeDb[channel])
        envelopeDb[channel] = attackCoeff * (envelopeDb[channel] - gainReductionDb) + gainReductionDb;
    else
        envelopeDb[channel] = releaseCoeff * (envelopeDb[channel] - gainReductionDb) + gainReductionDb;

    // (env�amos el GR al VUmetro)
    if (channel == 0)  // usar el primer canal como referencia
        this->gainReductionDb.store(-envelopeDb[channel]);

    // 4. Ganancia total
    //float totalGainDbUnLink = envelopeDb[channel] + makeup;
    //float totalGainUnLink = juce::Decibels::decibelsToGain(totalGainDbUnLink);

    // 5. Devolver muestra procesada
    return inputSample * juce::Decibels::decibelsToGain(envelopeDb[channel]);
}

float DistressorCloneAudioProcessor::LinkComp(float inputSample)
{
    float thresh = this->threshold;
    float ratio = this->ratio->get();
    //float makeup = 0;

    float inputLevelDb = juce::Decibels::gainToDecibels(inputSample + 1.0e-10f);

    float gainReductionDb = 0.0f;

    if (inputLevelDb > thresh)
        //gainReductionDb = (thresh + (inputLevelDb - thresh) / compRatio) - inputLevelDb;
        gainReductionDb = computeGainFromLevel(inputLevelDb, thresh, ratio);

    // suavizado com�n
    if (gainReductionDb < linkedEnvelopeDb)
        linkedEnvelopeDb = attackCoeff * (linkedEnvelopeDb - gainReductionDb) + gainReductionDb;
    else
        linkedEnvelopeDb = releaseCoeff * (linkedEnvelopeDb - gainReductionDb) + gainReductionDb;

    float totalGainDb = linkedEnvelopeDb /*+ makeup*/;
    return juce::Decibels::decibelsToGain(totalGainDb);
}

float DistressorCloneAudioProcessor::thDistortion(float inputSample)
{
    // TODO hay que programar el bypass del distorsionador.
    if (false) return(inputSample);

    int mode = distModeParam->getIndex();
    float x = inputSample;

    switch (mode)
    {
    case 0: //  Normal (Clean / Soft Clip)
    {
        // Soft clipper muy suave, casi transparente
        // Evita overs sin a�adir volumen extra
        const float ceiling = 0.8f;
        if (x > ceiling)      return ceiling - (ceiling - x) * 0.01f;
        else if (x < -ceiling) return -ceiling - (x + ceiling) * 0.01f;
        else                  return x;
    }

    case 1: //  Dist 2 (Tube-like, arm�nicos pares)
    {
        // Saturaci�n tipo v�lvula: tanh + cuadr�tica (2� arm�nico)
        float nonlinear = std::tanh(1.3f * x);
        nonlinear += 0.35f * x * x; // �nfasis 2� arm�nico
        return juce::jlimit(-1.0f, 1.0f, nonlinear);
    }

    case 2: //  Dist 3 (Tape-like, arm�nicos impares)
    {
        // Saturaci�n de cinta: compresi�n + arm�nicos impares
        float cubic = x - 0.7f * (std::pow(x, 3) - 0.35f * std::pow(x, 2));
        float nonlinear = std::tanh(1.7f * cubic);
        return juce::jlimit(-1.0f, 1.0f, nonlinear);
    }

    default:
        return x;
    }
}

float DistressorCloneAudioProcessor::hiPassFilter(float inputSample) {
    // TODO programar el hp filter.
    if (false) return inputSample;
    return inputSample;
}

float DistressorCloneAudioProcessor::postCompFxChain(float inputSample)
{
    //  huele espantoso? si. funciona? si.
    return hiPassFilter(thDistortion(inputSample));
}

//==============================================================================
juce::AudioProcessorEditor* DistressorCloneAudioProcessor::createEditor()
{
    // sin editor, el DAW muestra sus sliders.
    return nullptr;
}

bool DistressorCloneAudioProcessor::hasEditor() const
{
    return false;
}

//==============================================================================
const juce::String DistressorCloneAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DistressorCloneAudioProcessor::acceptsMidi() const
{
    return false;
}

bool DistressorCloneAudioProcessor::producesMidi() const
{
    return false;
}

bool DistressorCloneAudioProcessor::isMidiEffect() const
{
    return false;
}

double DistressorCloneAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

//==============================================================================
void DistressorCloneAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // serializamos todos los parametros registrados con addParameter en un XML.
    // usamos el paramID (AudioProcessorParameterWithID) como nombre del atributo.
    juce::XmlElement xml("DistressorCloneState");

    for (auto* param : getParameters())
    {
        if (auto* p = dynamic_cast<juce::AudioProcessorParameterWithID*>(param))
            xml.setAttribute(p->paramID, (double)p->getValue()); // valor normalizado 0..1
    }

    copyXmlToBinary(xml, destData);
}

void DistressorCloneAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
    {
        if (!xml->hasTagName("DistressorCloneState"))
            return;

        for (auto* param : getParameters())
        {
            if (auto* p = dynamic_cast<juce::AudioProcessorParameterWithID*>(param))
            {
                if (xml->hasAttribute(p->paramID))
                {
                    const float normalised = (float)xml->getDoubleAttribute(p->paramID, p->getValue());
                    p->setValueNotifyingHost(juce::jlimit(0.0f, 1.0f, normalised));
                }
            }
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DistressorCloneAudioProcessor();
}
