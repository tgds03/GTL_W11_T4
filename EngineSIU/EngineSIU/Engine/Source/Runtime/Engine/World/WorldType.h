#pragma once

enum class EWorldType
{
    None,
    Game,
    Editor,
    PIE,
    SkeletalMeshEditor,
    EditorPreview,
    GamePreview,
    GameRPC,
    Inactive
};

enum class EPreviewType
{
    None,
    SkeletalMesh,
    Animation,
};
