#pragma once

#include <cmath>
#include <array>
#include <random>
#include <algorithm>

static constexpr float TWO_PI = 6.28318530718f;

// =============================================================================
//  OnePoleLP  –  Simple one-pole lowpass filter
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
    float process(float x) { z = b * x + a * z; return z; }
    void  reset() { z = 0.0f; }
private:
    float a = 0.0f, b = 1.0f, z = 0.0f;
};

// =============================================================================
//  BiquadFilter  –  Resonant filter (LP / HP / BP / BR)
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
//  DriveProcessor  –  Pre-crusher saturation (Soft / Hard / Tube / Foldback)
//  Insert before Quantizer in the signal chain.
// =============================================================================
class DriveProcessor
{
public:
    enum class Mode { Soft = 0, Hard = 1, Tube = 2, Foldback = 3 };

    void setEnabled(bool e) { enabled = e; }
    void setAmount(float db) { driveDb = db; }
    void setBias(float b) { bias = b; }
    void setMode(Mode m) { mode = m; }
    void setMix(float m) { mix = m; }

    float process(float x)
    {
        if (!enabled || (driveDb < 0.001f && std::abs(bias) < 0.001f))
            return x;

        float gain = std::pow(10.0f, driveDb / 20.0f);   // dB to linear
        float driven = (x + bias) * gain;
        float shaped = 0.0f;

        switch (mode)
        {
        case Mode::Soft:
            shaped = std::tanh(driven);
            break;

        case Mode::Hard:
            shaped = std::max(-1.0f, std::min(1.0f, driven));
            break;

        case Mode::Tube:
            if (driven >= 0.0f)
                shaped = 1.0f - std::exp(-driven);
            else
                shaped = -std::tanh(-driven * 1.5f);
            break;

        case Mode::Foldback:
        {
            float v = driven;
            for (int i = 0; i < 8; ++i)           // bounded fold
            {
                if (v > 1.0f) v = 2.0f - v;
                else if (v < -1.0f) v = -2.0f - v;
                else break;
            }
            shaped = v;
            break;
        }
        }

        shaped -= bias * gain * 0.5f;                  // remove DC from bias
        return x + mix * (shaped - x);                 // parallel dry/wet
    }

private:
    bool  enabled = false;
    float driveDb = 0.0f;
    float bias = 0.0f;
    float mix = 1.0f;
    Mode  mode = Mode::Soft;
};

// =============================================================================
//  NoiseProcessor  –  Noise injection (White / Pink / Vinyl / Bitcrush)
//  Insert after the main FX chain, before output gain.
// =============================================================================
class NoiseProcessor
{
public:
    enum class Type { White = 0, Pink = 1, Vinyl = 2, Bitcrush = 3 };

    void prepare(float sampleRate) { sr = sampleRate; reset(); }

    void reset()
    {
        for (auto& b : pinkB) b = 0.0f;
        vinylPhase = 0.0f;
        for (auto& f : colourFilter) f.reset();
    }

    void setEnabled(bool e) { enabled = e; }
    void setAmount(float a) { amount = a; }
    void setType(Type t) { noiseType = t; }
    void setColour(float hz) { colourHz = hz; }

    // Call once per sample; returns the input + noise
    float process(float input, int channel)
    {
        if (!enabled || amount < 0.0001f) return input;

        float noise = 0.0f;
        int   ch = (channel < 2) ? channel : 0;

        switch (noiseType)
        {
        case Type::White:
            noise = rng.nextFloat() * 2.0f - 1.0f;
            break;

        case Type::Pink:
        {
            float w = rng.nextFloat() * 2.0f - 1.0f;
            pinkB[0] = 0.99886f * pinkB[0] + w * 0.0555179f;
            pinkB[1] = 0.99332f * pinkB[1] + w * 0.0750759f;
            pinkB[2] = 0.96900f * pinkB[2] + w * 0.1538520f;
            pinkB[3] = 0.86650f * pinkB[3] + w * 0.3104856f;
            pinkB[4] = 0.55000f * pinkB[4] + w * 0.5329522f;
            pinkB[5] = -0.7616f * pinkB[5] - w * 0.0168980f;
            noise = (pinkB[0] + pinkB[1] + pinkB[2] + pinkB[3]
                + pinkB[4] + pinkB[5] + pinkB[6] + w * 0.5362f) * 0.11f;
            pinkB[6] = w * 0.115926f;
            break;
        }

        case Type::Vinyl:
        {
            float w = rng.nextFloat() * 2.0f - 1.0f;
            pinkB[0] = 0.99886f * pinkB[0] + w * 0.0555179f;
            float pink = pinkB[0] * 0.4f;
            vinylPhase += 0.5f / sr;
            float wow = std::sin(vinylPhase * TWO_PI) * 0.012f;
            float crackle = (rng.nextFloat() < 0.0003f)
                ? (rng.nextFloat() * 2.0f - 1.0f) * 0.35f : 0.0f;
            noise = pink + wow + crackle;
            break;
        }

        case Type::Bitcrush:
        {
            float raw = rng.nextFloat() * 2.0f - 1.0f;
            float step = 2.0f / 16.0f;                    // 4-bit
            noise = std::round(raw / step) * step;
            break;
        }
        }

        // Colour filter (lowpass on the noise)
        colourFilter[ch].setCutoff(colourHz, sr);
        noise = colourFilter[ch].process(noise);

        return input + noise * amount;
    }

private:
    struct MinLP
    {
        float z = 0.0f;
        void  reset() { z = 0.0f; }
        void  setCutoff(float hz, float sr2)
        {
            hz = std::max(20.0f, std::min(hz, sr2 * 0.49f));
            coeff = 1.0f - std::exp(-TWO_PI * hz / sr2);
        }
        float process(float x) { z += coeff * (x - z); return z; }
        float coeff = 1.0f;
    };

    bool  enabled = false;
    float amount = 0.0f;
    float colourHz = 8000.0f;
    Type  noiseType = Type::White;
    float sr = 44100.0f;
    float pinkB[7] = {};
    float vinylPhase = 0.0f;
    MinLP colourFilter[2];
    juce::Random rng;
};

// =============================================================================
//  Quantizer  –  Bit-depth reduction
// =============================================================================
class Quantizer
{
public:
    void setBits(float bits) { levels = std::pow(2.0f, std::max(1.0f, std::min(bits, 24.0f))); }
    void setDithering(float d) { ditheringAmount = d; }
    void setDcShift(bool on) { dcShift = on; }
    void setEnabled(bool e) { enabled = e; }
    bool isEnabled() const { return enabled; }

    float process(float x)
    {
        if (!enabled) return x;
        if (ditheringAmount > 0.0f)
            x += (dist(rng) * 0.5f) * ditheringAmount / levels;
        x = std::max(-1.0f, std::min(x, 1.0f));
        return dcShift
            ? std::round(x * levels) / levels
            : (std::floor(x * levels) + 0.5f) / levels;
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
//  Resampler  –  Sample rate reduction with optional clock jitter
// =============================================================================
class Resampler
{
public:
    void setFrequency(float targetFreqHz, float hostSampleRate)
    {
        samplesPerStep = hostSampleRate / std::max(1.0f, targetFreqHz);
    }
    void setJitter(float j) { jitter = j; }
    void setEnabled(bool e) { enabled = e; }
    bool isEnabled() const { return enabled; }

    float process(float x)
    {
        if (!enabled) return x;
        counter += 1.0f;
        float jitterOffset = (jitter > 0.001f)
            ? (rng.nextFloat() * 2.0f - 1.0f) * jitter * 2.0f
            : 0.0f;
        if (counter >= samplesPerStep + jitterOffset)
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
    float jitter = 0.0f;
    bool  enabled = true;
    juce::Random rng;
};

// =============================================================================
//  WaveCrusher  –  Waveshaping distortion (Fold / Wrap / Tanh)
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
//  RingMod  –  Ring modulator
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
//  LFO  –  6 waveforms, 4 targets
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
            value = std::sin(phase * TWO_PI); break;
        case Waveform::Triangle:
            value = (phase < 0.5f) ? (4.0f * phase - 1.0f) : (3.0f - 4.0f * phase); break;
        case Waveform::Square:
            value = (phase < 0.5f) ? 1.0f : -1.0f; break;
        case Waveform::SawUp:
            value = 2.0f * phase - 1.0f; break;
        case Waveform::SawDown:
            value = 1.0f - 2.0f * phase; break;
        case Waveform::Random:
            if (phase < lastPhase) randomValue = dist(rng);
            value = randomValue; break;
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
//  StepSequencer  –  16 steps, 4 directions, swing, variable length
// =============================================================================
class StepSequencer
{
public:
    static constexpr int NUM_STEPS = 16;
    static constexpr int NUM_BANKS = 4;
    enum class Target { BitDepth = 0, SampleRate = 1, WaveCrush = 2, RingModFreq = 3 };
    enum class Direction { Forward = 0, Reverse = 1, PingPong = 2, Random = 3 };

    StepSequencer()
    {
        for (auto& bank : banks) bank.fill(0.0f);
    }

    void setSampleRate(float sr) { sampleRate = sr; }
    void setDepth(float d) { depth = std::max(0.0f, std::min(d, 1.0f)); }
    void setTarget(Target t) { target = t; }
    void setEnabled(bool e) { enabled = e; }
    void setDirection(Direction d) { direction = d; }
    void setLength(int len) { stepLength = std::max(1, std::min(len, NUM_STEPS)); }
    void setActiveBank(int bank) { activeBank = std::max(0, std::min(bank, NUM_BANKS - 1)); }

    void setTempo(float bpm, float stepsPerBeat = 4.0f)
    {
        float stepsPerSecond = (bpm / 60.0f) * stepsPerBeat;
        samplesPerStep = sampleRate / std::max(0.1f, stepsPerSecond);
    }

    void setSwing(float s) { swing = std::max(0.0f, std::min(s, 0.5f)); }

    void setStep(int bank, int index, float value)
    {
        if (bank >= 0 && bank < NUM_BANKS && index >= 0 && index < NUM_STEPS)
            banks[bank][index] = std::max(-1.0f, std::min(value, 1.0f));
    }
    float getStep(int bank, int index) const
    {
        if (bank >= 0 && bank < NUM_BANKS && index >= 0 && index < NUM_STEPS)
            return banks[bank][index];
        return 0.0f;
    }

    Target getTarget()   const { return target; }
    int getCurrentStep() const { return currentStep; }

    float tick()
    {
        if (!enabled) return 0.0f;

        float swingOffset = (currentStep % 2 == 1) ? swing * samplesPerStep : 0.0f;
        sampleCounter += 1.0f;

        if (sampleCounter >= samplesPerStep + swingOffset)
        {
            sampleCounter -= samplesPerStep + swingOffset;
            advanceStep();
        }

        return banks[activeBank][currentStep] * depth;
    }

    void reset() { sampleCounter = 0.0f; currentStep = 0; pingPongDir = 1; }

private:
    void advanceStep()
    {
        switch (direction)
        {
        case Direction::Forward:
            currentStep = (currentStep + 1) % stepLength;
            break;
        case Direction::Reverse:
            currentStep = (currentStep - 1 + stepLength) % stepLength;
            break;
        case Direction::PingPong:
            currentStep += pingPongDir;
            if (currentStep >= stepLength) { currentStep = stepLength - 2; pingPongDir = -1; }
            if (currentStep < 0) { currentStep = 1;              pingPongDir = 1; }
            currentStep = std::max(0, std::min(currentStep, stepLength - 1));
            break;
        case Direction::Random:
            currentStep = rng.nextInt(stepLength);
            break;
        }
    }

    float sampleRate = 44100.0f;
    float samplesPerStep = 11025.0f;
    float sampleCounter = 0.0f;
    float depth = 1.0f;
    float swing = 0.0f;
    int   currentStep = 0;
    int   pingPongDir = 1;
    int   stepLength = NUM_STEPS;
    int   activeBank = 0;
    bool  enabled = false;
    Target    target = Target::SampleRate;
    Direction direction = Direction::Forward;
    std::array<std::array<float, NUM_STEPS>, NUM_BANKS> banks;
    juce::Random rng;
};

// =============================================================================
//  BitMorphProcessor  –  Full signal chain for one channel
//
//  Order: Drive → Quantizer → Resampler → Filter → WaveCrusher → RingMod
//         Then Noise is applied afterwards in PluginProcessor per-sample.
// =============================================================================
class BitMorphProcessor
{
public:
    DriveProcessor drive;
    OnePoleLP      approxFilter;
    OnePoleLP      imagesFilter;
    BiquadFilter   mainFilter;
    Quantizer      quantizer;
    Resampler      resampler;
    WaveCrusher    waveCrusher;
    RingMod        ringMod;
    NoiseProcessor noise;

    void prepare(float sampleRate)
    {
        sr = sampleRate;
        approxFilter.reset();
        imagesFilter.reset();
        mainFilter.prepare(sampleRate);
        resampler.reset();
        ringMod.setSampleRate(sampleRate);
        ringMod.reset();
        noise.prepare(sampleRate);
    }

    void reset()
    {
        approxFilter.reset();
        imagesFilter.reset();
        mainFilter.reset();
        resampler.reset();
        ringMod.reset();
        noise.reset();
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

    // channel: 0 = left, 1 = right  (for stereo noise)
    float process(float x,
        float preampGain,
        bool  approxEnabled,
        bool  imagesEnabled,
        float bitDepth,
        BiquadFilter::Type filterType,
        bool  filterPre,
        float fxMix,
        float outputGain,
        int   channel)
    {
        float dry = x;

        // 1. Preamp
        x *= preampGain;

        // 2. Drive (pre-crush saturation)
        x = drive.process(x);

        // 3. Pre-filter
        if (filterPre)
            x = mainFilter.process(x);

        // 4. Approximative filter
        if (approxEnabled)
            x = approxFilter.process(x);

        // 5. Quantizer
        quantizer.setBits(bitDepth);
        if (quantizer.isEnabled())
            x = quantizer.process(x);

        // 6. Resampler
        if (resampler.isEnabled())
            x = resampler.process(x);

        // 7. Images filter
        if (imagesEnabled)
            x = imagesFilter.process(x);

        // 8. Post-filter
        if (!filterPre)
            x = mainFilter.process(x);

        // 9. WaveCrusher
        x = waveCrusher.process(x);

        // 10. Ring mod
        x = ringMod.process(x);

        // 11. Noise injection
        x = noise.process(x, channel);

        // 12. FX mix
        x = dry * (1.0f - fxMix) + x * fxMix;

        // 13. Output gain
        x *= outputGain;

        return x;
    }

private:
    float sr = 44100.0f;
};