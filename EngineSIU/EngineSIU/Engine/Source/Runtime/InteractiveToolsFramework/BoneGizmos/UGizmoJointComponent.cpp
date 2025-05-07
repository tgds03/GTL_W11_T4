#include "UGizmoJointComponent.h"

void UGizmoJointComponent::InitializeComponent()
{
    Super::InitializeComponent();
    SetGizmoType(UGizmoBaseComponent::BoneJoint);
}

void UGizmoJointComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}
