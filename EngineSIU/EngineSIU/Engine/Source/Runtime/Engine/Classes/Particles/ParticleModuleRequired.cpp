#include "ParticleModuleRequired.h"

UParticleModuleRequired::UParticleModuleRequired()
{
    Flags = static_cast<EModuleFlag::EModuleFlags>(EModuleFlag::SpawnModule | EModuleFlag::UpdateModule);

    EmitterDuration = 1.0f;
    EmitterDurationLow = 0.0f;
    bEmitterDurationUseRange = false;
    EmitterDelay = 0.0f;
    EmitterDelayLow = 0.0f;
    bEmitterDelayUseRange = false;
    EmitterLoops = 0;	
    SubImages_Horizontal = 1;
    SubImages_Vertical = 1;
}

void UParticleModuleRequired::SetToSensibleDefaults(UParticleEmitter* Owner)
{
    Super::SetToSensibleDefaults(Owner);
}
