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
