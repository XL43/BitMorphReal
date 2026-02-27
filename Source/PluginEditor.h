#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

// ── Custom LookAndFeel ────────────────────────────────────────────────────────
class BitMorphLookAndFeel : public juce::LookAndFeel_V4
{
public:
    BitMorphLookAndFeel();

    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
        float sliderPos, float startAngle, float endAngle,
        juce::Slider&) override;

    void drawToggleButton(juce::Graphics&, juce::ToggleButton&,
        bool highlighted, bool down) override;
};

// ── Section panel ─────────────────────────────────────────────────────────────
class SectionPanel : public juce::Component
{
public:
    SectionPanel(const juce::String& title);
    void paint(juce::Graphics&) override;
private:
    juce::String sectionTitle;
};

// ── Main editor ───────────────────────────────────────────────────────────────
class BitMorphAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    BitMorphAudioProcessorEditor(BitMorphAudioProcessor&);
    ~BitMorphAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    BitMorphAudioProcessor& audioProcessor;
    BitMorphLookAndFeel     lookAndFeel;

    // ── Knob + label pair ─────────────────────────────────────────────────────
    struct KnobSet
    {
        juce::Slider knob{ juce::Slider::RotaryVerticalDrag,
                            juce::Slider::TextBoxBelow };
        juce::Label  label;
    };

    void setupKnob(KnobSet&, const juce::String& labelText);
    void placeKnob(KnobSet&, juce::Rectangle<int> bounds);
    void setupCombo(juce::ComboBox&, juce::Label&,
        const juce::String& labelText,
        const juce::StringArray& items);

    // ── Section panels ────────────────────────────────────────────────────────
    SectionPanel quantizerPanel{ "QUANTIZER" };
    SectionPanel resamplerPanel{ "RESAMPLER" };
    SectionPanel filterPanel{ "FILTER" };
    SectionPanel waveCrushPanel{ "WAVECRUSHER" };
    SectionPanel ringModPanel{ "RING MOD" };
    SectionPanel lfoPanel{ "LFO" };
    SectionPanel stepSeqPanel{ "STEP SEQ" };
    SectionPanel masterPanel{ "MASTER" };

    // ── Quantizer ─────────────────────────────────────────────────────────────
    KnobSet            bitDepthKnob;
    KnobSet            ditheringKnob;
    juce::ToggleButton bitDepthOnBtn{ "Bit Depth On" };
    juce::ToggleButton dcShiftBtn{ "DC Shift" };

    // ── Resampler ─────────────────────────────────────────────────────────────
    KnobSet            resampleFreqKnob;
    KnobSet            approxDeviationKnob;
    KnobSet            imagesShiftKnob;
    juce::ToggleButton resampleOnBtn{ "Resample On" };
    juce::ToggleButton approxOnBtn{ "Approx Filter" };
    juce::ToggleButton imagesOnBtn{ "Images Filter" };

    // ── Filter ────────────────────────────────────────────────────────────────
    KnobSet            filterCutoffKnob;
    KnobSet            filterResonanceKnob;
    juce::ComboBox     filterTypeCombo;
    juce::ComboBox     filterOrderCombo;
    juce::Label        filterTypeLabel{ {}, "Type" };
    juce::Label        filterOrderLabel{ {}, "Order" };

    // ── WaveCrusher ───────────────────────────────────────────────────────────
    KnobSet            waveCrushAmountKnob;
    juce::ToggleButton waveCrushOnBtn{ "WaveCrusher On" };
    juce::ComboBox     waveCrushModeCombo;
    juce::Label        waveCrushModeLabel{ {}, "Mode" };

    // ── Ring Mod ──────────────────────────────────────────────────────────────
    KnobSet            ringModFreqKnob;
    KnobSet            ringModMixKnob;
    juce::ToggleButton ringModOnBtn{ "Ring Mod On" };

    // ── LFO ───────────────────────────────────────────────────────────────────
    KnobSet            lfoRateKnob;
    KnobSet            lfoDepthKnob;
    juce::ComboBox     lfoWaveformCombo;
    juce::ComboBox     lfoTargetCombo;
    juce::Label        lfoWaveformLabel{ {}, "Waveform" };
    juce::Label        lfoTargetLabel{ {}, "Target" };

    // ── Step Sequencer ────────────────────────────────────────────────────────
    KnobSet            stepSeqRateKnob;
    KnobSet            stepSeqDepthKnob;
    juce::ToggleButton stepSeqOnBtn{ "Step Seq On" };
    juce::ComboBox     stepSeqTargetCombo;
    juce::Label        stepSeqTargetLabel{ {}, "Target" };

    // ── Master ────────────────────────────────────────────────────────────────
    KnobSet            preampKnob;
    KnobSet            fxMixKnob;
    KnobSet            outputVolumeKnob;

    // ── Attachments ───────────────────────────────────────────────────────────
    using SliderAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAtt = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAtt = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<SliderAtt> bitDepthAtt, ditheringAtt;
    std::unique_ptr<SliderAtt> resampleFreqAtt, approxDeviationAtt, imagesShiftAtt;
    std::unique_ptr<SliderAtt> filterCutoffAtt, filterResonanceAtt;
    std::unique_ptr<SliderAtt> waveCrushAmountAtt;
    std::unique_ptr<SliderAtt> ringModFreqAtt, ringModMixAtt;
    std::unique_ptr<SliderAtt> lfoRateAtt, lfoDepthAtt;
    std::unique_ptr<SliderAtt> stepSeqRateAtt, stepSeqDepthAtt;
    std::unique_ptr<SliderAtt> preampAtt, fxMixAtt, outputVolumeAtt;

    std::unique_ptr<ButtonAtt> bitDepthOnAtt, dcShiftAtt;
    std::unique_ptr<ButtonAtt> resampleOnAtt, approxOnAtt, imagesOnAtt;
    std::unique_ptr<ButtonAtt> waveCrushOnAtt, ringModOnAtt, stepSeqOnAtt;

    std::unique_ptr<ComboAtt>  filterTypeAtt, filterOrderAtt;
    std::unique_ptr<ComboAtt>  waveCrushModeAtt;
    std::unique_ptr<ComboAtt>  lfoWaveformAtt, lfoTargetAtt;
    std::unique_ptr<ComboAtt>  stepSeqTargetAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BitMorphAudioProcessorEditor)
};