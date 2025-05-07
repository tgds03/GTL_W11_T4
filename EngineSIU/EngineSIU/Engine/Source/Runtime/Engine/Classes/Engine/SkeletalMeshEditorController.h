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

    void SetBoneGizmo(FSkeleton* InSkeleton);

    int GetBoneIndex(ABoneGizmo* InBoneGizmo);

    int GetSelectedBoneIndex();
      
public:
    USkeletalMesh* OriginalMesh = nullptr;
    USkeletalMesh* EditingMesh = nullptr;

    TArray<ABoneGizmo*> BoneGizmos;

    FEditorViewportClient* AttachedViewport = nullptr;

    ABoneGizmo* SelectedGizmo = nullptr;
};
