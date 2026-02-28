#include "PluginEditor.h"

static const juce::Colour BG_DARK(0xff0d0d1a);
static const juce::Colour BG_PANEL(0xff141428);
static const juce::Colour BG_HEADER(0xff1a1a35);
static const juce::Colour ACCENT(0xffe94560);
static const juce::Colour TEXT_LIGHT(0xffddddee);
static const juce::Colour TEXT_MUTED(0xffaabbcc);
static const juce::Colour KNOB_BODY(0xff252538);
static const juce::Colour BORDER_CLR(0xff2a2a45);

BitMorphLookAndFeel::BitMorphLookAndFeel()
{
    setColour(juce::Slider::textBoxTextColourId, TEXT_LIGHT);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0x00000000));
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0x00000000));
    setColour(juce::ComboBox::backgroundColourId, BG_PANEL);
    setColour(juce::ComboBox::textColourId, TEXT_LIGHT);
    setColour(juce::ComboBox::outlineColourId, BORDER_CLR);
    setColour(juce::ComboBox::arrowColourId, ACCENT);
    setColour(juce::PopupMenu::backgroundColourId, BG_DARK);
    setColour(juce::PopupMenu::textColourId, TEXT_LIGHT);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, ACCENT);
    setColour(juce::Label::textColourId, TEXT_MUTED);
}

void BitMorphLookAndFeel::drawRotarySlider(juce::Graphics& g,
    int x, int y, int width, int height,
    float sliderPos,
    float rotaryStartAngle,
    float rotaryEndAngle,
    juce::Slider&)
{
    float radius = juce::jmin(width / 2.0f, height / 2.0f) - 6.0f;
    float centreX = x + width * 0.5f;
    float centreY = y + height * 0.5f;
    float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    g.setColour(BORDER_CLR);
    g.fillEllipse(centreX - radius - 3, centreY - radius - 3,
        (radius + 3) * 2.0f, (radius + 3) * 2.0f);

    g.setColour(KNOB_BODY);
    g.fillEllipse(centreX - radius, centreY - radius,
        radius * 2.0f, radius * 2.0f);

    juce::Path trackArc;
    trackArc.addCentredArc(centreX, centreY, radius + 5, radius + 5,
        0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(juce::Colour(0xff1e1e32));
    g.strokePath(trackArc, juce::PathStrokeType(3.5f));

    juce::Path filledArc;
    filledArc.addCentredArc(centreX, centreY, radius + 5, radius + 5,
        0.0f, rotaryStartAngle, angle, true);
    g.setColour(ACCENT);
    g.strokePath(filledArc, juce::PathStrokeType(3.5f));

    float pointerLength = radius * 0.55f;
    float px = centreX + pointerLength * std::sin(angle);
    float py = centreY - pointerLength * std::cos(angle);
    g.setColour(ACCENT);
    g.drawLine(centreX, centreY, px, py, 2.0f);

    g.setColour(juce::Colour(0xff333350));
    g.fillEllipse(centreX - 3.0f, centreY - 3.0f, 6.0f, 6.0f);
}

void BitMorphLookAndFeel::drawToggleButton(juce::Graphics& g,
    juce::ToggleButton& button,
    bool, bool)
{
    const float ledSize = 10.0f;
    const float ledX = 6.0f;
    const float ledY = (button.getHeight() - ledSize) * 0.5f;

    if (button.getToggleState())
    {
        g.setColour(ACCENT.withAlpha(0.3f));
        g.fillEllipse(ledX - 4, ledY - 4, ledSize + 8, ledSize + 8);
        g.setColour(ACCENT);
    }
    else
    {
        g.setColour(juce::Colour(0xff2a2a40));
    }

    g.fillEllipse(ledX, ledY, ledSize, ledSize);
    g.setColour(BORDER_CLR);
    g.drawEllipse(ledX, ledY, ledSize, ledSize, 1.0f);

    g.setColour(button.getToggleState() ? TEXT_LIGHT : TEXT_MUTED);
    g.setFont(10.0f);
    g.drawText(button.getButtonText(),
        (int)(ledX + ledSize + 5), 0,
        button.getWidth() - (int)(ledX + ledSize + 5),
        button.getHeight(),
        juce::Justification::centredLeft);
}

SectionPanel::SectionPanel(const juce::String& title) : sectionTitle(title) {}

void SectionPanel::paint(juce::Graphics& g)
{
    auto b = getLocalBounds();

    g.setColour(BG_PANEL);
    g.fillRoundedRectangle(b.toFloat(), 4.0f);

    g.setColour(BORDER_CLR);
    g.drawRoundedRectangle(b.toFloat().reduced(0.5f), 4.0f, 1.0f);

    auto header = b.removeFromTop(22);
    g.setColour(BG_HEADER);
    g.fillRect(header);

    g.setColour(ACCENT);
    g.fillRect(header.getX(), header.getBottom() - 1, header.getWidth(), 1);

    g.setColour(TEXT_LIGHT);
    g.setFont(juce::Font(10.0f, juce::Font::bold));
    g.drawText(sectionTitle, header.reduced(6, 0),
        juce::Justification::centredLeft);
}

void BitMorphAudioProcessorEditor::setupKnob(KnobSet& k, const juce::String& labelText)
{
    k.knob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 16);
    k.label.setText(labelText, juce::dontSendNotification);
    k.label.setJustificationType(juce::Justification::centred);
    k.label.setFont(12.0f);
    k.label.setColour(juce::Label::textColourId, TEXT_MUTED);
    addAndMakeVisible(k.knob);
    addAndMakeVisible(k.label);
}

void BitMorphAudioProcessorEditor::placeKnob(KnobSet& k, juce::Rectangle<int> bounds)
{
    k.label.setBounds(bounds.removeFromBottom(16));
    const int sz = 60;
    int ox = bounds.getX() + (bounds.getWidth() - sz) / 2;
    int oy = bounds.getY() + (bounds.getHeight() - sz) / 2;
    k.knob.setBounds(ox, oy, sz, sz);
}

void BitMorphAudioProcessorEditor::setupCombo(juce::ComboBox& combo,
    juce::Label& label,
    const juce::String& labelText,
    const juce::StringArray& items)
{
    label.setText(labelText, juce::dontSendNotification);
    label.setFont(10.0f);
    label.setColour(juce::Label::textColourId, TEXT_MUTED);
    label.setJustificationType(juce::Justification::centredLeft);
    combo.addItemList(items, 1);
    addAndMakeVisible(label);
    addAndMakeVisible(combo);
}

BitMorphAudioProcessorEditor::BitMorphAudioProcessorEditor(BitMorphAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&lookAndFeel);

    // Preset bar
    auto styleBtn = [](juce::TextButton& b, juce::Colour bg, juce::Colour fg)
        {
            b.setColour(juce::TextButton::buttonColourId, bg);
            b.setColour(juce::TextButton::textColourOffId, fg);
        };

    styleBtn(presetPrevBtn, juce::Colour(0xff1a1a35), TEXT_LIGHT);
    styleBtn(presetNextBtn, juce::Colour(0xff1a1a35), TEXT_LIGHT);
    styleBtn(presetSaveBtn, ACCENT, TEXT_LIGHT);
    styleBtn(presetLoadBtn, juce::Colour(0xff252538), TEXT_LIGHT);
    styleBtn(presetRandBtn, juce::Colour(0xff252538), TEXT_LIGHT);
    styleBtn(presetRandParamBtn, juce::Colour(0xff252538), ACCENT);
    styleBtn(presetNameBtn, juce::Colour(0xff1a1a35), TEXT_LIGHT);
    presetNameBtn.setButtonText("-- Init --");

    addAndMakeVisible(presetPrevBtn);
    addAndMakeVisible(presetNextBtn);
    addAndMakeVisible(presetSaveBtn);
    addAndMakeVisible(presetLoadBtn);
    addAndMakeVisible(presetRandBtn);
    addAndMakeVisible(presetRandParamBtn);
    addAndMakeVisible(presetNameBtn);

    presetPrevBtn.onClick = [this]
        {
            if (allPresets.isEmpty()) return;
            currentPresetIndex = (currentPresetIndex <= 0)
                ? allPresets.size() - 1
                : currentPresetIndex - 1;
            loadPresetByIndex(currentPresetIndex);
        };

    presetNextBtn.onClick = [this]
        {
            if (allPresets.isEmpty()) return;
            currentPresetIndex = (currentPresetIndex >= allPresets.size() - 1)
                ? 0
                : currentPresetIndex + 1;
            loadPresetByIndex(currentPresetIndex);
        };

    presetNameBtn.onClick = [this] { showPresetMenu(); };
    presetSaveBtn.onClick = [this] { savePreset(); };
    presetLoadBtn.onClick = [this]
        {
            auto chooser = std::make_shared<juce::FileChooser>(
                "Load Preset", getPresetsFolder(), "*.xml");
            chooser->launchAsync(juce::FileBrowserComponent::openMode |
                juce::FileBrowserComponent::canSelectFiles,
                [this, chooser](const juce::FileChooser& fc)
                {
                    auto result = fc.getResult();
                    if (!result.existsAsFile()) return;
                    juce::MemoryBlock data;
                    result.loadFileAsData(data);
                    audioProcessor.setStateInformation(data.getData(), (int)data.getSize());
                    for (int i = 0; i < allPresets.size(); ++i)
                    {
                        if (allPresets[i].file == result)
                        {
                            currentPresetIndex = i;
                            break;
                        }
                    }
                    updatePresetLabel();
                });
        };
    presetRandBtn.onClick = [this] { randomPreset(); };
    presetRandParamBtn.onClick = [this] { randomizeParameters(); };

    refreshPresetList();

    auto& apvts = audioProcessor.apvts;

    addAndMakeVisible(quantizerPanel);
    addAndMakeVisible(resamplerPanel);
    addAndMakeVisible(filterPanel);
    addAndMakeVisible(waveCrushPanel);
    addAndMakeVisible(ringModPanel);
    addAndMakeVisible(lfoPanel);
    addAndMakeVisible(stepSeqPanel);
    addAndMakeVisible(masterPanel);

    setupKnob(bitDepthKnob, "Bit Depth");
    setupKnob(ditheringKnob, "Dithering");
    addAndMakeVisible(bitDepthOnBtn);
    addAndMakeVisible(dcShiftBtn);
    bitDepthAtt = std::make_unique<SliderAtt>(apvts, "bitDepth", bitDepthKnob.knob);
    ditheringAtt = std::make_unique<SliderAtt>(apvts, "dithering", ditheringKnob.knob);
    bitDepthOnAtt = std::make_unique<ButtonAtt>(apvts, "bitDepthEnabled", bitDepthOnBtn);
    dcShiftAtt = std::make_unique<ButtonAtt>(apvts, "dcShift", dcShiftBtn);

    setupKnob(resampleFreqKnob, "Frequency");
    setupKnob(approxDeviationKnob, "Deviation");
    setupKnob(imagesShiftKnob, "Img Shift");
    addAndMakeVisible(resampleOnBtn);
    addAndMakeVisible(approxOnBtn);
    addAndMakeVisible(imagesOnBtn);
    resampleFreqAtt = std::make_unique<SliderAtt>(apvts, "resampleFreq", resampleFreqKnob.knob);
    approxDeviationAtt = std::make_unique<SliderAtt>(apvts, "approxDeviation", approxDeviationKnob.knob);
    imagesShiftAtt = std::make_unique<SliderAtt>(apvts, "imagesShift", imagesShiftKnob.knob);
    resampleOnAtt = std::make_unique<ButtonAtt>(apvts, "resampleEnabled", resampleOnBtn);
    approxOnAtt = std::make_unique<ButtonAtt>(apvts, "approxEnabled", approxOnBtn);
    imagesOnAtt = std::make_unique<ButtonAtt>(apvts, "imagesEnabled", imagesOnBtn);

    setupKnob(filterCutoffKnob, "Cutoff");
    setupKnob(filterResonanceKnob, "Resonance");
    setupCombo(filterTypeCombo, filterTypeLabel, "Type",
        { "Off", "Low Pass", "Band Pass", "High Pass", "Band Reject" });
    setupCombo(filterOrderCombo, filterOrderLabel, "Order", { "Post", "Pre" });
    filterCutoffAtt = std::make_unique<SliderAtt>(apvts, "filterCutoff", filterCutoffKnob.knob);
    filterResonanceAtt = std::make_unique<SliderAtt>(apvts, "filterResonance", filterResonanceKnob.knob);
    filterTypeAtt = std::make_unique<ComboAtt>(apvts, "filterType", filterTypeCombo);
    filterOrderAtt = std::make_unique<ComboAtt>(apvts, "filterOrder", filterOrderCombo);

    setupKnob(waveCrushAmountKnob, "Amount");
    setupCombo(waveCrushModeCombo, waveCrushModeLabel, "Mode", { "Fold", "Wrap", "Tanh" });
    addAndMakeVisible(waveCrushOnBtn);
    waveCrushAmountAtt = std::make_unique<SliderAtt>(apvts, "waveCrushAmount", waveCrushAmountKnob.knob);
    waveCrushOnAtt = std::make_unique<ButtonAtt>(apvts, "waveCrushEnabled", waveCrushOnBtn);
    waveCrushModeAtt = std::make_unique<ComboAtt>(apvts, "waveCrushMode", waveCrushModeCombo);

    setupKnob(ringModFreqKnob, "Frequency");
    setupKnob(ringModMixKnob, "Mix");
    addAndMakeVisible(ringModOnBtn);
    ringModFreqAtt = std::make_unique<SliderAtt>(apvts, "ringModFreq", ringModFreqKnob.knob);
    ringModMixAtt = std::make_unique<SliderAtt>(apvts, "ringModMix", ringModMixKnob.knob);
    ringModOnAtt = std::make_unique<ButtonAtt>(apvts, "ringModEnabled", ringModOnBtn);

    setupKnob(lfoRateKnob, "Rate");
    setupKnob(lfoDepthKnob, "Depth");
    setupCombo(lfoWaveformCombo, lfoWaveformLabel, "Waveform",
        { "Sine", "Triangle", "Square", "Saw Up", "Saw Down", "Random" });
    setupCombo(lfoTargetCombo, lfoTargetLabel, "Target",
        { "Bit Depth", "Sample Rate", "WaveCrush", "Ring Freq" });
    lfoRateAtt = std::make_unique<SliderAtt>(apvts, "lfoRate", lfoRateKnob.knob);
    lfoDepthAtt = std::make_unique<SliderAtt>(apvts, "lfoDepth", lfoDepthKnob.knob);
    lfoWaveformAtt = std::make_unique<ComboAtt>(apvts, "lfoWaveform", lfoWaveformCombo);
    lfoTargetAtt = std::make_unique<ComboAtt>(apvts, "lfoTarget", lfoTargetCombo);

    setupKnob(stepSeqRateKnob, "Rate");
    setupKnob(stepSeqDepthKnob, "Depth");
    setupCombo(stepSeqTargetCombo, stepSeqTargetLabel, "Target",
        { "Bit Depth", "Sample Rate", "WaveCrush", "Ring Freq" });
    addAndMakeVisible(stepSeqOnBtn);
    stepSeqRateAtt = std::make_unique<SliderAtt>(apvts, "stepSeqRate", stepSeqRateKnob.knob);
    stepSeqDepthAtt = std::make_unique<SliderAtt>(apvts, "stepSeqDepth", stepSeqDepthKnob.knob);
    stepSeqOnAtt = std::make_unique<ButtonAtt>(apvts, "stepSeqEnabled", stepSeqOnBtn);
    stepSeqTargetAtt = std::make_unique<ComboAtt>(apvts, "stepSeqTarget", stepSeqTargetCombo);

    stepSeqGrid = std::make_unique<StepSequencerGrid>(apvts);
    addAndMakeVisible(*stepSeqGrid);

    setupKnob(preampKnob, "Preamp");
    setupKnob(fxMixKnob, "FX Mix");
    setupKnob(outputVolumeKnob, "Output");
    preampAtt = std::make_unique<SliderAtt>(apvts, "preampGain", preampKnob.knob);
    fxMixAtt = std::make_unique<SliderAtt>(apvts, "fxMix", fxMixKnob.knob);
    outputVolumeAtt = std::make_unique<SliderAtt>(apvts, "outputVolume", outputVolumeKnob.knob);

    setSize(1200, 800);


}

BitMorphAudioProcessorEditor::~BitMorphAudioProcessorEditor()
{
    stepSeqGrid.reset();
    setLookAndFeel(nullptr);
}

void BitMorphAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(BG_DARK);

    g.setColour(juce::Colour(0xff0a0a18));
    g.fillRect(0, 0, getWidth(), 60);

    g.setColour(ACCENT);
    g.fillRect(0, 58, getWidth(), 2);

    g.setColour(TEXT_LIGHT);
    g.setFont(juce::Font(20.0f, juce::Font::bold));
    g.drawText("BITMORPH", 16, 0, 200, 60, juce::Justification::centredLeft);

    g.setColour(TEXT_MUTED);
    g.setFont(9.5f);
    g.drawText("evrshade 2026", 0, 0, getWidth() - 16, 60,
        juce::Justification::centredRight);

    g.setColour(juce::Colour(0xff12121f));
    g.fillRect(0, 40, getWidth(), 20);
}

void BitMorphAudioProcessorEditor::resized()
{
    const int W = getWidth();
    const int GAP = 3;
    const int HEADERH = 60;
    const int MASTERH = 95;
    const int STEPSEQH = 200;
    const int ROW1H = 185;
    const int ROW2H = 180;

    // ── Preset bar ────────────────────────────────────────────────────────────
    {
        int y = 40;
        int h = 18;
        int bw = 44;
        presetPrevBtn.setBounds(4, y, 22, h);
        presetNextBtn.setBounds(28, y, 22, h);
        presetNameBtn.setBounds(52, y, W - 52 - bw * 4 - 10, h);
        presetRandBtn.setBounds(W - bw * 4 - 6, y, bw, h);
        presetRandParamBtn.setBounds(W - bw * 3 - 4, y, bw, h);
        presetSaveBtn.setBounds(W - bw * 2 - 2, y, bw, h);
        presetLoadBtn.setBounds(W - bw, y, bw, h);
    }

    auto placeKnobAt = [](KnobSet& k, int x, int y, int w)
        {
            int kx = x + (w - 56) / 2;
            k.knob.setBounds(kx, y, 56, 56);
            k.label.setBounds(x, y + 58, w, 14);
        };

    // ── Row 1 ─────────────────────────────────────────────────────────────────
    int r1y = HEADERH + GAP;
    int sw1 = W / 3;
    quantizerPanel.setBounds(juce::Rectangle<int>(0, r1y, sw1, ROW1H).reduced(GAP));
    resamplerPanel.setBounds(juce::Rectangle<int>(sw1, r1y, sw1, ROW1H).reduced(GAP));
    filterPanel.setBounds(juce::Rectangle<int>(sw1 * 2, r1y, sw1, ROW1H).reduced(GAP));

    // Quantizer
    {
        auto b = quantizerPanel.getBounds();
        int  ix = b.getX() + 6;
        int  iy = b.getY() + 30;
        int  kw = (b.getWidth() - 12) / 2;
        placeKnobAt(bitDepthKnob, ix, iy, kw);
        placeKnobAt(ditheringKnob, ix + kw, iy, kw);
        bitDepthOnBtn.setBounds(ix, iy + 74, kw, 18);
        dcShiftBtn.setBounds(ix + kw, iy + 74, kw, 18);
    }

    // Resampler
    {
        auto b = resamplerPanel.getBounds();
        int  ix = b.getX() + 6;
        int  iy = b.getY() + 30;
        int  kw = (b.getWidth() - 12) / 3;
        int  fw = b.getWidth() - 12;
        placeKnobAt(resampleFreqKnob, ix, iy, kw);
        placeKnobAt(approxDeviationKnob, ix + kw, iy, kw);
        placeKnobAt(imagesShiftKnob, ix + kw * 2, iy, kw);
        resampleOnBtn.setBounds(ix, iy + 74, fw, 16);
        approxOnBtn.setBounds(ix, iy + 74 + 18, fw, 16);
        imagesOnBtn.setBounds(ix, iy + 74 + 36, fw, 16);
    }

    // Filter
    {
        auto b = filterPanel.getBounds();
        int  ix = b.getX() + 6;
        int  iy = b.getY() + 30;
        int  kw = (b.getWidth() - 12) / 2;
        int  fw = b.getWidth() - 12;
        placeKnobAt(filterCutoffKnob, ix, iy, kw);
        placeKnobAt(filterResonanceKnob, ix + kw, iy, kw);
        filterTypeLabel.setBounds(ix, iy + 74, 38, 22);
        filterTypeCombo.setBounds(ix + 38, iy + 74, fw - 38, 22);
        filterOrderLabel.setBounds(ix, iy + 74 + 26, 38, 22);
        filterOrderCombo.setBounds(ix + 38, iy + 74 + 26, fw - 38, 22);
    }

    // ── Row 2 ─────────────────────────────────────────────────────────────────
    int r2y = r1y + ROW1H + GAP;
    int sw2 = W / 3;
    waveCrushPanel.setBounds(juce::Rectangle<int>(0, r2y, sw2, ROW2H).reduced(GAP));
    ringModPanel.setBounds(juce::Rectangle<int>(sw2, r2y, sw2, ROW2H).reduced(GAP));
    lfoPanel.setBounds(juce::Rectangle<int>(sw2 * 2, r2y, sw2, ROW2H).reduced(GAP));

    // WaveCrusher
    {
        auto b = waveCrushPanel.getBounds();
        int  ix = b.getX() + 6;
        int  iy = b.getY() + 30;
        int  fw = b.getWidth() - 12;
        placeKnobAt(waveCrushAmountKnob, ix, iy, fw);
        waveCrushOnBtn.setBounds(ix, iy + 74, fw, 18);
        waveCrushModeLabel.setBounds(ix, iy + 74 + 22, 38, 22);
        waveCrushModeCombo.setBounds(ix + 38, iy + 74 + 22, fw - 38, 22);
    }

    // Ring Mod
    {
        auto b = ringModPanel.getBounds();
        int  ix = b.getX() + 6;
        int  iy = b.getY() + 30;
        int  kw = (b.getWidth() - 12) / 2;
        int  fw = b.getWidth() - 12;
        placeKnobAt(ringModFreqKnob, ix, iy, kw);
        placeKnobAt(ringModMixKnob, ix + kw, iy, kw);
        ringModOnBtn.setBounds(ix, iy + 74, fw, 18);
    }

    // LFO
    {
        auto b = lfoPanel.getBounds();
        int  ix = b.getX() + 6;
        int  iy = b.getY() + 30;
        int  kw = (b.getWidth() - 12) / 2;
        int  fw = b.getWidth() - 12;
        placeKnobAt(lfoRateKnob, ix, iy, kw);
        placeKnobAt(lfoDepthKnob, ix + kw, iy, kw);
        lfoWaveformLabel.setBounds(ix, iy + 74, 46, 22);
        lfoWaveformCombo.setBounds(ix + 46, iy + 74, fw - 46, 22);
        lfoTargetLabel.setBounds(ix, iy + 74 + 26, 46, 22);
        lfoTargetCombo.setBounds(ix + 46, iy + 74 + 26, fw - 46, 22);
    }

    // ── Row 3: Step Sequencer ─────────────────────────────────────────────────
    int r3y = r2y + ROW2H + GAP;
    int ctrlW = 150;
    stepSeqPanel.setBounds(GAP, r3y, W - GAP * 2, STEPSEQH);
    {
        auto b = stepSeqPanel.getBounds();
        int  ix = b.getX() + 6;
        int  iy = b.getY() + 30;
        int  kw = ctrlW / 2;

        placeKnobAt(stepSeqRateKnob, ix, iy, kw);
        placeKnobAt(stepSeqDepthKnob, ix + kw, iy, kw);
        stepSeqOnBtn.setBounds(ix, iy + 74, ctrlW, 18);
        stepSeqTargetLabel.setBounds(ix, iy + 96, 44, 22);
        stepSeqTargetCombo.setBounds(ix + 44, iy + 96, ctrlW - 44, 22);

        int gx = b.getX() + 6 + ctrlW + 8;
        int gy = b.getY() + 26;
        int gw = b.getWidth() - 12 - ctrlW - 8;
        int gh = b.getHeight() - 32;
        if (stepSeqGrid != nullptr)
            stepSeqGrid->setBounds(gx, gy, gw, gh);
    }

    // ── Row 4: Master ─────────────────────────────────────────────────────────
    int r4y = r3y + STEPSEQH + GAP;
    masterPanel.setBounds(GAP, r4y, W - GAP * 2, MASTERH - GAP);
    {
        auto b = masterPanel.getBounds();
        int  ix = b.getX() + 6;
        int  iy = b.getY() + 22;
        int  kw = (b.getWidth() - 12) / 3;
        placeKnobAt(preampKnob, ix, iy, kw);
        placeKnobAt(fxMixKnob, ix + kw, iy, kw);
        placeKnobAt(outputVolumeKnob, ix + kw * 2, iy, kw);
    }
}