#include "ParticleModuleTypeDataMesh.h"
//#include "Particles/ParticleEmitterInstances.h"
#include "ParticleEmitterInstances.h"

// 이게 틱마다 호출되는지 다시 확인해야 함.
void UParticleModuleTypeDataMesh::SetToSensibleDefaults()
{
    if ((Mesh == NULL))
    {
        Mesh = FResourceManager::GetStaticMesh(L"Contents/Reference/Reference.obj");
    }
}

FParticleEmitterInstance* UParticleModuleTypeDataMesh::CreateInstance(UParticleEmitter* InEmitterParent, UParticleSystemComponent* InComponent)
{
    SetToSensibleDefaults();
    FParticleEmitterInstance* Instance = new FParticleMeshEmitterInstance();
    assert(Instance);

    Instance->InitParameters(InEmitterParent, InComponent);

    //CreateDistribution();
    // 랜덤하는거 여기 넣긴 해야 함.

    // mesh가 변경되었을때 로직도 추가하긴 해야 함.

    return Instance;
}

//// 사실상 이거 필요 없는 것 아닌가?
//void UParticleModuleTypeDataMesh::CreateDistribution()
//{
//    // 1. range가 잘뽑히는지 확인하고
//    // 2. 그걸로 UObject를 만든다고..?
//    // 3. FDistribution타입에 Distribution을 넣어줘야 하는데 이렇게 해야 나중에 데이터를 가져다 쓸 수 있는건가?
//
//    RollPitchYawRange.DefaultValue;
//    
//
//
//    // 아래는 언리얼 코드
//    //if (!RollPitchYawRange.IsCreated())
//    //{
//    //    RollPitchYawRange.Distribution = NewObject<UDistributionVectorUniform>(this, TEXT("DistributionRollPitchYaw"));
//    //}
//}
