#include "ParticleModule.h"
#include "ParticleHelper.h"
#include "ParticleEmitterInstances.h"

FRandomStream& UParticleModule::GetRandomStream(FParticleEmitterInstance* Owner)
{
    FParticleRandomSeedInstancePayload* Payload = Owner->GetModuleRandomSeedInstanceData(this);
    FRandomStream& RandomStream = (Payload != nullptr) ? Payload->RandomStream : Owner->Component->RandomStream;
    return RandomStream;
}
