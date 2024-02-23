# SpectralMorphingPedal
 
### Compressor Class Documentation

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

### EnvelopeFollower Class Documentation

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
