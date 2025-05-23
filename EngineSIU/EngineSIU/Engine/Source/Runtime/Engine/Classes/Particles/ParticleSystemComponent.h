#pragma once
#include "RandomStream.h"
#include "Components/PrimitiveComponent.h"
#include "UObject/ObjectMacros.h"

class FParticleDynamicData;
struct FDynamicEmitterReplayDataBase;
struct FParticleEmitterInstance;
struct FDynamicEmitterDataBase;
class UParticleSystem;
class FFXSystem;
class FRandomStream;

class UFXSystemComponent : public UPrimitiveComponent
{
    DECLARE_CLASS(UFXSystemComponent, UPrimitiveComponent)
public:
    UFXSystemComponent() = default;
};


class UParticleSystemComponent : public UFXSystemComponent
{
    DECLARE_CLASS(UParticleSystemComponent, UFXSystemComponent)
public:

    UParticleSystemComponent();
    
    virtual void TickComponent(float DeltaTime) override;

    void ForceReset();
    /** Possibly parallel phase of TickComponent **/
    void ComputeTickComponent();

    // 임시임
public:
    void InitializeSystem();

    // If particles have not already been initialised (ie. EmitterInstances created) do it now.
    virtual void InitParticles();


    /**
     * Static: Supplied with a chunk of replay data, this method will create dynamic emitter data that can
     * be used to render the particle system
     *
     * @param	EmitterInstance		Emitter instance this replay is playing on
     * @param	EmitterReplayData	Incoming replay data of any time, cannot be NULL
     * @param	bSelected			true if the particle system is currently selected
     *
     * @return	The newly created dynamic data, or NULL on failure
     */
    // static FDynamicEmitterDataBase* CreateDynamicDataFromReplay(FParticleEmitterInstance* EmitterInstance, const FDynamicEmitterReplayDataBase* EmitterReplayData, bool bSelected);

    /**
     * Creates dynamic particle data for rendering the particle system this frame.  This function
     * handle creation of dynamic data for regularly simulated particles, but also handles capture
     * and playback of particle replay data.
     * @return	Returns the dynamic data to render this frame
     */
    FParticleDynamicData* GetDynamicData();



    // 언리얼 코드, GetDynamicData로 함.
    
    /** Clears dynamic data on the rendering thread. */
    // void ClearDynamicData();
    // virtual void UpdateDynamicData();


    float DeltaTime;
private:
    uint32 TotalActiveParticles;
public:
    uint8 bWasDeactivated : 1;

    /** Used to accumulate total tick time to determine whether system can be skipped ticking if not visible. */
    float AccumTickTime;

    UParticleSystem* Template;
    FFXSystem* FXSystem;

    /** This is created at start up and then added to each emitter */
    float EmitterDelay;

    /** Stream of random values to use with this component */
    FRandomStream RandomStream;

    float WarmupTime;
    float WarmupTickRate;

    /** If true, the ViewRelevanceFlags are dirty and should be recached */
    uint8 bIsViewRelevanceDirty : 1;

    uint8 bResetTriggered = false;
    uint8 bWasCompleted : 1;

public:
    TArray<struct FParticleEmitterInstance*> EmitterInstances;
    
private:
    int32 LODLevel = 0;

    bool bFirstTick = true;
};
