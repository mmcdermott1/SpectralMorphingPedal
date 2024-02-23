// Sampler.cpp
#include <libraries/AudioFile/AudioFile.h>
#include "Sampler.h"

// Constructor taking the path of a file to load
Sampler::Sampler(const std::string &filename, bool loop, bool autostart)
{
    setup(filename, loop, autostart);
}

// Load an audio file from the given filename. Returns true on success.
bool Sampler::setup(const std::string &filename, bool loop, bool autostart)
{
    readPointer_ = 0.0f;
    isPlaying_ = autostart;
    loop_ = loop;

    // Load the file
    sampleBuffer_ = AudioFileUtilities::loadMono(filename);

    // Check for error
    if (sampleBuffer_.empty())
    {
        isPlaying_ = false;
        return false;
    }

    return true;
}

// Tell the buffer to start playing from the beginning
void Sampler::trigger()
{
    if (sampleBuffer_.empty())
        return;
    readPointer_ = 0.0f;
    isPlaying_ = true;
}

// Perform cubic interpolation
float Sampler::cubicInterpolation(float x, float y0, float y1, float y2, float y3)
{
    float a0, a1, a2, a3;   // Coefficients of the cubic polynomial
    float t = x - floor(x); // x is in the range [0, 1]

    // Calculate the coefficients
    a0 = y3 - y2 - y0 + y1;
    a1 = y0 - y1 - a0;
    a2 = y2 - y0;
    a3 = y1;

    return a0 * t * t * t + a1 * t * t + a2 * t + a3; // Return the interpolated value
}

// Return the next sample of the loaded audio file
float Sampler::process(float frequency, float baseFrequency)
// float Sampler::process(float pitchShift)
{
    if (!isPlaying_)
        return 0;

    // Calculate the pitch shift factor based on the desired frequency and base frequency
    float pitchShift = frequency / baseFrequency; // 1.0f is no shift

    // Calculate the read increment based on the pitch shift
    float readIncrement = pitchShift;

    // Calculate the current and next read positions in the buffer
    float currentPos = readPointer_;            // The current position is the read pointer
    float nextPos = currentPos + readIncrement; // The next position is the current position plus the read increment

    // Handle boundary cases (looping or clamping)
    if (nextPos >= sampleBuffer_.size()) // If we reach the end of the buffer
    {
        if (loop_)
            nextPos -= sampleBuffer_.size(); // Loop back to the beginning
        else
            nextPos = sampleBuffer_.size() - 1; // Clamp to the end
    }

    // Retrieve the four neighboring samples for cubic interpolation
    float y0 = sampleBuffer_[static_cast<int>(currentPos)];
    float y1 = sampleBuffer_[static_cast<int>(currentPos) + 1];
    float y2 = sampleBuffer_[static_cast<int>(nextPos)];
    float y3 = sampleBuffer_[static_cast<int>(nextPos) + 1];

    // Perform cubic interpolation
    float out = cubicInterpolation(nextPos, y0, y1, y2, y3); // output interpolated value

    // Increment the read pointer by the pitch-shifted amount
    readPointer_ += readIncrement;

    // If we reach the end, decide whether to loop or stop
    if (readPointer_ >= sampleBuffer_.size())
    {
        readPointer_ = 0;
        if (!loop_)
            isPlaying_ = false;
    }

    return out;
}
