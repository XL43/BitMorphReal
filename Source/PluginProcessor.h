#pragma once

#include <JuceHeader.h>
#include "BitCrusherEngine.h"

// =============================================================================
//  Parameter ID strings
// =============================================================================
namespace ParamID
{
    // Master input
    inline constexpr const char* PREAMP_GAIN = "preampGain";

    // Drive
    inline constexpr const char* DRIVE_ENABLED = "driveEnabled";
    inline constexpr const char* DRIVE_AMOUNT = "driveAmount";
    inline constexpr const char* DRIVE_BIAS = "driveBias";
    inline constexpr const char* DRIVE_MODE = "driveMode";
    inline constexpr const char* DRIVE_MIX = "driveMix";

    // Quantizer
    inline constexpr const char* BIT_DEPTH = "bitDepth";
    inline constexpr const char* BIT_DEPTH_ENABLED = "bitDepthEnabled";
    inline constexpr const char* DITHERING = "dithering";
    inline constexpr const char* DC_SHIFT = "dcShift";

    // Resampler
    inline constexpr const char* RESAMPLE_FREQ = "resampleFreq";
    inline constexpr const char* RESAMPLE_ENABLED = "resampleEnabled";
    inline constexpr const char* JITTER = "jitter";
    inline constexpr const char* APPROX_ENABLED = "approxEnabled";
    inline constexpr const char* APPROX_DEVIATION = "approxDeviation";
    inline constexpr const char* IMAGES_ENABLED = "imagesEnabled";
    inline constexpr const char* IMAGES_SHIFT = "imagesShift";

    // Filter
    inline constexpr const char* FILTER_TYPE = "filterType";
    inline constexpr const char* FILTER_ORDER = "filterOrder";
    inline constexpr const char* FILTER_CUTOFF = "filterCutoff";
    inline constexpr const char* FILTER_RESONANCE = "filterResonance";

    // WaveCrusher
    inline constexpr const char* WAVE_CRUSH_ENABLED = "waveCrushEnabled";
    inline constexpr const char* WAVE_CRUSH_AMOUNT = "waveCrushAmount";
    inline constexpr const char* WAVE_CRUSH_MODE = "waveCrushMode";

    // Ring Modulator
    inline constexpr const char* RING_MOD_ENABLED = "ringModEnabled";
    inline constexpr const char* RING_MOD_FREQ = "ringModFreq";
    inline constexpr const char* RING_MOD_MIX = "ringModMix";

    // Noise
    inline constexpr const char* NOISE_ENABLED = "noiseEnabled";
    inline constexpr const char* NOISE_AMOUNT = "noiseAmount";
    inline constexpr const char* NOISE_TYPE = "noiseType";
    inline constexpr const char* NOISE_COLOUR = "noiseColour";

    // LFO
    inline constexpr const char* LFO_RATE = "lfoRate";
    inline constexpr const char* LFO_DEPTH = "lfoDepth";
    inline constexpr const char* LFO_WAVEFORM = "lfoWaveform";
    inline constexpr const char* LFO_TARGET = "lfoTarget";

    // Step Sequencer
    inline constexpr const char* STEP_SEQ_ENABLED = "stepSeqEnabled";
    inline constexpr const char* STEP_SEQ_RATE = "stepSeqRate";
    inline constexpr const char* STEP_SEQ_DEPTH = "stepSeqDepth";
    inline constexpr const char* STEP_SEQ_TARGET = "stepSeqTarget";
    inline constexpr const char* STEP_SEQ_SWING = "stepSeqSwing";
    inline constexpr const char* STEP_SEQ_DIR = "stepSeqDir";
    inline constexpr const char* STEP_SEQ_LENGTH = "stepSeqLength";
    inline constexpr const char* STEP_SEQ_BANK = "stepSeqBank";

    // Step value prefixes — one per bank
    // Bank 0: "stepSeqStep0"  .. "stepSeqStep15"
    // Bank 1: "stepSeqB1Step0" .. "stepSeqB1Step15"
    // Bank 2: "stepSeqB2Step0" .. "stepSeqB2Step15"
    // Bank 3: "stepSeqB3Step0" .. "stepSeqB3Step15"
    inline constexpr const char* STEP_PREFIX_B0 = "stepSeqStep";
    inline constexpr const char* STEP_PREFIX_B1 = "stepSeqB1Step";
    inline constexpr const char* STEP_PREFIX_B2 = "stepSeqB2Step";
    inline constexpr const char* STEP_PREFIX_B3 = "stepSeqB3Step";

    // Master
    inline constexpr const char* FX_MIX = "fxMix";
    inline constexpr const char* OUTPUT_VOLUME = "outputVolume";
}

// Returns the step-parameter prefix string for bank 0-3
inline juce::String stepPrefixForBank(int bank)
{
    switch (bank)
    {
    case 1:  return ParamID::STEP_PREFIX_B1;
    case 2:  return ParamID::STEP_PREFIX_B2;
    case 3:  return ParamID::STEP_PREFIX_B3;
    default: return ParamID::STEP_PREFIX_B0;
    }
}

// =============================================================================
//  BitMorphAudioProcessor
// =============================================================================
class BitMorphAudioProcessor : public juce::AudioProcessor
{
public:
    BitMorphAudioProcessor();
    ~BitMorphAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool   acceptsMidi()  const override { return false; }
    bool   producesMidi() const override { return false; }
    bool   isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int  getNumPrograms()    override { return 1; }
    int  getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    // GUI timer reads this to drive the step grid playhead
    int getCurrentStepSeqStep() const { return stepSeq.getCurrentStep(); }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    BitMorphProcessor processorL;
    BitMorphProcessor processorR;
    LFO               lfo;
    StepSequencer     stepSeq;

    struct CachedParams
    {
        float preampGain = 1.0f;

        bool  driveEnabled = false;
        float driveAmount = 0.0f;
        float driveBias = 0.0f;
        int   driveMode = 0;
        float driveMix = 1.0f;

        float bitDepth = 8.0f;
        bool  bitDepthEnabled = true;
        float dithering = 0.0f;
        bool  dcShift = false;

        float resampleFreq = 44100.0f;
        bool  resampleEnabled = true;
        float jitter = 0.0f;
        bool  approxEnabled = false;
        float approxDeviation = 0.0f;
        bool  imagesEnabled = false;
        float imagesShift = 0.0f;

        int   filterType = 0;
        bool  filterPre = false;
        float filterCutoff = 2000.0f;
        float filterResonance = 0.0f;

        bool  waveCrushEnabled = false;
        float waveCrushAmount = 0.0f;
        int   waveCrushMode = 0;

        bool  ringModEnabled = false;
        float ringModFreq = 440.0f;
        float ringModMix = 1.0f;

        bool  noiseEnabled = false;
        float noiseAmount = 0.0f;
        int   noiseType = 0;
        float noiseColour = 8000.0f;

        float lfoRate = 1.0f;
        float lfoDepth = 0.0f;
        int   lfoWaveform = 0;
        int   lfoTarget = 0;

        bool  stepSeqEnabled = false;
        float stepSeqRate = 1.0f;
        float stepSeqDepth = 0.5f;
        int   stepSeqTarget = 1;
        float stepSeqSwing = 0.0f;
        int   stepSeqDir = 0;
        int   stepSeqLength = 16;
        int   stepSeqBank = 0;

        float fxMix = 1.0f;
        float outputVolume = 1.0f;
    };

    CachedParams params;
    double currentBPM = 120.0;

    void snapshotParameters();
    void applyModulations(float lfoOut, float stepSeqOut,
        float& bitDepth, float& resampleFreq,
        float& waveCrushAmt, float& ringFreq);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BitMorphAudioProcessor)
};