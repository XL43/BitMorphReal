#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout
BitMorphAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

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

    addFloat("preampGain", "Preamp Gain", 0.0f, 48.0f, 0.1f, 1.0f, 0.0f);
    addFloat("bitDepth", "Bit Depth", 1.0f, 24.0f, 0.01f, 1.0f, 8.0f);
    addBool("bitDepthEnabled", "Bit Depth On", true);
    addFloat("dithering", "Dithering", 0.0f, 1.0f, 0.001f, 0.5f, 0.0f);
    addBool("dcShift", "DC Shift", false);
    addFloat("resampleFreq", "Resample Freq", 44.0f, 44100.0f, 1.0f, 0.25f, 44100.0f);
    addBool("resampleEnabled", "Resample On", true);
    addBool("approxEnabled", "Approx Filter On", false);
    addFloat("approxDeviation", "Approx Deviation", -100.0f, 100.0f, 0.1f, 1.0f, 0.0f);
    addBool("imagesEnabled", "Images Filter On", false);
    addFloat("imagesShift", "Images Shift", 0.0f, 1.0f, 0.001f, 1.0f, 0.0f);

    addChoice("filterType", "Filter Type",
        { "Off", "Low Pass", "Band Pass", "High Pass", "Band Reject" }, 0);
    addChoice("filterOrder", "Filter Order", { "Post", "Pre" }, 0);

    addFloat("filterCutoff", "Filter Cutoff", 20.0f, 20000.0f, 1.0f, 0.25f, 2000.0f);
    addFloat("filterResonance", "Filter Resonance", 0.0f, 1.0f, 0.001f, 1.0f, 0.0f);
    addBool("waveCrushEnabled", "WaveCrusher On", false);
    addFloat("waveCrushAmount", "WaveCrush Amount", 0.0f, 1.0f, 0.001f, 1.0f, 0.0f);
    addChoice("waveCrushMode", "WaveCrush Mode", { "Fold", "Wrap", "Tanh" }, 0);
    addBool("ringModEnabled", "Ring Mod On", false);
    addFloat("ringModFreq", "Ring Mod Freq", 0.1f, 1000.0f, 0.01f, 0.3f, 440.0f);
    addFloat("ringModMix", "Ring Mod Mix", 0.0f, 1.0f, 0.001f, 1.0f, 1.0f);
    addFloat("lfoRate", "LFO Rate", 0.01f, 20.0f, 0.01f, 0.3f, 1.0f);
    addFloat("lfoDepth", "LFO Depth", 0.0f, 1.0f, 0.001f, 1.0f, 0.0f);
    addChoice("lfoWaveform", "LFO Waveform",
        { "Sine", "Triangle", "Square", "Saw Up", "Saw Down", "Random" }, 0);
    addChoice("lfoTarget", "LFO Target",
        { "Bit Depth", "Sample Rate", "WaveCrush", "Ring Freq" }, 0);
    addBool("stepSeqEnabled", "Step Seq On", false);
    addFloat("stepSeqRate", "Step Seq Rate", 0.25f, 8.0f, 0.25f, 1.0f, 1.0f);
    addFloat("stepSeqDepth", "Step Seq Depth", 0.0f, 1.0f, 0.001f, 1.0f, 0.5f);
    addChoice("stepSeqTarget", "Step Seq Target",
        { "Bit Depth", "Sample Rate", "WaveCrush", "Ring Freq" }, 1);

    for (int i = 0; i < 16; ++i)
    {
        auto id = "stepSeqStep" + juce::String(i);
        auto name = "Step " + juce::String(i + 1);
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(id.toStdString().c_str(), 1), name,
            juce::NormalisableRange<float>(-1.0f, 1.0f, 0.001f),
            (i % 4 == 0) ? 0.8f : 0.0f));
    }

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

void BitMorphAudioProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    float sr = static_cast<float> (sampleRate);
    processorL.prepare(sr);
    processorR.prepare(sr);
    lfo.setSampleRate(sr);
    stepSeq.setSampleRate(sr);
    snapshotParameters();
}

void BitMorphAudioProcessor::releaseResources()
{
    processorL.reset();
    processorR.reset();
    lfo.reset();
    stepSeq.reset();
}

void BitMorphAudioProcessor::snapshotParameters()
{
    float sr = static_cast<float> (getSampleRate());

    params.preampGain = juce::Decibels::decibelsToGain(
        apvts.getRawParameterValue("preampGain")->load(), -96.0f);
    params.bitDepth = apvts.getRawParameterValue("bitDepth")->load();
    params.bitDepthEnabled = apvts.getRawParameterValue("bitDepthEnabled")->load() > 0.5f;
    params.dithering = apvts.getRawParameterValue("dithering")->load();
    params.dcShift = apvts.getRawParameterValue("dcShift")->load() > 0.5f;
    params.resampleFreq = apvts.getRawParameterValue("resampleFreq")->load();
    params.resampleEnabled = apvts.getRawParameterValue("resampleEnabled")->load() > 0.5f;
    params.approxEnabled = apvts.getRawParameterValue("approxEnabled")->load() > 0.5f;
    params.approxDeviation = apvts.getRawParameterValue("approxDeviation")->load();
    params.imagesEnabled = apvts.getRawParameterValue("imagesEnabled")->load() > 0.5f;
    params.imagesShift = apvts.getRawParameterValue("imagesShift")->load();
    params.filterType = static_cast<int> (apvts.getRawParameterValue("filterType")->load());
    params.filterPre = apvts.getRawParameterValue("filterOrder")->load() > 0.5f;
    params.filterCutoff = apvts.getRawParameterValue("filterCutoff")->load();
    params.filterResonance = apvts.getRawParameterValue("filterResonance")->load();
    params.waveCrushEnabled = apvts.getRawParameterValue("waveCrushEnabled")->load() > 0.5f;
    params.waveCrushAmount = apvts.getRawParameterValue("waveCrushAmount")->load();
    params.waveCrushMode = static_cast<int> (apvts.getRawParameterValue("waveCrushMode")->load());
    params.ringModEnabled = apvts.getRawParameterValue("ringModEnabled")->load() > 0.5f;
    params.ringModFreq = apvts.getRawParameterValue("ringModFreq")->load();
    params.ringModMix = apvts.getRawParameterValue("ringModMix")->load();
    params.lfoRate = apvts.getRawParameterValue("lfoRate")->load();
    params.lfoDepth = apvts.getRawParameterValue("lfoDepth")->load();
    params.lfoWaveform = static_cast<int> (apvts.getRawParameterValue("lfoWaveform")->load());
    params.lfoTarget = static_cast<int> (apvts.getRawParameterValue("lfoTarget")->load());
    params.stepSeqEnabled = apvts.getRawParameterValue("stepSeqEnabled")->load() > 0.5f;
    params.stepSeqRate = apvts.getRawParameterValue("stepSeqRate")->load();
    params.stepSeqDepth = apvts.getRawParameterValue("stepSeqDepth")->load();
    params.stepSeqTarget = static_cast<int> (apvts.getRawParameterValue("stepSeqTarget")->load());
    params.fxMix = apvts.getRawParameterValue("fxMix")->load();
    params.outputVolume = juce::Decibels::decibelsToGain(
        apvts.getRawParameterValue("outputVolume")->load(), -96.0f);

    for (int i = 0; i < 16; ++i)
    {
        auto id = "stepSeqStep" + juce::String(i);
        stepSeq.setStep(i, apvts.getRawParameterValue(id.toStdString().c_str())->load());
    }

    // Configure DSP objects
    processorL.quantizer.setEnabled(params.bitDepthEnabled);
    processorR.quantizer.setEnabled(params.bitDepthEnabled);
    processorL.quantizer.setDithering(params.dithering);
    processorR.quantizer.setDithering(params.dithering);
    processorL.quantizer.setDcShift(params.dcShift);
    processorR.quantizer.setDcShift(params.dcShift);

    processorL.resampler.setEnabled(params.resampleEnabled);
    processorR.resampler.setEnabled(params.resampleEnabled);
    processorL.resampler.setFrequency(params.resampleFreq, sr);
    processorR.resampler.setFrequency(params.resampleFreq, sr);

    processorL.updateFilterCutoffs(params.resampleFreq, params.approxDeviation,
        params.imagesShift, params.approxEnabled,
        params.imagesEnabled);
    processorR.updateFilterCutoffs(params.resampleFreq, params.approxDeviation,
        params.imagesShift, params.approxEnabled,
        params.imagesEnabled);

    BiquadFilter::Type fType = BiquadFilter::Type::Off;
    switch (params.filterType)
    {
    case 1: fType = BiquadFilter::Type::LowPass;    break;
    case 2: fType = BiquadFilter::Type::BandPass;   break;
    case 3: fType = BiquadFilter::Type::HighPass;   break;
    case 4: fType = BiquadFilter::Type::BandReject; break;
    default: break;
    }
    processorL.mainFilter.setParameters(fType, params.filterCutoff, params.filterResonance);
    processorR.mainFilter.setParameters(fType, params.filterCutoff, params.filterResonance);

    processorL.waveCrusher.setEnabled(params.waveCrushEnabled);
    processorR.waveCrusher.setEnabled(params.waveCrushEnabled);
    processorL.waveCrusher.setMode(static_cast<WaveCrusher::Mode> (params.waveCrushMode));
    processorR.waveCrusher.setMode(static_cast<WaveCrusher::Mode> (params.waveCrushMode));
    processorL.waveCrusher.setAmount(params.waveCrushAmount);
    processorR.waveCrusher.setAmount(params.waveCrushAmount);

    processorL.ringMod.setEnabled(params.ringModEnabled);
    processorR.ringMod.setEnabled(params.ringModEnabled);
    processorL.ringMod.setFrequency(params.ringModFreq);
    processorR.ringMod.setFrequency(params.ringModFreq);
    processorL.ringMod.setMix(params.ringModMix);
    processorR.ringMod.setMix(params.ringModMix);

    lfo.setRate(params.lfoRate);
    lfo.setDepth(params.lfoDepth);
    lfo.setWaveform(static_cast<LFO::Waveform> (params.lfoWaveform));
    lfo.setTarget(static_cast<LFO::Target>   (params.lfoTarget));

    stepSeq.setEnabled(params.stepSeqEnabled);
    stepSeq.setDepth(params.stepSeqDepth);
    stepSeq.setTarget(static_cast<StepSequencer::Target> (params.stepSeqTarget));
    stepSeq.setTempo(static_cast<float> (currentBPM), params.stepSeqRate);
}

void BitMorphAudioProcessor::applyModulations(float lfoOut, float stepSeqOut,
    float& bitDepth, float& resampleFreq,
    float& waveCrushAmt, float& ringFreq)
{
    switch (static_cast<LFO::Target> (params.lfoTarget))
    {
    case LFO::Target::BitDepth:
        bitDepth = juce::jlimit(1.0f, 24.0f, bitDepth + lfoOut * 8.0f); break;
    case LFO::Target::SampleRate:
        resampleFreq = juce::jlimit(44.0f, 44100.0f, resampleFreq * (1.0f + lfoOut * 0.5f)); break;
    case LFO::Target::WaveCrush:
        waveCrushAmt = juce::jlimit(0.0f, 1.0f, waveCrushAmt + lfoOut * 0.5f); break;
    case LFO::Target::RingModFreq:
        ringFreq = juce::jlimit(0.1f, 1000.0f, ringFreq + lfoOut * 200.0f); break;
    }

    switch (static_cast<StepSequencer::Target> (params.stepSeqTarget))
    {
    case StepSequencer::Target::BitDepth:
        bitDepth = juce::jlimit(1.0f, 24.0f, bitDepth + stepSeqOut * 8.0f); break;
    case StepSequencer::Target::SampleRate:
        resampleFreq = juce::jlimit(44.0f, 44100.0f, resampleFreq * (1.0f + stepSeqOut * 0.5f)); break;
    case StepSequencer::Target::WaveCrush:
        waveCrushAmt = juce::jlimit(0.0f, 1.0f, waveCrushAmt + stepSeqOut * 0.5f); break;
    case StepSequencer::Target::RingModFreq:
        ringFreq = juce::jlimit(0.1f, 1000.0f, ringFreq + stepSeqOut * 200.0f); break;
    }
}

void BitMorphAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    bool isPlaying = false;
    if (auto* ph = getPlayHead())
    {
        if (auto pos = ph->getPosition())
        {
            if (auto bpm = pos->getBpm())
                currentBPM = *bpm;
            isPlaying = pos->getIsPlaying();

        }
    }

    snapshotParameters();

    float* dataL = buffer.getWritePointer(0);
    float* dataR = (numChannels > 1) ? buffer.getWritePointer(1) : dataL;

    BiquadFilter::Type fType = BiquadFilter::Type::Off;
    switch (params.filterType)
    {
    case 1: fType = BiquadFilter::Type::LowPass;    break;
    case 2: fType = BiquadFilter::Type::BandPass;   break;
    case 3: fType = BiquadFilter::Type::HighPass;   break;
    case 4: fType = BiquadFilter::Type::BandReject; break;
    default: break;
    }

    float sr = static_cast<float> (getSampleRate());

    for (int i = 0; i < numSamples; ++i)
    {
        float lfoOut = isPlaying ? lfo.tick() : 0.0f;
        float stepSeqOut = isPlaying ? stepSeq.tick() : 0.0f;

        float modBitDepth = params.bitDepth;
        float modResampleFreq = params.resampleFreq;
        float modWaveCrushAmt = params.waveCrushAmount;
        float modRingFreq = params.ringModFreq;

        applyModulations(lfoOut, stepSeqOut,
            modBitDepth, modResampleFreq,
            modWaveCrushAmt, modRingFreq);

        processorL.resampler.setFrequency(modResampleFreq, sr);
        processorR.resampler.setFrequency(modResampleFreq, sr);
        processorL.waveCrusher.setAmount(modWaveCrushAmt);
        processorR.waveCrusher.setAmount(modWaveCrushAmt);
        processorL.ringMod.setFrequency(modRingFreq);
        processorR.ringMod.setFrequency(modRingFreq);

        dataL[i] = processorL.process(dataL[i], params.preampGain,
            params.approxEnabled, params.imagesEnabled,
            modBitDepth, modResampleFreq, fType,
            params.filterPre, params.fxMix, params.outputVolume);

        dataR[i] = processorR.process(dataR[i], params.preampGain,
            params.approxEnabled, params.imagesEnabled,
            modBitDepth, modResampleFreq, fType,
            params.filterPre, params.fxMix, params.outputVolume);
    } // end sample loop



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