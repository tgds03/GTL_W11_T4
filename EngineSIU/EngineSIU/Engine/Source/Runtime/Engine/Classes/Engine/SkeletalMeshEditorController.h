#pragma onece
#include "Components/SkeletalMesh/SkeletalMesh.h"

enum class EPreviewType
{
    None,
    SkeletalMesh,
    Animation,
};

class USkeletalMesh;
class UAnimInstance;
class FSkeleton;
class ABoneGizmo;

class UDataPreviewController
{
public:
    void Initialize(USkeletalMesh* InMesh, FEditorViewportClient* InViewport);
    void Initialize(UAnimInstance* InAnim, FEditorViewportClient* InViewport);
    void Release();

    TArray<ABoneGizmo*> GetBoneGizmos() { return BoneGizmos; }

    void SetBoneGizmo(USkeletalMesh* InMesh);

    int GetBoneIndex(ABoneGizmo* InBoneGizmo);

    int GetSelectedBoneIndex();

    void SetSelectedBoneIndex(int index);

    void SetType(EPreviewType InType) { PreviewType = InType; }
    EPreviewType GetType() const { return PreviewType; }
      
public:
    FEditorViewportClient* AttachedViewport = nullptr;

    USkeletalMesh* OriginalMesh = nullptr;
    USkeletalMesh* EditingMesh = nullptr;

    UAnimInstance* OriginalAnim = nullptr;
    UAnimInstance* EditingAnim = nullptr;

    TArray<ABoneGizmo*> BoneGizmos;
    ABoneGizmo* SelectedGizmo = nullptr;
    bool isVisibleBone = true;

private:
    EPreviewType PreviewType = EPreviewType::None;
};
