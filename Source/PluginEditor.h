#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

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

class SectionPanel : public juce::Component
{
public:
    SectionPanel(const juce::String& title);
    void paint(juce::Graphics&) override;
private:
    juce::String sectionTitle;
};

// ── VU Meter ──────────────────────────────────────────────────────────────────
class VUMeter : public juce::Component, public juce::Timer
{
public:

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        float w = bounds.getWidth();
        float h = bounds.getHeight();
        float barW = (w - 6.0f) / 2.0f;

        // Background
        g.setColour(juce::Colour(0xff0d0d1a));
        g.fillRoundedRectangle(bounds, 3.0f);

        // Draw L and R bars
        drawBar(g, levelL, bounds.getX() + 2, 2.0f, barW, h - 4.0f);
        drawBar(g, levelR, bounds.getX() + barW + 4.0f, 2.0f, barW, h - 4.0f);

        // Labels
        g.setColour(juce::Colour(0xff888899));
        g.setFont(8.0f);
        g.drawText("L", (int)bounds.getX(), (int)(h - 12), (int)barW, 12,
            juce::Justification::centred);
        g.drawText("R", (int)(bounds.getX() + barW + 4), (int)(h - 12), (int)barW, 12,
            juce::Justification::centred);

        // Border
        g.setColour(juce::Colour(0xff2a2a45));
        g.drawRoundedRectangle(bounds.reduced(0.5f), 3.0f, 1.0f);
    }

private:
    void drawBar(juce::Graphics& g, float level,
        float x, float y, float w, float h)
    {
        // Background track
        g.setColour(juce::Colour(0xff12121f));
        g.fillRect(x, y, w, h - 12.0f);

        float filled = juce::jlimit(0.0f, 1.0f, level);
        float barH = filled * (h - 12.0f);
        float barY = y + (h - 12.0f) - barH;

        // Colour zones: green → yellow → red
        if (filled > 0.0f)
        {
            if (filled < 0.7f)
            {
                g.setColour(juce::Colour(0xff2ecc71));
                g.fillRect(x, barY, w, barH);
            }
            else if (filled < 0.9f)
            {
                float greenH = 0.7f * (h - 12.0f);
                float yellH = barH - greenH;
                g.setColour(juce::Colour(0xff2ecc71));
                g.fillRect(x, y + (h - 12.0f) - greenH, w, greenH);
                g.setColour(juce::Colour(0xfff39c12));
                g.fillRect(x, barY, w, yellH);
            }
            else
            {
                float greenH = 0.7f * (h - 12.0f);
                float yellH = 0.2f * (h - 12.0f);
                float redH = barH - greenH - yellH;
                g.setColour(juce::Colour(0xff2ecc71));
                g.fillRect(x, y + (h - 12.0f) - greenH, w, greenH);
                g.setColour(juce::Colour(0xfff39c12));
                g.fillRect(x, y + (h - 12.0f) - greenH - yellH, w, yellH);
                g.setColour(juce::Colour(0xffe94560));
                g.fillRect(x, barY, w, redH);
            }
        }
    }

    BitMorphAudioProcessor& processor;
    float levelL = 0.0f;
    float levelR = 0.0f;
};

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

    SectionPanel quantizerPanel{ "QUANTIZER" };
    SectionPanel resamplerPanel{ "RESAMPLER" };
    SectionPanel filterPanel{ "FILTER" };
    SectionPanel waveCrushPanel{ "WAVECRUSHER" };
    SectionPanel ringModPanel{ "RING MOD" };
    SectionPanel lfoPanel{ "LFO" };
    SectionPanel stepSeqPanel{ "STEP SEQ" };
    SectionPanel masterPanel{ "MASTER" };

    KnobSet            bitDepthKnob;
    KnobSet            ditheringKnob;
    juce::ToggleButton bitDepthOnBtn{ "Bit Depth On" };
    juce::ToggleButton dcShiftBtn{ "DC Shift" };

    KnobSet            resampleFreqKnob;
    KnobSet            approxDeviationKnob;
    KnobSet            imagesShiftKnob;
    juce::ToggleButton resampleOnBtn{ "Resample On" };
    juce::ToggleButton approxOnBtn{ "Approx Filter" };
    juce::ToggleButton imagesOnBtn{ "Images Filter" };

    KnobSet            filterCutoffKnob;
    KnobSet            filterResonanceKnob;
    juce::ComboBox     filterTypeCombo;
    juce::ComboBox     filterOrderCombo;
    juce::Label        filterTypeLabel{ {}, "Type" };
    juce::Label        filterOrderLabel{ {}, "Order" };

    KnobSet            waveCrushAmountKnob;
    juce::ToggleButton waveCrushOnBtn{ "WaveCrusher On" };
    juce::ComboBox     waveCrushModeCombo;
    juce::Label        waveCrushModeLabel{ {}, "Mode" };

    KnobSet            ringModFreqKnob;
    KnobSet            ringModMixKnob;
    juce::ToggleButton ringModOnBtn{ "Ring Mod On" };

    KnobSet            lfoRateKnob;
    KnobSet            lfoDepthKnob;
    juce::ComboBox     lfoWaveformCombo;
    juce::ComboBox     lfoTargetCombo;
    juce::Label        lfoWaveformLabel{ {}, "Waveform" };
    juce::Label        lfoTargetLabel{ {}, "Target" };

    KnobSet            stepSeqRateKnob;
    KnobSet            stepSeqDepthKnob;
    juce::ToggleButton stepSeqOnBtn{ "Step Seq On" };
    juce::ComboBox     stepSeqTargetCombo;
    juce::Label        stepSeqTargetLabel{ {}, "Target" };

    KnobSet            preampKnob;
    KnobSet            fxMixKnob;
    KnobSet            outputVolumeKnob;

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