#pragma once
#include <JuceHeader.h>
#include "BitCrusherEngine.h"

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


    int getStepSequencerCurrentStep() const { return stepSeq.getCurrentStep(); }

    // For step sequencer highlight in UI
    int getCurrentStepSeqStep() const noexcept { return stepSeq.getCurrentStep(); }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    BitMorphProcessor processorL;
    BitMorphProcessor processorR;
    LFO           lfo;
    StepSequencer stepSeq;

    struct CachedParams
    {
        float preampGain = 1.0f;
        float bitDepth = 8.0f;
        bool  bitDepthEnabled = true;
        float dithering = 0.0f;
        bool  dcShift = false;
        float resampleFreq = 44100.0f;
        bool  resampleEnabled = true;
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
        float lfoRate = 1.0f;
        float lfoDepth = 0.0f;
        int   lfoWaveform = 0;
        int   lfoTarget = 0;
        bool  stepSeqEnabled = false;
        float stepSeqRate = 1.0f;
        float stepSeqDepth = 0.5f;
        int   stepSeqTarget = 1;
        float fxMix = 1.0f;
        float outputVolume = 1.0f;
    };

    CachedParams params;
    double currentBPM = 120.0;

    void snapshotParameters();
    void applyModulations(float lfoOut, float stepSeqOut,
        float& bitDepth, float& resampleFreq,
        float& waveCrushAmt, float& ringModFreq);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BitMorphAudioProcessor)
};