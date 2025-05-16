#pragma once
#include "Math/Color.h"
#include "Math/Vector.h"

struct FBaseParticle
{
    // 16 bytes
    FVector		OldLocation;			// Last frame's location, used for collision
    float			RelativeTime;			// Relative time, range is 0 (==spawn) to 1 (==death)

    // 16 bytes
    FVector		Location;				// Current location
    float			OneOverMaxLifetime;		// Reciprocal of lifetime
    
    // 16 bytes
    FVector		BaseVelocity;			// Velocity = BaseVelocity at the start of each frame.
    float			Rotation;				// Rotation of particle (in Radians)

    // 16 bytes
    FVector		Velocity;				// Current velocity, gets reset to BaseVelocity each frame to allow 
    float			BaseRotationRate;		// Initial angular velocity of particle (in Radians per second)

    // 16 bytes
    FVector		BaseSize;				// Size = BaseSize at the start of each frame
    float			RotationRate;			// Current rotation rate, gets reset to BaseRotationRate each frame

    // 16 bytes
    FVector		Size;					// Current size, gets reset to BaseSize each frame
    int32			Flags;					// Flags indicating various particle states

    // 16 bytes
    FLinearColor	Color;					// Current color of particle.

    // 16 bytes
    FLinearColor	BaseColor;				// Base color of the particle
};

/**
 *	General event instance payload.
 */
struct FParticleEventInstancePayload
{
    uint32 bSpawnEventsPresent:1;
    uint32 bDeathEventsPresent:1;
    uint32 bCollisionEventsPresent:1;
    uint32 bBurstEventsPresent:1;

    int32 SpawnTrackingCount;
    int32 DeathTrackingCount;
    int32 CollisionTrackingCount;
    int32 BurstTrackingCount;
};


struct FParticleDataContainer
{
    
};

struct FDynamicEmitterReplayDataBase
{
    
};

struct FDynamicEmitterDataBase
{
};
