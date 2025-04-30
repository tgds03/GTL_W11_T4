#pragma once

#include "UObject/ObjectMacros.h"
#include "Engine/EngineTypes.h"
#include "Components/SceneComponent.h"

class USpringArmComponent : public USceneComponent
{
    DECLARE_CLASS(USpringArmComponent, USceneComponent)
public:
    USpringArmComponent();
    virtual ~USpringArmComponent() override = default;

    virtual void TickComponent(float DeltaTime) override;

};
