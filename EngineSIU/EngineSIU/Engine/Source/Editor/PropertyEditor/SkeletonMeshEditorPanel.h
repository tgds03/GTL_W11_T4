#pragma once

#include "Components/SkeletalMesh/SkeletalMeshComponent.h"
#include "UnrealEd/EditorPanel.h"

struct FSkeletonSequencePlayer
{
    UAnimSequence* Sequence = nullptr;
    
};

class SkeletonMeshEditorPanel : public UEditorPanel
{
public:
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;

private:
    void RenderCommonEditorHeader();

    void RenderSkeletalMeshEditorUI();
    void RenderAnimationEditorUI();

    // 재귀적으로 본 트리를 렌더링
    void RenderBoneTree(int32 BoneIndex);
    void RenderBonePoseEditor(USkeletalMesh* SkelMesh);

    // 선택된 본 인덱스
    int32 SelectedBoneIndex = INDEX_NONE;

    USkeletalMesh* GetCurrentEdittingMesh() const;
    int32 GetRootBoneIndex(USkeletalMesh* Mesh) const;
    void RenderSequenceUI();

    float Width = 0, Height = 0;
};
