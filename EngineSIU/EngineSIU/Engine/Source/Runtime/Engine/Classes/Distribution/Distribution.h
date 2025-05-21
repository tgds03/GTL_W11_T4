#pragma once
#include "RandomStream.h"
#include "Math/Vector.h"

enum class EDistributionType: uint8
{
    None,
    FloatConstant,
    FloatUniform,
    VectorConstant,
    VectorUniform
};

struct FDistribution
{
    static const float DefaultValue;
    
    virtual void RenderProperty() {};
    virtual EDistributionType GetType() { return EDistributionType::None; }   
};


struct FDistributionFloat: public FDistribution
{
    virtual float GetValue(float F = 0.f, FRandomStream* InRandomStream = nullptr) const;
    virtual void GetOutRange(float& Min, float& Max) const;

};

struct FDistributionFloatConstant: public FDistributionFloat
{
    float Constant;

    virtual float GetValue(float F = 0.f, FRandomStream* InRandomStream = nullptr) const override;
    virtual void GetOutRange(float& Min, float& Max) const override;
    virtual void RenderProperty() override;
    virtual EDistributionType GetType() override { return EDistributionType::FloatConstant; }   
};

struct FDistributionFloatUniform: public FDistributionFloat
{
    float Min;
    float Max;

    virtual float GetValue(float F = 0.f, FRandomStream* InRandomStream = nullptr) const override;
    virtual void GetOutRange(float& Min, float& Max) const override;
    virtual void RenderProperty() override;
    virtual EDistributionType GetType() override { return EDistributionType::FloatUniform; }   
};



struct FDistributionVector: public FDistribution
{
    virtual FVector GetValue(float F = 0.f, FRandomStream* InRandomStream = nullptr) const;
    virtual void GetOutRange(FVector& Min, FVector& Max) const;
};

struct FDistributionVectorConstant: public FDistributionVector
{
    FVector Constant;

    virtual FVector GetValue(float F = 0.f, FRandomStream* InRandomStream = nullptr) const override;
    virtual void GetOutRange(FVector& Min, FVector& Max) const override;
    virtual void RenderProperty() override;
    virtual EDistributionType GetType() override { return EDistributionType::VectorConstant; }   
};

struct FDistributionVectorUniform: public FDistributionVector
{
    FVector Min;
    FVector Max;

    virtual FVector GetValue(float F = 0.f, FRandomStream* InRandomStream = nullptr) const override;
    virtual void GetOutRange(FVector& Min, FVector& Max) const override;
    virtual void RenderProperty() override;
    virtual EDistributionType GetType() override { return EDistributionType::VectorUniform; }   
};
