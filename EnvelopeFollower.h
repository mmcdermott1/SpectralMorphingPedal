// EnvelopeFollower.h
#include <cmath>

class EnvelopeFollower
{
private:
    float attackTime;
    float releaseTime;
    float smoothingTime;
    float attackGain;
    float releaseGain;
    float smoothingGain;
    float envelope;
    float smoothedEnvelope;
    float sampleRate;

    void calculateGainFactors();

public:
    EnvelopeFollower(float attack, float release, float smoothing, float sr);

    void setAttackTime(float attack);
    void setReleaseTime(float release);
    void setSmoothingTime(float smoothing);

    float process(float input);
};
