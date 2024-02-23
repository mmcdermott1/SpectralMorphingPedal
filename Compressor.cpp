// Compressor.cpp
#include "Compressor.h"
#include <algorithm>
#include <cmath>

Compressor::Compressor(float threshold, float ratio, float attackTime, float releaseTime, float kneeWidth, float makeupGain, float sampleRate)
    : threshold_(dBToLinear(threshold)), ratio_(ratio), attackTime_(attackTime), releaseTime_(releaseTime), kneeWidth_(dBToLinear(kneeWidth)), makeupGain_(dBToLinear(makeupGain)), envelope_(0.0f),
      gain_(1.0f), sampleRate_(sampleRate)
{
    // Calculate attack and release coefficients based on sample rate
    updateCoefficients();
}

float Compressor::process(float inputSample)
{
    // Calculate the envelope of the input sample
    float inputMagnitude = std::fabs(inputSample); // get the absolute value of the input
    if (inputMagnitude > envelope_)                // if the absolute value of the input is greater than the envelope
    {
        envelope_ += attackCoefficient_ * (inputMagnitude - envelope_); // attack phase
    }
    else
    {
        envelope_ += releaseCoefficient_ * (inputMagnitude - envelope_); // release phase
    }

    // Calculate desired gain
    float desiredGain;
    if (envelope_ <= threshold_ - kneeWidth_ / 2) // if the envelope is below the threshold
    {
        desiredGain = 1.0f; // no compression
    }
    else if (envelope_ > threshold_ + kneeWidth_ / 2) // if the envelope is above the threshold
    {
        desiredGain = powf(envelope_ / threshold_, -ratio_); // apply compression
    }
    else
    {
        // We're in the 'knee'. interpolate smoothly between uncompressed and compressed.
        float x = (envelope_ - threshold_ + kneeWidth_ / 2) / kneeWidth_; // calculate the x value
        desiredGain = powf(1.0f + (ratio_ - 1.0f) * x * x, -1.0f);        // interpolate between uncompressed and compressed
    }

    // Smooth gain
    gain_ = desiredGain + (attackCoefficient_ * (desiredGain - gain_));

    // Apply makeup gain
    return inputSample * gain_ * makeupGain_; // apply makeup gain
}

void Compressor::setThreshold(float threshold) // set the threshold
{
    threshold_ = dBToLinear(threshold);
}

void Compressor::setRatio(float ratio) // set the ratio
{
    ratio_ = ratio;
}

void Compressor::setAttackTime(float attackTime) // set the attack time
{
    attackTime_ = attackTime;
    updateCoefficients();
}

void Compressor::setReleaseTime(float releaseTime) // set the release time
{
    releaseTime_ = releaseTime;
    updateCoefficients();
}

void Compressor::setKneeWidth(float kneeWidth) // set the knee width
{
    kneeWidth_ = dBToLinear(kneeWidth); // convert dB to linear scale
}

void Compressor::setMakeupGain(float makeupGain) // set the makeup gain
{
    makeupGain_ = dBToLinear(makeupGain);
}

// Convert dB to linear scale
float Compressor::dBToLinear(float dB)
{
    return powf(10.0f, dB / 20.0f); // convert dB to linear scale
}

// Update attack and release coefficients
void Compressor::updateCoefficients()
{
    attackCoefficient_ = 1.0f - std::exp(-1.0f / (attackTime_ * sampleRate_));   // calculate the attack coefficient
    releaseCoefficient_ = 1.0f - std::exp(-1.0f / (releaseTime_ * sampleRate_)); // calculate the release coefficient
}
