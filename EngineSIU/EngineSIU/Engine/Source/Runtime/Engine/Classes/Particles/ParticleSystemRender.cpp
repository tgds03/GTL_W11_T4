#include "ParticleEmitter.h"
#include "ParticleEmitterInstances.h"
#include "ParticleHelper.h"
#include "Components/Mesh/StaticMeshRenderData.h"
#include "Math/Quat.h"
#include "TypeData/ParticleModuleTypeDataMesh.h"

FVector2D GetParticleSize(const FBaseParticle& Particle, const FDynamicSpriteEmitterReplayDataBase& Source)
{
    FVector2D Size;
    Size.X = FMath::Abs(Particle.Size.X * Source.Scale.X);
    Size.Y = FMath::Abs(Particle.Size.Y * Source.Scale.Y);
    // if (Source.ScreenAlignment == PSA_Square || Source.ScreenAlignment == PSA_FacingCameraPosition || Source.ScreenAlignment == PSA_FacingCameraDistanceBlend)
    {
        Size.Y = Size.X;
    }

    return Size;
}

bool FDynamicSpriteEmitterData::GetVertexAndIndexData(void* VertexData, FParticleOrder* ParticleOrder, const FVector& InCameraPosition, const FMatrix& InLocalToWorld) const
{
    //SCOPE_CYCLE_COUNTER(STAT_ParticlePackingTime);
    int32 ParticleCount = Source.ActiveParticleCount;
    // 'clamp' the number of particles actually drawn
    //@todo.SAS. If sorted, we really want to render the front 'N' particles...
    // right now it renders the back ones. (Same for SubUV draws)
    if ((Source.MaxDrawCount >= 0) && (ParticleCount > Source.MaxDrawCount))
    {
        ParticleCount = Source.MaxDrawCount;
    }
 
    // Put the camera origin in the appropriate coordinate space.
    FVector CameraPosition = InCameraPosition;
    if (Source.bUseLocalSpace)
    {
        FMatrix InvSelf = FMatrix::Inverse(InLocalToWorld);
        CameraPosition = InvSelf.TransformPosition(InCameraPosition);
    }
 
    // Pack the data
    int32	ParticleIndex;
    // int32	ParticlePackingIndex = 0;
    // int32	IndexPackingIndex = 0;
 
    int32 VertexStride = sizeof(FParticleSpriteVertex);
    uint8* TempVert = (uint8*)VertexData;
 
    //FVector4 DynamicParameterValue(1.0f,1.0f,1.0f,1.0f);
    FVector ParticlePosition;
    FVector ParticleOldPosition;
    float SubImageIndex = 0.0f;
 
    const uint8* ParticleData = Source.DataContainer.ParticleData;
    const uint16* ParticleIndices = Source.DataContainer.ParticleIndices;
    const FParticleOrder* OrderedIndices = ParticleOrder;
 
    //const FVector LWCTileOffset = FVector(Source.LWCTile) * FLargeWorldRenderScalar::GetTileSize();
    for (int32 i = 0; i < ParticleCount; i++)
    {
        ParticleIndex = OrderedIndices ? OrderedIndices[i].ParticleIndex : i;
        DECLARE_PARTICLE_CONST(Particle, ParticleData + Source.ParticleStride * ParticleIndices[ParticleIndex]);
        if (i + 1 < ParticleCount)
        {
            int32 NextIndex = OrderedIndices ? OrderedIndices[i+1].ParticleIndex : (i + 1);
            DECLARE_PARTICLE_CONST(NextParticle, ParticleData + Source.ParticleStride * ParticleIndices[NextIndex]);
            // FPlatformMisc::Prefetch(&NextParticle);
        }
 
        const FVector2D Size = GetParticleSize(Particle, Source);
 
        ParticlePosition = Particle.Location;
        ParticleOldPosition = Particle.OldLocation;
 
        // ApplyOrbitToPosition(Particle, Source, InLocalToWorld, ParticlePosition, ParticleOldPosition);
 
        // if (Source.CameraPayloadOffset != 0)
        // {
        //     FVector CameraOffset = GetCameraOffsetFromPayload(Source.CameraPayloadOffset, Particle, ParticlePosition, CameraPosition);
        //     ParticlePosition += CameraOffset;
        //     ParticleOldPosition += CameraOffset;
        // }
 
        if (Source.SubUVDataOffset > 0)
        {
            // FFullSubUVPayload* SubUVPayload = (FFullSubUVPayload*)(((uint8*)&Particle) + Source.SubUVDataOffset);
            // SubImageIndex = SubUVPayload->ImageIndex;
        }
 
        // if (Source.DynamicParameterDataOffset > 0)
        // {
        //     GetDynamicValueFromPayload(Source.DynamicParameterDataOffset, Particle, DynamicParameterValue);
        // }
 
        //@todo - refactor into instance step rate in the RHI
		
		
        FParticleSpriteVertex* FillVertex = (FParticleSpriteVertex*)TempVert;
        FillVertex->Position = FVector(ParticlePosition);
        FillVertex->RelativeTime = Particle.RelativeTime;
        FillVertex->OldPosition = FVector(ParticleOldPosition);
        // Create a floating point particle ID from the counter, map into approximately 0-1
        // FillVertex->ParticleId = (Particle.Flags & STATE_CounterMask) / 10000.0f;
        // FillVertex->Size = FVector2D(GetParticleSizeWithUVFlipInSign(Particle, Size));
        FillVertex->Rotation = Particle.Rotation;
        FillVertex->SubImageIndex = SubImageIndex;
        FillVertex->Color = Particle.Color;
 
        TempVert += VertexStride;
    }
 
    return true;
}


///////////////////////////////////////////////////////////////////////////////
//	FDynamicMeshEmitterData
///////////////////////////////////////////////////////////////////////////////

FDynamicMeshEmitterData::FDynamicMeshEmitterData(const UParticleModuleRequired* RequiredModule)
    : FDynamicSpriteEmitterDataBase(RequiredModule)
    , LastFramePreRendered(-1)
    , StaticMesh( NULL )
    , MeshTypeDataOffset(0xFFFFFFFF)
    , bApplyPreRotation(false)
    , bUseMeshLockedAxis(false)
    , bUseCameraFacing(false)
    , bApplyParticleRotationAsSpin(false)
    , bFaceCameraDirectionRatherThanPosition(false)
    , CameraFacingOption(0)
    // , LastCalculatedMeshLOD(0)
    , EmitterInstance(nullptr)
{
    // only update motion blur transforms if we are not paused
    // bPlayersOnlyPending allows us to keep the particle transforms 
    // from the last ticked frame
}

FDynamicMeshEmitterData::~FDynamicMeshEmitterData()
{
}

/** Initialize this emitter's dynamic rendering data, called after source data has been filled in */
void FDynamicMeshEmitterData::Init( const FParticleMeshEmitterInstance* InEmitterInstance,
									UStaticMesh* InStaticMesh,
									float InLODSizeScale)
{
	EmitterInstance = InEmitterInstance;

	// // @todo: For replays, currently we're assuming the original emitter instance is bound to the same mesh as
	// //        when the replay was generated (safe), and various mesh/material indices are intact.  If
	// //        we ever support swapping meshes/material on the fly, we'll need cache the mesh
	// //        reference and mesh component/material indices in the actual replay data.
 //
	// StaticMesh = InStaticMesh;
	// LODSizeScale = InLODSizeScale;
 //    
	// if (StaticMesh == nullptr) return;
 //    if (Source.ParticleStride >= 2 * 1024) return;
 //
	// TArray<UMaterialInterface*, TInlineAllocator<2> > MeshMaterialsGT;
	// InEmitterInstance->GetMeshMaterials(
	// 	MeshMaterialsGT,
	// 	InEmitterInstance->SpriteTemplate->LODLevels[InEmitterInstance->CurrentLODLevelIndex]);	
 //
	// for (int32 i = 0; i < MeshMaterialsGT.Num(); ++i)
	// {
	// 	UMaterialInterface* RenderMaterial = MeshMaterialsGT[i];
	// 	if (RenderMaterial == NULL  || (RenderMaterial->CheckMaterialUsage_Concurrent(MATUSAGE_MeshParticles) == false))
	// 	{
	// 		MeshMaterialsGT[i] = UMaterial::GetDefaultMaterial(MD_Surface);
	// 	}
	// }
 //
	// MeshMaterials.AddZeroed(MeshMaterialsGT.Num());
	// for (int32 i = 0; i < MeshMaterialsGT.Num(); ++i)
	// {
	// 	MeshMaterials[i] = MeshMaterialsGT[i]->GetRenderProxy();
	// }
 //
 //
	// // Find the offset to the mesh type data 
	// if (InEmitterInstance->MeshTypeData != NULL)
	// {
	// 	UParticleModuleTypeDataMesh* MeshTD = InEmitterInstance->MeshTypeData;
	// 	// offset to the mesh emitter type data
	// 	MeshTypeDataOffset = InEmitterInstance->TypeDataOffset;
 //
	// 	FVector Mins, Maxs;
	// 	MeshTD->RollPitchYawRange.GetRange(Mins, Maxs);
 //
	// 	// Enable/Disable pre-rotation
	// 	if (Mins.SizeSquared() || Maxs.SizeSquared())
	// 	{
	// 		bApplyPreRotation = true;
	// 	}
	// 	else
	// 	{
	// 		bApplyPreRotation = false;
	// 	}
 //
	// 	// Setup the camera facing options
	// 	if (MeshTD->bCameraFacing == true)
	// 	{
	// 		bUseCameraFacing = true;
	// 		CameraFacingOption = MeshTD->CameraFacingOption;
	// 		bApplyParticleRotationAsSpin = MeshTD->bApplyParticleRotationAsSpin;
	// 		bFaceCameraDirectionRatherThanPosition = MeshTD->bFaceCameraDirectionRatherThanPosition;
	// 	}
	// 	else
	// 	{
	// 		bUseCameraFacing = false;
	// 		CameraFacingOption = 0;
	// 		bApplyParticleRotationAsSpin = false;
	// 		bFaceCameraDirectionRatherThanPosition = false;
	// 	}
 //
	// 	// Camera facing trumps locked axis... but can still use it.
	// 	// Setup the locked axis option
	// 	uint8 CheckAxisLockOption = MeshTD->AxisLockOption;
	// 	if ((CheckAxisLockOption >= EPAL_X) && (CheckAxisLockOption <= EPAL_NEGATIVE_Z))
	// 	{
	// 		bUseMeshLockedAxis = true;
	// 		Source.LockedAxis = FVector(
	// 			(CheckAxisLockOption == EPAL_X) ? 1.0f : ((CheckAxisLockOption == EPAL_NEGATIVE_X) ? -1.0f :  0.0),
	// 			(CheckAxisLockOption == EPAL_Y) ? 1.0f : ((CheckAxisLockOption == EPAL_NEGATIVE_Y) ? -1.0f :  0.0),
	// 			(CheckAxisLockOption == EPAL_Z) ? 1.0f : ((CheckAxisLockOption == EPAL_NEGATIVE_Z) ? -1.0f :  0.0)
	// 			);
	// 	}
	// 	else if ((CameraFacingOption >= LockedAxis_ZAxisFacing) && (CameraFacingOption <= LockedAxis_NegativeYAxisFacing))
	// 	{
	// 		// Catch the case where we NEED locked axis...
	// 		bUseMeshLockedAxis = true;
	// 		Source.LockedAxis = FVector(1.0f, 0.0f, 0.0f);
	// 	}
	// }
 //
	// // We won't need this on the render thread
	// Source.MaterialInterface = NULL;
}

// TODO DynamicParameterData, PrevTransformBuffer, UV, LOD는 구현 안함.
void FDynamicMeshEmitterData::GetInstanceData(void* InstanceData, const FMatrix& InLocalToWorld) const
{
	// SCOPE_CYCLE_COUNTER(STAT_ParticlePackingTime);
	// SCOPE_CYCLE_COUNTER(STAT_ParticlesOverview_RT_CNC);

	int32 SubImagesX = Source.SubImages_Horizontal;
	int32 SubImagesY = Source.SubImages_Vertical;

    // TODO 코드 변경 시 렌더링 시에도 Size 체크하세요
	int32 ParticleCount = Source.ActiveParticleCount;
	if ((Source.MaxDrawCount >= 0) && (ParticleCount > Source.MaxDrawCount))
	{
		ParticleCount = Source.MaxDrawCount;
	}

	int32 InstanceVertexStride = sizeof(FMeshParticleInstanceVertex);

	uint8* TempVert = (uint8*)InstanceData;

	for (int32 i = ParticleCount - 1; i >= 0; i--)
	{
		const int32	CurrentIndex	= Source.DataContainer.ParticleIndices[i];
		const uint8* ParticleBase	= Source.DataContainer.ParticleData + CurrentIndex * Source.ParticleStride;
		const FBaseParticle& Particle = *((const FBaseParticle*) ParticleBase);

		FMeshParticleInstanceVertex CurrentInstanceVertex;
		
		// Populate instance buffer;
		// The particle color.
		CurrentInstanceVertex.Color = Particle.Color;
		
		// Instance to world transformation. Translation (Instance world position) is packed into W
		FMatrix TransMat(FMatrix::Identity);
		GetParticleTransform(Particle, InLocalToWorld, TransMat);
		
		// Transpose on CPU to allow for simpler shader code to perform the transform. 
		const FMatrix Transpose = FMatrix::Transpose(TransMat);
		CurrentInstanceVertex.Transform[0] = FVector4(Transpose.M[0][0], Transpose.M[0][1], Transpose.M[0][2], Transpose.M[0][3]);
		CurrentInstanceVertex.Transform[1] = FVector4(Transpose.M[1][0], Transpose.M[1][1], Transpose.M[1][2], Transpose.M[1][3]);
		CurrentInstanceVertex.Transform[2] = FVector4(Transpose.M[2][0], Transpose.M[2][1], Transpose.M[2][2], Transpose.M[2][3]);

		// Particle velocity. Calculate on CPU to avoid computing in vertex shader.
		// Note: It would be preferred if we could check whether the material makes use of the 'Particle Direction' node to avoid this work.
		FVector DeltaPosition(Particle.Location - Particle.OldLocation);

	    // TODO 궤도 회전
		int32 CurrentOffset = Source.OrbitModuleOffset;
		if (CurrentOffset != 0)
		{
			// FOrbitChainModuleInstancePayload& OrbitPayload = *((FOrbitChainModuleInstancePayload*)((uint8*)&Particle + CurrentOffset));																\
			// DeltaPosition = (Particle.Location + FVector(OrbitPayload.Offset)) - (Particle.OldLocation + FVector(OrbitPayload.PreviousOffset));
		}

		if (!DeltaPosition.IsZero())
		{
			if (Source.bUseLocalSpace)
			{
				DeltaPosition = FMatrix::TransformVector(DeltaPosition, InLocalToWorld);
			}
			FVector Direction;
			float Speed; 
			DeltaPosition.ToDirectionAndLength(Direction, Speed);

			// Pack direction and speed.
			CurrentInstanceVertex.Velocity = FVector4(FVector(Direction), Speed);
		}
		else
		{
			CurrentInstanceVertex.Velocity = FVector4::ZeroVector;
		}
	    
		// The particle's relative time
		CurrentInstanceVertex.RelativeTime = Particle.RelativeTime;

	    FPlatformMemory::Memcpy(TempVert, &CurrentInstanceVertex, InstanceVertexStride);

		TempVert += InstanceVertexStride;
	}
}

void FDynamicMeshEmitterData::GetParticleTransform(const FBaseParticle& InParticle, const FMatrix& InLocalToWorld, FMatrix& OutTransformMat) const
{
    const uint8* ParticleBase = (const uint8*)&InParticle;

    // TODO Mesh Rotation Payload
    // const FMeshRotationPayloadData* RotationPayload = (const FMeshRotationPayloadData*)((const uint8*)&InParticle + Source.MeshRotationOffset);
    // FVector RotationPayloadInitialOrientation(RotationPayload->InitialOrientation);
    // FVector RotationPayloadRotation(RotationPayload->Rotation);

    // TODO Temp
    FVector RotationPayloadInitialOrientation = FVector::ZeroVector; 
    FVector RotationPayloadRotation = FVector::ZeroVector;
    FVector CameraPayloadCameraOffset = FVector::ZeroVector;

    // TODO
    // if (Source.CameraPayloadOffset != 0)
    // {
    //     // Put the camera origin in the appropriate coordinate space.
    //     FVector CameraPosition = View->ViewMatrices.GetViewOrigin();
    //     if (Source.bUseLocalSpace)
    //     {
    //         const FMatrix InvLocalToWorld = FMatrix::Inverse(InLocalToWorld);
    //         CameraPosition = InvLocalToWorld.TransformPosition(CameraPosition);
    //     }
    //
    //     CameraPayloadCameraOffset = GetCameraOffsetFromPayload(Source.CameraPayloadOffset, InParticle, InParticle.Location, CameraPosition);
    // }

    FVector OrbitPayloadOrbitOffset = FVector::ZeroVector;
    // TODO 궤도 회전
    if (Source.OrbitModuleOffset != 0)
    {
        // int32 CurrentOffset = Source.OrbitModuleOffset;
        // PARTICLE_ELEMENT(FOrbitChainModuleInstancePayload, OrbitPayload);
        // OrbitPayloadOrbitOffset = (FVector)OrbitPayload.Offset;
    }

    CalculateParticleTransform(
        InLocalToWorld,
        InParticle.Location,
        InParticle.Rotation,
        InParticle.Velocity,
        InParticle.Size,
        (FVector)RotationPayloadInitialOrientation,
        (FVector)RotationPayloadRotation,
        CameraPayloadCameraOffset,
        (FVector)OrbitPayloadOrbitOffset,
        // View->ViewMatrices.GetViewOrigin(),
        // (FVector)View->GetViewDirection(),
        OutTransformMat
        );
}

void FDynamicMeshEmitterData::CalculateParticleTransform(const FMatrix& ProxyLocalToWorld, const FVector& ParticleLocation, float ParticleRotation,
    const FVector& ParticleVelocity, const FVector& ParticleSize, const FVector& ParticlePayloadInitialOrientation,
    const FVector& ParticlePayloadRotation, const FVector& ParticlePayloadCameraOffset, const FVector& ParticlePayloadOrbitOffset,
    // const FVector& ViewOrigin, const FVector& ViewDirection,
    FMatrix& OutTransformMat) const
{
	FVector CameraFacingOpVector = FVector::ZeroVector;
	if (CameraFacingOption != XAxisFacing_NoUp)
	{
		switch (CameraFacingOption)
		{
		case XAxisFacing_ZUp:
			CameraFacingOpVector = FVector(0.0f, 0.0f, 1.0f);
			break;
		case XAxisFacing_NegativeZUp:
			CameraFacingOpVector = FVector(0.0f, 0.0f, -1.0f);
			break;
		case XAxisFacing_YUp:
			CameraFacingOpVector = FVector(0.0f, 1.0f, 0.0f);
			break;
		case XAxisFacing_NegativeYUp:
			CameraFacingOpVector = FVector(0.0f, -1.0f, 0.0f);
			break;
		case LockedAxis_YAxisFacing:
		case VelocityAligned_YAxisFacing:
			CameraFacingOpVector = FVector(0.0f, 1.0f, 0.0f);
			break;
		case LockedAxis_NegativeYAxisFacing:
		case VelocityAligned_NegativeYAxisFacing:
			CameraFacingOpVector = FVector(0.0f, -1.0f, 0.0f);
			break;
		case LockedAxis_ZAxisFacing:
		case VelocityAligned_ZAxisFacing:
			CameraFacingOpVector = FVector(0.0f, 0.0f, 1.0f);
			break;
		case LockedAxis_NegativeZAxisFacing:
		case VelocityAligned_NegativeZAxisFacing:
			CameraFacingOpVector = FVector(0.0f, 0.0f, -1.0f);
			break;
		}
	}

	FQuat PointToLockedAxis = FQuat::Identity;
	if (bUseMeshLockedAxis == true)
	{
		// facing axis is taken to be the local x axis.	
		PointToLockedAxis = FQuat::FindBetweenNormals(FVector(1, 0, 0), (FVector)Source.LockedAxis);
	}

	OutTransformMat = FMatrix::Identity;

	FMatrix kTransMat = FMatrix::Identity;
	FMatrix kScaleMat = FMatrix::Identity;
	FQuat kLockedAxisQuat = FQuat::Identity;

	FVector ParticlePosition(ParticleLocation + ParticlePayloadCameraOffset);
	kTransMat.M[3][0] = ParticlePosition.X;
	kTransMat.M[3][1] = ParticlePosition.Y;
	kTransMat.M[3][2] = ParticlePosition.Z;

	FVector ScaledSize = ParticleSize * Source.Scale;
	kScaleMat.M[0][0] = ScaledSize.X;
	kScaleMat.M[1][1] = ScaledSize.Y;
	kScaleMat.M[2][2] = ScaledSize.Z;

	FMatrix kRotMat(FMatrix::Identity);
	FMatrix LocalToWorld = ProxyLocalToWorld;

	FVector	LocalSpaceFacingAxis;
	FVector	LocalSpaceUpAxis;
	FVector Location;
	FVector	DirToCamera;
	FQuat PointTo = PointToLockedAxis;

	// if (bUseCameraFacing)
	// {
	// 	Location = ParticlePosition;
	// 	FVector	VelocityDirection(ParticleVelocity);
	//
	// 	if (Source.bUseLocalSpace)
	// 	{
	// 		bool bClearLocal2World = false;
	//
	// 		// Transform the location to world space
	// 		Location = LocalToWorld.TransformPosition(Location);
	// 		if (CameraFacingOption <= XAxisFacing_NegativeYUp)
	// 		{
	// 			bClearLocal2World = true;
	// 		}
	// 		else if (CameraFacingOption >= VelocityAligned_ZAxisFacing)
	// 		{
	// 			bClearLocal2World = true;
	// 			VelocityDirection = LocalToWorld.InverseFast().GetTransposed().TransformVector(VelocityDirection);
	// 		}
	//
	// 		if (bClearLocal2World)
	// 		{
	// 			// Set the translation matrix to the location
	// 			kTransMat.SetOrigin(Location);
	// 			// Set Local2World to identify to remove any rotational information
	// 			LocalToWorld.SetIdentity();
	// 		}
	// 	}
	// 	VelocityDirection.Normalize();
	//
	// 	if (bFaceCameraDirectionRatherThanPosition)
	// 	{
	// 		DirToCamera = (FVector)-ViewDirection;
	// 	}
	// 	else
	// 	{
	// 		DirToCamera = ViewOrigin - Location;
	// 	}
	//
	// 	DirToCamera.Normalize();
	// 	if (DirToCamera.SizeSquared() < 0.5f)
	// 	{
	// 		// Assert possible if DirToCamera is not normalized
	// 		DirToCamera = FVector(1, 0, 0);
	// 	}
	//
	// 	bool bFacingDirectionIsValid = true;
	// 	if (CameraFacingOption != XAxisFacing_NoUp)
	// 	{
	// 		FVector FacingDir;
	// 		FVector DesiredDir;
	//
	// 		if ((CameraFacingOption >= VelocityAligned_ZAxisFacing) &&
	// 			(CameraFacingOption <= VelocityAligned_NegativeYAxisFacing))
	// 		{
	// 			if (VelocityDirection.IsNearlyZero())
	// 			{
	// 				// We have to fudge it
	// 				bFacingDirectionIsValid = false;
	// 			}
	//
	// 			// Velocity align the X-axis, and camera face the selected axis
	// 			PointTo = FQuat::FindBetweenNormals(FVector(1.0f, 0.0f, 0.0f), VelocityDirection);
	// 			FacingDir = VelocityDirection;
	// 			DesiredDir = DirToCamera;
	// 		}
	// 		else if (CameraFacingOption <= XAxisFacing_NegativeYUp)
	// 		{
	// 			// Camera face the X-axis, and point the selected axis towards the world up
	// 			PointTo = FQuat::FindBetweenNormals(FVector(1, 0, 0), DirToCamera);
	// 			FacingDir = DirToCamera;
	// 			DesiredDir = FVector(0, 0, 1);
	// 		}
	// 		else
	// 		{
	// 			// Align the X-axis with the selected LockAxis, and point the selected axis towards the camera
	// 			// PointTo will contain quaternion for locked axis rotation.
	// 			FacingDir = (FVector)Source.LockedAxis;
	//
	// 			if (Source.bUseLocalSpace)
	// 			{
	// 				//Transform the direction vector into local space.
	// 				DesiredDir = LocalToWorld.GetTransposed().TransformVector(DirToCamera);
	// 			}
	// 			else
	// 			{
	// 				DesiredDir = DirToCamera;
	// 			}
	// 		}
	//
	// 		FVector	DirToDesiredInRotationPlane = DesiredDir - ((DesiredDir | FacingDir) * FacingDir);
	// 		DirToDesiredInRotationPlane.Normalize();
	// 		FQuat FacingRotation = FQuat::FindBetweenNormals(PointTo.RotateVector(CameraFacingOpVector), DirToDesiredInRotationPlane);
	// 		PointTo = FacingRotation * PointTo;
	//
	// 		// Add in additional rotation about either the directional or camera facing axis
	// 		if (bApplyParticleRotationAsSpin)
	// 		{
	// 			if (bFacingDirectionIsValid)
	// 			{
	// 				FQuat AddedRotation = FQuat(FacingDir, ParticleRotation);
	// 				kLockedAxisQuat = (AddedRotation * PointTo);
	// 			}
	// 		}
	// 		else
	// 		{
	// 			FQuat AddedRotation = FQuat(DirToCamera, ParticleRotation);
	// 			kLockedAxisQuat = (AddedRotation * PointTo);
	// 		}
	// 	}
	// 	else
	// 	{
	// 		PointTo = FQuat::FindBetweenNormals(FVector(1, 0, 0), DirToCamera);
	// 		// Add in additional rotation about facing axis
	// 		FQuat AddedRotation = FQuat(DirToCamera, ParticleRotation);
	// 		kLockedAxisQuat = (AddedRotation * PointTo);
	// 	}
	// }
	// else if (bUseMeshLockedAxis)
	// {
	// 	// Add any 'sprite rotation' about the locked axis
	// 	FQuat AddedRotation = FQuat((FVector)Source.LockedAxis, ParticleRotation);
	// 	kLockedAxisQuat = (AddedRotation * PointTo);
	// }
	// else if (Source.ScreenAlignment == PSA_TypeSpecific)
	// {
	// 	Location = ParticlePosition;
	// 	if (Source.bUseLocalSpace)
	// 	{
	// 		// Transform the location to world space
	// 		Location = LocalToWorld.TransformPosition(Location);
	// 		kTransMat.SetOrigin(Location);
	// 		LocalToWorld.SetIdentity();
	// 	}
	//
	// 	DirToCamera = ViewOrigin - Location;
	// 	DirToCamera.Normalize();
	// 	if (DirToCamera.SizeSquared() < 0.5f)
	// 	{
	// 		// Assert possible if DirToCamera is not normalized
	// 		DirToCamera = FVector(1, 0, 0);
	// 	}
	//
	// 	LocalSpaceFacingAxis = FVector(1, 0, 0); // facing axis is taken to be the local x axis.	
	// 	LocalSpaceUpAxis = FVector(0, 0, 1); // up axis is taken to be the local z axis
	//
	// 	if (Source.MeshAlignment == PSMA_MeshFaceCameraWithLockedAxis)
	// 	{
	// 		// TODO: Allow an arbitrary	vector to serve	as the locked axis
	//
	// 		// For the locked axis behavior, only rotate to	face the camera	about the
	// 		// locked direction, and maintain the up vector	pointing towards the locked	direction
	// 		// Find	the	rotation that points the localupaxis towards the targetupaxis
	// 		FQuat PointToUp = FQuat::FindBetweenNormals(LocalSpaceUpAxis, (FVector)Source.LockedAxis);
	//
	// 		// Add in rotation about the TargetUpAxis to point the facing vector towards the camera
	// 		FVector	DirToCameraInRotationPlane = DirToCamera - FVector((DirToCamera | (FVector)Source.LockedAxis)*Source.LockedAxis);
	// 		DirToCameraInRotationPlane.Normalize();
	// 		FQuat PointToCamera = FQuat::FindBetweenNormals(PointToUp.RotateVector(LocalSpaceFacingAxis), DirToCameraInRotationPlane);
	//
	// 		// Set kRotMat to the composed rotation
	// 		FQuat MeshRotation = PointToCamera*PointToUp;
	// 		kRotMat = FQuatRotationMatrix(MeshRotation);
	// 	}
	// 	else if (Source.MeshAlignment == PSMA_MeshFaceCameraWithSpin)
	// 	{
	// 		// Implement a tangent-rotation	version	of point-to-camera.	 The facing	direction points to	the	camera,
	// 		// with	no roll, and has addtional sprite-particle rotation	about the tangential axis
	// 		// (c.f. the roll rotation is about	the	radial axis)
	//
	// 		// Find	the	rotation that points the facing	axis towards the camera
	// 		FRotator PointToRotation = FRotator(FQuat::FindBetweenNormals(LocalSpaceFacingAxis, DirToCamera));
	//
	// 		// When	constructing the rotation, we need to eliminate	roll around	the	dirtocamera	axis,
	// 		// otherwise the particle appears to rotate	around the dircamera axis when it or the camera	moves
	// 		PointToRotation.Roll = 0;
	//
	// 		// Add in the tangential rotation we do	want.
	// 		FVector	vPositivePitch = FVector(0, 0, 1); //	this is	set	by the rotator's yaw/pitch/roll	reference frame
	// 		FVector	vTangentAxis = vPositivePitch^DirToCamera;
	// 		vTangentAxis.Normalize();
	// 		if (vTangentAxis.SizeSquared() < 0.5f)
	// 		{
	// 			vTangentAxis = FVector(1, 0, 0); // assert is	possible if	FQuat axis/angle constructor is	passed zero-vector
	// 		}
	//
	// 		FQuat AddedTangentialRotation = FQuat(vTangentAxis, ParticleRotation);
	//
	// 		// Set kRotMat to the composed rotation
	// 		FQuat MeshRotation = AddedTangentialRotation*PointToRotation.Quaternion();
	// 		kRotMat = FQuatRotationMatrix(MeshRotation);
	// 	}
	// 	else
	// 	{
	// 		// Implement a roll-rotation version of	point-to-camera.  The facing direction points to the camera,
	// 		// with	no roll, and then rotates about	the	direction_to_camera	by the spriteparticle rotation.
	//
	// 		// Find	the	rotation that points the facing	axis towards the camera
	// 		FRotator PointToRotation = FRotator(FQuat::FindBetweenNormals(LocalSpaceFacingAxis, DirToCamera));
	//
	// 		// When	constructing the rotation, we need to eliminate	roll around	the	dirtocamera	axis,
	// 		// otherwise the particle appears to rotate	around the dircamera axis when it or the camera	moves
	// 		PointToRotation.Roll = 0;
	//
	// 		// Add in the roll we do want.
	// 		FQuat AddedRollRotation = FQuat(DirToCamera, ParticleRotation);
	//
	// 		// Set kRotMat to the composed	rotation
	// 		FQuat MeshRotation = AddedRollRotation*PointToRotation.Quaternion();
	// 		kRotMat = FQuatRotationMatrix(MeshRotation);
	// 	}
	// }
	// else
	{
		float fRot = ParticleRotation * 180.0f / PI;
		FVector kRotVec = FVector(fRot, fRot, fRot);
		FRotator kRotator = FRotator::MakeFromEuler(kRotVec);

		kRotator += FRotator::MakeFromEuler((FVector)ParticlePayloadRotation);

		kRotMat = FMatrix::CreateRotationMatrix(kRotator);
	}

	if (bApplyPreRotation == true)
	{
	    // TODO roll pitch yaw 확인
		FRotator MeshOrient = FRotator::MakeFromEuler((FVector)ParticlePayloadInitialOrientation);
		FMatrix OrientMat = FMatrix::CreateRotationMatrix(MeshOrient);

		if ((bUseCameraFacing == true) || (bUseMeshLockedAxis == true))
		{
			OutTransformMat = (OrientMat * kScaleMat) * kLockedAxisQuat.ToMatrix() * kRotMat * kTransMat;
		}
		else
		{
			OutTransformMat = (OrientMat*kScaleMat) * kRotMat * kTransMat;
		}
	}
	else if ((bUseCameraFacing == true) || (bUseMeshLockedAxis == true))
	{
		OutTransformMat = kScaleMat * kLockedAxisQuat.ToMatrix() * kRotMat * kTransMat;
	}
	else
	{
		OutTransformMat = kScaleMat * kRotMat * kTransMat;
	}

	FVector OrbitOffset(ParticlePayloadOrbitOffset);
	if (Source.bUseLocalSpace == false)
	{
		OrbitOffset = FMatrix::TransformVector(OrbitOffset, LocalToWorld);
	}

	FMatrix OrbitMatrix = FMatrix::CreateTranslationMatrix(OrbitOffset);
	OutTransformMat = OutTransformMat * OrbitMatrix;

	if (Source.bUseLocalSpace)
	{
		OutTransformMat = OutTransformMat * LocalToWorld;
	}
}
