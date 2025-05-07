#include "UGizmoFrameComponent.h"

void UGizmoFrameComponent::InitializeComponent()
{
    Super::InitializeComponent();
    SetGizmoType(UGizmoBaseComponent::BoneFrame);
}

void UGizmoFrameComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}
