#pragma once

class UParticleSystem
{
public:

    /** How long this Particle system should delay when ActivateSystem is called on it. */
    float Delay;

    /** The low end of the emitter delay if using a range. */
    float DelayLow;

    /**
     *	If true, select the emitter delay from the range
     *		[DelayLow..Delay]
     */
    uint8 bUseDelayRange : 1;

    float WarmupTime;
    float WarmupTickRate;

    TArray<UParticleEmitter*> Emitters;
};
