// EnvelopeFollower.cpp
#include "EnvelopeFollower.h"

EnvelopeFollower::EnvelopeFollower(float attack, float release, float smoothing, float sr)
    : attackTime(attack), releaseTime(release), smoothingTime(smoothing), sampleRate(sr)
{
    calculateGainFactors(); // calculate the attack and release gain factors
    envelope = 0.0;         // initialize the envelope to 0
    smoothedEnvelope = 0.0; // initialize the smoothed envelope to 0
}

void EnvelopeFollower::setAttackTime(float attack) // set the attack time
{
    attackTime = attack;
    calculateGainFactors();
}

void EnvelopeFollower::setReleaseTime(float release) // set the release time
{
    releaseTime = release;
    calculateGainFactors();
}

void EnvelopeFollower::setSmoothingTime(float smoothing) // set the smoothing time
{
    smoothingTime = smoothing;
    calculateGainFactors();
}

void EnvelopeFollower::calculateGainFactors() // calculate the attack and release gain factors
{
    attackGain = 1.0 - std::exp(-1.0 / (sampleRate * attackTime * 0.001));
    releaseGain = 1.0 - std::exp(-1.0 / (sampleRate * releaseTime * 0.001));
    smoothingGain = 1.0 - std::exp(-1.0 / (sampleRate * smoothingTime * 0.001));
}

float EnvelopeFollower::process(float input) // process the input sample
{
    float absInput = std::fabs(input);                            // get the absolute value of the input
    if (absInput > envelope)                                      // if the absolute value of the input is greater than the envelope
        envelope = attackGain * (absInput - envelope) + envelope; // attack phase
    else
        envelope = releaseGain * (absInput - envelope) + envelope; // release phase

    smoothedEnvelope = smoothingGain * (envelope - smoothedEnvelope) + smoothedEnvelope; // smooth the envelope

    return smoothedEnvelope; // return the smoothed envelope
}
