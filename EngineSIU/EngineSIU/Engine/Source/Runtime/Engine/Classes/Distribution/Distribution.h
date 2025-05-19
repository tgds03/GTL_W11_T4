#pragma once
#include "Math/Vector.h"

struct FDistribution
{
    static const float DefaultValue;
};


struct FDistributionFloat: public FDistribution
{
    virtual float GetValue(float F = 0.f) const;
    virtual void GetOutRange(float& Min, float& Max) const;
};

struct FDistributionFloatConstant: public FDistributionFloat
{
    float Constant;

    virtual float GetValue(float F = 0.f) const override;
    virtual void GetOutRange(float& Min, float& Max) const override;
};

struct FDistributionFloatUniform: public FDistributionFloat
{
    float Min;
    float Max;

    virtual float GetValue(float F = 0.f) const override;
    virtual void GetOutRange(float& Min, float& Max) const override;
};



struct FDistributionVector: public FDistribution
{
    virtual FVector GetValue(float F = 0.f) const;
};

struct FDistributionVectorConstant: public FDistributionVector
{
    FVector Constant;

    virtual FVector GetValue(float F = 0.f) const override;
};

struct FDistributionVectorUniform: public FDistributionVector
{
    FVector Min;
    FVector Max;

    virtual FVector GetValue(float F = 0.f) const override;
};
