#pragma once
#include "Components/PrimitiveComponent.h"
#include "UObject/ObjectMacros.h"

class FParticleDynamicData;
struct FDynamicEmitterReplayDataBase;
struct FParticleEmitterInstance;
struct FDynamicEmitterDataBase;

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

    UParticleSystemComponent() = default;


    virtual void TickComponent(float DeltaTime) override;
    /** Possibly parallel phase of TickComponent **/
    void ComputeTickComponent_Concurrent();

protected:

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


public:
    class UParticleSystem* Template;

public:
    TArray<struct FParticleEmitterInstance*> EmitterInstances;

    // mutable TArray<FDynamicEmitterDataBase*> DynamicDataForThisFrame;
    TArray<FDynamicEmitterDataBase*> EmitterRenderData;
};
