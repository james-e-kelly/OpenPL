// Fill out your copyright notice in the Description page of Project Settings.


#include "RuntimeOpenPL.h"
#include "OpenPropagationLibrary.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "OpenPLUtils.h"

using namespace OpenPL;

// Sets default values
ARuntimeOpenPL::ARuntimeOpenPL()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARuntimeOpenPL::BeginPlay()
{
	Super::BeginPlay();
	
    FOpenPropagationLibraryModule& OpenPLModule = FOpenPropagationLibraryModule::Get();
    Scene = TUniquePtr<PLScene>(OpenPLModule.CreateScene());
    
    if (!Scene)
    {
        return;
    }
    
    for (AStaticMeshActor* MeshActor : StaticMeshes)
    {
        TArray<PLVector> Vertices;
        TArray<int> ActualIndices;
        
        UStaticMeshComponent* StaticMeshComponent = MeshActor->GetStaticMeshComponent();
        UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();
        
        const int32 LOD = 0;
        
        if (StaticMesh->HasValidRenderData(true, LOD))
        {
            const auto& RenderData = StaticMesh->GetLODForExport(LOD);
            const FPositionVertexBuffer& VertexBuffer = RenderData.VertexBuffers.PositionVertexBuffer;
            
            /*GetCopy
             (
                 TArray < uint32 >& OutIndices
             )
            */
            
            
            
            const FRawStaticIndexBuffer& IndexBuffer = RenderData.IndexBuffer;
            TArray<uint32> Indices;
            IndexBuffer.GetCopy(Indices);
            auto TriangleCount = RenderData.GetNumTriangles();
            auto VertexCount = VertexBuffer.GetNumVertices();
            
            for (int i = 0; i < VertexCount; i++)
            {
                const FVector& VertexPos = VertexBuffer.VertexPosition(i);
                const FVector& VertexWorld = MeshActor->GetTransform().TransformPosition(VertexPos);
                Vertices.Add(PLVector(VertexWorld.X, VertexWorld.Y, VertexWorld.Z));
            }
            
            for (int i = 0; i < Indices.Num(); i++)
            {
                ActualIndices.Add(static_cast<int>(Indices[i]));
            }
            //        PL_RESULT JUCE_PUBLIC_FUNCTION AddMesh(PLVector WorldPosition, PLQuaternion WorldRotation, PLVector WorldScale, PLVector* Vertices, int VerticesLength, int* Indices, int IndicesLength, int* OutIndex);
            int32 AddedMeshIndex = -1;
            Scene->AddMesh(ConvertUnrealVectorToPL(MeshActor->GetActorLocation()), ConvertUnrealVectorToPL4(MeshActor->GetActorRotation().Euler()), ConvertUnrealVectorToPL(MeshActor->GetActorScale()), Vertices.GetData(), VertexCount, ActualIndices.GetData(), TriangleCount, &AddedMeshIndex);
        }
    }
}

// Called every frame
void ARuntimeOpenPL::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

