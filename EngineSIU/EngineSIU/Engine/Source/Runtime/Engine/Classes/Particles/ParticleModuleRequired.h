#pragma once
#include "Classes/Particles/ParticleModule.h"
#include "EnumAsByte.h"
#include "ParticleEmitter.h"

enum EParticleSortMode
{
    PSORTMODE_None,
    PSORTMODE_ViewProjDepth,
    PSORTMODE_DistanceToView,
    PSORTMODE_Age_OldestFirst,
    PSORTMODE_Age_NewestFirst,
    PSORTMODE_MAX,
};

class UParticleModuleRequired : public UParticleModule
{
    DECLARE_CLASS(UParticleModuleRequired, UParticleModule)
public:
    UParticleModuleRequired();

    UMaterial* Material;
    FVector EmitterOrigin;
    FRotator EmitterRotation;
    int32 EmitterLoops;
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

    /** The number of times to change a random image over the life of the particle.		*/
    int32 RandomImageChanges;

    /**
     *	The amount of time (particle-relative, 0.0 to 1.0) to 'lock' on a random sub image
     *	    0.0 = change every frame
     *      1.0 = select a random image at spawn and hold for the life of the particle
     */
    float RandomImageTime;

    /** If true, kill the emitter when the particle system is deactivated				*/
    uint8 bKillOnDeactivate : 1;

    /** If true, kill the emitter when it completes										*/
    uint8 bKillOnCompleted : 1;

    uint8 bUseLocalSpace: 1;

    /**
     *	The sorting mode to use for this emitter.
     *	PSORTMODE_None				- No sorting required.
     *	PSORTMODE_ViewProjDepth		- Sort by view projected depth of the particle.
     *	PSORTMODE_DistanceToView	- Sort by distance of particle to view in world space.
     *	PSORTMODE_Age_OldestFirst	- Sort by age, oldest drawn first.
     *	PSORTMODE_Age_NewestFirst	- Sort by age, newest drawn first.
     *
     */
    TEnumAsByte<EParticleSortMode> SortMode;
};

