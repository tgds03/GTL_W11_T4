
#include "TestCameraShake.h"

#include "Camera/Shakes/WaveOscillatorCameraShakePattern.h"

UTestCameraShake::UTestCameraShake()
{
    UWaveOscillatorCameraShakePattern* Pattern = new UWaveOscillatorCameraShakePattern();
    Pattern->Duration = 0.3f;
    SetRootShakePattern(Pattern);
}
