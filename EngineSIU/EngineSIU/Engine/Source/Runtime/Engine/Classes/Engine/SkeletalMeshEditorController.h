#pragma onece
#include "Components/SkeletalMesh/SkeletalMesh.h"

class USkeletalMesh;
class FSkeleton;
class ABoneGizmo;

class SkeletalMeshEditorController
{
public:
    void Initialize(USkeletalMesh* InMesh, FEditorViewportClient* InViewport);
    void Release();

    TArray<ABoneGizmo*> GetBoneGizmos() { return BoneGizmos; }

private:
    void SetBoneGizmo(FSkeleton* InSkeleton);
      
public:
    USkeletalMesh* OriginalMesh = nullptr;
    USkeletalMesh* EditingMesh = nullptr;

    TArray<ABoneGizmo*> BoneGizmos;

    FEditorViewportClient* AttachedViewport = nullptr;
};
