#pragma once

#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Engine/Source/Runtime/Launch/SkeletalDefine.h"
#include "Engine/Source/Runtime/Core/Container/Array.h"
#include "Engine/Source/Runtime/Engine/Classes/Components/SkeletalMesh/SkinnedMeshComponent.h"
#include <Animation/UAnimInstance.h>


class USkeletalMeshComponent : public USkinnedMeshComponent
{
    DECLARE_CLASS(USkeletalMeshComponent, USkinnedMeshComponent)

    std::shared_ptr<UAnimInstance> AnimInstance =nullptr;

public:
    USkeletalMeshComponent();
    ~USkeletalMeshComponent() = default;

    // 현재는 USkeletalMeshComponent에서 SkinnedMeshComponent에 비해 더 해주는 것이 없음
    // 이후에 애니메이션 쪽 구현이 들어갈 때 추가된다고 GPT 가 말하긴 함

    void UpdateAnimation(float deltatime);

    void InitializeAnimInstance();

    // OBJ 파일을 통한 Sample Skeletal Mesh 구현
    void GenerateSampleData();
    // 각도 변화 주기
    void TestSkeletalMesh();

    void TestFBXSkeletalMesh();
};
