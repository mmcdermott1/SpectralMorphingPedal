// PitchTracker.cpp
#include "PitchTracker.h"
#include <numeric>
#include <cmath>

PitchTracker::PitchTracker(float sampleRate, unsigned int bufferSize)
    : sampleRate(sampleRate), bufferSize(bufferSize), gCachedInputBufferPointer(0)
{
    gInputBuffer.resize(bufferSize, 0.0f); // Initialize input buffer to 0
}

float PitchTracker::process()
{
    int downsamplingFactor = 2;                                                          // Downsampling factor
    std::vector<float> downsampledSignal = downsample(gInputBuffer, downsamplingFactor); // Downsample input signal

    int size = downsampledSignal.size();                 // Size of downsampled signal
    std::vector<float> diff(size, 0.0);                  // Difference function
    std::vector<float> cumMeanNormalizedDiff(size, 0.0); // Cumulative mean normalized difference function

    // Difference function
    for (int tau = 1; tau < size; tau++) // For each tau
    {
        for (int i = 0; i < size - tau; i++) // For each sample
        {
            float delta = downsampledSignal[i] - downsampledSignal[i + tau]; // Calculate difference
            diff[tau] += delta * delta;                                      // Square difference and add to difference function
        }
    }

    // Cumulative mean normalized difference function
    cumMeanNormalizedDiff[0] = 1;        // Set first value to 1
    for (int tau = 1; tau < size; tau++) // For each tau
    {
        cumMeanNormalizedDiff[tau] = diff[tau] / ((1.0 / tau) * std::accumulate(diff.begin(), diff.begin() + tau, 0.0)); // Calculate CMND
    }

    // Absolute threshold
    int tauMin = 0;                      // Minimum tau
    for (int tau = 1; tau < size; tau++) // For each tau
    {
        if (cumMeanNormalizedDiff[tau] < 0.1) // If CMND is below threshold
        {
            tauMin = tau; // Set tauMin
            break;
        }
    }

    // Parabolic interpolation
    float betterTau;       // Var to store better tau
    if (tauMin < size - 1) // If tauMin is not the last value
    {
        float s0 = cumMeanNormalizedDiff[tauMin - 1];              // s0
        float s1 = cumMeanNormalizedDiff[tauMin];                  // s1
        float s2 = cumMeanNormalizedDiff[tauMin + 1];              // s2
        betterTau = tauMin + (s2 - s0) / (2 * (2 * s1 - s2 - s0)); // Calculate better tau
    }
    else
    {
        betterTau = tauMin; // Set better tau to tauMin
    }

    // Best local estimate
    float pitch;                              // Var to store pitch
    if (cumMeanNormalizedDiff[tauMin] >= 0.1) // If CMND is above threshold
    {
        pitch = -1; // Set pitch to -1
    }
    else // If CMND is below threshold
    {
        pitch = (sampleRate / downsamplingFactor) / betterTau; // Calculate pitch; adjust for downsampled rate
    }

    return pitch; // Return pitch
}

std::vector<float> PitchTracker::downsample(const std::vector<float> &signal, int factor)
{
    std::vector<float> downsampledSignal;           // Downsampled signal
    for (int i = 0; i < signal.size(); i += factor) // For each sample
    {
        downsampledSignal.push_back(signal[i]); // Add sample to downsampled signal
    }
    return downsampledSignal;
}