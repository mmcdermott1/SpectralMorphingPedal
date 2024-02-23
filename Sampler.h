// Sampler.h
#pragma once

#include <vector>
#include <string>
#include <cmath>

class Sampler
{
public:
    // Constructors: the one with arguments automatically calls setup()
    Sampler() {}
    Sampler(const std::string &filename, bool loop = true, bool autostart = true);

    // Load an audio file from the given filename. Returns true on success.
    bool setup(const std::string &filename, bool loop = true, bool autostart = true);

    // Start or stop the playback
    void trigger();
    void stop() { isPlaying_ = false; }

    // Return the length of the buffer in samples
    unsigned int size() { return sampleBuffer_.size(); }

    // Return the next sample of the loaded audio file
    float process(float frequency, float baseFrequency);
    // float process(float pitchShift);

    // Destructor
    ~Sampler() {}

private:
    std::vector<float> sampleBuffer_; // Buffer that holds the sound file
    float readPointer_ = 0.0f;        // Position of the last frame we played
    bool loop_ = false;               // Whether the playback loops at the end
    bool isPlaying_ = false;          // Whether we are currently playing

    // Perform cubic interpolation
    float cubicInterpolation(float x, float y0, float y1, float y2, float y3);
};