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

    FBone* GetPose() const { return TargetPose; }
    void SetPose(FBone* InPose);

private:
    UGizmoJointComponent* Joint;
    UGizmoFrameComponent* Frame;

    FBone* TargetPose;
    FEditorViewportClient* AttachedViewport;
};

