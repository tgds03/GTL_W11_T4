
#include "TestCameraShake.h"

#include "Camera/Shakes/WaveOscillatorCameraShakePattern.h"

UTestCameraShake::UTestCameraShake()
{
    UWaveOscillatorCameraShakePattern* Pattern = new UWaveOscillatorCameraShakePattern();
    Pattern->Duration = 0.3f;
    Pattern->BlendInTime = 0.f;
    Pattern->BlendOutTime = 0.2f;
    
    Pattern->Yaw.Amplitude = 3.f;
    Pattern->Yaw.Frequency = 5.f;

    Pattern->Pitch.Amplitude = 10.f;
    Pattern->Pitch.Frequency = 8.f;

    Pattern->Roll.Amplitude = 6.f;
    Pattern->Roll.Frequency = 20.f;
    
    SetRootShakePattern(Pattern);
}
