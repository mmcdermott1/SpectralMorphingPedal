// Morph.h
#ifndef Morph_h
#define Morph_h

#include <libraries/Fft/Fft.h>
#include <vector>

class Morph
{
public:
    Morph(int fftSize, int hopSize, int bufferSize); // constructor
    void setup();
    float wrapPhase(float phaseIn);
    void process_fft();
    float render(float guitarInput, float sampleInput);

    int gHopCounter;
    int gHopSize;
    int gInputBufferPointerGuitar;
    int gInputBufferPointerSample;
    int gCachedInputBufferPointerGuitar;
    int gCachedInputBufferPointerSample;
    float gAlpha; // Ratio of output to input frequency

private:
    Fft gFftGuitar;     // FFT processing object
    Fft gFftSample;     // FFT processing object
    int gFftSize;       // FFT window size in samples
    float gScaleFactor; // How much to scale the output, based on window type and overlap
    int gBufferSize;    // Circular buffer and pointer for assembling a window of samples
    std::vector<float> gInputBufferGuitar;
    std::vector<float> gInputBufferSample;
    std::vector<float> gOutputBuffer; // Circular buffer for collecting the output of the overlap-add process
    int gOutputBufferWritePointer;    // Start the write pointer ahead of the read pointer by at least window + hop, with some margin
    int gOutputBufferReadPointer;
    std::vector<float> gAnalysisWindowBuffer; // Buffer to hold the windows for FFT analysis and synthesis
    std::vector<float> gSynthesisWindowBuffer;
};

#endif