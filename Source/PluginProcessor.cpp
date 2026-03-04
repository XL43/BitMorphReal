#include "PluginProcessor.h"
#include "PluginEditor.h"

// =============================================================================
//  Parameter layout
// =============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
BitMorphAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;

    auto addFloat = [&](const char* id, const char* name,
        float lo, float hi, float step, float skew, float def)
        {
            p.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID(id, 1), name,
                juce::NormalisableRange<float>(lo, hi, step, skew), def));
        };
    auto addBool = [&](const char* id, const char* name, bool def)
        {
            p.push_back(std::make_unique<juce::AudioParameterBool>(
                juce::ParameterID(id, 1), name, def));
        };
    auto addChoice = [&](const char* id, const char* name,
        juce::StringArray choices, int def)
        {
            p.push_back(std::make_unique<juce::AudioParameterChoice>(
                juce::ParameterID(id, 1), name, choices, def));
        };

    addFloat(ParamID::PREAMP_GAIN, "Preamp Gain", 0.0f, 48.0f, 0.1f, 1.0f, 0.0f);

    addBool(ParamID::DRIVE_ENABLED, "Drive On", false);
    addFloat(ParamID::DRIVE_AMOUNT, "Drive", 0.0f, 24.0f, 0.1f, 1.0f, 0.0f);
    addFloat(ParamID::DRIVE_BIAS, "Drive Bias", -1.0f, 1.0f, 0.01f, 1.0f, 0.0f);
    addChoice(ParamID::DRIVE_MODE, "Drive Mode", { "Soft","Hard","Tube","Foldback" }, 0);
    addFloat(ParamID::DRIVE_MIX, "Drive Mix", 0.0f, 1.0f, 0.01f, 1.0f, 1.0f);

    addFloat(ParamID::BIT_DEPTH, "Bit Depth", 1.0f, 24.0f, 0.01f, 1.0f, 8.0f);
    addBool(ParamID::BIT_DEPTH_ENABLED, "Bit Depth On", true);
    addFloat(ParamID::DITHERING, "Dithering", 0.0f, 1.0f, 0.001f, 0.5f, 0.0f);
    addBool(ParamID::DC_SHIFT, "DC Shift", false);

    addFloat(ParamID::RESAMPLE_FREQ, "Resample Freq", 44.0f, 44100.0f, 1.0f, 0.25f, 44100.0f);
    addBool(ParamID::RESAMPLE_ENABLED, "Resample On", true);
    addFloat(ParamID::JITTER, "Jitter", 0.0f, 1.0f, 0.001f, 0.5f, 0.0f);
    addBool(ParamID::APPROX_ENABLED, "Approx Filter On", false);
    addFloat(ParamID::APPROX_DEVIATION, "Approx Deviation", -100.0f, 100.0f, 0.1f, 1.0f, 0.0f);
    addBool(ParamID::IMAGES_ENABLED, "Images Filter On", false);
    addFloat(ParamID::IMAGES_SHIFT, "Images Shift", 0.0f, 1.0f, 0.001f, 1.0f, 0.0f);

    addChoice(ParamID::FILTER_TYPE, "Filter Type",
        { "Off","Low Pass","Band Pass","High Pass","Band Reject" }, 0);
    addChoice(ParamID::FILTER_ORDER, "Filter Order", { "Post","Pre" }, 0);
    addFloat(ParamID::FILTER_CUTOFF, "Filter Cutoff", 20.0f, 20000.0f, 1.0f, 0.25f, 2000.0f);
    addFloat(ParamID::FILTER_RESONANCE, "Filter Resonance", 0.0f, 1.0f, 0.001f, 1.0f, 0.0f);

    addBool(ParamID::WAVE_CRUSH_ENABLED, "WaveCrusher On", false);
    addFloat(ParamID::WAVE_CRUSH_AMOUNT, "WaveCrush Amount", 0.0f, 1.0f, 0.001f, 1.0f, 0.0f);
    addChoice(ParamID::WAVE_CRUSH_MODE, "WaveCrush Mode", { "Fold","Wrap","Tanh" }, 0);

    addBool(ParamID::RING_MOD_ENABLED, "Ring Mod On", false);
    addFloat(ParamID::RING_MOD_FREQ, "Ring Mod Freq", 0.1f, 1000.0f, 0.01f, 0.3f, 440.0f);
    addFloat(ParamID::RING_MOD_MIX, "Ring Mod Mix", 0.0f, 1.0f, 0.001f, 1.0f, 1.0f);

    addBool(ParamID::NOISE_ENABLED, "Noise On", false);
    addFloat(ParamID::NOISE_AMOUNT, "Noise Amount", 0.0f, 1.0f, 0.001f, 0.5f, 0.0f);
    addChoice(ParamID::NOISE_TYPE, "Noise Type", { "White","Pink","Vinyl","Bitcrush" }, 0);
    addFloat(ParamID::NOISE_COLOUR, "Noise Colour", 20.0f, 20000.0f, 1.0f, 0.25f, 8000.0f);

    addFloat(ParamID::LFO_RATE, "LFO Rate", 0.01f, 20.0f, 0.01f, 0.3f, 1.0f);
    addFloat(ParamID::LFO_DEPTH, "LFO Depth", 0.0f, 1.0f, 0.001f, 1.0f, 0.0f);
    addChoice(ParamID::LFO_WAVEFORM, "LFO Waveform",
        { "Sine","Triangle","Square","Saw Up","Saw Down","Random" }, 0);
    addChoice(ParamID::LFO_TARGET, "LFO Target",
        { "Bit Depth","Sample Rate","WaveCrush","Ring Freq" }, 0);

    addBool(ParamID::STEP_SEQ_ENABLED, "Step Seq On", false);
    addFloat(ParamID::STEP_SEQ_RATE, "Step Seq Rate", 0.25f, 8.0f, 0.25f, 1.0f, 1.0f);
    addFloat(ParamID::STEP_SEQ_DEPTH, "Step Seq Depth", 0.0f, 1.0f, 0.001f, 1.0f, 0.5f);
    addChoice(ParamID::STEP_SEQ_TARGET, "Step Seq Target",
        { "Bit Depth","Sample Rate","WaveCrush","Ring Freq" }, 1);
    addFloat(ParamID::STEP_SEQ_SWING, "Step Seq Swing", 0.0f, 0.5f, 0.01f, 1.0f, 0.0f);
    addChoice(ParamID::STEP_SEQ_DIR, "Step Seq Dir",
        { "Forward","Reverse","Ping-Pong","Random" }, 0);
    addFloat(ParamID::STEP_SEQ_LENGTH, "Step Seq Length", 1.0f, 16.0f, 1.0f, 1.0f, 16.0f);
    addChoice(ParamID::STEP_SEQ_BANK, "Pattern Bank", { "1","2","3","4" }, 0);

    // 4 banks x 16 steps
    const char* prefixes[StepSequencer::NUM_BANKS] = {
        ParamID::STEP_PREFIX_B0, ParamID::STEP_PREFIX_B1,
        ParamID::STEP_PREFIX_B2, ParamID::STEP_PREFIX_B3
    };
    for (int bank = 0; bank < StepSequencer::NUM_BANKS; ++bank)
        for (int i = 0; i < StepSequencer::NUM_STEPS; ++i)
        {
            auto id = juce::String(prefixes[bank]) + juce::String(i);
            auto name = "Bank " + juce::String(bank + 1) + " Step " + juce::String(i + 1);
            p.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID(id.toStdString().c_str(), 1), name,
                juce::NormalisableRange<float>(-1.0f, 1.0f, 0.001f),
                (bank == 0 && i % 4 == 0) ? 0.8f : 0.0f));
        }

    addFloat(ParamID::FX_MIX, "FX Mix", 0.0f, 1.0f, 0.001f, 1.0f, 1.0f);
    addFloat(ParamID::OUTPUT_VOLUME, "Output Volume", -96.0f, 12.0f, 0.1f, 3.0f, 0.0f);

    return { p.begin(), p.end() };
}

// =============================================================================
//  Constructor / Destructor
// =============================================================================
BitMorphAudioProcessor::BitMorphAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    apvts(*this, nullptr, "BitMorphParams", createParameterLayout())
{
}

BitMorphAudioProcessor::~BitMorphAudioProcessor() {}

// =============================================================================
//  Prepare / Release
// =============================================================================
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

// =============================================================================
//  Parameter snapshot
// =============================================================================
void BitMorphAudioProcessor::snapshotParameters()
{
    float sr = static_cast<float> (getSampleRate());

    auto getF = [&](const char* id) { return apvts.getRawParameterValue(id)->load(); };
    auto getB = [&](const char* id) { return apvts.getRawParameterValue(id)->load() > 0.5f; };
    auto getI = [&](const char* id) { return static_cast<int> (apvts.getRawParameterValue(id)->load()); };

    params.preampGain = juce::Decibels::decibelsToGain(getF(ParamID::PREAMP_GAIN), -96.0f);

    params.driveEnabled = getB(ParamID::DRIVE_ENABLED);
    params.driveAmount = getF(ParamID::DRIVE_AMOUNT);
    params.driveBias = getF(ParamID::DRIVE_BIAS);
    params.driveMode = getI(ParamID::DRIVE_MODE);
    params.driveMix = getF(ParamID::DRIVE_MIX);

    params.bitDepth = getF(ParamID::BIT_DEPTH);
    params.bitDepthEnabled = getB(ParamID::BIT_DEPTH_ENABLED);
    params.dithering = getF(ParamID::DITHERING);
    params.dcShift = getB(ParamID::DC_SHIFT);

    params.resampleFreq = getF(ParamID::RESAMPLE_FREQ);
    params.resampleEnabled = getB(ParamID::RESAMPLE_ENABLED);
    params.jitter = getF(ParamID::JITTER);
    params.approxEnabled = getB(ParamID::APPROX_ENABLED);
    params.approxDeviation = getF(ParamID::APPROX_DEVIATION);
    params.imagesEnabled = getB(ParamID::IMAGES_ENABLED);
    params.imagesShift = getF(ParamID::IMAGES_SHIFT);

    params.filterType = getI(ParamID::FILTER_TYPE);
    params.filterPre = getB(ParamID::FILTER_ORDER);
    params.filterCutoff = getF(ParamID::FILTER_CUTOFF);
    params.filterResonance = getF(ParamID::FILTER_RESONANCE);

    params.waveCrushEnabled = getB(ParamID::WAVE_CRUSH_ENABLED);
    params.waveCrushAmount = getF(ParamID::WAVE_CRUSH_AMOUNT);
    params.waveCrushMode = getI(ParamID::WAVE_CRUSH_MODE);

    params.ringModEnabled = getB(ParamID::RING_MOD_ENABLED);
    params.ringModFreq = getF(ParamID::RING_MOD_FREQ);
    params.ringModMix = getF(ParamID::RING_MOD_MIX);

    params.noiseEnabled = getB(ParamID::NOISE_ENABLED);
    params.noiseAmount = getF(ParamID::NOISE_AMOUNT);
    params.noiseType = getI(ParamID::NOISE_TYPE);
    params.noiseColour = getF(ParamID::NOISE_COLOUR);

    params.lfoRate = getF(ParamID::LFO_RATE);
    params.lfoDepth = getF(ParamID::LFO_DEPTH);
    params.lfoWaveform = getI(ParamID::LFO_WAVEFORM);
    params.lfoTarget = getI(ParamID::LFO_TARGET);

    params.stepSeqEnabled = getB(ParamID::STEP_SEQ_ENABLED);
    params.stepSeqRate = getF(ParamID::STEP_SEQ_RATE);
    params.stepSeqDepth = getF(ParamID::STEP_SEQ_DEPTH);
    params.stepSeqTarget = getI(ParamID::STEP_SEQ_TARGET);
    params.stepSeqSwing = getF(ParamID::STEP_SEQ_SWING);
    params.stepSeqDir = getI(ParamID::STEP_SEQ_DIR);
    params.stepSeqLength = static_cast<int> (std::round(getF(ParamID::STEP_SEQ_LENGTH)));
    params.stepSeqBank = getI(ParamID::STEP_SEQ_BANK);

    params.fxMix = getF(ParamID::FX_MIX);
    params.outputVolume = juce::Decibels::decibelsToGain(getF(ParamID::OUTPUT_VOLUME), -96.0f);

    // Load all 4 banks into step sequencer
    const char* prefixes[StepSequencer::NUM_BANKS] = {
        ParamID::STEP_PREFIX_B0, ParamID::STEP_PREFIX_B1,
        ParamID::STEP_PREFIX_B2, ParamID::STEP_PREFIX_B3
    };
    for (int bank = 0; bank < StepSequencer::NUM_BANKS; ++bank)
        for (int i = 0; i < StepSequencer::NUM_STEPS; ++i)
        {
            auto id = juce::String(prefixes[bank]) + juce::String(i);
            stepSeq.setStep(bank, i, apvts.getRawParameterValue(id.toStdString().c_str())->load());
        }

    // Apply to DSP objects
    processorL.drive.setEnabled(params.driveEnabled);
    processorR.drive.setEnabled(params.driveEnabled);
    processorL.drive.setAmount(params.driveAmount);
    processorR.drive.setAmount(params.driveAmount);
    processorL.drive.setBias(params.driveBias);
    processorR.drive.setBias(params.driveBias);
    processorL.drive.setMode(static_cast<DriveProcessor::Mode> (params.driveMode));
    processorR.drive.setMode(static_cast<DriveProcessor::Mode> (params.driveMode));
    processorL.drive.setMix(params.driveMix);
    processorR.drive.setMix(params.driveMix);

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
    processorL.resampler.setJitter(params.jitter);
    processorR.resampler.setJitter(params.jitter);

    processorL.updateFilterCutoffs(params.resampleFreq, params.approxDeviation,
        params.imagesShift, params.approxEnabled, params.imagesEnabled);
    processorR.updateFilterCutoffs(params.resampleFreq, params.approxDeviation,
        params.imagesShift, params.approxEnabled, params.imagesEnabled);

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

    processorL.noise.setEnabled(params.noiseEnabled);
    processorR.noise.setEnabled(params.noiseEnabled);
    processorL.noise.setAmount(params.noiseAmount);
    processorR.noise.setAmount(params.noiseAmount);
    processorL.noise.setType(static_cast<NoiseProcessor::Type> (params.noiseType));
    processorR.noise.setType(static_cast<NoiseProcessor::Type> (params.noiseType));
    processorL.noise.setColour(params.noiseColour);
    processorR.noise.setColour(params.noiseColour);

    lfo.setRate(params.lfoRate);
    lfo.setDepth(params.lfoDepth);
    lfo.setWaveform(static_cast<LFO::Waveform> (params.lfoWaveform));
    lfo.setTarget(static_cast<LFO::Target>   (params.lfoTarget));

    stepSeq.setEnabled(params.stepSeqEnabled);
    stepSeq.setDepth(params.stepSeqDepth);
    stepSeq.setTarget(static_cast<StepSequencer::Target>    (params.stepSeqTarget));
    stepSeq.setDirection(static_cast<StepSequencer::Direction> (params.stepSeqDir));
    stepSeq.setLength(params.stepSeqLength);
    stepSeq.setSwing(params.stepSeqSwing);
    stepSeq.setActiveBank(params.stepSeqBank);
    stepSeq.setTempo(static_cast<float> (currentBPM), params.stepSeqRate);
}

// =============================================================================
//  Modulation routing
// =============================================================================
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

// =============================================================================
//  Process block
// =============================================================================
void BitMorphAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
            if (auto bpm = pos->getBpm())
                currentBPM = *bpm;

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
        float lfoOut = lfo.tick();
        float stepSeqOut = stepSeq.tick();

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
            modBitDepth, fType, params.filterPre,
            params.fxMix, params.outputVolume, 0);

        dataR[i] = processorR.process(dataR[i], params.preampGain,
            params.approxEnabled, params.imagesEnabled,
            modBitDepth, fType, params.filterPre,
            params.fxMix, params.outputVolume, 1);
    }
}

// =============================================================================
//  State save / restore
// =============================================================================
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

// =============================================================================
//  Editor + plugin entry point
// =============================================================================
juce::AudioProcessorEditor* BitMorphAudioProcessor::createEditor()
{
    return new BitMorphAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BitMorphAudioProcessor();
}