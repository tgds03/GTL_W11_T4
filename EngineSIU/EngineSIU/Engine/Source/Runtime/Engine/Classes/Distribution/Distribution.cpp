#include "Distribution.h"

#include "RandomStream.h"

float FDistributionFloat::GetValue(float F, FRandomStream* InRandomStream) const
{
    return 0.f;
}

void FDistributionFloat::GetOutRange(float& Min, float& Max) const
{
    Min = 0.f;
    Max = 0.f;
}

float FDistributionFloatConstant::GetValue(float F, FRandomStream* InRandomStream) const
{
    return Constant;
}

void FDistributionFloatConstant::GetOutRange(float& Min, float& Max) const
{
    Min = Constant;
    Max = Constant;
}

float FDistributionFloatUniform::GetValue(float F, FRandomStream* InRandomStream) const
{
    float fraction = (InRandomStream == nullptr) ? FMath::RandNormalized() : InRandomStream->GetFraction();
    return Min + (Max - Min) * fraction;
}

void FDistributionFloatUniform::GetOutRange(float& Min, float& Max) const
{
    Min = this->Min;
    Max = this->Max;
}


FVector FDistributionVector::GetValue(float F, FRandomStream* InRandomStream) const
{
    return FVector::ZeroVector;
}

FVector FDistributionVectorConstant::GetValue(float F, FRandomStream* InRandomStream) const
{
    return Constant;
}

FVector FDistributionVectorUniform::GetValue(float F, FRandomStream* InRandomStream) const
{
    float fraction = (InRandomStream == nullptr) ? FMath::RandNormalized() : InRandomStream->GetFraction();
    return Min + (Max - Min) * fraction;
}
