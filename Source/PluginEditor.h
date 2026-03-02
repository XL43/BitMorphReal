#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

// ── Colours (shared between .h and .cpp) ─────────────────────────────────────
static const juce::Colour BG_DARK(0xff0d0d1a);
static const juce::Colour BG_PANEL(0xff141428);
static const juce::Colour BG_HEADER(0xff1a1a35);
static const juce::Colour ACCENT(0xffe94560);
static const juce::Colour TEXT_LIGHT(0xffddddee);
static const juce::Colour TEXT_MUTED(0xffaabbcc);
static const juce::Colour KNOB_BODY(0xff252538);
static const juce::Colour BORDER_CLR(0xff2a2a45);

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
    void drawButtonText(juce::Graphics&, juce::TextButton&,
        bool highlighted, bool down) override;
};

// ── Section panel with optional LED toggle ────────────────────────────────────
class SectionPanel : public juce::Component
{
public:
    SectionPanel(const juce::String& title, bool hasToggle = false)
        : sectionTitle(title), showToggle(hasToggle)
    {
        if (showToggle)
        {
            toggleBtn.setClickingTogglesState(true);
            toggleBtn.setToggleState(true, juce::dontSendNotification);
            toggleBtn.onClick = [this] { repaint(); };
            addAndMakeVisible(toggleBtn);
        }
    }

    void paint(juce::Graphics& g) override
    {
        auto b = getLocalBounds();

        g.setColour(BG_PANEL);
        g.fillRoundedRectangle(b.toFloat(), 4.0f);

        g.setColour(BORDER_CLR);
        g.drawRoundedRectangle(b.toFloat().reduced(0.5f), 4.0f, 1.0f);

        auto header = b.removeFromTop(22);
        g.setColour(BG_HEADER);
        g.fillRect(header);

        bool enabled = !showToggle || toggleBtn.getToggleState();

        g.setColour(enabled ? ACCENT : juce::Colour(0xff333344));
        g.fillRect(header.getX(), header.getBottom() - 1, header.getWidth(), 1);

        // LED dot
        if (showToggle)
        {
            float dotX = (float)header.getX() + 8.0f;
            float dotY = (float)header.getCentreY() - 5.0f;
            if (enabled)
            {
                g.setColour(ACCENT.withAlpha(0.25f));
                g.fillEllipse(dotX - 3, dotY - 3, 16, 16);
                g.setColour(ACCENT);
            }
            else
            {
                g.setColour(juce::Colour(0xff2a2a3a));
            }
            g.fillEllipse(dotX, dotY, 10, 10);
            g.setColour(BORDER_CLR);
            g.drawEllipse(dotX, dotY, 10, 10, 1.0f);
        }

        g.setColour(TEXT_LIGHT);
        g.setFont(juce::Font(10.0f, juce::Font::bold));
        int textX = showToggle ? header.getX() + 24 : header.getX() + 6;
        g.drawText(sectionTitle,
            juce::Rectangle<int>(textX, header.getY(),
                header.getWidth() - textX - 4,
                header.getHeight()),
            juce::Justification::centredLeft);

        // Grey overlay when disabled
        if (showToggle && !toggleBtn.getToggleState())
        {
            g.setColour(juce::Colour(0xaa080810));
            g.fillRect(b);
        }
    }

    void resized() override
    {
        if (showToggle)
            toggleBtn.setBounds(0, 0, 22, 22);
    }

    juce::ToggleButton toggleBtn;

private:
    juce::String sectionTitle;
    bool         showToggle = false;
};

// ── Step Sequencer Grid ───────────────────────────────────────────────────────
class StepSequencerGrid : public juce::Component
{
public:
    StepSequencerGrid(juce::AudioProcessorValueTreeState& a)
        : apvts(a) {
    }

    ~StepSequencerGrid() override {}

    void setCurrentStep(int step)
    {
        if (step != currentStep)
        {
            currentStep = step;
            repaint();
        }
    }

    void paint(juce::Graphics& g) override
    {
        auto        bounds = getLocalBounds().toFloat();
        const int   numSteps = 16;
        const float stepW = bounds.getWidth() / numSteps;
        const float midY = bounds.getCentreY();
        const float maxH = bounds.getHeight() * 0.5f - 2.0f;

        g.setColour(juce::Colour(0xff0d0d1a));
        g.fillRoundedRectangle(bounds, 3.0f);

        g.setColour(juce::Colour(0xff2a2a45));
        g.fillRect(bounds.getX(), midY - 0.5f, bounds.getWidth(), 1.0f);

        for (int i = 0; i < numSteps; ++i)
        {
            auto  stepID = juce::String("stepSeqStep") + juce::String(i);
            auto* raw = apvts.getRawParameterValue(stepID);
            float val = (raw != nullptr) ? raw->load() : 0.0f;

            float barH = std::abs(val) * maxH;
            float barX = bounds.getX() + i * stepW + 1.0f;
            float barW = stepW - 2.0f;
            float barY = (val >= 0.0f) ? midY - barH : midY;

            bool isCurrent = (i == currentStep);

            g.setColour(isCurrent ? juce::Colour(0xff1e1e38)
                : juce::Colour(0xff12121f));
            g.fillRect(barX, bounds.getY() + 1.0f, barW, bounds.getHeight() - 2.0f);

            if (std::abs(val) > 0.01f)
            {
                juce::Colour barColour = isCurrent
                    ? juce::Colour(0xffff6680)
                    : (val >= 0.0f ? juce::Colour(0xffe94560)
                        : juce::Colour(0xff8b1a2e));
                g.setColour(barColour);
                g.fillRect(barX + 1.0f, barY, barW - 2.0f, barH);
            }

            // Current step highlight
            if (isCurrent)
            {
                g.setColour(juce::Colours::white.withAlpha(0.15f));
                g.fillRect(barX, bounds.getY() + 1.0f, barW, bounds.getHeight() - 2.0f);
                g.setColour(juce::Colours::white.withAlpha(0.6f));
                g.fillRect(barX, bounds.getY() + 1.0f, barW, 2.0f);
            }

            g.setColour(isCurrent ? juce::Colours::white
                : juce::Colour(0xff444466));
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

private:
    void setStepFromMouse(const juce::MouseEvent& e)
    {
        const int   numSteps = 16;
        const float stepW = getWidth() / (float)numSteps;
        int idx = juce::jlimit(0, numSteps - 1, (int)(e.position.x / stepW));

        float val = 1.0f - (e.position.y / getHeight()) * 2.0f;
        val = juce::jlimit(-1.0f, 1.0f, val);

        auto stepID = juce::String("stepSeqStep") + juce::String(idx);
        if (auto* param = apvts.getParameter(stepID))
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

// ── Main editor ───────────────────────────────────────────────────────────────
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
        if (isShowing()) startTimerHz(30);
        else             stopTimer();
    }

    void timerCallback() override
    {
        if (stepSeqGrid != nullptr)
            stepSeqGrid->setCurrentStep(audioProcessor.getCurrentStepSeqStep());
    }

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
    SectionPanel quantizerPanel{ "QUANTIZER",   true };
    SectionPanel resamplerPanel{ "RESAMPLER",   true };
    SectionPanel filterPanel{ "FILTER",      false };
    SectionPanel waveCrushPanel{ "WAVECRUSHER", true };
    SectionPanel ringModPanel{ "RING MOD",    true };
    SectionPanel lfoPanel{ "LFO",         false };
    SectionPanel stepSeqPanel{ "STEP SEQ",    true };
    SectionPanel masterPanel{ "MASTER",      false };

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
    KnobSet        filterCutoffKnob;
    KnobSet        filterResonanceKnob;
    juce::ComboBox filterTypeCombo;
    juce::ComboBox filterOrderCombo;
    juce::Label    filterTypeLabel{ {}, "Type" };
    juce::Label    filterOrderLabel{ {}, "Order" };

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
    KnobSet        lfoRateKnob;
    KnobSet        lfoDepthKnob;
    juce::ComboBox lfoWaveformCombo;
    juce::ComboBox lfoTargetCombo;
    juce::Label    lfoWaveformLabel{ {}, "Waveform" };
    juce::Label    lfoTargetLabel{ {}, "Target" };

    // ── Step Sequencer ────────────────────────────────────────────────────────
    KnobSet            stepSeqRateKnob;
    KnobSet            stepSeqDepthKnob;
    juce::ToggleButton stepSeqOnBtn{ "Step Seq On" };
    juce::ComboBox     stepSeqTargetCombo;
    juce::Label        stepSeqTargetLabel{ {}, "Target" };
    juce::TextButton   stepSeqRandBtn{ "DIE" };
    std::unique_ptr<StepSequencerGrid> stepSeqGrid;

    // ── Master ────────────────────────────────────────────────────────────────
    KnobSet preampKnob;
    KnobSet fxMixKnob;
    KnobSet outputVolumeKnob;

    // ── Preset system ─────────────────────────────────────────────────────────
    struct PresetEntry
    {
        juce::String name;
        juce::String folderName;
        juce::File   file;
    };

    juce::Array<PresetEntry> allPresets;
    int currentPresetIndex = -1;

    juce::TextButton presetPrevBtn{ "<" };
    juce::TextButton presetNextBtn{ ">" };
    juce::TextButton presetNameBtn{ "-- Init --" };
    juce::TextButton presetRandBtn{ "SHUFFLE" };
    juce::TextButton presetRandParamBtn{ "DIE" };
    juce::TextButton presetSaveBtn{ "Save" };
    juce::TextButton presetLoadBtn{ "Load" };
    juce::TextButton presetStarBtn{ "STAR_OFF" };

    juce::StringArray favorites;

    juce::File getFavoritesFile()
    {
        return getPresetsFolder().getChildFile("favorites.txt");
    }

    void loadFavorites()
    {
        favorites.clear();
        auto f = getFavoritesFile();
        if (f.existsAsFile())
            favorites.addTokens(f.loadFileAsString(), "\n", "");
        favorites.removeEmptyStrings();
    }

    void saveFavorites()
    {
        getFavoritesFile().replaceWithText(favorites.joinIntoString("\n"));
    }

    bool isFavorite(const juce::String& name)
    {
        return favorites.contains(name);
    }

    void toggleFavorite()
    {
        if (currentPresetIndex < 0 || currentPresetIndex >= allPresets.size()) return;
        auto name = allPresets[currentPresetIndex].name;
        if (isFavorite(name))
            favorites.removeString(name);
        else
            favorites.add(name);
        saveFavorites();
        updateStarBtn();
    }

    void updateStarBtn()
    {
        bool on = currentPresetIndex >= 0
            && currentPresetIndex < allPresets.size()
            && isFavorite(allPresets[currentPresetIndex].name);
        presetStarBtn.setButtonText(on ? "STAR_ON" : "STAR_OFF");
        presetStarBtn.repaint();
    }

    juce::File getPresetsFolder()
    {
        return juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
            .getChildFile("JUCE Projects/BitMorph/Presets");
    }

    void refreshPresetList()
    {
        allPresets.clear();
        auto root = getPresetsFolder();
        if (!root.isDirectory()) return;

        // Root presets
        for (auto& f : root.findChildFiles(juce::File::findFiles, false, "*.xml"))
            allPresets.add({ f.getFileNameWithoutExtension(), {}, f });

        // One level of subfolders
        for (auto& folder : root.findChildFiles(juce::File::findDirectories, false))
            for (auto& f : folder.findChildFiles(juce::File::findFiles, false, "*.xml"))
                allPresets.add({ f.getFileNameWithoutExtension(), folder.getFileName(), f });
    }

    void updatePresetLabel()
    {
        if (currentPresetIndex >= 0 && currentPresetIndex < allPresets.size())
            presetNameBtn.setButtonText(allPresets[currentPresetIndex].name);
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
        audioProcessor.apvts.state.setProperty("currentPresetName", entry.name, nullptr);
        updateStarBtn();
        if (stepSeqGrid != nullptr) stepSeqGrid->repaint();
    }

    void showPresetMenu()
    {
        juce::PopupMenu menu;
        menu.addItem(9000, "Initialize");
        menu.addSeparator();

        juce::PopupMenu currentSub;
        juce::String    currentFolder;
        int             itemID = 1;

        for (auto& p : allPresets)
        {
            if (p.folderName.isEmpty())
            {
                menu.addItem(itemID++, p.name);
            }
            else
            {
                if (p.folderName != currentFolder)
                {
                    if (currentFolder.isNotEmpty())
                        menu.addSubMenu(currentFolder, currentSub);
                    currentSub = juce::PopupMenu();
                    currentFolder = p.folderName;
                }
                currentSub.addItem(itemID++, p.name);
            }
        }
        if (currentFolder.isNotEmpty())
            menu.addSubMenu(currentFolder, currentSub);

        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(presetNameBtn),
            [this](int result)
            {
                if (result == 9000) initializePreset();
                else if (result > 0) loadPresetByIndex(result - 1);
            });
    }

    void savePreset()
    {
        auto chooser = std::make_shared<juce::FileChooser>(
            "Save Preset", getPresetsFolder(), "*.xml");
        chooser->launchAsync(juce::FileBrowserComponent::saveMode |
            juce::FileBrowserComponent::canSelectFiles,
            [this, chooser](const juce::FileChooser& fc)
            {
                auto result = fc.getResult();
                if (result == juce::File{}) return;
                auto file = result.withFileExtension(".xml");
                juce::MemoryBlock data;
                audioProcessor.getStateInformation(data);
                file.replaceWithData(data.getData(), data.getSize());
                refreshPresetList();
                for (int i = 0; i < allPresets.size(); ++i)
                    if (allPresets[i].file == file) { currentPresetIndex = i; break; }
                updatePresetLabel();
            });
    }

    void randomPreset()
    {
        if (allPresets.isEmpty()) return;
        int idx = juce::Random::getSystemRandom().nextInt(allPresets.size());
        loadPresetByIndex(idx);
    }

    void randomizeParameters()
    {
        juce::Random rng;
        auto& apvts = audioProcessor.apvts;

        // Randomize all parameters freely
        for (auto* param : apvts.processor.getParameters())
        {
            param->beginChangeGesture();
            param->setValueNotifyingHost(rng.nextFloat());
            param->endChangeGesture();
        }

        // Override preamp: 0.0 to 5.0 dB
        if (auto* preamp = dynamic_cast<juce::RangedAudioParameter*> (
            apvts.getParameter("preampGain")))
        {
            float norm = preamp->convertTo0to1(rng.nextFloat() * 5.0f);
            preamp->beginChangeGesture();
            preamp->setValueNotifyingHost(norm);
            preamp->endChangeGesture();
        }

        // Override output: -5.0 to 5.0 dB
        if (auto* output = dynamic_cast<juce::RangedAudioParameter*> (
            apvts.getParameter("outputVolume")))
        {
            float norm = output->convertTo0to1((rng.nextFloat() * 10.0f) - 5.0f);
            output->beginChangeGesture();
            output->setValueNotifyingHost(norm);
            output->endChangeGesture();
        }

        currentPresetIndex = -1;
        presetNameBtn.setButtonText("-- Randomized --");
        if (stepSeqGrid != nullptr) stepSeqGrid->repaint();
    }

    void initializePreset()
    {
        for (auto* param : audioProcessor.apvts.processor.getParameters())
            param->setValueNotifyingHost(param->getDefaultValue());
        currentPresetIndex = -1;
        presetNameBtn.setButtonText("-- Init --");
        audioProcessor.apvts.state.setProperty("currentPresetName", "-- Init --", nullptr);
        updateStarBtn();
        if (stepSeqGrid != nullptr) stepSeqGrid->repaint();
    }

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

    std::unique_ptr<ComboAtt> filterTypeAtt, filterOrderAtt;
    std::unique_ptr<ComboAtt> waveCrushModeAtt;
    std::unique_ptr<ComboAtt> lfoWaveformAtt, lfoTargetAtt;
    std::unique_ptr<ComboAtt> stepSeqTargetAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BitMorphAudioProcessorEditor)
};