// render.cpp
#include <Bela.h>
#include <libraries/Gui/Gui.h>
#include <libraries/GuiController/GuiController.h>
#include <libraries/Biquad/Biquad.h>
#include "Sampler.h"
#include "EnvelopeFollower.h"
#include "PitchTracker.h"
#include "Morph.h"
#include "Compressor.h"

// SELECT SAMPLE USING INDEX BELOW
unsigned int gSampleIndex = 1;

// SAMPLE DIRECTORY + INDEX
std::vector<std::string> gFilename = {
    "00-TONAL-SOFT-Dissolver.wav",        // sample index 0
    "01-TONAL-SOFT-ElectricOwl.wav",      // sample index 1
    "02-TONAL-SOFT-InfinityOwl.wav",      // sample index 2
    "03-TONAL-SOFT-LowTideBell.wav",      // sample index 3
    "04-TONAL-SOFT-MetalBottle.wav",      // sample index 4
    "05-TONAL-SOFT-SilkSiren.wav",        // sample index 5
    "06-TONAL-SOFT-TheRing.wav",          // sample index 6
    "07-TONAL-INTENSE-Burn.wav",          // sample index 7
    "08-TONAL-INTENSE-DNA.wav",           // sample index 8
    "09-TONAL-INTENSE-OldUprightTwo.wav", // sample index 9
    "10-TONAL-INTENSE-Throat.wav",        // sample index 10
    "11-NOISE-CoinJangle.wav",            // sample index 11
    "12-NOISE-CoinOne.wav",               // sample index 12
    "13-NOISE-CoinTwo.wav",               // sample index 13
    "14-NOISE-GhoulOpera.wav",            // sample index 14
    "15-NOISE-LaundryBang.wav",           // sample index 15
    "16-NOISE-Lightbulb.wav",             // sample index 16
    "17-NOISE-MetalLid.wav",              // sample index 17
    "18-NOISE-OrganNoise.wav",            // sample index 18
    "19-NOISE-SoftScreech.wav",           // sample index 19
    "20-NOISE-Station.wav",               // sample index 20
    "21-NOISE-SyntheticRain.wav",         // sample index 21
    "22-NOISE-TimeTravel.wav",            // sample index 22
};

// SAMPLER
Sampler gSampler;                   // Sampler object
unsigned int gPitchOffsetSliderIdx; // Slider index for pitch offset
float gBaseFrequency = 261.626;     // Base frequency for pitch offset
float gPitchOffset = 1.0;           // Pitch offset
float gFrequency = 261.626;         // Frequency of the sample

// MORPH
const unsigned int gFftSize_morph = 512;    // FFT size for morphing
const unsigned int gBufferSize_pitch = 512; // Buffer size for pitch tracking
Morph *morph;                               // Morph object
unsigned int gMorphAmountIdx;               // Slider index for morph amount

// COMPRESSOR
unsigned int gComp_ThresholdSliderIdx;  // Slider index for threshold
unsigned int gComp_RatioSliderIdx;      // Slider index for ratio
unsigned int gComp_MakeupGainSliderIdx; // Slider index for makeup gain
Compressor *compressor;

// GUI
Gui gui;                  // GUI object
GuiController controller; // GUI controller object

// PITCH TRACKER
PitchTracker *pitchTracker; // Pitch tracker object
Biquad hpFilter;            // Biquad high-pass frequency;

// ENVELOPE FOLLOWER
EnvelopeFollower *envFollower; // Envelope follower object
unsigned int gGuitarGainIdx;   // Slider index for guitar gain
unsigned int gSamplerGainIdx;  // Slider index for sampler gain

// THREAD HANDLING
AuxiliaryTask gPitchTask;                     // Auxiliary task for pitch tracking
AuxiliaryTask gFftTask;                       // Auxiliary task for FFT
void process_pitchTracker_background(void *); // Function for pitch tracking
void process_fft_background(void *);          // Function for FFT

// SETUP
//============================================================================================================
bool setup(BelaContext *context, void *userData)
{
    // Load the audio files
    if (!gSampler.setup(gFilename[gSampleIndex]))
    {
        rt_printf("Error loading audio file '%s'\n", gFilename[gSampleIndex].c_str());
        return false;
    }

    // Set up the high-pass filter
    Biquad::Settings settings{
        .fs = context->audioSampleRate,
        .type = Biquad::highpass,
        .cutoff = 80.0,
        .q = 0.707,
        .peakGainDb = 0,
    };
    hpFilter.setup(settings);

    // Set up the envelope follower + pitch tracker + compressor
    envFollower = new EnvelopeFollower(1.0, 100.0, 0.1, context->audioSampleRate);               // Set up the envelope follower
    pitchTracker = new PitchTracker(context->audioSampleRate, gBufferSize_pitch);                // Set up the pitch tracker
    compressor = new Compressor(-20.0, 4.0, 0.010, 0.100, 10.0, 12.0, context->audioSampleRate); // Set up the compressor

    // Set up the morph
    const int gHopSize_morph = gFftSize_morph / 2;                        // 50% overlap
    const int gBufferSize_morph = gFftSize_morph * context->audioFrames;  // Buffer size for the morph
    morph = new Morph(gFftSize_morph, gHopSize_morph, gBufferSize_morph); // Set up the morph
    morph->setup();                                                       // Initialise the morph

    // Set up the GUI
    gui.setup(context->projectName);
    controller.setup(&gui, "Spectral Morphing Pedal");

    // Add sliders to the GUI
    gMorphAmountIdx = controller.addSlider("Morph: Amount", 0.0, 0, 1, 0);
    gGuitarGainIdx = controller.addSlider("Gain: Guitar", 1.0, 0, 2.0, 0);
    gSamplerGainIdx = controller.addSlider("Gain: Sampler", 1.5, 0, 2.0, 0);
    gPitchOffsetSliderIdx = controller.addSlider("Sampler: Pitch Offset", 1.0, 0.5, 2.0, 0.5);
    gComp_ThresholdSliderIdx = controller.addSlider("Comp: Threshold", -20.0, -60.0, 0.0, 0.1);
    gComp_RatioSliderIdx = controller.addSlider("Comp: Ratio", 10.0, 1.0, 20.0, 0.1);
    gComp_MakeupGainSliderIdx = controller.addSlider("Comp: MakeupGain", 12.0, 0.0, 20.0, 0.1);

    // Set up the auxiliary task for pitch tracking
    gFftTask = Bela_createAuxiliaryTask(process_fft_background, 70, "bela-process-fft");
    gPitchTask = Bela_createAuxiliaryTask(process_pitchTracker_background, 50, "bela-process-yin");

    return true;
}

void process_fft_background(void *)
{
    morph->process_fft(); // Process the FFT
}

void process_pitchTracker_background(void *)
{
    gFrequency = pitchTracker->process(); // Get the frequency from the pitch tracker
}

void render(BelaContext *context, void *userData)
{
    // Set values from sliders
    float morphAmount = controller.getSliderValue(gMorphAmountIdx);
    morph->gAlpha = morphAmount;

    float guitarGain = controller.getSliderValue(gGuitarGainIdx);
    float samplerGain = controller.getSliderValue(gSamplerGainIdx);

    gPitchOffset = controller.getSliderValue(gPitchOffsetSliderIdx);

    compressor->setThreshold(controller.getSliderValue(gComp_ThresholdSliderIdx));
    compressor->setRatio(controller.getSliderValue(gComp_RatioSliderIdx));
    compressor->setMakeupGain(controller.getSliderValue(gComp_MakeupGainSliderIdx));

    // Loop through the audio frames
    for (unsigned int n = 0; n < context->audioFrames; n++)
    {
        float guitar = audioRead(context, n, 0);               // Read guitar input
        float smoothedEnvelope = envFollower->process(guitar); // Analyze Envelope

        pitchTracker->setBufferValue(pitchTracker->getBufferPointer(), guitar); // Store the audio sample in the buffer
        pitchTracker->setBufferPointer(pitchTracker->getBufferPointer() + 1);   // Increment the buffer pointer

        // If the buffer is full, schedule the pitch tracking task to run
        if (pitchTracker->getBufferPointer() >= pitchTracker->getBufferSize()) // If the buffer pointer is equal to the buffer size
        {
            pitchTracker->setBufferPointer(0);      // Reset the buffer pointer
            Bela_scheduleAuxiliaryTask(gPitchTask); // Schedule the pitch tracking task
        }

        // If the buffer is full, schedule the fft task to run
        if (++morph->gHopCounter >= morph->gHopSize) // If the hop counter is equal to the hop size
        {
            morph->gHopCounter = 0;                                                    // Reset the hop counter
            morph->gCachedInputBufferPointerGuitar = morph->gInputBufferPointerGuitar; // Cache the input buffer pointer
            morph->gCachedInputBufferPointerSample = morph->gInputBufferPointerSample; // Cache the input buffer pointer
            Bela_scheduleAuxiliaryTask(gFftTask);                                      // Schedule the FFT task
        }

        float sampler = gSampler.process(gFrequency, gBaseFrequency * gPitchOffset); // Process the sampler
        sampler = hpFilter.process(sampler);                                         // Apply the high-pass filter

        float morphOutput = morph->render(guitar * guitarGain, sampler * samplerGain); // Process the morphing
        float masterOutput = morphOutput * smoothedEnvelope;                           // Apply the envelope
        masterOutput = compressor->process(masterOutput);                              // Apply the compressor
        audioWrite(context, n, 0, masterOutput);                                       // Write to output

        // Write the audio to the output
        for (unsigned int channel = 0; channel < context->audioOutChannels; channel++)
        {
            audioWrite(context, n, channel, masterOutput);
        }
    }
}

void cleanup(BelaContext *context, void *userData)
{
    delete pitchTracker;
    delete envFollower;
    delete morph;
    delete compressor;
}
