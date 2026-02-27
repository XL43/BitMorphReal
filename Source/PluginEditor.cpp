#include "PluginEditor.h"

BitMorphAudioProcessorEditor::BitMorphAudioProcessorEditor(BitMorphAudioProcessor& p)
    : AudioProcessorEditor(&p),
    audioProcessor(p),
    genericEditor(p)
{
    addAndMakeVisible(genericEditor);
    setSize(400, 800);
}

BitMorphAudioProcessorEditor::~BitMorphAudioProcessorEditor() {}

void BitMorphAudioProcessorEditor::paint(juce::Graphics&) {}

void BitMorphAudioProcessorEditor::resized()
{
    genericEditor.setBounds(getLocalBounds());
}