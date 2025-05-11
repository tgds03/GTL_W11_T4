#pragma once
#include "GameFramework/Actor.h"

class UGizmoJointComponent;
class UGizmoFrameComponent;

struct FBone;

class ABoneGizmo : public AActor
{
    DECLARE_CLASS(ABoneGizmo, AActor)

public:
    ABoneGizmo();

    virtual void Tick(float DeltaTime) override;
    void Initialize(FEditorViewportClient* InViewport);

    //FBone* GetPose() const { return TargetBone; }
    //void SetPose(FBone* InPose, TArray<FBone>& Bones);
    void SetPose(USkeletalMesh* InMesh, int32 InBoneIndex);

    void UpdatePose();

    UGizmoJointComponent* GetJointComponent() const { return Joint; }
    UGizmoFrameComponent* GetFrameComponent() const { return Frame; }

    void SetSelected(bool b) { bIsSelected = b; }
    bool IsSelected() const { return bIsSelected; }
    void ToggleSelected() { bIsSelected = !bIsSelected; }
    void Deselect() { bIsSelected = false; }

private:
    bool bIsSelected = false;  

    UGizmoJointComponent* Joint = nullptr;
    UGizmoFrameComponent* Frame = nullptr;

    USkeletalMesh* TargetMesh = nullptr;
    int32 TargetBoneIndex = INDEX_NONE;

    //FBone* TargetBone;
    //TArray<FBone>* TargetBones;
    FEditorViewportClient* AttachedViewport;
};

