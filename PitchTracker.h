// PitchTracker.h
#ifndef PITCHTRACKER_H
#define PITCHTRACKER_H

#include <vector>

class PitchTracker
{
public:
    PitchTracker(float sampleRate, unsigned int bufferSize);
    // float process(const std::vector<float> &signal);
    float process();

    void setBufferValue(int index, float value) { gInputBuffer[index] = value; }
    void setBufferPointer(int value) { gCachedInputBufferPointer = value; }
    int getBufferPointer() const { return gCachedInputBufferPointer; }
    unsigned int getBufferSize() const { return bufferSize; }

private:
    float sampleRate;
    unsigned int bufferSize;
    std::vector<float> gInputBuffer;
    int gCachedInputBufferPointer;
    std::vector<float> downsample(const std::vector<float> &signal, int factor);
};

#endif /* PITCHTRACKER_H */
