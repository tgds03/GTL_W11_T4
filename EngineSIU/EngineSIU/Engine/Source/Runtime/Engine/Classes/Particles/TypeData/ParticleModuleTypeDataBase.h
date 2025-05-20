#pragma once
#include "Particles/ParticleModule.h"
//#include "ParticleEmitterInstances.h"

struct FParticleEmitterInstance;
class UParticleSystemComponent;
class UParticleEmitter;

class UParticleModuleTypeDataBase : public UParticleModule
{
    DECLARE_CLASS(UParticleModuleTypeDataBase, UParticleModule)
public:
    UParticleModuleTypeDataBase() = default;

    virtual EModuleType	GetModuleType() const override {	return EPMT_TypeData;	}
    virtual bool SupportsSubUV() const { return false; }
    virtual bool IsAMeshEmitter() const { return false; }

    /**
     * Build any resources required for simulating the emitter.
     * @param EmitterBuildInfo - Information compiled for the emitter.
     */
    virtual void Build(struct FParticleEmitterBuildInfo& EmitterBuildInfo) {}

    /**
     * Return whether the type data module requires a build step.
     */
    virtual bool RequiresBuild() const { return false; }

    virtual void CacheModuleInfo(UParticleEmitter* Emitter) {};


    virtual FParticleEmitterInstance* CreateInstance(UParticleEmitter* InEmitterParent, UParticleSystemComponent* InComponent);


    // 일단 하라고 해서 해놨는데 필요가 없지 않나?
};
