#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class BitMorphAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    BitMorphAudioProcessorEditor(BitMorphAudioProcessor& p);
    ~BitMorphAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    BitMorphAudioProcessor& audioProcessor;
    juce::GenericAudioProcessorEditor genericEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BitMorphAudioProcessorEditor)
};