#pragma once

#include <cmath>
#include <array>
#include <random>
#include <algorithm>

static constexpr float TWO_PI = 6.28318530718f;

// =============================================================================
// OnePoleLP - Simple one-pole lowpass filter
// Used for the Approximative Filter (before resampler) and
// Images Filter (after resampler)
// =============================================================================
class OnePoleLP
{
public:
    void setCutoff(float cutoffHz, float hostSampleRate)
    {
        float omega = TWO_PI * cutoffHz / hostSampleRate;
        omega = std::max(0.0001f, std::min(omega, 3.1415f));
        a = std::exp(-omega);
        b = 1.0f - a;
    }

    float process(float x)
    {
        z = b * x + a * z;
        return z;
    }

    void reset() { z = 0.0f; }

private:
    float a = 0.0f, b = 1.0f, z = 0.0f;
};

// =============================================================================
// BiquadFilter - Resonant filter (LP / HP / BP / BR)
// Used for the main filter section
// =============================================================================
class BiquadFilter
{
public:
    enum class Type { Off, LowPass, HighPass, BandPass, BandReject };

    void prepare(float sampleRate) { sr = sampleRate; reset(); }

    void setParameters(Type t, float cutoffHz, float resonance)
    {
        type = t;
        if (type == Type::Off) return;

        float fc = std::max(20.0f, std::min(cutoffHz, sr * 0.49f));
        float Q = std::max(0.1f, std::min(resonance * 30.0f + 0.707f, 40.0f));
        float w0 = TWO_PI * fc / sr;
        float cosW0 = std::cos(w0);
        float sinW0 = std::sin(w0);
        float alpha = sinW0 / (2.0f * Q);
        float norm = 1.0f / (1.0f + alpha);

        switch (type)
        {
        case Type::LowPass:
            b0 = (1.0f - cosW0) * 0.5f * norm;
            b1 = (1.0f - cosW0) * norm;
            b2 = b0;
            a1 = -2.0f * cosW0 * norm;
            a2 = (1.0f - alpha) * norm;
            break;
        case Type::HighPass:
            b0 = (1.0f + cosW0) * 0.5f * norm;
            b1 = -(1.0f + cosW0) * norm;
            b2 = b0;
            a1 = -2.0f * cosW0 * norm;
            a2 = (1.0f - alpha) * norm;
            break;
        case Type::BandPass:
            b0 = sinW0 * 0.5f * norm;
            b1 = 0.0f;
            b2 = -b0;
            a1 = -2.0f * cosW0 * norm;
            a2 = (1.0f - alpha) * norm;
            break;
        case Type::BandReject:
            b0 = norm;
            b1 = -2.0f * cosW0 * norm;
            b2 = norm;
            a1 = b1;
            a2 = (1.0f - alpha) * norm;
            break;
        default: break;
        }
    }

    float process(float x)
    {
        if (type == Type::Off) return x;
        float y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return y;
    }

    void reset() { z1 = 0.0f; z2 = 0.0f; }

private:
    Type  type = Type::Off;
    float sr = 44100.0f;
    float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
    float a1 = 0.0f, a2 = 0.0f;
    float z1 = 0.0f, z2 = 0.0f;
};

// =============================================================================
// Quantizer - Reduces bit depth
// =============================================================================
class Quantizer
{
public:
    void setBits(float bits)
    {
        levels = std::pow(2.0f, std::max(1.0f, std::min(bits, 24.0f)));
    }

    void setDithering(float d) { ditheringAmount = d; }
    void setDcShift(bool on) { dcShift = on; }
    void setEnabled(bool e) { enabled = e; }
    bool isEnabled()  const { return enabled; }

    float process(float x)
    {
        if (!enabled) return x;

        if (ditheringAmount > 0.0f)
            x += (dist(rng) * 0.5f) * ditheringAmount / levels;

        x = std::max(-1.0f, std::min(x, 1.0f));

        if (dcShift)
            return std::round(x * levels) / levels;
        else
            return (std::floor(x * levels) + 0.5f) / levels;
    }

private:
    float levels = 256.0f;
    float ditheringAmount = 0.0f;
    bool  dcShift = false;
    bool  enabled = true;
    std::mt19937 rng{ std::random_device{}() };
    std::uniform_real_distribution<float> dist{ -1.0f, 1.0f };
};

// =============================================================================
// Resampler - Sample rate reduction (zero-order hold)
// =============================================================================
class Resampler
{
public:
    void setFrequency(float targetFreqHz, float hostSampleRate)
    {
        samplesPerStep = hostSampleRate / std::max(1.0f, targetFreqHz);
    }

    void setEnabled(bool e) { enabled = e; }
    bool isEnabled() const { return enabled; }

    float process(float x)
    {
        if (!enabled) return x;
        counter += 1.0f;
        if (counter >= samplesPerStep)
        {
            counter -= samplesPerStep;
            held = x;
        }
        return held;
    }

    void reset() { counter = 0.0f; held = 0.0f; }

private:
    float samplesPerStep = 1.0f;
    float counter = 0.0f;
    float held = 0.0f;
    bool  enabled = true;
};

// =============================================================================
// WaveCrusher - Waveshaping distortion (Fold / Wrap / Tanh)
// =============================================================================
class WaveCrusher
{
public:
    enum class Mode { Fold = 0, Wrap = 1, Tanh = 2 };

    void setAmount(float a) { amount = a; }
    void setMode(Mode m) { mode = m; }
    void setEnabled(bool e) { enabled = e; }

    float process(float x)
    {
        if (!enabled || amount < 0.001f) return x;

        float driven = x * (1.0f + amount * 8.0f);
        float shaped;

        switch (mode)
        {
        case Mode::Fold: shaped = waveFold(driven);  break;
        case Mode::Wrap: shaped = waveWrap(driven);  break;
        case Mode::Tanh: shaped = std::tanh(driven); break;
        default:         shaped = driven;             break;
        }

        return shaped / (1.0f + amount * 2.0f);
    }

private:
    static float waveFold(float x)
    {
        float v = std::fmod(x + 1.0f, 4.0f);
        if (v < 0.0f) v += 4.0f;
        return (v > 2.0f) ? (4.0f - v - 1.0f) : (v - 1.0f);
    }

    static float waveWrap(float x)
    {
        while (x > 1.0f) x -= 2.0f;
        while (x < -1.0f) x += 2.0f;
        return x;
    }

    float amount = 0.0f;
    Mode  mode = Mode::Fold;
    bool  enabled = true;
};

// =============================================================================
// RingMod - Ring modulator
// =============================================================================
class RingMod
{
public:
    void setSampleRate(float sr) { sampleRate = sr; }
    void setFrequency(float hz) { phaseIncrement = hz / sampleRate; }
    void setMix(float m) { mix = m; }
    void setEnabled(bool e) { enabled = e; }

    float process(float x)
    {
        if (!enabled) return x;
        float carrier = std::sin(phase * TWO_PI);
        phase += phaseIncrement;
        if (phase >= 1.0f) phase -= 1.0f;
        return x * (1.0f - mix) + (x * carrier) * mix;
    }

    void reset() { phase = 0.0f; }

private:
    float sampleRate = 44100.0f;
    float phase = 0.0f;
    float phaseIncrement = 0.0f;
    float mix = 1.0f;
    bool  enabled = false;
};

// =============================================================================
// LFO - Low frequency oscillator with 6 waveforms
// =============================================================================
class LFO
{
public:
    enum class Waveform { Sine = 0, Triangle = 1, Square = 2, SawUp = 3, SawDown = 4, Random = 5 };
    enum class Target { BitDepth = 0, SampleRate = 1, WaveCrush = 2, RingModFreq = 3 };

    void setSampleRate(float sr) { sampleRate = sr; }
    void setRate(float hz) { phaseIncrement = hz / sampleRate; }
    void setDepth(float d) { depth = std::max(0.0f, std::min(d, 1.0f)); }
    void setWaveform(Waveform w) { waveform = w; }
    void setTarget(Target t) { target = t; }
    Target getTarget() const { return target; }

    float tick()
    {
        float value = 0.0f;

        switch (waveform)
        {
        case Waveform::Sine:
            value = std::sin(phase * TWO_PI);
            break;
        case Waveform::Triangle:
            value = (phase < 0.5f) ? (4.0f * phase - 1.0f)
                : (3.0f - 4.0f * phase);
            break;
        case Waveform::Square:
            value = (phase < 0.5f) ? 1.0f : -1.0f;
            break;
        case Waveform::SawUp:
            value = 2.0f * phase - 1.0f;
            break;
        case Waveform::SawDown:
            value = 1.0f - 2.0f * phase;
            break;
        case Waveform::Random:
            if (phase < lastPhase) randomValue = dist(rng);
            value = randomValue;
            break;
        }

        lastPhase = phase;
        phase += phaseIncrement;
        if (phase >= 1.0f) phase -= 1.0f;

        return value * depth;
    }

    void reset() { phase = 0.0f; lastPhase = 0.0f; }

private:
    float    sampleRate = 44100.0f;
    float    phase = 0.0f;
    float    lastPhase = 0.0f;
    float    phaseIncrement = 0.0f;
    float    depth = 0.5f;
    float    randomValue = 0.0f;
    Waveform waveform = Waveform::Sine;
    Target   target = Target::BitDepth;
    std::mt19937 rng{ std::random_device{}() };
    std::uniform_real_distribution<float> dist{ -1.0f, 1.0f };
};

// =============================================================================
// StepSequencer - 16 step rhythmic modulator
// =============================================================================
class StepSequencer
{
public:
    static constexpr int NUM_STEPS = 16;
    enum class Target { BitDepth = 0, SampleRate = 1, WaveCrush = 2, RingModFreq = 3 };

    StepSequencer() { steps.fill(0.0f); }

    void setSampleRate(float sr) { sampleRate = sr; }

    void setTempo(float bpm, float stepsPerBeat = 4.0f)
    {
        float stepsPerSecond = (bpm / 60.0f) * stepsPerBeat;
        samplesPerStep = sampleRate / std::max(0.1f, stepsPerSecond);
    }

    void setStep(int index, float value)
    {
        if (index >= 0 && index < NUM_STEPS)
            steps[index] = std::max(-1.0f, std::min(value, 1.0f));
    }

    void setDepth(float d) { depth = std::max(0.0f, std::min(d, 1.0f)); }
    void setTarget(Target t) { target = t; }
    void setEnabled(bool e) { enabled = e; }
    Target getTarget()   const { return target; }
    int getCurrentStep() const { return currentStep; }

    float tick()
    {
        if (!enabled) return 0.0f;
        sampleCounter += 1.0f;
        if (sampleCounter >= samplesPerStep)
        {
            sampleCounter -= samplesPerStep;
            currentStep = (currentStep + 1) % NUM_STEPS;
        }
        return steps[currentStep] * depth;
    }

    void reset() { sampleCounter = 0.0f; currentStep = 0; }

private:
    float sampleRate = 44100.0f;
    float samplesPerStep = 11025.0f;
    float sampleCounter = 0.0f;
    int   currentStep = 0;
    float depth = 1.0f;
    bool  enabled = false;
    Target target = Target::SampleRate;
    std::array<float, NUM_STEPS> steps;
};

// =============================================================================
// BitMorphProcessor - The full signal chain for one channel
// =============================================================================
class BitMorphProcessor
{
public:
    OnePoleLP    approxFilter;
    OnePoleLP    imagesFilter;
    BiquadFilter mainFilter;
    Quantizer    quantizer;
    Resampler    resampler;
    WaveCrusher  waveCrusher;
    RingMod      ringMod;

    void prepare(float sampleRate)
    {
        sr = sampleRate;
        approxFilter.reset();
        imagesFilter.reset();
        mainFilter.prepare(sampleRate);
        resampler.reset();
        ringMod.setSampleRate(sampleRate);
        ringMod.reset();
    }

    void reset()
    {
        approxFilter.reset();
        imagesFilter.reset();
        mainFilter.reset();
        resampler.reset();
        ringMod.reset();
    }

    void updateFilterCutoffs(float resampleFreqHz, float approxDeviation,
        float imagesShift, bool  approxEnabled,
        bool  imagesEnabled)
    {
        float nyquist = resampleFreqHz * 0.5f;

        if (approxEnabled)
        {
            float devFactor = 1.0f + approxDeviation / 100.0f;
            float approxCutoff = std::max(20.0f, nyquist * devFactor);
            approxFilter.setCutoff(approxCutoff, sr);
        }

        if (imagesEnabled)
        {
            float imagesCutoff = std::max(20.0f, std::min(nyquist * (1.0f + imagesShift), sr * 0.49f));
            imagesFilter.setCutoff(imagesCutoff, sr);
        }
    }

    float process(float x,
        float preampGain,
        bool  approxEnabled,
        bool  imagesEnabled,
        float bitDepth,
        float resampleFreqHz,
        BiquadFilter::Type filterType,
        bool  filterPre,
        float fxMix,
        float outputGain)
    {
        float dry = x;

        // 1. Preamp
        x *= preampGain;

        // 2. Pre-filter
        if (filterPre)
            x = mainFilter.process(x);

        // 3. Approximative filter
        if (approxEnabled)
            x = approxFilter.process(x);

        // 4. Quantizer
        quantizer.setBits(bitDepth);
        if (quantizer.isEnabled())
            x = quantizer.process(x);

        // 5. Resampler
        if (resampler.isEnabled())
            x = resampler.process(x);

        // 6. Images filter
        if (imagesEnabled)
            x = imagesFilter.process(x);

        // 7. Post-filter
        if (!filterPre)
            x = mainFilter.process(x);

        // 8. WaveCrusher
        x = waveCrusher.process(x);

        // 9. Ring mod
        x = ringMod.process(x);

        // 10. FX mix
        x = dry * (1.0f - fxMix) + x * fxMix;

        // 11. Output gain
        x *= outputGain;

        return x;
    }

private:
    float sr = 44100.0f;
};