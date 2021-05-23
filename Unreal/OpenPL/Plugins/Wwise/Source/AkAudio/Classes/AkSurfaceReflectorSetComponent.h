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

#pragma once
#include "AkAcousticTexture.h"
#include "AkAcousticTextureSetComponent.h"
#include "AkGameObject.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "AkSurfaceReflectorSetComponent.generated.h"

class UAkRoomComponent;
struct FAkReverbDescriptor;

DECLARE_DELEGATE(FOnRefreshDetails);


USTRUCT(BlueprintType)
struct FAkPoly
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audiokinetic|AkSurfaceReflectorSet")
	UAkAcousticTexture* Texture = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audiokinetic|AkSurfaceReflectorSet", DisplayName = "Transmission Loss", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Occlusion = 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audiokinetic|AkSurfaceReflectorSet")
	bool EnableSurface = true;

	void SetSurfaceArea(float area) { SurfaceArea = area; }
	float GetSurfaceArea() const { return SurfaceArea; }

#if WITH_EDITOR
	FText GetPolyText(bool includeOcclusion) const;
#endif

private:
	UPROPERTY()
	float SurfaceArea = 0.0f;
};

/** An edge between two vertices */
USTRUCT()
struct FAkEdgeInfo
{
	GENERATED_BODY()
	FVector V0;
	FVector V1;
	FVector Normal;
	bool IsEnabled = true;
	bool IsBoundary = true;
	bool IsFlat = false;

	int64 GetHash()
	{
		int64 H0 = GetTypeHash(V0);
		int64 H1 = GetTypeHash(V1);
		if (H1 > H0)
		{
			int64 temp = H0;
			H0 = H1;
			H1 = temp;
		}
		return H1 << 32 | H0;
	}

	FAkEdgeInfo();
	FAkEdgeInfo(FVector InV0, FVector InV1);
}; 

/** Contains the properties of a face from the ParentBrush of a UAkSurfaceReflectorSetComponent. */
USTRUCT()
struct FAkSurfaceProperties
{
	GENERATED_BODY()
	FVector Normal;
	FVector MidPoint;
	/* Keys for edges in external edge map. The map is owned by a UAkSurfaceReflectorSetComponent. See UAkSurfaceReflectorSetComponent::EdgeMap. */
	TArray<int64> Edges;
	/* EdgeInvertFlags[i] == true implies that edge i should be inverted for this face
	   Faces may share edges from the edge map but disagree as to the inversion. */
	TArray<bool> EdgeInvertFlags;
	/* Keeps track of the optimal dot product between the chosen up vector and view camera up vector.
	   This is used to minimize the flickering of text as it switches between edges each frame. */
	mutable float OptimalEdgeDP = 0.0f;
	void AddEdgeHash(int64 hash)
	{
		Edges.Add(hash);
		EdgeInvertFlags.Add(false);
	}

	void RemoveEdgeHash(int64 hash)
	{
		// Must loop through to find invert flag as well as hash
		for (int i = 0; i < Edges.Num(); ++i)
		{
			if (Edges[i] == hash)
			{
				EdgeInvertFlags.RemoveAt(i);
				break;
			}
		}
		Edges.Remove(hash);
	}
};

UCLASS(ClassGroup = Audiokinetic, BlueprintType, hidecategories = (Transform, Rendering, Mobility, LOD, Component, Activation, Tags), meta = (BlueprintSpawnableComponent))
class AKAUDIO_API UAkSurfaceReflectorSetComponent : public UAkAcousticTextureSetComponent
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|AkSurfaceReflectorSet")
	void SendSurfaceReflectorSet();

	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|AkSurfaceReflectorSet")
	void RemoveSurfaceReflectorSet();

	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|AkSurfaceReflectorSet")
	void UpdateSurfaceReflectorSet();

	/** Enable reflection with geometry */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Toggle")
	uint32 bEnableSurfaceReflectors : 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category ="Geometry Surfaces")
	TArray<FAkPoly> AcousticPolys;

	/** Enable or disable geometric diffraction for this mesh. Check this box to have Wwise Spatial Audio generate diffraction edges on the geometry. The diffraction edges will be visible in the Wwise game object viewer when connected to the game. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geometry Settings")
	uint32 bEnableDiffraction : 1;

	/** Enable or disable geometric diffraction on boundary edges for this Geometry. Boundary edges are edges that are connected to only one triangle. Depending on the specific shape of the geometry, boundary edges may or may not be useful and it is beneficial to reduce the total number of diffraction edges to process.  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geometry Settings", meta = (EditCondition = "bEnableDiffraction"))
	uint32 bEnableDiffractionOnBoundaryEdges : 1;

	/** (Optional) Associate this Surface Reflector Set with a Room.
	Associating a surface reflector set with a particular room will limit the scope in which the geometry is visible/accessible. Leave it to None and this geometry will have a global scope.
	It is recommended to associate geometry with a room when the geometry is (1) fully contained within the room (ie. not visible to other rooms except by portals), and (2) the room does not share geometry with other rooms. Doing so reduces the search space for ray casting performed by reflection and diffraction calculations.
	Take note that once one or more geometry sets are associated with a room, that room will no longer be able to access geometry that is in the global scope.*/ 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geometry Settings")
	AActor* AssociatedRoom;

	UModel* ParentBrush;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	mutable TArray<UTextRenderComponent*> TextVisualizers;

	FText GetPolyText(int32 PolyIdx) const;

	void SetOnRefreshDetails(const FOnRefreshDetails& OnRefreshDetailsDelegate) { OnRefreshDetails = OnRefreshDetailsDelegate; }
	void ClearOnRefreshDetails() { OnRefreshDetails.Unbind(); }
	const FOnRefreshDetails* GetOnRefreshDetails() { return &OnRefreshDetails; }

	void AssignAcousticTexturesFromSamples(const TArray<FVector>& Points, const TArray<FVector>& Normals, const TArray< TWeakObjectPtr<class UPhysicalMaterial> >& Materials, int Num);
#endif

#if WITH_EDITOR
	/**
	* Check for errors
	*/
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditUndo() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction);

	/** Tracks whether the user is interacting with a UI element in the details panel (e.g. a slider) */
	bool UserInteractionInProgress = false;
	FDelegateHandle PropertyChangedHandle;
	void OnPropertyChanged(UObject* ObjectBeingModified, FPropertyChangedEvent& PropertyChangedEvent);

	void SchedulePolysUpdate();
	void UpdatePolys();
	void UpdateText(bool Visible);
	/** Align all of the text components (1 for each face) along one of the edges on the face */
	void UpdateTextPositions() const;
	void SurfacePropertiesChanged();
	void DestroyTextVisualizers();

	bool WasSelected;

	TSet<int> GetSelectedFaceIndices() const;
	bool TexturesDiffer() const;

	/** Used to delay the polys update by one frame when editing geometry */
	bool PolysNeedUpdate = false;
#endif

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TMap<int64, FAkEdgeInfo> EdgeMap;
	/** An array of the faces on the brush geometry */
	TArray<FAkSurfaceProperties> Faces;
#endif

	virtual void BeginDestroy() override;
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport) override;
	virtual bool MoveComponentImpl(
		const FVector& Delta,
		const FQuat& NewRotation,
		bool bSweep,
		FHitResult* Hit,
		EMoveComponentFlags MoveFlags,
		ETeleportType Teleport) override;

	void GetTexturesAndSurfaceAreas(TArray<FAkAcousticTextureParams>& textures, TArray<float>& surfaceAreas) const override;

private:
	virtual bool ShouldSendGeometry() const override;
	void InitializeParentBrush(bool fromTick = false);

#if WITH_EDITOR
	virtual bool ContainsTexture(const FGuid& textureID) override;
	virtual void RegisterAllTextureParamCallbacks() override;
	/* Sort the edges of a face such that they form a continuous loop */
	void SortFaceEdges(int FaceIndex);
	/** Identify the edges in the brush geometry and store in EdgeMap */
	void UpdateEdgeMap();
	/** Get the edge for the given face from the edge map, inverting if necessary. 
		Return false if the edge is not found */
	bool GetV0V1(int FaceIndex, int EdgeIndex, FVector& V0, FVector& V1) const;
	void AlignTextWithEdge(int FaceIndex) const;
	/* Choose the edge upon which to align the text. The 'optimal' edge is that which aligns the 
	   up vector of the text closest to the up vector of the view camera. */
	int ChooseAlignmentEdge(int FaceIndex) const;
	/* Positions the text at the beginning of the AlignmentEdge (V0).
	   Shifts the text along the AlignmentEdge by a certain amount.
	   The amount of shift is proportional to the dot product between AlignmentEdge and the edge that connects to V0. */
	FVector GetTextAnchorPosition(int FaceIndex, const FAkEdgeInfo& AlignmentEdge, int AlignmentEdgeIndex) const;
	/* Progressively scale down the text visualizer at FaceIndex until it is completely contained within the face.  */
	void SetTextScale(int FaceIndex, int AlignmentEdgeIndex, const FVector& TextAnchorPosition, const struct FFacePlane& FacePlane) const;
#endif

#if WITH_EDITORONLY_DATA
	FOnRefreshDetails OnRefreshDetails;
#endif
};
