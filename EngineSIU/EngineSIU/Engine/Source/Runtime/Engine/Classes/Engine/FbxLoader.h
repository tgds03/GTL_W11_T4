#pragma once

#include <fbxsdk.h>
#include "Container/Map.h"
#include "UObject/Object.h"
#include "Components/SkeletalMesh/SkeletalMesh.h"

struct FFbxLoader 
{
public:
    static USkeletalMesh* LoadFBXSkeletalMeshAsset(const FString& filePathName);

    static USkeletalMesh* GetSkeletalMesh(const FWString& name);

private:
    inline static TMap<FWString, USkeletalMesh*> SkeletalMeshMap;
};
