#include "PluginEditor.h"

// =============================================================================
//  LookAndFeel
// =============================================================================
BitMorphLookAndFeel::BitMorphLookAndFeel()
{
    setColour(juce::Slider::textBoxTextColourId, TEXT_LIGHT);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0x00000000));
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0x00000000));
    setColour(juce::ComboBox::backgroundColourId, BG_PANEL);
    setColour(juce::ComboBox::textColourId, TEXT_LIGHT);
    setColour(juce::ComboBox::outlineColourId, BORDER_CLR);
    setColour(juce::ComboBox::arrowColourId, ACCENT);
    setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff0d0d0d));
    setColour(juce::PopupMenu::textColourId, TEXT_LIGHT);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, ACCENT);
    setColour(juce::Label::textColourId, TEXT_MUTED);

    setColour(juce::Slider::thumbColourId, ACCENT);
    setColour(juce::Slider::trackColourId, ACCENT);
    setColour(juce::Slider::backgroundColourId, juce::Colour(0xff1a1a1a));
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
    g.fillEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);

    juce::Path trackArc;
    trackArc.addCentredArc(centreX, centreY, radius + 5, radius + 5,
        0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(juce::Colour(0xff1a1a1a));
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

    g.setColour(juce::Colour(0xff2a2a2a));
    g.fillEllipse(centreX - 3.0f, centreY - 3.0f, 6.0f, 6.0f);
}

void BitMorphLookAndFeel::drawToggleButton(juce::Graphics& g,
    juce::ToggleButton& button,
    bool, bool)
{
    // LED size is relative to the button's own height so it scales cleanly
    const float ledSize = button.getHeight() * 0.55f;
    const float ledX = button.getHeight() * 0.3f;
    const float ledY = (button.getHeight() - ledSize) * 0.5f;

    if (button.getToggleState())
    {
        g.setColour(ACCENT.withAlpha(0.3f));
        g.fillEllipse(ledX - ledSize * 0.35f, ledY - ledSize * 0.35f,
            ledSize * 1.7f, ledSize * 1.7f);
        g.setColour(ACCENT);
    }
    else { g.setColour(juce::Colour(0xff252525)); }

    g.fillEllipse(ledX, ledY, ledSize, ledSize);
    g.setColour(BORDER_CLR);
    g.drawEllipse(ledX, ledY, ledSize, ledSize, 1.0f);
    g.setColour(button.getToggleState() ? TEXT_LIGHT : TEXT_MUTED);
    g.setFont(juce::jmax(7.0f, button.getHeight() * 0.55f));
    g.drawText(button.getButtonText(),
        (int)(ledX + ledSize + 4), 0,
        button.getWidth() - (int)(ledX + ledSize + 4), button.getHeight(),
        juce::Justification::centredLeft);
}

void BitMorphLookAndFeel::drawLinearSlider(juce::Graphics& g,
    int x, int y, int width, int height,
    float sliderPos,
    float /*minSliderPos*/, float /*maxSliderPos*/,
    juce::Slider::SliderStyle style,
    juce::Slider& slider)
{
    if (style == juce::Slider::LinearHorizontal)
    {
        float trackY = y + height * 0.5f;
        float trackH = juce::jmax(2.0f, height * 0.2f);

        g.setColour(juce::Colour(0xff1a1a1a));
        g.fillRoundedRectangle((float)x, trackY - trackH * 0.5f, (float)width, trackH, 1.5f);

        g.setColour(ACCENT);
        g.fillRoundedRectangle((float)x, trackY - trackH * 0.5f,
            sliderPos - (float)x, trackH, 1.5f);

        float thumbR = juce::jmax(4.0f, height * 0.4f);
        g.setColour(ACCENT);
        g.fillEllipse(sliderPos - thumbR, trackY - thumbR, thumbR * 2.0f, thumbR * 2.0f);
        g.setColour(BORDER_CLR);
        g.drawEllipse(sliderPos - thumbR, trackY - thumbR, thumbR * 2.0f, thumbR * 2.0f, 1.0f);
    }
    else
    {
        LookAndFeel_V4::drawLinearSlider(g, x, y, width, height,
            sliderPos, 0.0f, 0.0f, style, slider);
    }
}

void BitMorphLookAndFeel::drawButtonText(juce::Graphics& g,
    juce::TextButton& button,
    bool isHighlighted, bool isDown)
{
    auto  name = button.getName();
    auto  bounds = button.getLocalBounds().toFloat();
    float cx = bounds.getCentreX();
    float cy = bounds.getCentreY();
    float s = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.52f;

    auto col = button.findColour(juce::TextButton::textColourOffId);
    if (isHighlighted || isDown) col = col.brighter(0.3f);
    g.setColour(col);

    if (name == "dice")
    {
        float sz = s * 0.85f;
        juce::Rectangle<float> face(cx - sz * 0.5f, cy - sz * 0.5f, sz, sz);
        g.drawRoundedRectangle(face, 2.0f, 1.5f);
        float d = sz * 0.22f;
        float r = sz * 0.11f;
        auto dot = [&](float dx, float dy) { g.fillEllipse(cx + dx - r, cy + dy - r, r * 2, r * 2); };
        dot(-d, -d); dot(d, -d);
        dot(0, 0);
        dot(-d, d);  dot(d, d);
    }
    else if (name == "xarr")
    {
        float hs = s * 0.38f;
        float ah = s * 0.18f;
        g.drawLine(cx - hs, cy - hs, cx + hs, cy + hs, 1.5f);
        juce::Path head1;
        head1.addTriangle(cx + hs, cy + hs,
            cx + hs - ah, cy + hs - ah * 0.3f,
            cx + hs - ah * 0.3f, cy + hs - ah);
        g.fillPath(head1);
        g.drawLine(cx - hs, cy + hs, cx + hs, cy - hs, 1.5f);
        juce::Path head2;
        head2.addTriangle(cx + hs, cy - hs,
            cx + hs - ah, cy - hs + ah * 0.3f,
            cx + hs - ah * 0.3f, cy - hs + ah);
        g.fillPath(head2);
    }
    else if (name == "star")
    {
        juce::Path star;
        const int pts = 5;
        float     outerR = s * 0.5f;
        float     innerR = s * 0.22f;
        for (int i = 0; i < pts * 2; ++i)
        {
            float angle = juce::MathConstants<float>::pi * i / pts
                - juce::MathConstants<float>::halfPi;
            float r2 = (i % 2 == 0) ? outerR : innerR;
            float px = cx + r2 * std::cos(angle);
            float py = cy + r2 * std::sin(angle);
            if (i == 0) star.startNewSubPath(px, py);
            else        star.lineTo(px, py);
        }
        star.closeSubPath();
        g.fillPath(star);
    }
    else if (name == "randstep")
    {
        float sz = s * 0.8f;
        g.drawRoundedRectangle(cx - sz * 0.5f, cy - sz * 0.5f, sz, sz, 2.0f, 1.5f);
        float r = sz * 0.1f;
        g.fillEllipse(cx - sz * 0.22f - r, cy - r, r * 2, r * 2);
        g.fillEllipse(cx + sz * 0.22f - r, cy - r, r * 2, r * 2);
        g.fillEllipse(cx - r, cy + sz * 0.22f - r, r * 2, r * 2);
    }
    else
    {
        // No font cap — size scales with the button's own height
        g.setFont(juce::Font(bounds.getHeight() * 0.60f));
        g.drawFittedText(button.getButtonText(),
            button.getLocalBounds(), juce::Justification::centred, 1);
    }
}

juce::Font BitMorphLookAndFeel::getComboBoxFont(juce::ComboBox& box)
{
    // Scales with the combo's rendered height so text stays proportional
    // to knob labels and section headers at any window size.
    return juce::Font(juce::jmax(9.0f, box.getHeight() * 0.58f));
}

// =============================================================================
//  Editor helpers
// =============================================================================
void BitMorphAudioProcessorEditor::setupKnob(KnobSet& k, const juce::String& labelText)
{
    // Font sizes are set to a neutral default here; placeKnobAt() updates them
    // each time resized() runs so they always match the current scale.
    k.knob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 16);
    k.label.setText(labelText, juce::dontSendNotification);
    k.label.setJustificationType(juce::Justification::centred);
    k.label.setFont(11.0f);
    k.label.setColour(juce::Label::textColourId, TEXT_MUTED);
    addAndMakeVisible(k.knob);
    addAndMakeVisible(k.label);
}

void BitMorphAudioProcessorEditor::placeKnobAt(KnobSet& k, int x, int y, int w)
{
    const float s = sc();
    const int   kSz = juce::jmax(20, (int)(54.0f * s));   // knob diameter, min 20 px
    const int   lblH = juce::jmax(10, (int)(14.0f * s));
    const int   txH = juce::jmax(12, (int)(16.0f * s));

    int kx = x + (w - kSz) / 2;
    int kGap = juce::jmax(1, (int)(2.0f * s));   // small gap between knob bounds and label
    k.knob.setBounds(kx, y, kSz, kSz);
    k.knob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, kSz, txH);
    // The text box is rendered INSIDE kSz×kSz by JUCE, so the label goes just
    // below the slider bounds — not below bounds+txH, which was 16px too low.
    k.label.setBounds(x, y + kSz + kGap, w, lblH);
    k.label.setFont(juce::jmax(7.0f, 11.0f * s));
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

// =============================================================================
//  Constructor
// =============================================================================
BitMorphAudioProcessorEditor::BitMorphAudioProcessorEditor(BitMorphAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&lookAndFeel);
    auto& apvts = audioProcessor.apvts;

    // ── Resize setup ──────────────────────────────────────────────────────────
    // Aspect ratio locked to 5:3 (1200×720). Min 50%, max 200% of design size.
    resizeConstrainer.setFixedAspectRatio((double)DESIGN_W / (double)DESIGN_H);
    resizeConstrainer.setMinimumSize(600, 360);
    resizeConstrainer.setMaximumSize(2400, 1440);
    setResizable(true, &resizeConstrainer);

    loadFavorites();

    // ── Preset bar ────────────────────────────────────────────────────────────
    auto styleBtn = [](juce::TextButton& b, juce::Colour bg, juce::Colour fg)
        {
            b.setColour(juce::TextButton::buttonColourId, bg);
            b.setColour(juce::TextButton::textColourOffId, fg);
        };
    styleBtn(presetPrevBtn, juce::Colour(0xff111111), TEXT_LIGHT);
    styleBtn(presetNextBtn, juce::Colour(0xff111111), TEXT_LIGHT);
    styleBtn(presetNameBtn, juce::Colour(0xff111111), TEXT_LIGHT);
    styleBtn(presetSaveBtn, ACCENT, TEXT_LIGHT);
    styleBtn(presetLoadBtn, juce::Colour(0xff1a1a1a), TEXT_LIGHT);
    styleBtn(presetRandBtn, juce::Colour(0xff1a1a1a), TEXT_MUTED);
    styleBtn(presetRandParamBtn, juce::Colour(0xff1a1a1a), ACCENT);
    styleBtn(presetFaveBtn, juce::Colour(0xff1a1a1a), TEXT_MUTED);

    presetRandBtn.setName("dice");
    presetRandParamBtn.setName("xarr");
    presetFaveBtn.setName("star");
    randStepBtn.setName("randstep");

    addAndMakeVisible(presetPrevBtn);
    addAndMakeVisible(presetNextBtn);
    addAndMakeVisible(presetNameBtn);
    addAndMakeVisible(presetFaveBtn);
    addAndMakeVisible(presetRandBtn);
    addAndMakeVisible(presetRandParamBtn);
    addAndMakeVisible(presetSaveBtn);
    addAndMakeVisible(presetLoadBtn);

    presetPrevBtn.onClick = [this] {
        if (allPresets.isEmpty()) return;
        currentPresetIndex = (currentPresetIndex <= 0) ? allPresets.size() - 1 : currentPresetIndex - 1;
        loadPresetByIndex(currentPresetIndex);
        };
    presetNextBtn.onClick = [this] {
        if (allPresets.isEmpty()) return;
        currentPresetIndex = (currentPresetIndex >= allPresets.size() - 1) ? 0 : currentPresetIndex + 1;
        loadPresetByIndex(currentPresetIndex);
        };
    presetNameBtn.onClick = [this] { showPresetMenu(); };
    presetFaveBtn.onClick = [this] { toggleFavorite(); };
    presetSaveBtn.onClick = [this] { savePreset(); };
    presetRandBtn.onClick = [this] { randomPreset(); };
    presetRandParamBtn.onClick = [this] { randomizeParameters(); };
    presetLoadBtn.onClick = [this]
        {
            auto chooser = std::make_shared<juce::FileChooser>("Load Preset", getPresetsFolder(), "*.xml");
            chooser->launchAsync(juce::FileBrowserComponent::openMode |
                juce::FileBrowserComponent::canSelectFiles,
                [this, chooser](const juce::FileChooser& fc)
                {
                    auto result = fc.getResult();
                    if (!result.existsAsFile()) return;
                    applyXmlPreset(result);
                    refreshPresetList();
                    for (int i = 0; i < allPresets.size(); ++i)
                        if (allPresets[i].file == result) { currentPresetIndex = i; break; }
                    updatePresetLabel();
                });
        };

    refreshPresetList();
    updateFaveButton();

    // ── Section panels ────────────────────────────────────────────────────────
    addAndMakeVisible(quantizerPanel);
    addAndMakeVisible(drivePanel);
    addAndMakeVisible(resamplerPanel);
    addAndMakeVisible(filterPanel);
    addAndMakeVisible(waveCrushPanel);
    addAndMakeVisible(ringModPanel);
    addAndMakeVisible(noisePanel);
    addAndMakeVisible(lfoPanel);
    addAndMakeVisible(stepSeqPanel);
    addAndMakeVisible(masterPanel);

    // ── Quantizer ─────────────────────────────────────────────────────────────
    setupKnob(bitDepthKnob, "Bit Depth");
    setupKnob(ditheringKnob, "Dithering");
    addAndMakeVisible(dcShiftBtn);
    bitDepthAtt = std::make_unique<SliderAtt>(apvts, ParamID::BIT_DEPTH, bitDepthKnob.knob);
    ditheringAtt = std::make_unique<SliderAtt>(apvts, ParamID::DITHERING, ditheringKnob.knob);
    bitDepthOnAtt = std::make_unique<ButtonAtt>(apvts, ParamID::BIT_DEPTH_ENABLED, quantizerPanel.toggleBtn);
    dcShiftAtt = std::make_unique<ButtonAtt>(apvts, ParamID::DC_SHIFT, dcShiftBtn);

    // ── Drive ─────────────────────────────────────────────────────────────────
    setupKnob(driveAmountKnob, "Drive");
    setupKnob(driveBiasKnob, "Bias");
    setupKnob(driveMixKnob, "Mix");
    setupCombo(driveModeCombo, driveModeLabel, "Mode", { "Soft","Hard","Tube","Foldback" });
    driveAmountAtt = std::make_unique<SliderAtt>(apvts, ParamID::DRIVE_AMOUNT, driveAmountKnob.knob);
    driveBiasAtt = std::make_unique<SliderAtt>(apvts, ParamID::DRIVE_BIAS, driveBiasKnob.knob);
    driveMixAtt = std::make_unique<SliderAtt>(apvts, ParamID::DRIVE_MIX, driveMixKnob.knob);
    driveEnabledAtt = std::make_unique<ButtonAtt>(apvts, ParamID::DRIVE_ENABLED, drivePanel.toggleBtn);
    driveModeAtt = std::make_unique<ComboAtt>(apvts, ParamID::DRIVE_MODE, driveModeCombo);

    // ── Resampler ─────────────────────────────────────────────────────────────
    setupKnob(resampleFreqKnob, "Frequency");
    setupKnob(approxDeviationKnob, "Deviation");
    setupKnob(imagesShiftKnob, "Img Shift");
    setupKnob(jitterKnob, "Jitter");
    addAndMakeVisible(approxOnBtn);
    addAndMakeVisible(imagesOnBtn);
    resampleFreqAtt = std::make_unique<SliderAtt>(apvts, ParamID::RESAMPLE_FREQ, resampleFreqKnob.knob);
    approxDeviationAtt = std::make_unique<SliderAtt>(apvts, ParamID::APPROX_DEVIATION, approxDeviationKnob.knob);
    imagesShiftAtt = std::make_unique<SliderAtt>(apvts, ParamID::IMAGES_SHIFT, imagesShiftKnob.knob);
    jitterAtt = std::make_unique<SliderAtt>(apvts, ParamID::JITTER, jitterKnob.knob);
    resampleOnAtt = std::make_unique<ButtonAtt>(apvts, ParamID::RESAMPLE_ENABLED, resamplerPanel.toggleBtn);
    approxOnAtt = std::make_unique<ButtonAtt>(apvts, ParamID::APPROX_ENABLED, approxOnBtn);
    imagesOnAtt = std::make_unique<ButtonAtt>(apvts, ParamID::IMAGES_ENABLED, imagesOnBtn);

    // ── Filter ────────────────────────────────────────────────────────────────
    setupKnob(filterCutoffKnob, "Cutoff");
    setupKnob(filterResonanceKnob, "Resonance");
    setupCombo(filterTypeCombo, filterTypeLabel, "Type",
        { "Off","Low Pass","Band Pass","High Pass","Band Reject" });
    setupCombo(filterOrderCombo, filterOrderLabel, "Order", { "Post","Pre" });
    filterCutoffAtt = std::make_unique<SliderAtt>(apvts, ParamID::FILTER_CUTOFF, filterCutoffKnob.knob);
    filterResonanceAtt = std::make_unique<SliderAtt>(apvts, ParamID::FILTER_RESONANCE, filterResonanceKnob.knob);
    filterTypeAtt = std::make_unique<ComboAtt>(apvts, ParamID::FILTER_TYPE, filterTypeCombo);
    filterOrderAtt = std::make_unique<ComboAtt>(apvts, ParamID::FILTER_ORDER, filterOrderCombo);

    // ── WaveCrusher ───────────────────────────────────────────────────────────
    setupKnob(waveCrushAmountKnob, "Amount");
    setupCombo(waveCrushModeCombo, waveCrushModeLabel, "Mode", { "Fold","Wrap","Tanh" });
    waveCrushAmountAtt = std::make_unique<SliderAtt>(apvts, ParamID::WAVE_CRUSH_AMOUNT, waveCrushAmountKnob.knob);
    waveCrushOnAtt = std::make_unique<ButtonAtt>(apvts, ParamID::WAVE_CRUSH_ENABLED, waveCrushPanel.toggleBtn);
    waveCrushModeAtt = std::make_unique<ComboAtt>(apvts, ParamID::WAVE_CRUSH_MODE, waveCrushModeCombo);

    // ── Ring Mod ──────────────────────────────────────────────────────────────
    setupKnob(ringModFreqKnob, "Frequency");
    setupKnob(ringModMixKnob, "Mix");
    ringModFreqAtt = std::make_unique<SliderAtt>(apvts, ParamID::RING_MOD_FREQ, ringModFreqKnob.knob);
    ringModMixAtt = std::make_unique<SliderAtt>(apvts, ParamID::RING_MOD_MIX, ringModMixKnob.knob);
    ringModOnAtt = std::make_unique<ButtonAtt>(apvts, ParamID::RING_MOD_ENABLED, ringModPanel.toggleBtn);

    // ── Noise ─────────────────────────────────────────────────────────────────
    setupKnob(noiseAmountKnob, "Amount");
    setupKnob(noiseColourKnob, "Colour");
    setupCombo(noiseTypeCombo, noiseTypeLabel, "Type", { "White","Pink","Vinyl","Bitcrush" });
    noiseAmountAtt = std::make_unique<SliderAtt>(apvts, ParamID::NOISE_AMOUNT, noiseAmountKnob.knob);
    noiseColourAtt = std::make_unique<SliderAtt>(apvts, ParamID::NOISE_COLOUR, noiseColourKnob.knob);
    noiseEnabledAtt = std::make_unique<ButtonAtt>(apvts, ParamID::NOISE_ENABLED, noisePanel.toggleBtn);
    noiseTypeAtt = std::make_unique<ComboAtt>(apvts, ParamID::NOISE_TYPE, noiseTypeCombo);

    // ── LFO ───────────────────────────────────────────────────────────────────
    setupKnob(lfoRateKnob, "Rate");
    setupKnob(lfoDepthKnob, "Depth");
    setupCombo(lfoWaveformCombo, lfoWaveformLabel, "Waveform",
        { "Sine","Triangle","Square","Saw Up","Saw Down","Random" });
    setupCombo(lfoTargetCombo, lfoTargetLabel, "Target",
        { "Bit Depth","Sample Rate","WaveCrush","Ring Freq" });
    lfoRateAtt = std::make_unique<SliderAtt>(apvts, ParamID::LFO_RATE, lfoRateKnob.knob);
    lfoDepthAtt = std::make_unique<SliderAtt>(apvts, ParamID::LFO_DEPTH, lfoDepthKnob.knob);
    lfoWaveformAtt = std::make_unique<ComboAtt>(apvts, ParamID::LFO_WAVEFORM, lfoWaveformCombo);
    lfoTargetAtt = std::make_unique<ComboAtt>(apvts, ParamID::LFO_TARGET, lfoTargetCombo);

    // ── Step Sequencer ────────────────────────────────────────────────────────
    setupKnob(stepSeqRateKnob, "Rate");
    setupKnob(stepSeqDepthKnob, "Depth");
    setupKnob(stepSeqSwingKnob, "Swing");
    setupCombo(stepSeqTargetCombo, stepSeqTargetLabel, "Target",
        { "Bit Depth","Sample Rate","WaveCrush","Ring Freq" });
    setupCombo(stepSeqDirCombo, stepSeqDirLabel, "Dir",
        { "Forward","Reverse","Ping-Pong","Random" });

    stepSeqLenLabel.setText("Steps", juce::dontSendNotification);
    stepSeqLenLabel.setFont(10.0f);
    stepSeqLenLabel.setColour(juce::Label::textColourId, TEXT_MUTED);
    stepSeqLenSlider.setRange(1.0, 16.0, 1.0);
    stepSeqLenSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 28, 18);
    addAndMakeVisible(stepSeqLenLabel);
    addAndMakeVisible(stepSeqLenSlider);

    auto styleBankBtn = [&](juce::TextButton& btn, int idx)
        {
            btn.setColour(juce::TextButton::buttonColourId,
                (idx == 0) ? ACCENT : juce::Colour(0xff1a1a1a));
            btn.setColour(juce::TextButton::textColourOffId,
                (idx == 0) ? TEXT_LIGHT : TEXT_MUTED);
            addAndMakeVisible(btn);
        };
    styleBankBtn(bankBtn1, 0); styleBankBtn(bankBtn2, 1);
    styleBankBtn(bankBtn3, 2); styleBankBtn(bankBtn4, 3);
    bankBtn1.onClick = [this] { selectBank(0); };
    bankBtn2.onClick = [this] { selectBank(1); };
    bankBtn3.onClick = [this] { selectBank(2); };
    bankBtn4.onClick = [this] { selectBank(3); };

    randStepBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1a1a1a));
    randStepBtn.setColour(juce::TextButton::textColourOffId, ACCENT);
    randStepBtn.onClick = [this] { randomizeStepSeq(); };
    addAndMakeVisible(randStepBtn);

    stepSeqRateAtt = std::make_unique<SliderAtt>(apvts, ParamID::STEP_SEQ_RATE, stepSeqRateKnob.knob);
    stepSeqDepthAtt = std::make_unique<SliderAtt>(apvts, ParamID::STEP_SEQ_DEPTH, stepSeqDepthKnob.knob);
    stepSeqSwingAtt = std::make_unique<SliderAtt>(apvts, ParamID::STEP_SEQ_SWING, stepSeqSwingKnob.knob);
    stepSeqLenAtt = std::make_unique<SliderAtt>(apvts, ParamID::STEP_SEQ_LENGTH, stepSeqLenSlider);
    stepSeqOnAtt = std::make_unique<ButtonAtt>(apvts, ParamID::STEP_SEQ_ENABLED, stepSeqPanel.toggleBtn);
    stepSeqTargetAtt = std::make_unique<ComboAtt>(apvts, ParamID::STEP_SEQ_TARGET, stepSeqTargetCombo);
    stepSeqDirAtt = std::make_unique<ComboAtt>(apvts, ParamID::STEP_SEQ_DIR, stepSeqDirCombo);

    stepSeqGrid = std::make_unique<StepSequencerGrid>(apvts);
    addAndMakeVisible(*stepSeqGrid);

    // ── Master ────────────────────────────────────────────────────────────────
    setupKnob(preampKnob, "Preamp");
    setupKnob(fxMixKnob, "FX Mix");
    setupKnob(outputVolumeKnob, "Output");
    preampAtt = std::make_unique<SliderAtt>(apvts, ParamID::PREAMP_GAIN, preampKnob.knob);
    fxMixAtt = std::make_unique<SliderAtt>(apvts, ParamID::FX_MIX, fxMixKnob.knob);
    outputVolumeAtt = std::make_unique<SliderAtt>(apvts, ParamID::OUTPUT_VOLUME, outputVolumeKnob.knob);

    setSize(DESIGN_W, DESIGN_H);   // 1200×720
}

BitMorphAudioProcessorEditor::~BitMorphAudioProcessorEditor()
{
    stopTimer();
    stepSeqGrid.reset();
    setLookAndFeel(nullptr);
}

// =============================================================================
//  Paint  —  all values multiplied by sc() so they track window size
// =============================================================================
void BitMorphAudioProcessorEditor::paint(juce::Graphics& g)
{
    const float s = sc();

    g.fillAll(BG_DARK);

    // Title bar
    g.setColour(juce::Colour(0xff080808));
    g.fillRect(0, 0, getWidth(), (int)(60 * s));

    // Orange accent line under header
    g.setColour(ACCENT);
    g.fillRect(0, (int)(58 * s), getWidth(), juce::jmax(1, (int)(2 * s)));

    // Logo
    g.setColour(TEXT_LIGHT);
    g.setFont(juce::Font(20.0f * s, juce::Font::bold));
    g.drawText("BITMORPH", (int)(16 * s), 0, (int)(200 * s), (int)(58 * s),
        juce::Justification::centredLeft);

    // Orange dot accent after logo
    g.setColour(ACCENT);
    g.fillEllipse(122.0f * s, 26.0f * s, 6.0f * s, 6.0f * s);

    // Subtitle
    g.setColour(TEXT_MUTED);
    g.setFont(9.0f * s);
    g.drawText("evrshade 2026", 0, 0, getWidth() - (int)(14 * s), (int)(58 * s),
        juce::Justification::centredRight);

    // Preset bar background strip
    g.setColour(juce::Colour(0xff050505));
    g.fillRect(0, (int)(40 * s), getWidth(), (int)(20 * s));
}

// =============================================================================
//  Resized  —  4-4-1-1 layout, every constant multiplied by sc()
// =============================================================================
void BitMorphAudioProcessorEditor::resized()
{
    const float s = sc();
    const int   W = getWidth();

    // ── Scaled layout constants ───────────────────────────────────────────────
    const int GAP = juce::jmax(1, (int)(3 * s));
    const int HEADERH = (int)(60 * s);
    const int ROW1H = (int)(170 * s);   // was 185 — trimmed to fit DESIGN_H=720
    const int ROW2H = (int)(170 * s);
    const int SEQH = (int)(196 * s);   // was 210
    const int MASTERH = (int)(112 * s);   // was 115
    // Sanity: 170+170+196+112 = 648; 60+648+4*3 = 720 == DESIGN_H ✓

    // Scaled inner-panel offsets (authored at 1x)
    const int M = (int)(6 * s);   // margin from panel edge
    const int HO = (int)(28 * s);   // top of controls inside panel (header + gap)
    const int BK = (int)(74 * s);   // y below knobs for combos / toggles
    const int CH = (int)(22 * s);   // combo / toggle row height
    const int LH = (int)(18 * s);   // label / small control height

    // Scaled combo-label column width
    const int CL38 = (int)(38 * s);
    const int CL46 = (int)(46 * s);
    const int CL36 = (int)(36 * s);

    // ── Preset bar ────────────────────────────────────────────────────────────
    {
        int y = (int)(40 * s);
        int h = (int)(18 * s);
        int bw = (int)(22 * s);
        int bbw = (int)(44 * s);

        presetPrevBtn.setBounds(4, y, bw, h);
        presetNextBtn.setBounds(4 + bw + 2, y, bw, h);
        presetFaveBtn.setBounds(4 + bw * 2 + 4, y, bw, h);
        presetNameBtn.setBounds(4 + bw * 3 + 6, y,
            W - (4 + bw * 3 + 6) - bbw * 3 - (int)(26 * s), h);
        presetRandBtn.setBounds(W - bbw * 3 - (int)(22 * s), y, bw, h);
        presetRandParamBtn.setBounds(W - bbw * 3 - (int)(22 * s) + bw + 2, y, bw, h);
        presetSaveBtn.setBounds(W - bbw * 2 - (int)(8 * s), y, bbw, h);
        presetLoadBtn.setBounds(W - bbw - (int)(6 * s), y, bbw, h);
    }

    // ── Row 1: QUANTIZER | DRIVE | RESAMPLER | FILTER ────────────────────────
    {
        const int r1y = HEADERH + GAP;
        const int pw = W / 4;

        quantizerPanel.setBounds(juce::Rectangle<int>(pw * 0, r1y, pw, ROW1H).reduced(GAP));
        drivePanel.setBounds(juce::Rectangle<int>(pw * 1, r1y, pw, ROW1H).reduced(GAP));
        resamplerPanel.setBounds(juce::Rectangle<int>(pw * 2, r1y, pw, ROW1H).reduced(GAP));
        filterPanel.setBounds(juce::Rectangle<int>(pw * 3, r1y, pw, ROW1H).reduced(GAP));

        // Quantizer
        {
            auto b = quantizerPanel.getBounds();
            int  ix = b.getX() + M;
            int  iy = b.getY() + HO;
            int  kw = (b.getWidth() - M * 2) / 2;
            placeKnobAt(bitDepthKnob, ix, iy, kw);
            placeKnobAt(ditheringKnob, ix + kw, iy, kw);
            dcShiftBtn.setBounds(ix, iy + BK, b.getWidth() - M * 2, LH);
        }
        // Drive
        {
            auto b = drivePanel.getBounds();
            int  ix = b.getX() + M;
            int  iy = b.getY() + HO;
            int  fw = b.getWidth() - M * 2;
            int  kw = fw / 3;
            placeKnobAt(driveAmountKnob, ix, iy, kw);
            placeKnobAt(driveBiasKnob, ix + kw, iy, kw);
            placeKnobAt(driveMixKnob, ix + kw * 2, iy, kw);
            driveModeLabel.setBounds(ix, iy + BK, CL38, CH);
            driveModeCombo.setBounds(ix + CL38, iy + BK, fw - CL38, CH);
            driveModeLabel.setFont(juce::jmax(7.0f, 10.0f * s));
        }
        // Resampler
        {
            auto b = resamplerPanel.getBounds();
            int  ix = b.getX() + M;
            int  iy = b.getY() + HO;
            int  fw = b.getWidth() - M * 2;
            int  kw = fw / 4;
            placeKnobAt(resampleFreqKnob, ix, iy, kw);
            placeKnobAt(approxDeviationKnob, ix + kw, iy, kw);
            placeKnobAt(imagesShiftKnob, ix + kw * 2, iy, kw);
            placeKnobAt(jitterKnob, ix + kw * 3, iy, kw);
            approxOnBtn.setBounds(ix, iy + BK, fw, LH);
            imagesOnBtn.setBounds(ix, iy + BK + (int)(20 * s), fw, LH);
        }
        // Filter
        {
            auto b = filterPanel.getBounds();
            int  ix = b.getX() + M;
            int  iy = b.getY() + HO;
            int  fw = b.getWidth() - M * 2;
            int  kw = fw / 2;
            placeKnobAt(filterCutoffKnob, ix, iy, kw);
            placeKnobAt(filterResonanceKnob, ix + kw, iy, kw);
            filterTypeLabel.setBounds(ix, iy + BK, CL38, CH);
            filterTypeCombo.setBounds(ix + CL38, iy + BK, fw - CL38, CH);
            filterOrderLabel.setBounds(ix, iy + BK + (int)(26 * s), CL38, CH);
            filterOrderCombo.setBounds(ix + CL38, iy + BK + (int)(26 * s), fw - CL38, CH);
            filterTypeLabel.setFont(juce::jmax(7.0f, 10.0f * s));
            filterOrderLabel.setFont(juce::jmax(7.0f, 10.0f * s));
        }
    }

    // ── Row 2: WAVECRUSHER | RING MOD | NOISE | LFO ──────────────────────────
    {
        const int r2y = HEADERH + GAP + ROW1H + GAP;
        const int pw = W / 4;

        waveCrushPanel.setBounds(juce::Rectangle<int>(pw * 0, r2y, pw, ROW2H).reduced(GAP));
        ringModPanel.setBounds(juce::Rectangle<int>(pw * 1, r2y, pw, ROW2H).reduced(GAP));
        noisePanel.setBounds(juce::Rectangle<int>(pw * 2, r2y, pw, ROW2H).reduced(GAP));
        lfoPanel.setBounds(juce::Rectangle<int>(pw * 3, r2y, pw, ROW2H).reduced(GAP));

        // WaveCrusher
        {
            auto b = waveCrushPanel.getBounds();
            int  ix = b.getX() + M;
            int  iy = b.getY() + HO;
            int  fw = b.getWidth() - M * 2;
            placeKnobAt(waveCrushAmountKnob, ix, iy, fw);
            waveCrushModeLabel.setBounds(ix, iy + BK, CL38, CH);
            waveCrushModeCombo.setBounds(ix + CL38, iy + BK, fw - CL38, CH);
            waveCrushModeLabel.setFont(juce::jmax(7.0f, 10.0f * s));
        }
        // Ring Mod
        {
            auto b = ringModPanel.getBounds();
            int  ix = b.getX() + M;
            int  iy = b.getY() + HO;
            int  kw = (b.getWidth() - M * 2) / 2;
            placeKnobAt(ringModFreqKnob, ix, iy, kw);
            placeKnobAt(ringModMixKnob, ix + kw, iy, kw);
        }
        // Noise
        {
            auto b = noisePanel.getBounds();
            int  ix = b.getX() + M;
            int  iy = b.getY() + HO;
            int  fw = b.getWidth() - M * 2;
            int  kw = fw / 2;
            placeKnobAt(noiseAmountKnob, ix, iy, kw);
            placeKnobAt(noiseColourKnob, ix + kw, iy, kw);
            noiseTypeLabel.setBounds(ix, iy + BK, CL38, CH);
            noiseTypeCombo.setBounds(ix + CL38, iy + BK, fw - CL38, CH);
            noiseTypeLabel.setFont(juce::jmax(7.0f, 10.0f * s));
        }
        // LFO
        {
            auto b = lfoPanel.getBounds();
            int  ix = b.getX() + M;
            int  iy = b.getY() + HO;
            int  fw = b.getWidth() - M * 2;
            int  kw = fw / 2;
            placeKnobAt(lfoRateKnob, ix, iy, kw);
            placeKnobAt(lfoDepthKnob, ix + kw, iy, kw);
            lfoWaveformLabel.setBounds(ix, iy + BK, CL46, CH);
            lfoWaveformCombo.setBounds(ix + CL46, iy + BK, fw - CL46, CH);
            lfoTargetLabel.setBounds(ix, iy + BK + (int)(26 * s), CL46, CH);
            lfoTargetCombo.setBounds(ix + CL46, iy + BK + (int)(26 * s), fw - CL46, CH);
            lfoWaveformLabel.setFont(juce::jmax(7.0f, 10.0f * s));
            lfoTargetLabel.setFont(juce::jmax(7.0f, 10.0f * s));
        }
    }

    // ── Row 3: STEP SEQ ───────────────────────────────────────────────────────
    {
        const int r3y = HEADERH + GAP + ROW1H + GAP + ROW2H + GAP;
        const int ctrlW = (int)(185 * s);

        stepSeqPanel.setBounds(GAP, r3y, W - GAP * 2, SEQH);
        auto b = stepSeqPanel.getBounds();
        int  ix = b.getX() + M;
        int  iy = b.getY() + HO;
        int  kw = ctrlW / 3;

        placeKnobAt(stepSeqRateKnob, ix, iy, kw);
        placeKnobAt(stepSeqDepthKnob, ix + kw, iy, kw);
        placeKnobAt(stepSeqSwingKnob, ix + kw * 2, iy, kw);

        int cy = iy + (int)(70 * s);   // was 75 — tightened for SEQH=196
        stepSeqTargetLabel.setBounds(ix, cy, CL36, (int)(20 * s));
        stepSeqTargetCombo.setBounds(ix + CL36, cy, ctrlW - CL36, (int)(20 * s));
        stepSeqDirLabel.setBounds(ix, cy + (int)(24 * s), CL36, (int)(20 * s));
        stepSeqDirCombo.setBounds(ix + CL36, cy + (int)(24 * s), ctrlW - CL36, (int)(20 * s));
        stepSeqTargetLabel.setFont(juce::jmax(7.0f, 10.0f * s));
        stepSeqDirLabel.setFont(juce::jmax(7.0f, 10.0f * s));

        int sliderY = cy + (int)(50 * s);
        int tboxW = juce::jmax(20, (int)(28 * s));
        int tboxH = juce::jmax(12, (int)(18 * s));
        stepSeqLenLabel.setBounds(ix, sliderY, CL36, tboxH);
        stepSeqLenSlider.setBounds(ix + CL36, sliderY, ctrlW - CL36, tboxH);
        stepSeqLenSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, tboxW, tboxH);
        stepSeqLenLabel.setFont(juce::jmax(7.0f, 10.0f * s));

        int bankY = sliderY + (int)(24 * s);
        int bw = (ctrlW - (int)(22 * s)) / 4;
        bankBtn1.setBounds(ix, bankY, bw - 2, LH);
        bankBtn2.setBounds(ix + bw, bankY, bw - 2, LH);
        bankBtn3.setBounds(ix + bw * 2, bankY, bw - 2, LH);
        bankBtn4.setBounds(ix + bw * 3, bankY, bw - 2, LH);
        randStepBtn.setBounds(ix + bw * 4, bankY, (int)(20 * s), LH);

        // Grid
        int gx = b.getX() + M + ctrlW + (int)(8 * s);
        int gy = b.getY() + (int)(26 * s);
        int gw = b.getWidth() - M * 2 - ctrlW - (int)(8 * s);
        int gh = b.getHeight() - (int)(32 * s);
        if (stepSeqGrid != nullptr)
            stepSeqGrid->setBounds(gx, gy, gw, gh);
    }

    // ── Row 4: MASTER ─────────────────────────────────────────────────────────
    {
        const int r4y = HEADERH + GAP + ROW1H + GAP + ROW2H + GAP + SEQH + GAP;
        masterPanel.setBounds(GAP, r4y, W - GAP * 2, MASTERH - GAP);
        auto b = masterPanel.getBounds();
        int  ix = b.getX() + M;
        int  iy = b.getY() + HO;
        int  kw = (b.getWidth() - M * 2) / 3;
        placeKnobAt(preampKnob, ix, iy, kw);
        placeKnobAt(fxMixKnob, ix + kw, iy, kw);
        placeKnobAt(outputVolumeKnob, ix + kw * 2, iy, kw);
    }
}