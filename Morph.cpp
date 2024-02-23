// Morph.cpp
#include "Morph.h"
#include <cmath>
#include <cstring>
#include <algorithm>

Morph::Morph(int fftSize, int hopSize, int bufferSize)
{
    gFftSize = fftSize;
    gHopSize = hopSize;
    gBufferSize = bufferSize;
    gScaleFactor = 0.5; // How much to scale the output, based on window type and overlap
    gAlpha = 0.5;       // Ratio of output to input frequency
    gInputBufferPointerGuitar = 0;
    gInputBufferPointerSample = 0;
    gHopCounter = 0;
    gOutputBufferWritePointer = gFftSize + 2 * gHopSize; // Start the write pointer ahead of the read pointer by at least window + hop, with some margin
    gOutputBufferReadPointer = 0;
    std::vector<float> gAnalysisWindowBuffer; // Buffer to hold the windows for FFT analysis and synthesis
    std::vector<float> gSynthesisWindowBuffer;
    gCachedInputBufferPointerGuitar = 0;
    gCachedInputBufferPointerSample = 0;
}

void Morph::setup()
{
    // Set up the FFT and its buffers
    gFftGuitar.setup(gFftSize);
    gFftSample.setup(gFftSize);
    gInputBufferGuitar.resize(gBufferSize);
    gInputBufferSample.resize(gBufferSize);
    gOutputBuffer.resize(gBufferSize);

    // Calculate the windows
    gAnalysisWindowBuffer.resize(gFftSize);
    gSynthesisWindowBuffer.resize(gFftSize);
    for (int n = 0; n < gFftSize; n++)
    {
        // Hann window, split across analysis and synthesis windows
        gAnalysisWindowBuffer[n] = 0.5f * (1.0f - cosf(2.0 * M_PI * n / (float)(gFftSize - 1))); // window used for analysis
        gSynthesisWindowBuffer[n] = gAnalysisWindowBuffer[n];                                    // window used for synthesis
    }
}

float Morph::wrapPhase(float phaseIn) // This function wraps the phase to [-pi, pi]
{
    if (phaseIn >= 0) // Wrap positive phase to [-pi, pi]
        return fmodf(phaseIn + M_PI, 2.0 * M_PI) - M_PI;
    else
        return fmodf(phaseIn - M_PI, -2.0 * M_PI) + M_PI;
}

void Morph::process_fft() // This function processes the FFT
{
    // For guitar
    static std::vector<float> unwrappedBufferGuitar(gFftSize);             // buffer that holds the unwrapped input signal
    static std::vector<float> lastInputPhasesGuitar(gFftSize);             // buffer that holds the last input phases
    static std::vector<float> analysisMagnitudesGuitar(gFftSize / 2 + 1);  // buffer that holds the analysis magnitudes
    static std::vector<float> analysisFrequenciesGuitar(gFftSize / 2 + 1); // buffer that holds the analysis frequencies

    // For sample
    static std::vector<float> unwrappedBufferSample(gFftSize);
    static std::vector<float> lastInputPhasesSample(gFftSize);
    static std::vector<float> analysisMagnitudesSample(gFftSize / 2 + 1);
    static std::vector<float> analysisFrequenciesSample(gFftSize / 2 + 1);

    // These are shared by both inputs
    static std::vector<float> synthesisMagnitudes(gFftSize / 2 + 1);  // buffer that holds the synthesis magnitudes
    static std::vector<float> synthesisFrequencies(gFftSize / 2 + 1); // buffer that holds the synthesis frequencies
    static std::vector<float> lastOutputPhases(gFftSize);             // buffer that holds the last output phases

    // Process the FFT for both
    for (int n = 0; n < gFftSize; n++) // Unwrap the input signal
    {
        int circularBufferIndexGuitar = (gInputBufferPointerGuitar + n - gFftSize + gBufferSize) % gBufferSize; // Get the index of the circular buffer
        int circularBufferIndexSample = (gInputBufferPointerSample + n - gFftSize + gBufferSize) % gBufferSize; // Get the index of the circular buffer
        unwrappedBufferGuitar[n] = gInputBufferGuitar[circularBufferIndexGuitar] * gAnalysisWindowBuffer[n];    // Unwrap the input signal
        unwrappedBufferSample[n] = gInputBufferSample[circularBufferIndexSample] * gAnalysisWindowBuffer[n];    // Unwrap the input signal
    }
    gFftGuitar.fft(unwrappedBufferGuitar); // FFT for guitar
    gFftSample.fft(unwrappedBufferSample); // FFT for sample

    // ANALYSIS
    //==========================================================================
    for (int n = 0; n <= gFftSize / 2; n++)
    {
        // Turn real and imaginary components into amplitude and phase
        float amplitudeGuitar = gFftGuitar.fda(n);                        // Get the amplitude of the nth bin
        float amplitudeSample = gFftSample.fda(n);                        // Get the amplitude of the nth bin
        float phaseGuitar = atan2f(gFftGuitar.fdi(n), gFftGuitar.fdr(n)); // Get the phase of the nth bin
        float phaseSample = atan2f(gFftSample.fdi(n), gFftSample.fdr(n)); // Get the phase of the nth bin

        // Calculate the phase difference in this bin between the last
        // hop and this one, which will indirectly give us the exact frequency
        float phaseDiffGuitar = phaseGuitar - lastInputPhasesGuitar[n]; // Get the phase difference of the nth bin
        float phaseDiffSample = phaseSample - lastInputPhasesSample[n]; // Get the phase difference of the nth bin

        // Subtract the amount of phase increment we'd expect to see based
        // on the centre frequency of this bin (2*pi*n/gFftSize) for this
        // hop size, then wrap to the range -pi to pi
        float binCentreFrequency = 2.0 * M_PI * (float)n / (float)gFftSize;           // Get the centre frequency of the nth bin
        phaseDiffGuitar = wrapPhase(phaseDiffGuitar - binCentreFrequency * gHopSize); // Wrap the phase difference of the nth bin
        phaseDiffSample = wrapPhase(phaseDiffSample - binCentreFrequency * gHopSize); // Wrap the phase difference of the nth bin

        // Find deviation in (fractional) number of bins from the centre frequency
        float binDeviationGuitar = phaseDiffGuitar * (float)gFftSize / (float)gHopSize / (2.0 * M_PI); // Get the deviation of the nth bin
        float binDeviationSample = phaseDiffGuitar * (float)gFftSize / (float)gHopSize / (2.0 * M_PI); // Get the deviation of the nth bin

        // Add the original bin number to get the fractional bin where this partial belongs
        analysisFrequenciesGuitar[n] = (float)n + binDeviationGuitar; // Get the frequency of the nth bin
        analysisFrequenciesSample[n] = (float)n + binDeviationSample; // Get the frequency of the nth bin

        // Save the magnitude for later and for the GUI
        analysisMagnitudesGuitar[n] = amplitudeGuitar; // Get the magnitude of the nth bin
        analysisMagnitudesSample[n] = amplitudeSample; // Get the magnitude of the nth bin

        // Save the phase for next hop
        lastInputPhasesGuitar[n] = phaseGuitar; // Get the phase of the nth bin
        lastInputPhasesSample[n] = phaseSample; // Get the phase of the nth bin
    }

    // SYNTHESIS
    //==========================================================================

    // Zero out the synthesis bins, ready for new data
    for (int n = 0; n <= gFftSize / 2; n++)
    {
        synthesisMagnitudes[n] = synthesisFrequencies[n] = 0; // Set the magnitude and frequency of the nth bin to 0
    }

    // Handle the spectral morphing, storing frequencies into new bins
    for (int n = 0; n <= gFftSize / 2; n++)
    {
        // Perform linear interpolation between the magnitudes and frequencies of the two signals
        synthesisFrequencies[n] = (1 - gAlpha) * analysisFrequenciesGuitar[n] + gAlpha * analysisFrequenciesSample[n]; // Get the frequency of the nth bin
        synthesisMagnitudes[n] = (1 - gAlpha) * analysisMagnitudesGuitar[n] + gAlpha * analysisMagnitudesSample[n];    // Get the magnitude of the nth bin
    }

    // Synthesise frequencies into new magnitude and phase values for FFT bins
    for (int n = 0; n <= gFftSize / 2; n++)
    {
        float amplitude = synthesisMagnitudes[n]; // Get the magnitude of the nth bin

        //  Get the fractional offset from the bin centre frequency
        float binDeviation = synthesisFrequencies[n] - n; // Get the deviation of the nth bin

        //  Multiply to get back to a phase value
        float phaseDiff = binDeviation * 2.0 * M_PI * (float)gHopSize / (float)gFftSize; // Get the phase difference of the nth bin

        //  Add the expected phase increment based on the bin centre frequency
        float binCentreFrequency = 2.0 * M_PI * (float)n / (float)gFftSize; // Get the centre frequency of the nth bin
        phaseDiff += binCentreFrequency * gHopSize;                         // Get the phase difference of the nth bin

        //  Advance the phase from the previous hop
        float outPhase = wrapPhase(lastOutputPhases[n] + phaseDiff); // Wrap the phase difference of the nth bin

        //  Now convert magnitude and phase back to real and imaginary components
        gFftGuitar.fdr(n) = amplitude * cosf(outPhase); // Get the real component of the nth bin
        gFftGuitar.fdi(n) = amplitude * sinf(outPhase); // Get the imaginary component of the nth bin

        // Also store the complex conjugate in the upper half of the spectrum
        if (n > 0 && n < gFftSize / 2)
        {
            gFftGuitar.fdr(gFftSize - n) = gFftGuitar.fdr(n);  // Get the real component of the nth bin
            gFftGuitar.fdi(gFftSize - n) = -gFftGuitar.fdi(n); // Get the imaginary component of the nth bin
        }

        //  Save the phase for the next hop
        lastOutputPhases[n] = outPhase; // Get the phase of the nth bin
    }

    // Run the inverse FFT
    gFftGuitar.ifft();

    // Add timeDomainOut into the output buffer
    for (int n = 0; n < gFftSize; n++)
    {
        int circularBufferIndex = (gOutputBufferWritePointer + n - gFftSize + gBufferSize) % gBufferSize; // Get the index of the circular buffer
        gOutputBuffer[circularBufferIndex] += gFftGuitar.td(n) * gSynthesisWindowBuffer[n];               // Get the output buffer
    }

    // update output buffer write pointer
    gOutputBufferWritePointer = (gOutputBufferWritePointer + gHopSize) % gBufferSize; // Get the index of the circular buffer
}

float Morph::render(float guitarInput, float sampleInput)
{
    // Store the guitar input in a buffer for the FFT
    gInputBufferGuitar[gInputBufferPointerGuitar++] = guitarInput; // Get the guitar input
    if (gInputBufferPointerGuitar >= gBufferSize)                  // Check if the pointer is greater than the buffer size
    {
        // Wrap the circular buffer for guitar input
        gInputBufferPointerGuitar = 0;
    }

    // Store the sample input in a buffer for the FFT
    gInputBufferSample[gInputBufferPointerSample++] = sampleInput; // Get the sample input
    if (gInputBufferPointerSample >= gBufferSize)                  // Check if the pointer is greater than the buffer size
    {
        // Wrap the circular buffer for sample input
        gInputBufferPointerSample = 0;
    }

    // Get the output sample from the output buffer
    float output = gOutputBuffer[gOutputBufferReadPointer]; // Get the output buffer

    // Then clear the output sample in the buffer so it is ready for the next overlap-add
    gOutputBuffer[gOutputBufferReadPointer] = 0;

    // Scale the output down by the scale factor, compensating for the overlap
    output *= gScaleFactor;

    // Increment the read pointer in the output cicular buffer
    gOutputBufferReadPointer++;
    if (gOutputBufferReadPointer >= gBufferSize) // Check if the pointer is greater than the buffer size
        gOutputBufferReadPointer = 0;            // Wrap the circular buffer

    // return output
    return output;
}
