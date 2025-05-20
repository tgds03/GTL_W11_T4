#pragma once
#include <windows.h>

#include "World/WorldType.h" 

#ifndef __ICON_FONT_INDEX__

#define __ICON_FONT_INDEX__
#define DEFAULT_FONT        0
#define    FEATHER_FONT        1

#endif // !__ICON_FONT_INDEX__

class FEditorPanel
{
public:
    virtual ~FEditorPanel() = default;

    virtual void Render() = 0;
    virtual void OnResize(HWND hWnd) = 0;

    void SetVisibleInWorldType(EWorldType InWorldType)
    {
        VisibleInWorldType = InWorldType;
    }

public:
    EWorldType VisibleInWorldType = EWorldType::None;
};
