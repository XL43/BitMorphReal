#include "PluginEditor.h"

static const juce::Colour BG_DARK(0xff0d0d1a);
static const juce::Colour BG_PANEL(0xff141428);
static const juce::Colour BG_HEADER(0xff1a1a35);
static const juce::Colour ACCENT(0xffe94560);
static const juce::Colour TEXT_LIGHT(0xffddddee);
static const juce::Colour TEXT_MUTED(0xff888899);
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
    k.label.setFont(10.0f);
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
    setSize(1200, 660);

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

    setupKnob(preampKnob, "Preamp");
    setupKnob(fxMixKnob, "FX Mix");
    setupKnob(outputVolumeKnob, "Output");
    preampAtt = std::make_unique<SliderAtt>(apvts, "preampGain", preampKnob.knob);
    fxMixAtt = std::make_unique<SliderAtt>(apvts, "fxMix", fxMixKnob.knob);
    outputVolumeAtt = std::make_unique<SliderAtt>(apvts, "outputVolume", outputVolumeKnob.knob);


}

BitMorphAudioProcessorEditor::~BitMorphAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void BitMorphAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(BG_DARK);

    g.setColour(juce::Colour(0xff0a0a18));
    g.fillRect(0, 0, getWidth(), 40);

    g.setColour(ACCENT);
    g.fillRect(0, 38, getWidth(), 2);

    g.setColour(TEXT_LIGHT);
    g.setFont(juce::Font(20.0f, juce::Font::bold));
    g.drawText("BITMORPH", 16, 0, 200, 40, juce::Justification::centredLeft);

    g.setColour(TEXT_MUTED);
    g.setFont(9.5f);
    g.drawText("evrshade 2026", 0, 0, getWidth() - 16, 40,
        juce::Justification::centredRight);
}

void BitMorphAudioProcessorEditor::resized()
{
    const int W = getWidth();
    const int GAP = 4;
    const int HEADERH = 40;
    const int MASTERH = 130;
    const int ROWH = (getHeight() - HEADERH - MASTERH - GAP * 3) / 2;

    int r1y = HEADERH + GAP;
    int secW1 = W / 3;

    quantizerPanel.setBounds(juce::Rectangle<int>(0, r1y, secW1, ROWH).reduced(GAP));
    resamplerPanel.setBounds(juce::Rectangle<int>(secW1, r1y, secW1, ROWH).reduced(GAP));
    filterPanel.setBounds(juce::Rectangle<int>(secW1 * 2, r1y, secW1, ROWH).reduced(GAP));

    // Quantizer
    {
        auto area = quantizerPanel.getBounds().reduced(6).withTrimmedTop(22);
        auto knobRow = area.removeFromTop(area.getHeight() - 54);
        auto left = knobRow.removeFromLeft(knobRow.getWidth() / 2);
        placeKnob(bitDepthKnob, left.reduced(8, 4));
        placeKnob(ditheringKnob, knobRow.reduced(8, 4));
        bitDepthOnBtn.setBounds(area.removeFromTop(22).reduced(4, 2));
        dcShiftBtn.setBounds(area.removeFromTop(22).reduced(4, 2));
    }

    // Resampler
    {
        auto area = resamplerPanel.getBounds().reduced(6).withTrimmedTop(22);
        auto knobRow = area.removeFromTop(area.getHeight() - 74);
        auto freqArea = knobRow.removeFromLeft(knobRow.getWidth() / 3);
        auto devArea = knobRow.removeFromLeft(knobRow.getWidth() / 2);
        placeKnob(resampleFreqKnob, freqArea.reduced(6, 4));
        placeKnob(approxDeviationKnob, devArea.reduced(6, 4));
        placeKnob(imagesShiftKnob, knobRow.reduced(6, 4));
        resampleOnBtn.setBounds(area.removeFromTop(22).reduced(4, 2));
        approxOnBtn.setBounds(area.removeFromTop(22).reduced(4, 2));
        imagesOnBtn.setBounds(area.removeFromTop(22).reduced(4, 2));
    }

    // Filter
    {
        auto area = filterPanel.getBounds().reduced(6).withTrimmedTop(22);
        auto knobRow = area.removeFromTop(area.getHeight() - 64);
        auto left = knobRow.removeFromLeft(knobRow.getWidth() / 2);
        placeKnob(filterCutoffKnob, left.reduced(8, 4));
        placeKnob(filterResonanceKnob, knobRow.reduced(8, 4));
        auto typeRow = area.removeFromTop(30);
        filterTypeLabel.setBounds(typeRow.removeFromLeft(40).reduced(0, 4));
        filterTypeCombo.setBounds(typeRow.reduced(2, 4));
        auto orderRow = area.removeFromTop(30);
        filterOrderLabel.setBounds(orderRow.removeFromLeft(40).reduced(0, 4));
        filterOrderCombo.setBounds(orderRow.reduced(2, 4));
    }

    int r2y = r1y + ROWH + GAP;
    int secW2 = W / 4;

    waveCrushPanel.setBounds(juce::Rectangle<int>(0, r2y, secW2, ROWH).reduced(GAP));
    ringModPanel.setBounds(juce::Rectangle<int>(secW2, r2y, secW2, ROWH).reduced(GAP));
    lfoPanel.setBounds(juce::Rectangle<int>(secW2 * 2, r2y, secW2, ROWH).reduced(GAP));
    stepSeqPanel.setBounds(juce::Rectangle<int>(secW2 * 3, r2y, secW2, ROWH).reduced(GAP));

    // WaveCrusher
    {
        auto area = waveCrushPanel.getBounds().reduced(6).withTrimmedTop(22);
        auto knobRow = area.removeFromTop(area.getHeight() - 50);
        placeKnob(waveCrushAmountKnob, knobRow.reduced(20, 4));
        waveCrushOnBtn.setBounds(area.removeFromTop(22).reduced(4, 2));
        auto modeRow = area.removeFromTop(28);
        waveCrushModeLabel.setBounds(modeRow.removeFromLeft(44).reduced(0, 4));
        waveCrushModeCombo.setBounds(modeRow.reduced(2, 4));
    }

    // Ring Mod
    {
        auto area = ringModPanel.getBounds().reduced(6).withTrimmedTop(22);
        auto knobRow = area.removeFromTop(area.getHeight() - 30);
        auto left = knobRow.removeFromLeft(knobRow.getWidth() / 2);
        placeKnob(ringModFreqKnob, left.reduced(6, 4));
        placeKnob(ringModMixKnob, knobRow.reduced(6, 4));
        ringModOnBtn.setBounds(area.removeFromTop(22).reduced(4, 2));
    }

    // LFO
    {
        auto area = lfoPanel.getBounds().reduced(6).withTrimmedTop(22);
        auto knobRow = area.removeFromTop(area.getHeight() - 60);
        auto left = knobRow.removeFromLeft(knobRow.getWidth() / 2);
        placeKnob(lfoRateKnob, left.reduced(6, 4));
        placeKnob(lfoDepthKnob, knobRow.reduced(6, 4));
        auto waveRow = area.removeFromTop(28);
        lfoWaveformLabel.setBounds(waveRow.removeFromLeft(50).reduced(0, 4));
        lfoWaveformCombo.setBounds(waveRow.reduced(2, 4));
        auto targRow = area.removeFromTop(28);
        lfoTargetLabel.setBounds(targRow.removeFromLeft(50).reduced(0, 4));
        lfoTargetCombo.setBounds(targRow.reduced(2, 4));
    }

    // Step Sequencer
    {
        auto area = stepSeqPanel.getBounds().reduced(6).withTrimmedTop(22);
        auto knobRow = area.removeFromTop(area.getHeight() - 54);
        auto left = knobRow.removeFromLeft(knobRow.getWidth() / 2);
        placeKnob(stepSeqRateKnob, left.reduced(6, 4));
        placeKnob(stepSeqDepthKnob, knobRow.reduced(6, 4));
        stepSeqOnBtn.setBounds(area.removeFromTop(22).reduced(4, 2));
        auto targRow = area.removeFromTop(28);
        stepSeqTargetLabel.setBounds(targRow.removeFromLeft(44).reduced(0, 4));
        stepSeqTargetCombo.setBounds(targRow.reduced(2, 4));
    }

    // Master
    int masy = r2y + ROWH + GAP;
    masterPanel.setBounds(GAP, masy, W - GAP * 2, MASTERH - GAP);
    {
        auto area = masterPanel.getBounds().reduced(6).withTrimmedTop(22);
        auto vuArea = area.removeFromRight(60);
        int kw = area.getWidth() / 3;
        placeKnob(preampKnob, area.removeFromLeft(kw).reduced(16, 4));
        placeKnob(fxMixKnob, area.removeFromLeft(kw).reduced(16, 4));
        placeKnob(outputVolumeKnob, area.reduced(16, 4));
    }
}