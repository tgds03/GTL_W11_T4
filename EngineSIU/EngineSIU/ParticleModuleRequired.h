#pragma once
#include "Classes/Particles/ParticleModule.h"
#include "EnumAsByte.h"

class UParticleModuleRequired : public UParticleModule
{

public:

    /**
     *	The interpolation method to used for the SubUV image selection.
     *	One of the following:
     *	PSUVIM_None			- Do not apply SubUV modules to this emitter.
     *	PSUVIM_Linear		- Smoothly transition between sub-images in the given order,
     *						  with no blending between the current and the next
     *	PSUVIM_Linear_Blend	- Smoothly transition between sub-images in the given order,
     *						  blending between the current and the next
     *	PSUVIM_Random		- Pick the next image at random, with no blending between
     *						  the current and the next
     *	PSUVIM_Random_Blend	- Pick the next image at random, blending between the current
     *						  and the next
     */
    TEnumAsByte<EParticleSubUVInterpMethod> InterpolationMethod;

    /**
     *	Indicates the time (in seconds) that this emitter should be delayed in the particle system.
     */
    float EmitterDelay;

    /**
     *	If true, select the emitter delay from the range
     *		[EmitterDelayLow..EmitterDelay]
     */
    uint8 bEmitterDelayUseRange : 1;

    /**
     *	The low end of the emitter delay if using a range.
     */
    float EmitterDelayLow;

    /**
     *	If true, select the emitter duration from the range
     *		[EmitterDurationLow..EmitterDuration]
     */
    uint8 bEmitterDurationUseRange : 1;
    /**
     *	The low end of the emitter duration if using a range.
     */
    float EmitterDurationLow;

    /**
     *	How long, in seconds, the emitter will run before looping.
     */
    float EmitterDuration;

    /**
     *	If true, the emitter will be delayed only on the first loop.
     */
    uint8 bDelayFirstLoopOnly : 1;

    /** The number of times to loop the emitter.
     *	0 indicates loop continuously
     */
    int32 EmitterLoops;

};

