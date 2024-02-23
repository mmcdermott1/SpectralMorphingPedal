# SpectralMorphingPedal

### render.cpp Documentation

#### Overview
`render.cpp` is the main entry point for an audio processing application running on the Bela platform. This file orchestrates the interaction between various audio processing components, including a sampler, an envelope follower, a pitch tracker, a spectral morpher, and a compressor. It handles real-time audio input, processes it through these components, and outputs the processed audio.

#### Setup Function
```cpp
bool setup(BelaContext *context, void *userData);
```
- Initializes the application, including loading audio samples, setting up DSP components, and configuring the GUI.
- Parameters:
  - `context`: Provides information about the audio and digital signal processing environment.
  - `userData`: A pointer to any user-defined data needed for initialization.
- Returns `true` if setup is successful, `false` otherwise.

#### Render Function
```cpp
void render(BelaContext *context, void *userData);
```
- The core audio processing loop called by the Bela system for each block of audio frames.
- Parameters:
  - `context`: Contains information about the current block of audio frames, including input and output buffers.
  - `userData`: A pointer to user-defined data or state needed for processing.

#### Cleanup Function
```cpp
void cleanup(BelaContext *context, void *userData);
```
- Cleans up resources upon termination of the application.
- Parameters:
  - `context`: Contains context-specific information for cleanup.
  - `userData`: A pointer to user-defined data or state that may need to be cleaned up.

#### Key Components and Functionality
- **Sampler**: Handles loading and playback of audio samples, including pitch shifting.
- **Envelope Follower**: Analyzes the amplitude envelope of the incoming audio signal.
- **Pitch Tracker**: Estimates the pitch of the incoming audio signal in real-time.
- **Morph**: Performs spectral morphing between the live input signal and the loaded sample.
- **Compressor**: Applies dynamic range compression to the processed audio signal.
- **GUI**: Provides real-time control over various parameters through a web interface.
- **Auxiliary Tasks**: Offloads CPU-intensive tasks (e.g., pitch tracking, FFT processing) to auxiliary threads to maintain real-time audio performance.

#### DSP Flow
1. **Input Processing**: The guitar input is read and processed through a high-pass filter to remove low-frequency noise.
2. **Pitch Tracking**: The pitch of the guitar input is tracked in real-time, and this information is used to control aspects of the audio processing, such as pitch shifting of the samples.
3. **Sample Playback**: The selected audio sample is pitch-shifted and played back in sync with the live input.
4. **Spectral Morphing**: The live input and the sampled audio are morphed together based on the control parameters set by the user.
5. **Envelope Following**: The amplitude envelope of the live input is followed to modulate the output signal dynamically.
6. **Compression**: The final output signal is dynamically compressed to control its dynamic range before being sent to the audio output.

#### Control Parameters
- **Morph Amount**: Controls the balance between the live input and the sampled audio in the spectral morph.
- **Guitar Gain**: Adjusts the gain of the guitar input signal.
- **Sampler Gain**: Adjusts the gain of the sampled audio signal.
- **Pitch Offset**: Controls the pitch shifting of the sampled audio relative to the live input.
- **Compression Settings**: Threshold, ratio, and makeup gain parameters for the compressor.

#### Auxiliary Task Handling
- **Pitch Tracking and FFT Processing**: Due to their computational intensity, these tasks are performed in auxiliary tasks to prevent audio dropouts.

#### Usage
This application is intended for use on the Bela platform, leveraging its real-time audio processing capabilities for live performance and experimentation with audio effects. The GUI sliders allow performers and creators to interactively control the audio processing parameters in real-time.

### Compressor Class

#### Overview
The `Compressor` class is designed for dynamic range compression in real-time audio processing. It reduces the volume of loud sounds or amplifies quiet sounds by narrowing or compressing an audio signal's dynamic range. This class provides functionalities to set compression parameters and process individual audio samples through the compressor.

#### Constructor
```cpp
Compressor(float threshold, float ratio, float attackTime, float releaseTime, float kneeWidth, float makeupGain, float sampleRate);
```
- Initializes the compressor with user-defined settings for threshold, ratio, attack time, release time, knee width, makeup gain, and sample rate.
- Parameters:
  - `threshold`: The level above which compression starts to take effect, in dB.
  - `ratio`: The ratio of input to output change above the threshold.
  - `attackTime`: The time taken for the compressor to apply gain reduction after the signal exceeds the threshold.
  - `releaseTime`: The time taken for the compressor to revert to its original gain after the signal falls below the threshold.
  - `kneeWidth`: The range around the threshold where the transition from uncompressed to compressed signal is smoothed out.
  - `makeupGain`: The gain applied after compression to bring the signal back up to a desired level.
  - `sampleRate`: The sample rate of the audio signal being processed.

#### Public Methods
```cpp
float process(float inputSample);
```
- Processes a single audio sample and applies compression based on the current settings.
- Parameters:
  - `inputSample`: The audio sample to be processed.
- Returns: The processed (compressed) audio sample.

```cpp
void setThreshold(float threshold);
void setRatio(float ratio);
void setAttackTime(float attackTime);
void setReleaseTime(float releaseTime);
void setKneeWidth(float kneeWidth);
void setMakeupGain(float makeupGain);
```
- These methods allow setting the compressor's parameters at runtime. The parameters are similar to those described in the constructor.

#### Private Methods
```cpp
float dBToLinear(float dB);
```
- Converts a value from decibels (dB) to linear scale. This is used internally for threshold, knee width, and makeup gain calculations.

```cpp
void updateCoefficients();
```
- Calculates and updates the attack and release coefficients based on the current attack time, release time, and sample rate. This method ensures the compressor reacts appropriately to the dynamics of the incoming audio signal.

#### Private Members
- `threshold_`, `ratio_`, `attackTime_`, `releaseTime_`, `kneeWidth_`, `makeupGain_`: Store the compressor's parameters.
- `envelope_`: Tracks the level of the input signal to determine when and how much compression should be applied.
- `gain_`: The current gain applied by the compressor.
- `sampleRate_`: The sample rate of the audio signal being processed.
- `attackCoefficient_`, `releaseCoefficient_`: Coefficients used to smooth the transition between the uncompressed and compressed states of the signal.

#### Usage Example
```cpp
Compressor myCompressor(-24, 4, 0.01, 0.3, 6, 0, 44100);
float processedSample = myCompressor.process(inputSample);
```
- This example creates a `Compressor` instance with specified parameters and processes an `inputSample` through it.

### Notes
- The compressor uses a linear scale for internal calculations, requiring conversions from and to dB where applicable.
- Attack and release times are adaptive, based on the sample rate, to ensure consistent behavior across different audio formats.
- The implementation includes a soft knee option for smoother compression onset around the threshold.

This documentation provides a comprehensive overview of the `Compressor` class's functionality, suitable for developers integrating dynamic range compression into real-time audio processing applications.

### EnvelopeFollower Class 

#### Overview
The `EnvelopeFollower` class implements an envelope following algorithm, which tracks the amplitude envelope of an audio signal. It is commonly used in audio processing for dynamics processing, such as compression, expansion, and gating. This class provides functionalities for setting the attack, release, and smoothing times, which control how quickly the envelope follower responds to changes in the signal's amplitude.

#### Constructor
```cpp
EnvelopeFollower(float attack, float release, float smoothing, float sr);
```
- Initializes an envelope follower with specific time constants and sample rate.
- Parameters:
  - `attack`: The time it takes for the envelope to adapt to an increasing signal level, in milliseconds.
  - `release`: The time it takes for the envelope to adapt to a decreasing signal level, in milliseconds.
  - `smoothing`: Additional smoothing applied to the envelope to prevent rapid changes, in milliseconds.
  - `sr`: The sample rate of the audio signal being processed.

#### Public Methods
```cpp
void setAttackTime(float attack);
```
- Sets the attack time of the envelope follower.
- Parameters:
  - `attack`: The new attack time, in milliseconds.

```cpp
void setReleaseTime(float release);
```
- Sets the release time of the envelope follower.
- Parameters:
  - `release`: The new release time, in milliseconds.

```cpp
void setSmoothingTime(float smoothing);
```
- Sets the smoothing time, adding an extra layer of smoothing to the envelope follower's response.
- Parameters:
  - `smoothing`: The new smoothing time, in milliseconds.

```cpp
float process(float input);
```
- Processes a single audio sample and updates the envelope follower's state based on the input signal's amplitude.
- Parameters:
  - `input`: The input audio sample to be processed.
- Returns: The current value of the smoothed envelope.

#### Private Methods
```cpp
void calculateGainFactors();
```
- Calculates the gain factors for attack, release, and smoothing operations based on the current settings and sample rate. This method ensures that the envelope follower's response is calibrated correctly to the set time constants.

#### Private Members
- `attackTime`, `releaseTime`, `smoothingTime`: Store the time constants for attack, release, and smoothing operations, respectively.
- `attackGain`, `releaseGain`, `smoothingGain`: Gain factors calculated from the time constants and sample rate, used in the envelope following process.
- `envelope`: Represents the current state of the envelope follower without additional smoothing.
- `smoothedEnvelope`: The envelope after the application of smoothing, representing the output of the envelope follower.
- `sampleRate`: The sample rate of the audio signal being processed.

#### Usage Example
```cpp
EnvelopeFollower myEnvelopeFollower(10.0, 200.0, 50.0, 44100.0);
float envelopeValue = myEnvelopeFollower.process(inputSample);
```
- This example creates an `EnvelopeFollower` instance with specified parameters for attack, release, smoothing times, and sample rate. It then processes an input sample to obtain the current envelope value.

### Notes
- The envelope follower is designed to be highly responsive to changes in the input signal's amplitude, with customizable response times for both increasing and decreasing signal levels.
- The additional smoothing time parameter allows for finer control over the envelope's behavior, making it possible to achieve more musical results in audio processing applications.
- The implementation assumes that time constants are provided in milliseconds and converts them to appropriate gain factors for internal processing.

This documentation provides a comprehensive overview of the `EnvelopeFollower` class's functionality, suitable for developers working on audio processing applications that require amplitude envelope tracking.

### Morph Class 

#### Overview
The `Morph` class implements a spectral morphing algorithm that combines aspects of two audio signals (e.g., guitar and sample inputs) into a single output signal. It utilizes Fast Fourier Transform (FFT) to analyze the spectral content of both inputs, morphs their spectra according to a specified ratio, and then synthesizes a new signal from the morphed spectrum. This process allows for creative audio effects, such as blending the timbral characteristics of two different sounds.

#### Constructor
```cpp
Morph(int fftSize, int hopSize, int bufferSize);
```
- Initializes the `Morph` class with specified FFT size, hop size, and buffer size.
- Parameters:
  - `fftSize`: The size of the FFT window, determining the resolution of the spectral analysis.
  - `hopSize`: The number of samples between successive FFT frames. Smaller values increase time resolution but decrease frequency resolution.
  - `bufferSize`: The size of the circular buffers used for input and output, ensuring continuous processing of audio streams.

#### Public Methods
```cpp
void setup();
```
- Prepares the `Morph` object for processing by setting up FFT objects, initializing buffers, and calculating analysis and synthesis windows.

```cpp
float wrapPhase(float phaseIn);
```
- Wraps a phase value to the range [-π, π].
- Parameters:
  - `phaseIn`: The phase value to wrap.
- Returns: The wrapped phase value.

```cpp
void process_fft();
```
- Performs the FFT-based processing for both input signals, including phase unwrapping, spectral analysis, spectral morphing, and inverse FFT for synthesis.

```cpp
float render(float guitarInput, float sampleInput);
```
- Processes input samples from guitar and sample sources, applies spectral morphing, and returns the next output sample.
- Parameters:
  - `guitarInput`: The next input sample from the guitar.
  - `sampleInput`: The next input sample from the sample source.
- Returns: The next output sample resulting from the spectral morphing process.

#### Public Members
- `gHopCounter`: A counter for managing the hop size during processing.
- `gHopSize`: The hop size between successive FFT frames.
- `gInputBufferPointerGuitar`, `gInputBufferPointerSample`: Circular buffer pointers for managing the input samples for guitar and sample, respectively.
- `gCachedInputBufferPointerGuitar`, `gCachedInputBufferPointerSample`: Used to manage the positions in the input buffers where samples are cached for processing.
- `gAlpha`: The ratio of output to input frequency, controlling the balance of spectral characteristics between the two inputs during morphing.

#### Private Members
- `gFftGuitar`, `gFftSample`: FFT processing objects for guitar and sample inputs.
- `gFftSize`: The size of the FFT window.
- `gScaleFactor`: A factor for scaling the output, based on window type and overlap.
- `gBufferSize`: The size of the circular buffers.
- `gInputBufferGuitar`, `gInputBufferSample`: Circular buffers for collecting input samples from guitar and sample sources.
- `gOutputBuffer`: Circular buffer for collecting the output of the overlap-add process.
- `gOutputBufferWritePointer`, `gOutputBufferReadPointer`: Pointers for managing positions in the output buffer for writing and reading.
- `gAnalysisWindowBuffer`, `gSynthesisWindowBuffer`: Buffers holding the windows for FFT analysis and synthesis.

#### Usage Example
```cpp
Morph morphProcessor(1024, 256, 4096);
morphProcessor.setup();
float outputSample = morphProcessor.render(guitarInputSample, sampleInputSample);
```
- This example sets up a `Morph` instance with a specific FFT size, hop size, and buffer size, prepares it for processing, and then processes individual samples from guitar and sample inputs to produce a morphed output sample.

### Notes
- The `Morph` class leverages FFT to perform spectral analysis and synthesis, requiring careful management of phase information to ensure coherent output.
- Spectral morphing is achieved by linear interpolation between the spectral magnitudes and frequencies of the two input signals, controlled by the `gAlpha` parameter.
- This documentation provides a comprehensive overview of the `Morph` class's functionality, suitable for developers working on audio processing applications that involve spectral morphing techniques.

### PitchTracker Class 

#### Overview
The `PitchTracker` class is designed for pitch detection in audio signals. It utilizes a time-domain method known as the YIN algorithm for pitch tracking. This method is effective for monophonic signals and operates by analyzing the autocorrelation of the input signal. The class provides functionality to process audio samples, dynamically update the input buffer, and retrieve pitch information.

#### Constructor
```cpp
PitchTracker(float sampleRate, unsigned int bufferSize);
```
- Initializes a `PitchTracker` object with the given sample rate and buffer size.
- Parameters:
  - `sampleRate`: The sample rate of the audio signal in Hz.
  - `bufferSize`: The size of the input buffer to hold audio samples for processing.

#### Public Methods
```cpp
float process();
```
- Processes the buffered audio samples to detect the pitch.
- Returns: The detected pitch in Hz. A return value of -1 indicates that no pitch was detected.

```cpp
void setBufferValue(int index, float value);
```
- Updates a specific value in the input buffer.
- Parameters:
  - `index`: The index in the buffer to update.
  - `value`: The new sample value to set at the specified index.

```cpp
void setBufferPointer(int value);
```
- Sets the current position in the input buffer where the next audio sample will be placed.
- Parameters:
  - `value`: The new buffer pointer position.

```cpp
int getBufferPointer() const;
```
- Retrieves the current position in the input buffer.
- Returns: The current buffer pointer position.

```cpp
unsigned int getBufferSize() const;
```
- Retrieves the size of the input buffer.
- Returns: The buffer size.

#### Private Methods
```cpp
std::vector<float> downsample(const std::vector<float> &signal, int factor);
```
- Downsamples the given audio signal by the specified factor.
- Parameters:
  - `signal`: The audio signal to downsample.
  - `factor`: The downsampling factor.
- Returns: The downsampled signal.

#### Private Members
- `sampleRate`: The sample rate of the audio signal in Hz.
- `bufferSize`: The size of the input buffer for audio samples.
- `gInputBuffer`: The input buffer holding audio samples for pitch detection.
- `gCachedInputBufferPointer`: The current position in the input buffer for the next audio sample.

#### Algorithm Details
- The pitch detection algorithm implemented in this class is based on the YIN algorithm, a popular method for fundamental frequency estimation.
- The process involves creating a downsampled version of the input signal to reduce computational complexity, computing a difference function to identify periodicity, and then applying a cumulative mean normalized difference function to pinpoint the fundamental frequency.
- Parabolic interpolation is used to refine the pitch estimate, enhancing the accuracy of the detected pitch.

#### Usage Example
```cpp
PitchTracker pitchTracker(44100, 1024); // Initialize with sample rate and buffer size
// Fill the buffer with audio samples
for (int i = 0; i < 1024; i++) {
    pitchTracker.setBufferValue(i, audioSamples[i]);
}
float pitch = pitchTracker.process(); // Detect the pitch
```
- This example demonstrates initializing a `PitchTracker` with a sample rate of 44100 Hz and a buffer size of 1024 samples, filling the buffer with audio data, and then calling the `process` method to detect the pitch of the buffered audio samples.

### Notes
- The `PitchTracker` class is designed for monophonic signals and may not accurately detect pitches in polyphonic content.
- The accuracy of pitch detection can be influenced by the quality of the input signal, the chosen buffer size, and the sample rate. Adjusting the downsampling factor may help to balance accuracy and computational load.

### Sampler Class 

#### Overview
The `Sampler` class provides functionality to load and play back audio samples from a file with support for looping, pitch shifting, and cubic interpolation for smooth playback. It's designed to be used in audio processing applications where high-quality sample playback is required.

#### Constructor
```cpp
Sampler(const std::string &filename, bool loop, bool autostart);
```
- Initializes a new `Sampler` instance, loading an audio file and setting playback options.
- Parameters:
  - `filename`: Path to the audio file to be loaded.
  - `loop`: Indicates whether the sample should loop when it reaches the end.
  - `autostart`: Determines whether playback should start immediately upon loading the sample.

#### Public Methods
```cpp
bool setup(const std::string &filename, bool loop, bool autostart);
```
- Loads an audio file and sets playback options. This method can be used to reconfigure the `Sampler` after it has been constructed.
- Parameters are the same as the constructor.
- Returns `true` if the file is loaded successfully, `false` otherwise.

```cpp
void trigger();
```
- Starts or restarts sample playback from the beginning.

```cpp
float process(float frequency, float baseFrequency);
```
- Processes and returns the next sample from the audio file, applying pitch shifting based on the specified frequency and the base frequency of the sample.
- Parameters:
  - `frequency`: The target frequency for playback.
  - `baseFrequency`: The original (base) frequency of the sample. Pitch shifting is relative to this frequency.
- Returns: The next audio sample, pitch-shifted as requested.

#### Private Methods
```cpp
float cubicInterpolation(float x, float y0, float y1, float y2, float y3);
```
- Performs cubic interpolation between four sample points to calculate a smoothly interpolated value.
- Parameters:
  - `x`: The fractional index between `y1` and `y2` for which the interpolated value is desired.
  - `y0`, `y1`, `y2`, `y3`: The sample values immediately before, at, and after the desired index, respectively.
- Returns: The interpolated sample value.

#### Usage Example
```cpp
Sampler mySampler("path/to/sample.wav", true, false);
mySampler.trigger(); // Start playback
float frequency = 440; // Target frequency for pitch shifting
float baseFrequency = 440; // Original frequency of the sample
float nextSample = mySampler.process(frequency, baseFrequency); // Retrieve the next pitch-shifted sample
```
- This example demonstrates initializing a `Sampler` with a specified audio file, enabling looping, and disabling autostart. Playback is started manually with `trigger()`. The `process` method is then called to retrieve pitch-shifted samples, where the pitch shifting is based on the desired frequency.

### Notes
- The class supports only mono audio files for simplicity and efficiency in real-time audio processing contexts.
- Cubic interpolation is used to provide smooth pitch shifting and playback, significantly improving the quality over simpler linear interpolation methods, especially noticeable when the pitch shift factor is far from 1.
- The sampler is designed to handle looping and non-looping playback modes, making it suitable for a wide range of musical applications, from one-shot sound effects to continuous background music loops.
