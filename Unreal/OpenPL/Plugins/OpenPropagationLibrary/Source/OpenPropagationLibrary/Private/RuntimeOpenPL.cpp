// Fill out your copyright notice in the Description page of Project Settings.


#include "RuntimeOpenPL.h"
#include "OpenPropagationLibrary.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

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
        
        UStaticMeshComponent* StaticMeshComponent = MeshActor->GetStaticMeshComponent();
        UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();
        
        const int32 LOD = 0;
        
        if (StaticMesh->HasValidRenderData(true, LOD))
        {
            const auto& RenderData = StaticMesh->GetLODForExport(LOD);
            const auto& VertexBuffer = RenderData.VertexBuffers.PositionVertexBuffer;
            
            auto IndexBuffer = RenderData.IndexBuffer.GetArrayView();
            auto TriangleCount = RenderData.GetNumTriangles();
            auto VertexCount = VertexBuffer.GetNumVertices();
            
            for (int i = 0; i < VertexCount; i++)
            {
                // If actor is provided, its actor-to-world transform is used,
                // otherwise vertices are interpreted to be directly in world coordinates
                const FVector& VertexPos = VertexBuffer.VertexPosition(i);
                const FVector& VertexWorld = MeshActor->GetTransform().TransformPosition(VertexPos);
                Vertices.Add(PLVector(VertexWorld.X, VertexWorld.Y, VertexWorld.Z));
            }
        }
    }
}

// Called every frame
void ARuntimeOpenPL::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

