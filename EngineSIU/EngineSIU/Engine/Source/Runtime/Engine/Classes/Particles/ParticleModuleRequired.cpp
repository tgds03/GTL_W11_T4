#include "ParticleModuleRequired.h"

UParticleModuleRequired::UParticleModuleRequired()
{
    Flags = static_cast<EModuleFlag::EModuleFlags>(EModuleFlag::SpawnModule | EModuleFlag::UpdateModule);
    EmitterDuration = 1.f;
    EmitterDelay = 0.f;
    EmitterLoops = 0;
}
