#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

// ── Colour palette  (black / orange industrial) ───────────────────────────────
static const juce::Colour BG_DARK(0xff020202);
static const juce::Colour BG_PANEL(0xff0d0d0d);
static const juce::Colour BG_HEADER(0xff141414);
static const juce::Colour ACCENT(0xffcc5500);   // orange
static const juce::Colour ACCENT2(0xffdd2200);   // red (danger / active)
static const juce::Colour TEXT_LIGHT(0xffeeeeee);
static const juce::Colour TEXT_MUTED(0xff888888);
static const juce::Colour KNOB_BODY(0xff1c1c1c);
static const juce::Colour BORDER_CLR(0xff2e2e2e);

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

    // Icon buttons: name the button "dice", "xarr", or "star" to get custom drawing
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
        g.setColour(enabled ? ACCENT : juce::Colour(0xff2a2a2a));
        g.fillRect(header.getX(), header.getBottom() - 1, header.getWidth(), 1);

        if (showToggle)
        {
            float dotX = (float)header.getX() + 8.0f;
            float dotY = (float)header.getCentreY() - 5.0f;
            if (enabled) { g.setColour(ACCENT.withAlpha(0.3f)); g.fillEllipse(dotX - 3, dotY - 3, 16, 16); g.setColour(ACCENT); }
            else           g.setColour(juce::Colour(0xff252525));
            g.fillEllipse(dotX, dotY, 10, 10);
            g.setColour(BORDER_CLR);
            g.drawEllipse(dotX, dotY, 10, 10, 1.0f);
        }

        g.setColour(TEXT_LIGHT);
        g.setFont(juce::Font(10.0f, juce::Font::bold));
        int textX = showToggle ? header.getX() + 24 : header.getX() + 6;
        g.drawText(sectionTitle,
            juce::Rectangle<int>(textX, header.getY(),
                header.getWidth() - textX - 4, header.getHeight()),
            juce::Justification::centredLeft);

        if (showToggle && !toggleBtn.getToggleState())
        {
            g.setColour(juce::Colour(0xaa010101));
            g.fillRect(b);
        }
    }

    void resized() override { if (showToggle) toggleBtn.setBounds(0, 0, 22, 22); }

    juce::ToggleButton toggleBtn;

private:
    juce::String sectionTitle;
    bool         showToggle = false;
};

// ── Step Sequencer Grid ───────────────────────────────────────────────────────
class StepSequencerGrid : public juce::Component
{
public:
    StepSequencerGrid(juce::AudioProcessorValueTreeState& a) : apvts(a) {}
    ~StepSequencerGrid() override {}

    void setCurrentStep(int step) { if (step != currentStep) { currentStep = step; repaint(); } }
    void setActiveBank(int bank) { if (bank != activeBank) { activeBank = bank;  repaint(); } }

    void paint(juce::Graphics& g) override
    {
        auto        bounds = getLocalBounds().toFloat();
        const int   numSteps = 16;
        const float stepW = bounds.getWidth() / numSteps;
        const float midY = bounds.getCentreY();
        const float maxH = bounds.getHeight() * 0.5f - 2.0f;

        g.setColour(BG_DARK);
        g.fillRoundedRectangle(bounds, 3.0f);
        g.setColour(BORDER_CLR);
        g.fillRect(bounds.getX(), midY - 0.5f, bounds.getWidth(), 1.0f);

        juce::String prefix = stepPrefixForBank(activeBank);

        for (int i = 0; i < numSteps; ++i)
        {
            auto  stepID = prefix + juce::String(i);
            auto* raw = apvts.getRawParameterValue(stepID);
            float val = (raw != nullptr) ? raw->load() : 0.0f;

            float barH = std::abs(val) * maxH;
            float barX = bounds.getX() + i * stepW + 1.0f;
            float barW = stepW - 2.0f;
            float barY = (val >= 0.0f) ? midY - barH : midY;

            bool isCurrent = (i == currentStep);
            g.setColour(isCurrent ? juce::Colour(0xff1a1000) : juce::Colour(0xff0a0a0a));
            g.fillRect(barX, bounds.getY() + 1.0f, barW, bounds.getHeight() - 2.0f);

            if (std::abs(val) > 0.01f)
            {
                g.setColour(isCurrent ? ACCENT.brighter(0.4f)
                    : (val >= 0.0f ? ACCENT : ACCENT2));
                g.fillRect(barX + 1.0f, barY, barW - 2.0f, barH);
            }

            if (isCurrent)
            {
                g.setColour(ACCENT.withAlpha(0.15f));
                g.fillRect(barX, bounds.getY() + 1.0f, barW, bounds.getHeight() - 2.0f);
                g.setColour(ACCENT.withAlpha(0.8f));
                g.fillRect(barX, bounds.getY() + 1.0f, barW, 2.0f);
            }

            g.setColour(isCurrent ? ACCENT : juce::Colour(0xff333333));
            g.setFont(8.0f);
            g.drawText(juce::String(i + 1), (int)barX, (int)(bounds.getBottom() - 12),
                (int)barW, 12, juce::Justification::centred);
        }

        g.setColour(BORDER_CLR);
        g.drawRoundedRectangle(bounds.reduced(0.5f), 3.0f, 1.0f);
    }

    void mouseDown(const juce::MouseEvent& e) override { setStepFromMouse(e); }
    void mouseDrag(const juce::MouseEvent& e) override { setStepFromMouse(e); }

private:
    void setStepFromMouse(const juce::MouseEvent& e)
    {
        const float stepW = getWidth() / 16.0f;
        int idx = juce::jlimit(0, 15, (int)(e.position.x / stepW));
        float val = juce::jlimit(-1.0f, 1.0f, 1.0f - (e.position.y / getHeight()) * 2.0f);
        auto stepID = stepPrefixForBank(activeBank) + juce::String(idx);
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
    int activeBank = 0;
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
        if (stepSeqGrid == nullptr) return;
        stepSeqGrid->setCurrentStep(audioProcessor.getCurrentStepSeqStep());
        int bank = static_cast<int> (
            audioProcessor.apvts.getRawParameterValue(ParamID::STEP_SEQ_BANK)->load());
        stepSeqGrid->setActiveBank(bank);
        updateBankButtons(bank);
    }

private:
    BitMorphAudioProcessor& audioProcessor;
    BitMorphLookAndFeel     lookAndFeel;

    // ── Knob + label pair ─────────────────────────────────────────────────────
    struct KnobSet
    {
        juce::Slider knob{ juce::Slider::RotaryVerticalDrag, juce::Slider::TextBoxBelow };
        juce::Label  label;
    };

    void setupKnob(KnobSet&, const juce::String& labelText);
    void placeKnobAt(KnobSet&, int x, int y, int w);
    void setupCombo(juce::ComboBox&, juce::Label&,
        const juce::String& labelText,
        const juce::StringArray& items);

    // ── Section panels ────────────────────────────────────────────────────────
    SectionPanel quantizerPanel{ "QUANTIZER",   true };
    SectionPanel drivePanel{ "DRIVE",       true };
    SectionPanel resamplerPanel{ "RESAMPLER",   true };
    SectionPanel filterPanel{ "FILTER",      false };
    SectionPanel waveCrushPanel{ "WAVECRUSHER", true };
    SectionPanel ringModPanel{ "RING MOD",    true };
    SectionPanel noisePanel{ "NOISE",       true };
    SectionPanel lfoPanel{ "LFO",         false };
    SectionPanel stepSeqPanel{ "STEP SEQ",    true };
    SectionPanel masterPanel{ "MASTER",      false };

    // ── Quantizer ─────────────────────────────────────────────────────────────
    KnobSet            bitDepthKnob;
    KnobSet            ditheringKnob;
    juce::ToggleButton dcShiftBtn{ "DC Shift" };

    // ── Drive ─────────────────────────────────────────────────────────────────
    KnobSet        driveAmountKnob;
    KnobSet        driveBiasKnob;
    KnobSet        driveMixKnob;
    juce::ComboBox driveModeCombo;
    juce::Label    driveModeLabel{ {}, "Mode" };

    // ── Resampler ─────────────────────────────────────────────────────────────
    KnobSet            resampleFreqKnob;
    KnobSet            approxDeviationKnob;
    KnobSet            imagesShiftKnob;
    KnobSet            jitterKnob;
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
    KnobSet        waveCrushAmountKnob;
    juce::ComboBox waveCrushModeCombo;
    juce::Label    waveCrushModeLabel{ {}, "Mode" };

    // ── Ring Mod ──────────────────────────────────────────────────────────────
    KnobSet ringModFreqKnob;
    KnobSet ringModMixKnob;

    // ── Noise ─────────────────────────────────────────────────────────────────
    KnobSet        noiseAmountKnob;
    KnobSet        noiseColourKnob;
    juce::ComboBox noiseTypeCombo;
    juce::Label    noiseTypeLabel{ {}, "Type" };

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
    KnobSet            stepSeqSwingKnob;
    juce::ComboBox     stepSeqTargetCombo;
    juce::ComboBox     stepSeqDirCombo;
    juce::Label        stepSeqTargetLabel{ {}, "Target" };
    juce::Label        stepSeqDirLabel{ {}, "Dir" };
    juce::Slider       stepSeqLenSlider{ juce::Slider::LinearHorizontal,
                                            juce::Slider::TextBoxRight };
    juce::Label        stepSeqLenLabel{ {}, "Steps" };
    juce::TextButton   bankBtn1{ "1" };
    juce::TextButton   bankBtn2{ "2" };
    juce::TextButton   bankBtn3{ "3" };
    juce::TextButton   bankBtn4{ "4" };
    juce::TextButton   randStepBtn{ "rand" };   // randomize step seq
    std::unique_ptr<StepSequencerGrid> stepSeqGrid;

    // ── Master ────────────────────────────────────────────────────────────────
    KnobSet preampKnob;
    KnobSet fxMixKnob;
    KnobSet outputVolumeKnob;

    // ── Preset system ─────────────────────────────────────────────────────────
    struct PresetEntry { juce::String name, folderName; juce::File file; };

    juce::Array<PresetEntry> allPresets;
    juce::StringArray        favorites;      // stores file paths of favourited presets
    int currentPresetIndex = -1;

    // Preset bar buttons
    // "dice" name → dice icon drawn in drawButtonText
    // "xarr" name → crossed arrows icon
    // "star" name → star icon (favourite toggle)
    juce::TextButton presetPrevBtn{ "<" };
    juce::TextButton presetNextBtn{ ">" };
    juce::TextButton presetNameBtn{ "-- Init --" };
    juce::TextButton presetFaveBtn{ "*" };   // name = "star"
    juce::TextButton presetRandBtn{ "?" };   // name = "dice"
    juce::TextButton presetRandParamBtn{ "~" };   // name = "xarr"
    juce::TextButton presetSaveBtn{ "Save" };
    juce::TextButton presetLoadBtn{ "Load" };

    juce::File getPresetsFolder()
    {
        return juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
            .getChildFile("JUCE Projects/BitMorph/Presets");
    }

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

    bool isCurrentFavorite()
    {
        if (currentPresetIndex < 0 || currentPresetIndex >= allPresets.size()) return false;
        return favorites.contains(allPresets[currentPresetIndex].file.getFullPathName());
    }

    void toggleFavorite()
    {
        if (currentPresetIndex < 0 || currentPresetIndex >= allPresets.size()) return;
        auto path = allPresets[currentPresetIndex].file.getFullPathName();
        if (favorites.contains(path))
            favorites.removeString(path);
        else
            favorites.add(path);
        saveFavorites();
        updateFaveButton();
    }

    void updateFaveButton()
    {
        bool faved = isCurrentFavorite();
        presetFaveBtn.setColour(juce::TextButton::buttonColourId,
            faved ? ACCENT : juce::Colour(0xff1a1a1a));
        presetFaveBtn.setColour(juce::TextButton::textColourOffId,
            faved ? TEXT_LIGHT : TEXT_MUTED);
        presetFaveBtn.repaint();
    }

    void refreshPresetList()
    {
        allPresets.clear();
        auto root = getPresetsFolder();
        if (!root.isDirectory()) return;
        for (auto& f : root.findChildFiles(juce::File::findFiles, false, "*.xml"))
            allPresets.add({ f.getFileNameWithoutExtension(), {}, f });
        for (auto& folder : root.findChildFiles(juce::File::findDirectories, false))
            for (auto& f : folder.findChildFiles(juce::File::findFiles, false, "*.xml"))
                allPresets.add({ f.getFileNameWithoutExtension(), folder.getFileName(), f });
    }

    void updatePresetLabel()
    {
        if (currentPresetIndex >= 0 && currentPresetIndex < allPresets.size())
            presetNameBtn.setButtonText(allPresets[currentPresetIndex].name);
        updateFaveButton();
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
        if (stepSeqGrid != nullptr) stepSeqGrid->repaint();
    }

    void initializePreset()
    {
        // Reset every parameter to its default value
        for (auto* param : audioProcessor.apvts.processor.getParameters())
        {
            param->beginChangeGesture();
            param->setValueNotifyingHost(param->getDefaultValue());
            param->endChangeGesture();
        }
        currentPresetIndex = -1;
        presetNameBtn.setButtonText("-- Init --");
        updateFaveButton();
        if (stepSeqGrid != nullptr) stepSeqGrid->repaint();
    }

    void showPresetMenu()
    {
        juce::PopupMenu menu, currentSub;
        juce::String    currentFolder;
        int             itemID = 2;   // 1 reserved for Init

        // Init option always at top
        menu.addItem(1, "-- Initialize --");
        menu.addSeparator();

        // Favourites submenu
        if (favorites.size() > 0)
        {
            juce::PopupMenu favMenu;
            for (auto& p : allPresets)
                if (favorites.contains(p.file.getFullPathName()))
                    favMenu.addItem(itemID++, p.name);
            menu.addSubMenu("Favorites", favMenu);
            menu.addSeparator();
        }

        for (auto& pr : allPresets)
        {
            if (pr.folderName.isEmpty()) { menu.addItem(itemID++, pr.name); }
            else
            {
                if (pr.folderName != currentFolder)
                {
                    if (currentFolder.isNotEmpty()) menu.addSubMenu(currentFolder, currentSub);
                    currentSub = juce::PopupMenu(); currentFolder = pr.folderName;
                }
                currentSub.addItem(itemID++, pr.name);
            }
        }
        if (currentFolder.isNotEmpty()) menu.addSubMenu(currentFolder, currentSub);

        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(presetNameBtn),
            [this](int r)
            {
                if (r == 1) { initializePreset(); return; }
                if (r >= 2)
                {
                    // Map back: first count favs to skip them in the main list
                    int favCount = 0;
                    if (favorites.size() > 0)
                        for (auto& p : allPresets)
                            if (favorites.contains(p.file.getFullPathName())) ++favCount;

                    // r=2 starts the fav entries, then after favCount comes main list
                    int favStart = 2;
                    int mainStart = favStart + favCount;

                    if (favorites.size() > 0 && r >= favStart && r < mainStart)
                    {
                        // It's a favourite — find which one
                        int favIdx = r - favStart;
                        int count = 0;
                        for (int i = 0; i < allPresets.size(); ++i)
                            if (favorites.contains(allPresets[i].file.getFullPathName()))
                            {
                                if (count == favIdx) { loadPresetByIndex(i); return; }
                                ++count;
                            }
                    }
                    else
                    {
                        loadPresetByIndex(r - mainStart);
                    }
                }
            });
    }

    void savePreset()
    {
        auto chooser = std::make_shared<juce::FileChooser>("Save Preset", getPresetsFolder(), "*.xml");
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
        loadPresetByIndex(juce::Random::getSystemRandom().nextInt(allPresets.size()));
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
        updateFaveButton();
        if (stepSeqGrid != nullptr) stepSeqGrid->repaint();
    }

    void randomizeStepSeq()
    {
        juce::Random rng;
        int bank = static_cast<int> (
            audioProcessor.apvts.getRawParameterValue(ParamID::STEP_SEQ_BANK)->load());
        auto prefix = stepPrefixForBank(bank);
        for (int i = 0; i < 16; ++i)
        {
            auto stepID = prefix + juce::String(i);
            if (auto* param = audioProcessor.apvts.getParameter(stepID))
            {
                param->beginChangeGesture();
                // Random value biased toward positive range
                float v = rng.nextFloat();
                param->setValueNotifyingHost(v);
                param->endChangeGesture();
            }
        }
        if (stepSeqGrid != nullptr) stepSeqGrid->repaint();
    }

    void updateBankButtons(int activeBank)
    {
        auto style = [&](juce::TextButton& btn, int idx)
            {
                btn.setColour(juce::TextButton::buttonColourId,
                    (idx == activeBank) ? ACCENT : juce::Colour(0xff1a1a1a));
                btn.setColour(juce::TextButton::textColourOffId,
                    (idx == activeBank) ? TEXT_LIGHT : TEXT_MUTED);
            };
        style(bankBtn1, 0); style(bankBtn2, 1);
        style(bankBtn3, 2); style(bankBtn4, 3);
    }

    void selectBank(int bank)
    {
        if (auto* param = audioProcessor.apvts.getParameter(ParamID::STEP_SEQ_BANK))
        {
            param->beginChangeGesture();
            param->setValueNotifyingHost(param->convertTo0to1((float)bank));
            param->endChangeGesture();
        }
        updateBankButtons(bank);
        if (stepSeqGrid != nullptr) { stepSeqGrid->setActiveBank(bank); stepSeqGrid->repaint(); }
    }

    // ── APVTS attachments ─────────────────────────────────────────────────────
    using SliderAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAtt = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAtt = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<SliderAtt> bitDepthAtt, ditheringAtt;
    std::unique_ptr<ButtonAtt> bitDepthOnAtt, dcShiftAtt;

    std::unique_ptr<SliderAtt> driveAmountAtt, driveBiasAtt, driveMixAtt;
    std::unique_ptr<ButtonAtt> driveEnabledAtt;
    std::unique_ptr<ComboAtt>  driveModeAtt;

    std::unique_ptr<SliderAtt> resampleFreqAtt, approxDeviationAtt, imagesShiftAtt, jitterAtt;
    std::unique_ptr<ButtonAtt> resampleOnAtt, approxOnAtt, imagesOnAtt;

    std::unique_ptr<SliderAtt> filterCutoffAtt, filterResonanceAtt;
    std::unique_ptr<ComboAtt>  filterTypeAtt, filterOrderAtt;

    std::unique_ptr<SliderAtt> waveCrushAmountAtt;
    std::unique_ptr<ButtonAtt> waveCrushOnAtt;
    std::unique_ptr<ComboAtt>  waveCrushModeAtt;

    std::unique_ptr<SliderAtt> ringModFreqAtt, ringModMixAtt;
    std::unique_ptr<ButtonAtt> ringModOnAtt;

    std::unique_ptr<SliderAtt> noiseAmountAtt, noiseColourAtt;
    std::unique_ptr<ButtonAtt> noiseEnabledAtt;
    std::unique_ptr<ComboAtt>  noiseTypeAtt;

    std::unique_ptr<SliderAtt> lfoRateAtt, lfoDepthAtt;
    std::unique_ptr<ComboAtt>  lfoWaveformAtt, lfoTargetAtt;

    std::unique_ptr<SliderAtt> stepSeqRateAtt, stepSeqDepthAtt, stepSeqSwingAtt, stepSeqLenAtt;
    std::unique_ptr<ButtonAtt> stepSeqOnAtt;
    std::unique_ptr<ComboAtt>  stepSeqTargetAtt, stepSeqDirAtt;

    std::unique_ptr<SliderAtt> preampAtt, fxMixAtt, outputVolumeAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BitMorphAudioProcessorEditor)
};