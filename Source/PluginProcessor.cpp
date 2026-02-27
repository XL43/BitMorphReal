#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout
BitMorphAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // ── Helpers ───────────────────────────────────────────────────────────────
    auto addFloat = [&](const char* id, const char* name,
        float min, float max, float step,
        float skew, float defaultVal)
        {
            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID(id, 1), name,
                juce::NormalisableRange<float>(min, max, step, skew), defaultVal));
        };

    auto addBool = [&](const char* id, const char* name, bool defaultVal)
        {
            params.push_back(std::make_unique<juce::AudioParameterBool>(
                juce::ParameterID(id, 1), name, defaultVal));
        };

    auto addChoice = [&](const char* id, const char* name,
        juce::StringArray choices, int defaultIdx)
        {
            params.push_back(std::make_unique<juce::AudioParameterChoice>(
                juce::ParameterID(id, 1), name, choices, defaultIdx));
        };

    // ── Preamp ────────────────────────────────────────────────────────────────
    addFloat("preampGain", "Preamp Gain", 0.0f, 48.0f, 0.1f, 1.0f, 0.0f);

    // ── Quantizer ─────────────────────────────────────────────────────────────
    addFloat("bitDepth", "Bit Depth", 1.0f, 24.0f, 0.01f, 1.0f, 8.0f);
    addBool("bitDepthEnabled", "Bit Depth On", true);
    addFloat("dithering", "Dithering", 0.0f, 1.0f, 0.001f, 0.5f, 0.0f);
    addBool("dcShift", "DC Shift", false);

    // ── Resampler ─────────────────────────────────────────────────────────────
    addFloat("resampleFreq", "Resample Freq", 44.0f, 44100.0f, 1.0f, 0.25f, 44100.0f);
    addBool("resampleEnabled", "Resample On", true);

    // ── Approximative filter ──────────────────────────────────────────────────
    addBool("approxEnabled", "Approx Filter On", false);
    addFloat("approxDeviation", "Approx Deviation", -100.0f, 100.0f, 0.1f, 1.0f, 0.0f);

    // ── Images filter ─────────────────────────────────────────────────────────
    addBool("imagesEnabled", "Images Filter On", false);
    addFloat("imagesShift", "Images Shift", 0.0f, 1.0f, 0.001f, 1.0f, 0.0f);

    // ── Main filter ───────────────────────────────────────────────────────────
    addChoice("filterType", "Filter Type",
        { "Off", "Low Pass", "Band Pass", "High Pass", "Band Reject" }, 0);
    addChoice("filterOrder", "Filter Order",
        { "Post", "Pre" }, 0);
    addFloat("filterCutoff", "Filter Cutoff", 20.0f, 20000.0f, 1.0f, 0.25f, 2000.0f);
    addFloat("filterResonance", "Filter Resonance", 0.0f, 1.0f, 0.001f, 1.0f, 0.0f);

    // ── WaveCrusher ───────────────────────────────────────────────────────────
    addBool("waveCrushEnabled", "WaveCrusher On", false);
    addFloat("waveCrushAmount", "WaveCrush Amount", 0.0f, 1.0f, 0.001f, 1.0f, 0.0f);
    addChoice("waveCrushMode", "WaveCrush Mode",
        { "Fold", "Wrap", "Tanh" }, 0);

    // ── Ring modulator ────────────────────────────────────────────────────────
    addBool("ringModEnabled", "Ring Mod On", false);
    addFloat("ringModFreq", "Ring Mod Freq", 0.1f, 1000.0f, 0.01f, 0.3f, 440.0f);
    addFloat("ringModMix", "Ring Mod Mix", 0.0f, 1.0f, 0.001f, 1.0f, 1.0f);

    // ── LFO ───────────────────────────────────────────────────────────────────
    addFloat("lfoRate", "LFO Rate", 0.01f, 20.0f, 0.01f, 0.3f, 1.0f);
    addFloat("lfoDepth", "LFO Depth", 0.0f, 1.0f, 0.001f, 1.0f, 0.0f);
    addChoice("lfoWaveform", "LFO Waveform",
        { "Sine", "Triangle", "Square", "Saw Up", "Saw Down", "Random" }, 0);
    addChoice("lfoTarget", "LFO Target",
        { "Bit Depth", "Sample Rate", "WaveCrush", "Ring Freq" }, 0);

    // ── Step sequencer ────────────────────────────────────────────────────────
    addBool("stepSeqEnabled", "Step Seq On", false);
    addFloat("stepSeqRate", "Step Seq Rate", 0.25f, 8.0f, 0.25f, 1.0f, 1.0f);
    addFloat("stepSeqDepth", "Step Seq Depth", 0.0f, 1.0f, 0.001f, 1.0f, 0.5f);
    addChoice("stepSeqTarget", "Step Seq Target",
        { "Bit Depth", "Sample Rate", "WaveCrush", "Ring Freq" }, 1);

    // 16 individual step values
    for (int i = 0; i < 16; ++i)
    {
        auto id = "stepSeqStep" + juce::String(i);
        auto name = "Step " + juce::String(i + 1);
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(id.toStdString().c_str(), 1), name,
            juce::NormalisableRange<float>(-1.0f, 1.0f, 0.001f),
            (i % 4 == 0) ? 0.8f : 0.0f));
    }

    // ── Master ────────────────────────────────────────────────────────────────
    addFloat("fxMix", "FX Mix", 0.0f, 1.0f, 0.001f, 1.0f, 1.0f);
    addFloat("outputVolume", "Output Volume", -96.0f, 12.0f, 0.1f, 3.0f, 0.0f);

    return { params.begin(), params.end() };
}

BitMorphAudioProcessor::BitMorphAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    apvts(*this, nullptr, "BitMorphParams", createParameterLayout())
{
}

BitMorphAudioProcessor::~BitMorphAudioProcessor() {}

void BitMorphAudioProcessor::prepareToPlay(double /*sampleRate*/, int /*samplesPerBlock*/) {}
void BitMorphAudioProcessor::releaseResources() {}

void BitMorphAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& /*midiMessages*/)
{
    // Pure passthrough for now — DSP gets added in Phase 2
    juce::ScopedNoDenormals noDenormals;
    (void)buffer;
}

void BitMorphAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void BitMorphAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorEditor* BitMorphAudioProcessor::createEditor()
{
    return new BitMorphAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BitMorphAudioProcessor();
}