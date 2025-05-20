#pragma once
#include "Particles/ParticleModule.h"

class UParticleEmitter;

class UParticleModuleTypeDataBase : public UParticleModule
{
    DECLARE_CLASS(UParticleModuleTypeDataBase, UParticleModule)
public:
    UParticleModuleTypeDataBase() {}
    virtual EModuleType	GetModuleType() const override {	return EPMT_TypeData;	}
    virtual bool SupportsSubUV() const { return false; }

    virtual void CacheModuleInfo(UParticleEmitter* Emitter) {};
};
