#pragma once
#include "ParticleModuleTypeDataBase.h"


class UParticleModuleTypeDataMesh : public UParticleModuleTypeDataBase
{
public:
    UStaticMesh* Mesh;

    /** Random stream for the initial rotation distribution */
    // FRandomStream RandomStream;
    
    /** use the static mesh's LOD setup and switch LODs based on largest particle's screen size*/
    float LODSizeScale;

    /** use the static mesh's LOD setup and switch LODs based on largest particle's screen size*/
    uint8 bUseStaticMeshLODs : 1;

    /** If true, has the meshes cast shadows */
    uint8 CastShadows:1;

    /** UNUSED (the collision module dictates doing collisions) */
    uint8 DoCollisions:1;
    
    virtual bool SupportsSubUV() const override { return true; }
};
