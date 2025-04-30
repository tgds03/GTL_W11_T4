#pragma once
#include "CameraTypes.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class APlayerCameraManager;
class UCameraShakePattern;

struct FCameraShakePatternUpdateParams
{
    FCameraShakePatternUpdateParams()
    {}

    FCameraShakePatternUpdateParams(const FMinimalViewInfo& InPOV)
        : POV(InPOV)
    {}

    /** The time elapsed since last update */
    float DeltaTime = 0.f;

    /** The base scale for this shake */
    float ShakeScale = 1.f;
    /** The dynamic scale being passed down from the camera manger for the next update */
    float DynamicScale = 1.f;

    /** The current view that this camera shake should modify */
    FMinimalViewInfo POV;

    /** The total scale to apply to the camera shake during the current update. Equals ShakeScale * DynamicScale */
    float GetTotalScale() const
    {
        return FMath::Max(ShakeScale * DynamicScale, 0.f);
    }
};

struct FCameraShakePatternUpdateResult
{
    FCameraShakePatternUpdateResult()
        : Location(FVector::ZeroVector)
        , Rotation(FRotator::ZeroRotator)
        , FOV(0.f)
    {}

    /** Location offset for the view, or new absolute location if ApplyAsAbsolute flag is set */
    FVector Location;
    /** Rotation offset for the view, or new absolute rotation if ApplyAsAbsolute flag is set */
    FRotator Rotation;
    /** Field-of-view offset for the view, or new absolute field-of-view if ApplyAsAbsolute flag is set */
    float FOV;
};

class UCameraShakeBase : public UObject
{
    DECLARE_CLASS(UCameraShakeBase, UObject)

public:
    UCameraShakeBase();
    
    virtual ~UCameraShakeBase() override = default;

    void UpdateAndApplyCameraShake(float DeltaTime, float Alpha, FMinimalViewInfo& InOutPOV);

    void StartShake();
    
    void StopShake(bool bImmediately = true);

    float ShakeScale;

    UCameraShakePattern* GetRootShakePattern() const { return RootShakePattern; }

    void SetRootShakePattern(UCameraShakePattern* InPattern) { RootShakePattern = InPattern; }

    void ApplyResult(float Scale, const FCameraShakePatternUpdateResult& InResult, FMinimalViewInfo& InOutPOV);

    bool IsFinished() const;
    
private:
    UCameraShakePattern* RootShakePattern;

    APlayerCameraManager* CameraManager;

    bool bIsActive;
};

class UCameraShakePattern : public UObject
{
    DECLARE_CLASS(UCameraShakePattern, UObject)

public:
    UCameraShakePattern() = default;
    virtual ~UCameraShakePattern() override = default;

	/** Called when the shake pattern starts */
	void StartShakePattern();
	/** Updates the shake pattern, which should add its generated offset to the given result */
	void UpdateShakePattern(const FCameraShakePatternUpdateParams& Params, FCameraShakePatternUpdateResult& OutResult);
	/** Returns whether this shake pattern is finished */
	bool IsFinished() const;
	/** Called when the shake pattern is manually stopped */
	void StopShakePattern(bool bImmediately = true);

private:
	// UCameraShakePattern interface
	virtual void StartShakePatternImpl() {}
	virtual void UpdateShakePatternImpl(const FCameraShakePatternUpdateParams& Params, FCameraShakePatternUpdateResult& OutResult) {}
	virtual bool IsFinishedImpl() const { return true; }
	virtual void StopShakePatternImpl(bool bImmediately) {}
	virtual void TeardownShakePatternImpl()  {}
};
