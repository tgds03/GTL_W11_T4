#pragma once
#include "Particles/ParticleModule.h"

class UParticleSystemComponent;
class UParticleEmitter;

class UParticleModuleTypeDataBase : public UParticleModule
{
    DECLARE_CLASS(UParticleModuleTypeDataBase, UParticleModule)
public:
    UParticleModuleTypeDataBase();
    virtual EModuleType	GetModuleType() const override {	return EPMT_TypeData;	}
    virtual bool SupportsSubUV() const { return false; }

    
	virtual FParticleEmitterInstance* CreateInstance(UParticleEmitter* InEmitterParent, UParticleSystemComponent* InComponent);
};
