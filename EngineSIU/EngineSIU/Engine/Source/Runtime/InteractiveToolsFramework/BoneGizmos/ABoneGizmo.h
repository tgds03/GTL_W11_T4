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

    FBone* GetPose() const { return TargetBone; }
    void SetPose(FBone* InPose);

    UGizmoJointComponent* GetJointComponent() const { return Joint; }
    UGizmoFrameComponent* GetFrameComponent() const { return Frame; }

private:
    UGizmoJointComponent* Joint = nullptr;
    UGizmoFrameComponent* Frame = nullptr;

    FBone* TargetBone;
    FEditorViewportClient* AttachedViewport;
};

