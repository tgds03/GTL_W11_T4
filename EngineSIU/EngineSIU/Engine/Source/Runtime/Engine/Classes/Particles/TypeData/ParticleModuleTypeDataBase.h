#pragma once
#include "Particles/ParticleModule.h"

class UParticleModuleTypeDataBase : public UParticleModule
{
public:
    virtual EModuleType	GetModuleType() const override {	return EPMT_TypeData;	}
    virtual bool SupportsSubUV() const { return false; }

};
