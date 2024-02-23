// Compressor.h
#ifndef COMPRESSOR_H
#define COMPRESSOR_H

class Compressor
{
public:
    Compressor(float threshold, float ratio, float attackTime, float releaseTime, float kneeWidth, float makeupGain, float sampleRate);

    float process(float inputSample);
    void setThreshold(float threshold);
    void setRatio(float ratio);
    void setAttackTime(float attackTime);
    void setReleaseTime(float releaseTime);
    void setKneeWidth(float kneeWidth);
    void setMakeupGain(float makeupGain);

private:
    float threshold_;
    float ratio_;
    float attackTime_;
    float releaseTime_;
    float kneeWidth_;
    float makeupGain_;
    float envelope_;
    float gain_;
    float sampleRate_;

    float attackCoefficient_;
    float releaseCoefficient_;

    float dBToLinear(float dB);
    void updateCoefficients();
};

#endif // COMPRESSOR_H
