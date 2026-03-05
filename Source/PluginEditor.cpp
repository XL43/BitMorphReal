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

    // Linear slider colours (orange thumb / track, dark background)
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
    const float ledSize = 10.0f;
    const float ledX = 6.0f;
    const float ledY = (button.getHeight() - ledSize) * 0.5f;

    if (button.getToggleState())
    {
        g.setColour(ACCENT.withAlpha(0.3f));
        g.fillEllipse(ledX - 4, ledY - 4, ledSize + 8, ledSize + 8);
        g.setColour(ACCENT);
    }
    else { g.setColour(juce::Colour(0xff252525)); }

    g.fillEllipse(ledX, ledY, ledSize, ledSize);
    g.setColour(BORDER_CLR);
    g.drawEllipse(ledX, ledY, ledSize, ledSize, 1.0f);
    g.setColour(button.getToggleState() ? TEXT_LIGHT : TEXT_MUTED);
    g.setFont(10.0f);
    g.drawText(button.getButtonText(),
        (int)(ledX + ledSize + 5), 0,
        button.getWidth() - (int)(ledX + ledSize + 5), button.getHeight(),
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
        float trackH = 3.0f;

        // Track background
        g.setColour(juce::Colour(0xff1a1a1a));
        g.fillRoundedRectangle((float)x, trackY - trackH * 0.5f, (float)width, trackH, 1.5f);

        // Filled portion
        g.setColour(ACCENT);
        g.fillRoundedRectangle((float)x, trackY - trackH * 0.5f,
            sliderPos - (float)x, trackH, 1.5f);

        // Thumb
        float thumbR = 7.0f;
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
    auto name = button.getName();
    auto bounds = button.getLocalBounds().toFloat();
    float cx = bounds.getCentreX();
    float cy = bounds.getCentreY();
    float s = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.52f;

    auto col = button.findColour(juce::TextButton::textColourOffId);
    if (isHighlighted || isDown) col = col.brighter(0.3f);
    g.setColour(col);

    if (name == "dice")
    {
        // Draw a simple dice face (square + dots)
        float sz = s * 0.85f;
        juce::Rectangle<float> face(cx - sz * 0.5f, cy - sz * 0.5f, sz, sz);
        g.drawRoundedRectangle(face, 2.0f, 1.5f);
        float d = sz * 0.22f;
        float r = sz * 0.11f;
        // 5-dot pattern
        auto dot = [&](float dx, float dy) { g.fillEllipse(cx + dx - r, cy + dy - r, r * 2, r * 2); };
        dot(-d, -d); dot(d, -d);
        dot(0, 0);
        dot(-d, d); dot(d, d);
    }
    else if (name == "xarr")
    {
        // Draw crossed arrows (shuffle icon)
        float hs = s * 0.38f;
        float ah = s * 0.18f;   // arrowhead half-width
        g.setColour(col);
        // Arrow 1: top-left → bottom-right
        g.drawLine(cx - hs, cy - hs, cx + hs, cy + hs, 1.5f);
        juce::Path head1;
        head1.addTriangle(cx + hs, cy + hs,
            cx + hs - ah, cy + hs - ah * 0.3f,
            cx + hs - ah * 0.3f, cy + hs - ah);
        g.fillPath(head1);
        // Arrow 2: bottom-left → top-right
        g.drawLine(cx - hs, cy + hs, cx + hs, cy - hs, 1.5f);
        juce::Path head2;
        head2.addTriangle(cx + hs, cy - hs,
            cx + hs - ah, cy - hs + ah * 0.3f,
            cx + hs - ah * 0.3f, cy - hs + ah);
        g.fillPath(head2);
    }
    else if (name == "star")
    {
        // Draw a 5-point star
        juce::Path star;
        const int pts = 5;
        float outerR = s * 0.5f;
        float innerR = s * 0.22f;
        for (int i = 0; i < pts * 2; ++i)
        {
            float angle = juce::MathConstants<float>::pi * i / pts - juce::MathConstants<float>::halfPi;
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
        // Dice with a small musical note hint — just use two dots + staff lines
        float sz = s * 0.8f;
        g.drawRoundedRectangle(cx - sz * 0.5f, cy - sz * 0.5f, sz, sz, 2.0f, 1.5f);
        float r = sz * 0.1f;
        g.fillEllipse(cx - sz * 0.22f - r, cy - r, r * 2, r * 2);
        g.fillEllipse(cx + sz * 0.22f - r, cy - r, r * 2, r * 2);
        g.fillEllipse(cx - r, cy + sz * 0.22f - r, r * 2, r * 2);
    }
    else
    {
        // Default: draw the button text normally
        g.setFont(juce::Font(juce::jmin(bounds.getHeight() * 0.65f, 12.0f)));
        g.drawFittedText(button.getButtonText(),
            button.getLocalBounds(), juce::Justification::centred, 1);
    }
}

// =============================================================================
//  Editor helpers
// =============================================================================
void BitMorphAudioProcessorEditor::setupKnob(KnobSet& k, const juce::String& labelText)
{
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
    int kx = x + (w - 54) / 2;
    k.knob.setBounds(kx, y, 54, 54);
    k.label.setBounds(x, y + 56, w, 14);
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

    // Name buttons so drawButtonText can draw icons
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

    // Bank buttons
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

    // Randomize step seq button
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

    setSize(1200, 840);
}

BitMorphAudioProcessorEditor::~BitMorphAudioProcessorEditor()
{
    stopTimer();
    stepSeqGrid.reset();
    setLookAndFeel(nullptr);
}

// =============================================================================
//  Paint
// =============================================================================
void BitMorphAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(BG_DARK);

    // Title bar
    g.setColour(juce::Colour(0xff080808));
    g.fillRect(0, 0, getWidth(), 60);

    // Orange accent line
    g.setColour(ACCENT);
    g.fillRect(0, 58, getWidth(), 2);

    // Logo
    g.setColour(TEXT_LIGHT);
    g.setFont(juce::Font(20.0f, juce::Font::bold));
    g.drawText("BITMORPH", 16, 0, 200, 58, juce::Justification::centredLeft);

    // Orange dot accent after logo
    g.setColour(ACCENT);
    g.fillEllipse(122, 26, 6, 6);

    // Subtitle
    g.setColour(TEXT_MUTED);
    g.setFont(9.0f);
    g.drawText("evrshade 2026", 0, 0, getWidth() - 14, 58,
        juce::Justification::centredRight);

    // Preset bar background strip
    g.setColour(juce::Colour(0xff050505));
    g.fillRect(0, 40, getWidth(), 20);
}

// =============================================================================
//  Resized  –  4-4-1-1 layout
// =============================================================================
void BitMorphAudioProcessorEditor::resized()
{
    const int W = getWidth();
    const int GAP = 3;
    const int HEADERH = 60;
    const int ROW1H = 185;
    const int ROW2H = 185;
    const int SEQH = 210;
    const int MASTERH = 115;

    // ── Preset bar ────────────────────────────────────────────────────────────
    {
        int y = 40, h = 18, bw = 22, bbw = 44;
        presetPrevBtn.setBounds(4, y, bw, h);
        presetNextBtn.setBounds(4 + bw + 2, y, bw, h);
        presetFaveBtn.setBounds(4 + bw * 2 + 4, y, bw, h);
        presetNameBtn.setBounds(4 + bw * 3 + 6, y, W - (4 + bw * 3 + 6) - bbw * 3 - 26, h);
        presetRandBtn.setBounds(W - bbw * 3 - 22, y, bw, h);
        presetRandParamBtn.setBounds(W - bbw * 3 - 22 + bw + 2, y, bw, h);
        presetSaveBtn.setBounds(W - bbw * 2 - 2, y, bbw, h);
        presetLoadBtn.setBounds(W - bbw, y, bbw, h);
    }

    // ── Row 1: QUANTIZER | DRIVE | RESAMPLER | FILTER ────────────────────────
    {
        int r1y = HEADERH + GAP;
        int pw = W / 4;
        quantizerPanel.setBounds(juce::Rectangle<int>(pw * 0, r1y, pw, ROW1H).reduced(GAP));
        drivePanel.setBounds(juce::Rectangle<int>(pw * 1, r1y, pw, ROW1H).reduced(GAP));
        resamplerPanel.setBounds(juce::Rectangle<int>(pw * 2, r1y, pw, ROW1H).reduced(GAP));
        filterPanel.setBounds(juce::Rectangle<int>(pw * 3, r1y, pw, ROW1H).reduced(GAP));

        // Quantizer
        {
            auto b = quantizerPanel.getBounds();
            int  ix = b.getX() + 6;   int  iy = b.getY() + 28;
            int  kw = (b.getWidth() - 12) / 2;
            placeKnobAt(bitDepthKnob, ix, iy, kw);
            placeKnobAt(ditheringKnob, ix + kw, iy, kw);
            dcShiftBtn.setBounds(ix, iy + 74, b.getWidth() - 12, 18);
        }
        // Drive
        {
            auto b = drivePanel.getBounds();
            int  ix = b.getX() + 6;   int  iy = b.getY() + 28;
            int  kw = (b.getWidth() - 12) / 3;
            int  fw = b.getWidth() - 12;
            placeKnobAt(driveAmountKnob, ix, iy, kw);
            placeKnobAt(driveBiasKnob, ix + kw, iy, kw);
            placeKnobAt(driveMixKnob, ix + kw * 2, iy, kw);
            driveModeLabel.setBounds(ix, iy + 74, 38, 22);
            driveModeCombo.setBounds(ix + 38, iy + 74, fw - 38, 22);
        }
        // Resampler
        {
            auto b = resamplerPanel.getBounds();
            int  ix = b.getX() + 6;   int  iy = b.getY() + 28;
            int  kw = (b.getWidth() - 12) / 4;
            int  fw = b.getWidth() - 12;
            placeKnobAt(resampleFreqKnob, ix, iy, kw);
            placeKnobAt(approxDeviationKnob, ix + kw, iy, kw);
            placeKnobAt(imagesShiftKnob, ix + kw * 2, iy, kw);
            placeKnobAt(jitterKnob, ix + kw * 3, iy, kw);
            approxOnBtn.setBounds(ix, iy + 74, fw, 16);
            imagesOnBtn.setBounds(ix, iy + 74 + 20, fw, 16);
        }
        // Filter
        {
            auto b = filterPanel.getBounds();
            int  ix = b.getX() + 6;   int  iy = b.getY() + 28;
            int  kw = (b.getWidth() - 12) / 2;
            int  fw = b.getWidth() - 12;
            placeKnobAt(filterCutoffKnob, ix, iy, kw);
            placeKnobAt(filterResonanceKnob, ix + kw, iy, kw);
            filterTypeLabel.setBounds(ix, iy + 74, 38, 22);
            filterTypeCombo.setBounds(ix + 38, iy + 74, fw - 38, 22);
            filterOrderLabel.setBounds(ix, iy + 74 + 26, 38, 22);
            filterOrderCombo.setBounds(ix + 38, iy + 74 + 26, fw - 38, 22);
        }
    }

    // ── Row 2: WAVECRUSHER | RING MOD | NOISE | LFO ──────────────────────────
    {
        int r2y = HEADERH + GAP + ROW1H + GAP;
        int pw = W / 4;
        waveCrushPanel.setBounds(juce::Rectangle<int>(pw * 0, r2y, pw, ROW2H).reduced(GAP));
        ringModPanel.setBounds(juce::Rectangle<int>(pw * 1, r2y, pw, ROW2H).reduced(GAP));
        noisePanel.setBounds(juce::Rectangle<int>(pw * 2, r2y, pw, ROW2H).reduced(GAP));
        lfoPanel.setBounds(juce::Rectangle<int>(pw * 3, r2y, pw, ROW2H).reduced(GAP));

        // WaveCrusher
        {
            auto b = waveCrushPanel.getBounds();
            int  ix = b.getX() + 6;   int  iy = b.getY() + 28;
            int  fw = b.getWidth() - 12;
            placeKnobAt(waveCrushAmountKnob, ix, iy, fw);
            waveCrushModeLabel.setBounds(ix, iy + 74, 38, 22);
            waveCrushModeCombo.setBounds(ix + 38, iy + 74, fw - 38, 22);
        }
        // Ring Mod
        {
            auto b = ringModPanel.getBounds();
            int  ix = b.getX() + 6;   int  iy = b.getY() + 28;
            int  kw = (b.getWidth() - 12) / 2;
            placeKnobAt(ringModFreqKnob, ix, iy, kw);
            placeKnobAt(ringModMixKnob, ix + kw, iy, kw);
        }
        // Noise
        {
            auto b = noisePanel.getBounds();
            int  ix = b.getX() + 6;   int  iy = b.getY() + 28;
            int  kw = (b.getWidth() - 12) / 2;
            int  fw = b.getWidth() - 12;
            placeKnobAt(noiseAmountKnob, ix, iy, kw);
            placeKnobAt(noiseColourKnob, ix + kw, iy, kw);
            noiseTypeLabel.setBounds(ix, iy + 74, 38, 22);
            noiseTypeCombo.setBounds(ix + 38, iy + 74, fw - 38, 22);
        }
        // LFO
        {
            auto b = lfoPanel.getBounds();
            int  ix = b.getX() + 6;   int  iy = b.getY() + 28;
            int  kw = (b.getWidth() - 12) / 2;
            int  fw = b.getWidth() - 12;
            placeKnobAt(lfoRateKnob, ix, iy, kw);
            placeKnobAt(lfoDepthKnob, ix + kw, iy, kw);
            lfoWaveformLabel.setBounds(ix, iy + 74, 46, 22);
            lfoWaveformCombo.setBounds(ix + 46, iy + 74, fw - 46, 22);
            lfoTargetLabel.setBounds(ix, iy + 74 + 26, 46, 22);
            lfoTargetCombo.setBounds(ix + 46, iy + 74 + 26, fw - 46, 22);
        }
    }

    // ── Row 3: STEP SEQ ───────────────────────────────────────────────────────
    {
        int r3y = HEADERH + GAP + ROW1H + GAP + ROW2H + GAP;
        int ctrlW = 185;

        stepSeqPanel.setBounds(GAP, r3y, W - GAP * 2, SEQH);
        auto b = stepSeqPanel.getBounds();
        int  ix = b.getX() + 6;
        int  iy = b.getY() + 28;
        int  kw = ctrlW / 3;

        placeKnobAt(stepSeqRateKnob, ix, iy, kw);
        placeKnobAt(stepSeqDepthKnob, ix + kw, iy, kw);
        placeKnobAt(stepSeqSwingKnob, ix + kw * 2, iy, kw);

        int cy = iy + 75;
        stepSeqTargetLabel.setBounds(ix, cy, 36, 20);
        stepSeqTargetCombo.setBounds(ix + 36, cy, ctrlW - 36, 20);
        stepSeqDirLabel.setBounds(ix, cy + 24, 36, 20);
        stepSeqDirCombo.setBounds(ix + 36, cy + 24, ctrlW - 36, 20);

        int sliderY = cy + 50;
        stepSeqLenLabel.setBounds(ix, sliderY, 36, 18);
        stepSeqLenSlider.setBounds(ix + 36, sliderY, ctrlW - 36, 18);

        // Bank buttons + rand step button in one row
        int bankY = sliderY + 24;
        int bw = (ctrlW - 22) / 4;
        bankBtn1.setBounds(ix, bankY, bw - 2, 18);
        bankBtn2.setBounds(ix + bw, bankY, bw - 2, 18);
        bankBtn3.setBounds(ix + bw * 2, bankY, bw - 2, 18);
        bankBtn4.setBounds(ix + bw * 3, bankY, bw - 2, 18);
        randStepBtn.setBounds(ix + bw * 4, bankY, 20, 18);

        // Grid
        int gx = b.getX() + 6 + ctrlW + 8;
        int gy = b.getY() + 26;
        int gw = b.getWidth() - 12 - ctrlW - 8;
        int gh = b.getHeight() - 32;
        if (stepSeqGrid != nullptr)
            stepSeqGrid->setBounds(gx, gy, gw, gh);
    }

    // ── Row 4: MASTER ─────────────────────────────────────────────────────────
    {
        int r4y = HEADERH + GAP + ROW1H + GAP + ROW2H + GAP + SEQH + GAP;
        masterPanel.setBounds(GAP, r4y, W - GAP * 2, MASTERH - GAP);
        auto b = masterPanel.getBounds();
        int  ix = b.getX() + 6;
        int  iy = b.getY() + 28;
        int  kw = (b.getWidth() - 12) / 3;
        placeKnobAt(preampKnob, ix, iy, kw);
        placeKnobAt(fxMixKnob, ix + kw, iy, kw);
        placeKnobAt(outputVolumeKnob, ix + kw * 2, iy, kw);
    }
}