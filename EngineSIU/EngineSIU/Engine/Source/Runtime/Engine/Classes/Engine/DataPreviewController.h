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
    UDataPreviewController(UWorld* InWorld, FEditorViewportClient* InViewport)
        : PreivewWorld(InWorld), AttachedViewport(InViewport) {
    }
    ~UDataPreviewController() = default;

    void Initialize(USkeletalMesh* InMesh);
    void Initialize(UAnimInstance* InAnim);
    void Release();

    TArray<ABoneGizmo*> GetBoneGizmos() { return BoneGizmos; }

    void SetBoneGizmo(USkeletalMesh* InMesh);

    int GetBoneIndex(ABoneGizmo* InBoneGizmo);

    int GetSelectedBoneIndex();

    void SetSelectedBoneIndex(int index);

    void SetType(EPreviewType InType) { PreviewType = InType; }
    EPreviewType GetType() const { return PreviewType; }
      
public:
    UWorld* PreivewWorld = nullptr;
    FEditorViewportClient* AttachedViewport = nullptr;

    USkeletalMesh* OriginalMesh = nullptr;
    USkeletalMesh* EdittingMesh = nullptr;

    UAnimInstance* OriginalAnim = nullptr;
    UAnimInstance* EdittingAnim = nullptr;

    TArray<ABoneGizmo*> BoneGizmos;
    ABoneGizmo* SelectedGizmo = nullptr;
    bool isVisibleBone = true;

private:
    EPreviewType PreviewType = EPreviewType::None;
};
