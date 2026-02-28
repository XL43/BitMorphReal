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

// ── Step Sequencer Grid - no timer, purely interactive ────────────────────────
class StepSequencerGrid : public juce::Component
{
public:
    StepSequencerGrid(juce::AudioProcessorValueTreeState& a) : apvts(a) {}
    ~StepSequencerGrid() override {}

    void paint(juce::Graphics& g) override
    {
        auto        bounds = getLocalBounds().toFloat();
        const int   numSteps = 16;
        const float stepW = bounds.getWidth() / numSteps;
        const float midY = bounds.getCentreY();
        const float maxH = bounds.getHeight() * 0.5f - 4.0f;

        g.setColour(juce::Colour(0xff0d0d1a));
        g.fillRoundedRectangle(bounds, 3.0f);

        g.setColour(juce::Colour(0xff2a2a45));
        g.fillRect(bounds.getX(), midY - 0.5f, bounds.getWidth(), 1.0f);

        for (int i = 0; i < numSteps; ++i)
        {
            auto  id = juce::String("stepSeqStep") + juce::String(i);
            auto* raw = apvts.getRawParameterValue(id.toStdString().c_str());
            float val = (raw != nullptr) ? raw->load() : 0.0f;

            float barH = std::abs(val) * maxH;
            float barX = bounds.getX() + i * stepW + 1.0f;
            float barW = stepW - 2.0f;
            float barY = (val >= 0.0f) ? midY - barH : midY;

            g.setColour(juce::Colour(0xff1a1a30));
            g.fillRect(barX, bounds.getY() + 1.0f, barW, bounds.getHeight() - 2.0f);

            if (std::abs(val) > 0.01f)
            {
                g.setColour(val >= 0.0f ? juce::Colour(0xffe94560)
                    : juce::Colour(0xff8b1a2e));
                g.fillRect(barX + 1.0f, barY, barW - 2.0f, barH);
            }

            // Current step highlight
            if (i == currentStep)
            {
                g.setColour(juce::Colours::white.withAlpha(0.15f));
                g.fillRect(barX, bounds.getY() + 1.0f, barW, bounds.getHeight() - 2.0f);
                g.setColour(juce::Colours::white.withAlpha(0.6f));
                g.fillRect(barX, bounds.getY() + 1.0f, barW, 2.0f);
            }

            g.setColour(juce::Colour(0xff444466));
            g.setFont(8.0f);
            g.drawText(juce::String(i + 1),
                (int)barX, (int)(bounds.getBottom() - 12),
                (int)barW, 12, juce::Justification::centred);
        }

        g.setColour(juce::Colour(0xff2a2a45));
        g.drawRoundedRectangle(bounds.reduced(0.5f), 3.0f, 1.0f);
    }

    void mouseDown(const juce::MouseEvent& e) override { setStepFromMouse(e); }
    void mouseDrag(const juce::MouseEvent& e) override { setStepFromMouse(e); }

    void setCurrentStep(int step)
    {
        if (step != currentStep)
        {
            currentStep = step;
            repaint();
        }
    }

private:
    void setStepFromMouse(const juce::MouseEvent& e)
    {
        const int   numSteps = 16;
        const float stepW = getWidth() / (float)numSteps;
        int idx = juce::jlimit(0, numSteps - 1, (int)(e.position.x / stepW));

        float val = 1.0f - (e.position.y / getHeight()) * 2.0f;
        val = juce::jlimit(-1.0f, 1.0f, val);

        auto id = juce::String("stepSeqStep") + juce::String(idx);
        if (auto* param = apvts.getParameter(id))
        {
            param->beginChangeGesture();
            param->setValueNotifyingHost((val + 1.0f) * 0.5f);
            param->endChangeGesture();
        }
        repaint();
    }

    juce::AudioProcessorValueTreeState& apvts;
    int currentStep = -1;
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

class BitMorphAudioProcessorEditor : public juce::AudioProcessorEditor,
    public juce::Timer
{
public:
    BitMorphAudioProcessorEditor(BitMorphAudioProcessor&);
    ~BitMorphAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

    void parentHierarchyChanged() override
    {
        if (isShowing())
            startTimerHz(30);
        else
            stopTimer();
    }

    void timerCallback() override
    {
        if (stepSeqGrid != nullptr)
            stepSeqGrid->setCurrentStep(audioProcessor.getCurrentStepSeqStep());
    }

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

    // ── Preset bar ────────────────────────────────────────────────────────────────
    juce::TextButton presetPrevBtn{ "<" };
    juce::TextButton presetNextBtn{ ">" };
    juce::TextButton presetSaveBtn{ "Save" };
    juce::TextButton presetLoadBtn{ "Load" };
    juce::TextButton presetRandBtn{ "?" };
    juce::TextButton presetRandParamBtn{ "~" };
    juce::TextButton presetNameBtn;
    int              currentPresetIndex = -1;

    struct PresetEntry
    {
        juce::String name;
        juce::String folderName; // empty = root
        juce::File   file;
    };

    juce::Array<PresetEntry> allPresets;

    juce::File getPresetsFolder() const
    {
        return juce::File("C:\\Users\\lickm\\OneDrive\\Documents\\JUCE Projects\\BitMorph\\Presets");
    }

    void refreshPresetList()
    {
        allPresets.clear();
        auto root = getPresetsFolder();
        if (!root.exists()) return;

        // Root level presets
        for (auto& f : root.findChildFiles(juce::File::findFiles, false, "*.xml"))
            allPresets.add({ f.getFileNameWithoutExtension(), "", f });

        // One level of subfolders
        for (auto& dir : root.findChildFiles(juce::File::findDirectories, false, "*"))
        {
            for (auto& f : dir.findChildFiles(juce::File::findFiles, false, "*.xml"))
                allPresets.add({ f.getFileNameWithoutExtension(), dir.getFileName(), f });
        }

        updatePresetLabel();
    }

    void updatePresetLabel()
    {
        if (allPresets.isEmpty())
            presetNameBtn.setButtonText("-- No Presets --");
        else if (currentPresetIndex >= 0 && currentPresetIndex < allPresets.size())
        {
            auto& e = allPresets[currentPresetIndex];
            juce::String display = e.folderName.isEmpty()
                ? e.name
                : e.folderName + " / " + e.name;
            presetNameBtn.setButtonText(display);
        }
        else
            presetNameBtn.setButtonText("-- Init --");
    }

    void loadPresetByIndex(int index)
    {
        if (index < 0 || index >= allPresets.size()) return;
        auto& entry = allPresets[index];
        if (!entry.file.existsAsFile()) return;
        juce::MemoryBlock data;
        entry.file.loadFileAsData(data);
        audioProcessor.setStateInformation(data.getData(), (int)data.getSize());
        currentPresetIndex = index;
        updatePresetLabel();
        if (stepSeqGrid != nullptr)
            stepSeqGrid->repaint();
    }

    void savePreset()
    {
        auto folder = getPresetsFolder();
        folder.createDirectory();

        auto chooser = std::make_shared<juce::FileChooser>(
            "Save Preset", folder, "*.xml");

        chooser->launchAsync(juce::FileBrowserComponent::saveMode |
            juce::FileBrowserComponent::canSelectFiles,
            [this, chooser](const juce::FileChooser& fc)
            {
                auto result = fc.getResult();
                if (result == juce::File{}) return;
                auto file = result.withFileExtension("xml");
                juce::MemoryBlock data;
                audioProcessor.getStateInformation(data);
                file.replaceWithData(data.getData(), data.getSize());
                refreshPresetList();
                for (int i = 0; i < allPresets.size(); ++i)
                {
                    if (allPresets[i].file == file)
                    {
                        currentPresetIndex = i;
                        break;
                    }
                }
                updatePresetLabel();
            });
    }

    void randomPreset()
    {
        if (allPresets.isEmpty()) return;
        juce::Random rng;
        loadPresetByIndex(rng.nextInt(allPresets.size()));
    }

    void randomizeParameters()
    {
        juce::Random rng;
        for (auto* param : audioProcessor.apvts.processor.getParameters())
        {
            param->beginChangeGesture();
            param->setValueNotifyingHost(rng.nextFloat());
            param->endChangeGesture();
        }
        currentPresetIndex = -1;
        presetNameBtn.setButtonText("-- Randomized --");
        if (stepSeqGrid != nullptr)
            stepSeqGrid->repaint();
    }

    void showPresetMenu()
    {
        if (allPresets.isEmpty()) return;

        juce::PopupMenu menu;

        // Collect unique folder names preserving order
        juce::StringArray folders;
        folders.add(""); // root first
        for (auto& e : allPresets)
            if (e.folderName.isNotEmpty() && !folders.contains(e.folderName))
                folders.add(e.folderName);

        int itemID = 1;

        for (auto& folder : folders)
        {
            if (folder.isEmpty())
            {
                // Root presets go directly into menu
                for (int i = 0; i < allPresets.size(); ++i)
                {
                    if (allPresets[i].folderName.isEmpty())
                    {
                        bool isCurrent = (i == currentPresetIndex);
                        menu.addItem(itemID + i, allPresets[i].name, true, isCurrent);
                    }
                }
            }
            else
            {
                juce::PopupMenu sub;
                for (int i = 0; i < allPresets.size(); ++i)
                {
                    if (allPresets[i].folderName == folder)
                    {
                        bool isCurrent = (i == currentPresetIndex);
                        sub.addItem(itemID + i, allPresets[i].name, true, isCurrent);
                    }
                }
                menu.addSubMenu(folder, sub);
            }
        }

        menu.showMenuAsync(juce::PopupMenu::Options()
            .withTargetComponent(presetNameBtn),
            [this](int result)
            {
                if (result <= 0) return;
                loadPresetByIndex(result - 1);
            });
    }

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
    std::unique_ptr<StepSequencerGrid> stepSeqGrid;

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