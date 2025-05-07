#pragma once
#include "BaseGizmos/GizmoBaseComponent.h"

class UGizmoFrameComponent : public UGizmoBaseComponent
{
    DECLARE_CLASS(UGizmoFrameComponent, UGizmoBaseComponent)

public:
    UGizmoFrameComponent() = default;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
};

