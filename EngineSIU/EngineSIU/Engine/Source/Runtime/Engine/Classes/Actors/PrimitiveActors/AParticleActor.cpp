#include "AParticleActor.h"

#include "ParticleEmitterInstances.h"
#include "ParticleHelper.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"

// Begin Test
#include "UObject/ObjectFactory.h"
#include "Particles/ParticleEmitter.h"
#include "Particles/ParticleLODLevel.h"
#include "Particles/ParticleModuleRequired.h"
#include "Particles/ParticleModuleSpawn.h"
#include "Particles/TypeData/ParticleModuleTypeDataBase.h"
#include "Particles/TypeData/ParticleModuleTypeDataMesh.h"

// End Test

AParticleActor::AParticleActor()
{
    ParticleSystemComponent = AddComponent<UParticleSystemComponent>();
    RootComponent = ParticleSystemComponent;

    {
        //FDynamicSpriteEmitterData* TempEmitterData = new FDynamicSpriteEmitterData(nullptr);
        //TempEmitterData->Source = FDynamicSpriteEmitterReplayData();
        //TempEmitterData->Source.ActiveParticleCount = 100;
        //TempEmitterData->Source.MaxDrawCount = 100;
    
        //ParticleSystemComponent->EmitterRenderData.Add(TempEmitterData);
    }
    
    {
        //FResourceManager::CreateStaticMesh("Contents/Reference/Reference.obj");
        //FDynamicMeshEmitterData* TempEmitterData = new FDynamicMeshEmitterData(nullptr);
        //TempEmitterData->StaticMesh = FResourceManager::GetStaticMesh(L"Contents/Reference/Reference.obj");
        //TempEmitterData->Source = FDynamicMeshEmitterReplayData();
        //TempEmitterData->Source.ActiveParticleCount = 100;
        //TempEmitterData->Source.MaxDrawCount = 100;
        //
        //ParticleSystemComponent->EmitterRenderData.Add(TempEmitterData);
    }
    {
        UParticleSystem* Template = new UParticleSystem();
        Template->Delay = 1.f;
        ParticleSystemComponent->Template = Template;
        
    }

    SetActorTickInEditor(true);
}

void AParticleActor::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    if (ParticleSystemComponent && ParticleSystemComponent->Template)
    {
        // 0. 현재 이미터가 정말 없는지 확인 (디버깅)
        UE_LOG(LogLevel::Warning, TEXT("AParticleActor::PostInitializeComponents() - Template Emitters Count BEFORE test: %d"), ParticleSystemComponent->Template->Emitters.Num());

        // 1. 테스트용 이미터 생성 (실제로는 UParticleSpriteEmitter 등 구체적인 클래스여야 함)
        // 직접 구현한 엔진의 UParticleEmitter 또는 그 파생 클래스를 사용해야 합니다.
        // 예시이므로, 실제로는 더 많은 설정이 필요합니다.

        UParticleEmitter* TestEmitter = FObjectFactory::ConstructObject<UParticleEmitter>(ParticleSystemComponent->Template); // Outer를 Template으로 설정
        if (TestEmitter)
        {
            // 2. 테스트용 LOD 레벨 생성 및 설정
            UParticleLODLevel* LODLevel = FObjectFactory::ConstructObject<UParticleLODLevel>(TestEmitter);
            if (LODLevel)
            {
                LODLevel->Level = 0; // LOD 인덱스
                LODLevel->bEnabled = true;

                // 3. 테스트용 필수 모듈(RequiredModule) 생성 및 설정
                UParticleModuleRequired* RequiredModule = FObjectFactory::ConstructObject<UParticleModuleRequired>(LODLevel);
                if (RequiredModule)
                {
                    // RequiredModule의 기본값 설정 (예: 머티리얼, 지속시간 등)
                    // RequiredModule->Material = LoadObject<UMaterial>(nullptr, TEXT("Material'/Game/Path/To/MyParticleMaterial.MyParticleMaterial'")); // 실제 머티리얼 경로
                    RequiredModule->EmitterDuration = 1.0f;
                    LODLevel->RequiredModule = RequiredModule;
                    LODLevel->Modules.Add(RequiredModule); // Modules 배열에도 추가
                }

                // 4. 테스트용 스폰 모듈(SpawnModule) 생성 및 설정
                UParticleModuleSpawn* SpawnModule = FObjectFactory::ConstructObject<UParticleModuleSpawn>(LODLevel);
                if (SpawnModule)
                {
                    // SpawnModule의 기본값 설정 (예: 스폰 속도)
                    // SpawnModule->Rate.Constant = 10.0f; // FDistributionFloatConstant 등을 사용해야 함
                    LODLevel->SpawnModule = SpawnModule;
                    LODLevel->Modules.Add(SpawnModule); // Modules 배열에도 추가
                    LODLevel->SpawnModules.Add(SpawnModule); // 특수 리스트에도 추가 (UpdateModuleLists가 처리하지만, 테스트를 위해 직접 추가)
                }
                UParticleModuleTypeDataMesh* TypeDataModule = FObjectFactory::ConstructObject<UParticleModuleTypeDataMesh>(LODLevel);
                if (TypeDataModule)
                {
                    //TypeDataModule->CreateInstance(TestEmitter, ParticleSystemComponent);
                    LODLevel->Modules.Add(TypeDataModule);
                    
                }
                TestEmitter->LODLevels.Add(LODLevel);
            }
            // 5. 생성한 테스트 이미터를 Template의 Emitters 배열에 추가
            ParticleSystemComponent->Template->Emitters.Add(TestEmitter);

            UE_LOG(LogLevel::Warning, TEXT("AParticleActor::PostInitializeComponents() - Added a test emitter. Template Emitters Count AFTER test: %d"), ParticleSystemComponent->Template->Emitters.Num());
        }
        else
        {
            UE_LOG(LogLevel::Error, TEXT("AParticleActor::PostInitializeComponents() - Failed to create TestEmitter."));
        }

        ParticleSystemComponent->InitializeSystem();
    }
    else
    {
        if (!ParticleSystemComponent)
        {
            UE_LOG(LogLevel::Error, TEXT("AParticleActor::PostInitializeComponents() - ParticleSystemComponent is null."));
        }
        else if (!ParticleSystemComponent->Template)
        {
            UE_LOG(LogLevel::Error, TEXT("AParticleActor::PostInitializeComponents() - ParticleSystemComponent->Template is null. Assign a UParticleSystem asset in the editor or code."));
        }
    }

}
