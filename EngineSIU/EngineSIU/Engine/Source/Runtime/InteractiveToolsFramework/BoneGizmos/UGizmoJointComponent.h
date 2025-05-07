#pragma once
#include "BaseGizmos/GizmoBaseComponent.h"

class UGizmoJointComponent : public UGizmoBaseComponent
{
    DECLARE_CLASS(UGizmoJointComponent, UGizmoBaseComponent)

public:
    UGizmoJointComponent() = default;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
};

