#pragma once

#include "Components/SkeletalMesh/SkeletalMeshComponent.h"
#include "UnrealEd/EditorPanel.h"


class SkeletonMeshEditorPanel : public UEditorPanel
{
public:
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;

private:
    // 재귀적으로 본 트리를 렌더링
    void RenderBoneTree(int32 BoneIndex);

    // 선택된 본 인덱스
    int32 SelectedBoneIndex = INDEX_NONE;

    float Width = 0, Height = 0;
};
