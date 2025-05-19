#include "Distribution.h"

float FDistributionFloat::GetValue(float F) const
{
    return 0.f;
}

void FDistributionFloat::GetOutRange(float& Min, float& Max) const
{
    Min = 0.f;
    Max = 0.f;
}

float FDistributionFloatConstant::GetValue(float F) const
{
    return Constant;
}

void FDistributionFloatConstant::GetOutRange(float& Min, float& Max) const
{
    Min = Constant;
    Max = Constant;
}

float FDistributionFloatUniform::GetValue(float F) const
{
    return Min + (Max - Min) * FMath::RandNormalized();
}

void FDistributionFloatUniform::GetOutRange(float& Min, float& Max) const
{
    Min = this->Min;
    Max = this->Max;
}


FVector FDistributionVector::GetValue(float F) const
{
    return FVector::ZeroVector;
}

FVector FDistributionVectorConstant::GetValue(float F) const
{
    return Constant;
}

FVector FDistributionVectorUniform::GetValue(float F) const
{
    return Min + (Max - Min) * FMath::RandNormalized();
}
