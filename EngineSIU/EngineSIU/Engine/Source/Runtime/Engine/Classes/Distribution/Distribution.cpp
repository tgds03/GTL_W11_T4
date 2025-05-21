#include "Distribution.h"

#include "RandomStream.h"
#include "ImGUI/imgui.h"
#include "UnrealEd/ImGuiWidget.h"

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

void FDistributionFloatConstant::RenderProperty()
{
    ImGui::InputFloat("Constant", &Constant);
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

void FDistributionFloatUniform::RenderProperty()
{
    ImGui::InputFloat("Min", &Min);
    ImGui::InputFloat("Max", &Max);
}


FVector FDistributionVector::GetValue(float F, FRandomStream* InRandomStream) const
{
    return FVector::ZeroVector;
}

void FDistributionVector::GetOutRange(FVector& Min, FVector& Max) const
{
    Min = FVector::ZeroVector;
    Max = FVector::ZeroVector;
}

FVector FDistributionVectorConstant::GetValue(float F, FRandomStream* InRandomStream) const
{
    return Constant;
}

void FDistributionVectorConstant::GetOutRange(FVector& Min, FVector& Max) const
{
    Min = Constant;
    Max = Constant;
}

void FDistributionVectorConstant::RenderProperty()
{
    FImGuiWidget::DrawVec3Control("Constant", Constant);
}

FVector FDistributionVectorUniform::GetValue(float F, FRandomStream* InRandomStream) const
{
    FVector LocalMax = Max;
    FVector LocalMin = Min;
    
    float fX = (InRandomStream == nullptr) ? FMath::RandNormalized() : InRandomStream->GetFraction();
    float fY = (InRandomStream == nullptr) ? FMath::RandNormalized() : InRandomStream->GetFraction();
    float fZ = (InRandomStream == nullptr) ? FMath::RandNormalized() : InRandomStream->GetFraction();


    fX = LocalMax.X + (LocalMin.X - LocalMax.X) * fX;
    fY = LocalMax.Y + (LocalMin.Y - LocalMax.Y) * fY;
    fZ = LocalMax.Z + (LocalMin.Z - LocalMax.Z) * fZ;
    
    return FVector(fX, fY, fZ);
}

void FDistributionVectorUniform::GetOutRange(FVector& Min, FVector& Max) const
{
    Min = this->Min;
    Max = this->Max;
}

void FDistributionVectorUniform::RenderProperty()
{
    FImGuiWidget::DrawVec3Control("Min", Min);
    FImGuiWidget::DrawVec3Control("Max", Max);
}
