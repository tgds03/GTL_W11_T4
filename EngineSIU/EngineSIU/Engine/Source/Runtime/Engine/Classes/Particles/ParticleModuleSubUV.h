#pragma once
#include "EnumAsByte.h"
#include "ParticleModule.h"
#include "Distribution/Distribution.h"

enum EParticleSubUVInterpMethod : int;

enum EOpacitySourceMode : int
{
    OSM_Alpha,
    OSM_ColorBrightness,
    OSM_RedChannel,
    OSM_GreenChannel,
    OSM_BlueChannel
};

class UParticleModuleSubUV: public UParticleModule
{
    DECLARE_CLASS(UParticleModuleSubUV, UParticleModule)
public:
    UParticleModuleSubUV();

    /** UTexture2D & USubUVAnimation */
    FTexture* SubUVTexture;

    /** The number of sub-images horizontally in the texture							*/
    int32 SubImages_Horizontal;

    /** The number of sub-images vertically in the texture								*/
    int32 SubImages_Vertical;
    
    TEnumAsByte<EOpacitySourceMode> OpacitySourceMode;
    float AlphaThreshold;

    /** UParticleModuleSubUV */
    FDistributionFloatUniform SubImageIndex;
    virtual EModuleType GetModuleType() const override { return EModuleType::EPMT_SubUV; }

    virtual void Spawn(FParticleEmitterInstance* Owner, uint32 Offset, float SpawnTime, FBaseParticle* ParticleBase) override;
    virtual void Update(FParticleEmitterInstance* Owner, uint32 Offset, float DeltaTime) override;

    virtual float DetermineImageIndex(FParticleEmitterInstance* Owner, int32 Offset, FBaseParticle* Particle, EParticleSubUVInterpMethod InterpMethod, FFullSubUVPayload& SubUVPayload, float DeltaTime);

    void SetToSensibleDefaults(UParticleEmitter* Owner) override;
};
