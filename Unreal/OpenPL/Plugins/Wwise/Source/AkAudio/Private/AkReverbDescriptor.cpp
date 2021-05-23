/*******************************************************************************
The content of the files in this repository include portions of the
AUDIOKINETIC Wwise Technology released in source code form as part of the SDK
package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use these files in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Copyright (c) 2021 Audiokinetic Inc.
*******************************************************************************/

#include "AkReverbDescriptor.h"
#include <AK/SpatialAudio/Common/AkReverbEstimation.h>
#include "AkAudioDevice.h"
#include "AkAcousticTextureSetComponent.h"
#include "AkLateReverbComponent.h"
#include "AkRoomComponent.h"
#include "AkComponentHelpers.h"
#include "AkSettings.h"

#include "Components/PrimitiveComponent.h"
#include "Rendering/PositionVertexBuffer.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/BrushComponent.h"

#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/ConvexElem.h"

#if PHYSICS_INTERFACE_PHYSX
#include "PhysXIncludes.h"
#endif

#if WITH_CHAOS
#include "Chaos/Convex.h"
#include "Chaos/Particles.h"
#include "Chaos/Plane.h"
#include "Chaos/AABB.h"
#include "Chaos/CollisionConvexMesh.h"
#include "ChaosLog.h"
#endif

/*=============================================================================
	Volume & Area Helpers
=============================================================================*/

// FKBoxElem has its own GetVolume function but it is inaccurate. It uses scale.GetMin() rather than using element-wise multiplication.
float BoxVolume(const FKBoxElem& box, const FVector& scale)
{
	return (box.X * scale.X) * (box.Y * scale.Y) * (box.Z * scale.Z);
}
// This is the volume calculated by Unreal in UBodySetup::GetVolume, for box elements. We'll use this to subtract from the total volume, and add the more accurate volume calculated by BoxVolume, above.
float InaccurateBoxVolume(const FKBoxElem& box, const FVector& scale)
{
	float MinScale = scale.GetMin();
	return (box.X * MinScale) * (box.Y * MinScale) * (box.Z * MinScale);
}

float BoxSurfaceArea(const FKBoxElem& box, const FVector& scale)
{
	return box.X * scale.X * box.Y * scale.Y * 2.0f /* top & bottom */
		+ box.X * scale.X * box.Z * scale.Z * 2.0f /* left & right */
		+ box.Y * scale.Y * box.Z * scale.Z * 2.0f; /* front & back */
}

float SphereSurfaceArea(const FKSphereElem& sphere, const FVector& scale)
{
	return 4.0f * PI * FMath::Pow(sphere.Radius * scale.GetMin(), 2.0f);
}

float CapsuleSurfaceArea(const FKSphylElem& capsule, const FVector& scale)
{
	const float r = capsule.Radius * FMath::Min(scale.X, scale.Y);
	return 2.0f * PI * r * (2.0f * r + capsule.Length * scale.Z);
}

#if PHYSICS_INTERFACE_PHYSX
FVector PxToFVector(const PxVec3& pVec) { return FVector(pVec.x, pVec.y, pVec.z); }

float ConvexHullSurfaceArea(const FKConvexElem& convexElem, const FVector& scale)
{
	float area = 0.0f;
	physx::PxConvexMesh* mesh = convexElem.GetConvexMesh();
	if (mesh != nullptr)
	{
		const physx::PxVec3* vertices = mesh->getVertices();
		int numPolygons = mesh->getNbPolygons();
		physx::PxHullPolygon polygonData;
		for (int polygon = 0; polygon < numPolygons; ++polygon)
		{
			if (mesh->getPolygonData(polygon, polygonData))
			{
				int numVerts = polygonData.mNbVerts;
				FVector centroidPosition = FVector::ZeroVector;
				for (int v = 0; v < numVerts; ++v)
				{
					FVector vert = PxToFVector(vertices[polygonData.mIndexBase + v]) * scale;
					centroidPosition += vert;
				}
				centroidPosition /= (float)numVerts;
				FVector v1 = FVector::ZeroVector;
				FVector v2 = FVector::ZeroVector;
				for (int v = 0; v < numVerts - 1; ++v)
				{
					v1 = PxToFVector(vertices[polygonData.mIndexBase + v]) * scale;
					v2 = PxToFVector(vertices[polygonData.mIndexBase + v + 1]) * scale;
					area += FAkReverbDescriptor::TriangleArea(centroidPosition, v1, v2);
				}
				v1 = PxToFVector(vertices[polygonData.mIndexBase + (numVerts - 1)]) * scale;
				v2 = PxToFVector(vertices[polygonData.mIndexBase]) * scale;
				area += FAkReverbDescriptor::TriangleArea(centroidPosition, v1, v2);
			}
		}
	}
	else
	{
		FVector max = convexElem.ElemBox.Max;
		FVector min = convexElem.ElemBox.Min;
		FVector dims = FVector(scale.X * (max.X - min.X), scale.Y * (max.Y - min.Y), scale.Z * (max.Z - min.Z));
		area = dims.X * dims.Y * 2.0f + dims.X * dims.Z * 2.0f + dims.Y * dims.Z * 2.0f;
	}
	return area;
}
#elif WITH_CHAOS
float ConvexHullSurfaceArea(const FKConvexElem& convexElem, const FVector& scale)
{
	float area = 0.0f;
	auto mesh = convexElem.GetChaosConvexMesh();
	if (mesh.IsValid())
	{
		const Chaos::TParticles<Chaos::FReal, 3>& particles = mesh->GetSurfaceParticles();
		TArray<TArray<int32>> FaceIndices;
		TArray<Chaos::TPlaneConcrete<Chaos::FReal, 3>> Planes;
		Chaos::TParticles<Chaos::FReal, 3> SurfaceParticles;
		Chaos::TAABB<Chaos::FReal, 3> LocalBoundingBox;
		Chaos::FConvexBuilder::Build(particles, Planes, FaceIndices, SurfaceParticles, LocalBoundingBox);

		for (int32 faceIdx = 0; faceIdx < FaceIndices.Num(); faceIdx++)
		{
			auto face = FaceIndices[faceIdx];
			int numVerts = face.Num();
			FVector centroidPosition = FVector::ZeroVector;
			for (int v = 0; v < numVerts; ++v)
			{
				FVector vert = particles.X(face[v]) * scale;
				centroidPosition += vert;
			}
			centroidPosition /= (float)numVerts;
			FVector v1 = FVector::ZeroVector;
			FVector v2 = FVector::ZeroVector;
			for (int v = 0; v < numVerts - 1; ++v)
			{
				v1 = particles.X(face[v]) * scale;
				v2 = particles.X(face[v+1]) * scale;
				area += FAkReverbDescriptor::TriangleArea(centroidPosition, v1, v2);
			}
			v1 = particles.X(face[numVerts - 1]) * scale;
			v1 = particles.X(face[0]) * scale;
			area += FAkReverbDescriptor::TriangleArea(centroidPosition, v1, v2);
		}
	}
	else
	{
		FVector max = convexElem.ElemBox.Max;
		FVector min = convexElem.ElemBox.Min;
		FVector dims = FVector(scale.X * (max.X - min.X), scale.Y * (max.Y - min.Y), scale.Z * (max.Z - min.Z));
		area = dims.X * dims.Y * 2.0f + dims.X * dims.Z * 2.0f + dims.Y * dims.Z * 2.0f;
	}
	return area;
}
#else
#error "The Wwise Unreal integration is only compatible with PhysX or Chaos physics engines"
#endif

bool HasSimpleCollisionGeometry(UBodySetup* bodySetup)
{
	FKAggregateGeom geometry = bodySetup->AggGeom;
	return geometry.BoxElems.Num() > 0 || geometry.ConvexElems.Num() > 0 || geometry.SphereElems.Num() > 0 || geometry.TaperedCapsuleElems.Num() > 0 || geometry.SphylElems.Num() > 0;

}

void UpdateVolumeAndArea(UBodySetup* bodySetup, const FVector& scale, float& volume, float& surfaceArea)
{
	surfaceArea = 0.0f;
	// Initially use the Unreal UBodySetup::GetVolume function to calculate volume...
	volume = bodySetup->GetVolume(scale);
	FKAggregateGeom geometry = bodySetup->AggGeom;

	for (const FKBoxElem& box : geometry.BoxElems)
	{
		surfaceArea += BoxSurfaceArea(box, scale);
		// ... correct for any FKBoxElem elements in the geometry.
		// UBodySetup::GetVolume has an inaccuracy for box elements. It is scaled uniformly by the minimum scale dimension (see FKBoxElem::GetVolume).
		// For our purposes we want to scale by each dimension individually.
		volume -= InaccurateBoxVolume(box, scale);
		volume += BoxVolume(box, scale);
	}
	for (const FKConvexElem& convexElem : geometry.ConvexElems)
	{
		surfaceArea += ConvexHullSurfaceArea(convexElem, scale);
	}
	for (const FKSphereElem& sphere : geometry.SphereElems)
	{
		surfaceArea += SphereSurfaceArea(sphere, scale);
	}
	for (const FKSphylElem& capsule : geometry.SphylElems)
	{
		surfaceArea += CapsuleSurfaceArea(capsule, scale);
	}
}

/*=============================================================================
	FAkReverbDescriptor:
=============================================================================*/

float FAkReverbDescriptor::TriangleArea(const FVector& v1, const FVector& v2, const FVector& v3)
{
	float mag = 0.0f;
	FVector dir;
	FVector::CrossProduct(v2 - v1, v3 - v1).ToDirectionAndLength(dir, mag);
	return 0.5f * mag;
}

bool FAkReverbDescriptor::ShouldEstimateDecay() const
{
	if (IsValid(ReverbComponent) && ReverbComponent->AutoAssignAuxBus)
		return true;
	if (!IsValid(Primitive) || AkComponentHelpers::GetChildComponentOfType<UAkRoomComponent>(*Primitive) == nullptr)
		return false;
	const UAkSettings* AkSettings = GetDefault<UAkSettings>();
	if (AkSettings != nullptr)
	{
		return AkSettings->DecayRTPCInUse();
	}
	return false;
}

bool FAkReverbDescriptor::ShouldEstimateDamping() const
{
	if (!IsValid(Primitive) || AkComponentHelpers::GetChildComponentOfType<UAkRoomComponent>(*Primitive) == nullptr)
		return false;
	const UAkSettings* AkSettings = GetDefault<UAkSettings>();
	if (AkSettings != nullptr)
	{
		return AkSettings->DampingRTPCInUse();
	}
	return false;
}

bool FAkReverbDescriptor::ShouldEstimatePredelay() const
{
	if (!IsValid(Primitive) || AkComponentHelpers::GetChildComponentOfType<UAkRoomComponent>(*Primitive) == nullptr)
		return false;
	const UAkSettings* AkSettings = GetDefault<UAkSettings>();
	if (AkSettings != nullptr)
	{
		return AkSettings->PredelayRTPCInUse();
	}
	return false;
}

bool FAkReverbDescriptor::RequiresUpdates() const
{
	return ShouldEstimateDecay() || ShouldEstimateDamping() || ShouldEstimatePredelay();
}

void FAkReverbDescriptor::SetPrimitive(UPrimitiveComponent* primitive)
{
	Primitive = primitive;
}

void FAkReverbDescriptor::SetReverbComponent(UAkLateReverbComponent* reverbComp)
{
	ReverbComponent = reverbComp;
}

void FAkReverbDescriptor::CalculateT60()
{
	if (IsValid(Primitive))
	{
		PrimitiveVolume = 0.0f;
		PrimitiveSurfaceArea = 0.0f;
		T60Decay = 0.0f;
		if (Primitive != nullptr)
		{
			FVector scale = Primitive->GetComponentScale();
			UBodySetup* primitiveBody = Primitive->GetBodySetup();
			if (primitiveBody != nullptr && HasSimpleCollisionGeometry(primitiveBody))
			{
				UpdateVolumeAndArea(primitiveBody, scale, PrimitiveVolume, PrimitiveSurfaceArea);
			}
			else
			{
				if (UBrushComponent* brush = Cast<UBrushComponent>(Primitive))
				{
					brush->BuildSimpleBrushCollision();
				}
				else
				{
					FString PrimitiveName = "";
					Primitive->GetName(PrimitiveName);
					FString ActorName = "";
					AActor* owner = Primitive->GetOwner();
					if (owner != nullptr)
						owner->GetName(ActorName);
					UE_LOG(LogAkAudio, Warning,
						TEXT("Primitive component %s on actor %s has no simple collision geometry.%sCalculations for reverb aux bus assignment will use component bounds. This could be less accurate than using simple collision geometry."),
						*PrimitiveName, *ActorName, LINE_TERMINATOR);
					// only apply scale to local bounds to calculate volume and surface area.
					FTransform transform = Primitive->GetComponentTransform();
					transform.SetRotation(FQuat::Identity);
					transform.SetLocation(FVector::ZeroVector);
					FBoxSphereBounds bounds = Primitive->CalcBounds(transform);
					FVector boxDimensions = bounds.BoxExtent * 2.0f;
					PrimitiveVolume = boxDimensions.X * boxDimensions.Y * boxDimensions.Z;
					PrimitiveSurfaceArea += boxDimensions.X * boxDimensions.Y * 2.0f;
					PrimitiveSurfaceArea += boxDimensions.X * boxDimensions.Z * 2.0f;
					PrimitiveSurfaceArea += boxDimensions.Y * boxDimensions.Z * 2.0f;
				}
			}

			PrimitiveVolume = FMath::Abs(PrimitiveVolume) / AkComponentHelpers::UnrealUnitsPerCubicMeter(Primitive);
			PrimitiveSurfaceArea /= AkComponentHelpers::UnrealUnitsPerSquaredMeter(Primitive);

			if (PrimitiveVolume > 0.0f && PrimitiveSurfaceArea > 0.0f)
			{
				float absorption = 0.5f;
				UAkSettings* AkSettings = GetMutableDefault<UAkSettings>();
				if (AkSettings != nullptr)
					absorption = AkSettings->GlobalDecayAbsorption;
				//calcuate t60 using the Sabine equation
				AK::SpatialAudio::ReverbEstimation::EstimateT60Decay(PrimitiveVolume, PrimitiveSurfaceArea, absorption, T60Decay);
			}
		}
	}
#if WITH_EDITOR
	if (IsValid(ReverbComponent))
		ReverbComponent->UpdateDecayEstimation(T60Decay, PrimitiveVolume, PrimitiveSurfaceArea);
#endif
	UpdateDecayRTPC();
}

void FAkReverbDescriptor::CalculateTimeToFirstReflection()
{
	FTransform transform = Primitive->GetComponentTransform();
	transform.SetRotation(FQuat::Identity);
	transform.SetLocation(FVector::ZeroVector);
	FBoxSphereBounds bounds = Primitive->CalcBounds(transform);
	AkVector extentMeters = FAkAudioDevice::FVectorToAKVector(bounds.BoxExtent / AkComponentHelpers::UnrealUnitsPerMeter(Primitive));
	AK::SpatialAudio::ReverbEstimation::EstimateTimeToFirstReflection(extentMeters, TimeToFirstReflection);
#if WITH_EDITOR
	if (IsValid(ReverbComponent))
		ReverbComponent->UpdatePredelayEstimation(TimeToFirstReflection);
#endif
	UpdatePredelaytRTPC();
}

void FAkReverbDescriptor::CalculateHFDamping(const UAkAcousticTextureSetComponent* acousticTextureSetComponent)
{
	HFDamping = 0.0f;
	if (IsValid(Primitive))
	{
		const UAkSettings* AkSettings = GetDefault<UAkSettings>();
		if (AkSettings != nullptr)
		{
			TArray<FAkAcousticTextureParams> texturesParams;
			TArray<float> surfaceAreas;
			acousticTextureSetComponent->GetTexturesAndSurfaceAreas(texturesParams, surfaceAreas);
			TArray<AkAcousticTexture> textures;
			for (const FAkAcousticTextureParams& params : texturesParams)
			{
				AkAcousticTexture texture;
				texture.fAbsorptionLow = params.AbsorptionLow();
				texture.fAbsorptionMidLow = params.AbsorptionMidLow();
				texture.fAbsorptionMidHigh = params.AbsorptionMidHigh();
				texture.fAbsorptionHigh = params.AbsorptionHigh();
				textures.Add(texture);
			}
			const int numTextures = textures.Num();
			if (numTextures == 0)
				HFDamping = 0.0f;
			else
				AK::SpatialAudio::ReverbEstimation::EstimateHFDamping(&textures[0], &surfaceAreas[0], textures.Num(), HFDamping);
		}
	}
#if WITH_EDITOR
	if (IsValid(ReverbComponent))
		ReverbComponent->UpdateHFDampingEstimation(HFDamping);
#endif
	UpdateDampingRTPC();
}

bool FAkReverbDescriptor::GetRTPCRoom(UAkRoomComponent*& room) const
{
	room = AkComponentHelpers::GetChildComponentOfType<UAkRoomComponent>(*Primitive);
	if (!CanSetRTPCOnRoom(room))
	{
		room = nullptr;
	}

	return room != nullptr;
}

bool FAkReverbDescriptor::CanSetRTPCOnRoom(const UAkRoomComponent* room) const
{
	if (FAkAudioDevice::Get() == nullptr
		|| room == nullptr
		|| !room->HasBeenRegisteredWithWwise()
		|| room->GetWorld() == nullptr
		|| (room->GetWorld()->WorldType != EWorldType::Game && room->GetWorld()->WorldType != EWorldType::PIE))
	{
		return false;
	}
	return true;
}

void FAkReverbDescriptor::UpdateDecayRTPC() const
{
	UAkRoomComponent* room = nullptr;
	if (GetRTPCRoom(room))
	{
		const UAkSettings* AkSettings = GetDefault<UAkSettings>();
		if (AkSettings != nullptr && AkSettings->ReverbRTPCsInUse())
		{
			room->SetRTPCValue(AkSettings->DecayEstimateRTPC.LoadSynchronous(), T60Decay, 0, AkSettings->DecayEstimateName);
		}
	}
}

void FAkReverbDescriptor::UpdateDampingRTPC() const
{
	UAkRoomComponent* room = nullptr;
	if (GetRTPCRoom(room))
	{
		const UAkSettings* AkSettings = GetDefault<UAkSettings>();
		if (AkSettings != nullptr && AkSettings->ReverbRTPCsInUse())
		{
			room->SetRTPCValue(AkSettings->HFDampingRTPC.LoadSynchronous(), HFDamping, 0, *AkSettings->HFDampingName);
		}
	}
}

void FAkReverbDescriptor::UpdatePredelaytRTPC() const
{
	UAkRoomComponent* room = nullptr;
	if (GetRTPCRoom(room))
	{
		const UAkSettings* AkSettings = GetDefault<UAkSettings>();
		if (AkSettings != nullptr && AkSettings->ReverbRTPCsInUse())
		{
			room->SetRTPCValue(AkSettings->TimeToFirstReflectionRTPC.LoadSynchronous(), TimeToFirstReflection, 0, *AkSettings->TimeToFirstReflectionName);
		}
	}
}

void FAkReverbDescriptor::UpdateAllRTPCs(const UAkRoomComponent* room) const
{
	AKASSERT(room != nullptr);

	if (CanSetRTPCOnRoom(room))
	{
		const UAkSettings* AkSettings = GetDefault<UAkSettings>();
		if (AkSettings != nullptr && AkSettings->ReverbRTPCsInUse())
		{
			if ((ReverbComponent != nullptr && ReverbComponent->AutoAssignAuxBus) || AkSettings->DecayRTPCInUse())
			{
				room->SetRTPCValue(AkSettings->DecayEstimateRTPC.LoadSynchronous(), T60Decay, 0, AkSettings->DecayEstimateName);
			}

			if (AkSettings->DampingRTPCInUse())
			{
				room->SetRTPCValue(AkSettings->HFDampingRTPC.LoadSynchronous(), HFDamping, 0, *AkSettings->HFDampingName);
			}

			if (AkSettings->PredelayRTPCInUse())
			{
				room->SetRTPCValue(AkSettings->TimeToFirstReflectionRTPC.LoadSynchronous(), TimeToFirstReflection, 0, *AkSettings->TimeToFirstReflectionName);
			}
		}
	}
}
