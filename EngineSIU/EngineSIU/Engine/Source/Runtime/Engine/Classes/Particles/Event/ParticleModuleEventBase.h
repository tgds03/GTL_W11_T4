#pragma once
#include "Particles/ParticleModule.h"

class UParticleModuleEventBase : public UParticleModule
{
    DECLARE_CLASS(UParticleModuleEventBase, UParticleModule)
public:
    UParticleModuleEventBase() = default;
    // virtual EModuleType	GetModuleType() const override {	return EPMT_Event;	}
};
