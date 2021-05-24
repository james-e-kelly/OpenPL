// Fill out your copyright notice in the Description page of Project Settings.


#include "RuntimeOpenPL.h"
#include "OpenPropagationLibrary.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "OpenPLUtils.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "FMODAudioComponent.h"
#include "DrawDebugHelpers.h"
#include "AkAudioDevice.h"
#include "AkComponent.h"

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
    
    Scene->CreateVoxels(ConvertUnrealVectorToPL(SimulationSize), VoxelSize / 100);
    
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
                Vertices.Add(ConvertUnrealVectorToPL(VertexPos));
            }
            
            for (int i = 0; i < Indices.Num(); i++)
            {
                ActualIndices.Add(static_cast<int>(Indices[i]));
            }
            
            FVector Scale = MeshActor->GetActorScale();

            int32 AddedMeshIndex = -1;
            Scene->AddMesh(ConvertUnrealVectorToPL(MeshActor->GetActorLocation()), ConvertUnrealVectorToPL4(MeshActor->GetActorRotation().Euler()),
                           PLVector(Scale.Y, Scale.Z, Scale.X), Vertices.GetData(), VertexCount, ActualIndices.GetData(), TriangleCount, &AddedMeshIndex);
        }
    }

    
    int VoxelCount;
    Scene->GetVoxelsCount(&VoxelCount);
        
    Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    
    FVector ListenerLocation = Player->GetActorLocation();
    
    OpenPLModule.GetSystem()->SetListenerPosition(ConvertUnrealVectorToPL(ListenerLocation));
                                                   
    Scene->FillVoxelsWithGeometry();
    
    Scene->Simulate(ConvertUnrealVectorToPL(ListenerLocation));
    
    if (bShowMeshes)
    {
        Scene->Debug();
    }

    DrawDebugBox(GetWorld(), FVector(0,0,0), SimulationSize, FColor::White, true, -1, 0, 10);
    
    for(int i = 0; i < VoxelCount; ++i)
    {
        PLVector Location;
        FVector UnrealLocation;
        float Absorpivity;
        Scene->GetVoxelLocation(&Location, i);
        Scene->GetVoxelAbsorpivity(&Absorpivity, i);
        UnrealLocation = ConvertPLToUnreal(Location);
        if (Absorpivity > 0.f)
        {
            DrawDebugBox(GetWorld(), UnrealLocation, FVector(VoxelSize,VoxelSize,VoxelSize), FColor::Green, true, -1, 0, 10);
        }
        else if (bShowAllVoxels)
        {
            DrawDebugBox(GetWorld(), UnrealLocation, FVector(VoxelSize,VoxelSize,VoxelSize), FColor::White, true, -1, 0, 10);
        }
    }
}

// Called every frame
void ARuntimeOpenPL::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (Player)
    {
        FVector ListenerLocation = Player->GetActorLocation();
        FVector EmitterLocation = bUseWwise ? WwiseEvent->GetActorLocation() : FMODEvent->GetActorLocation();
        EmitterLocation.Z = ListenerLocation.Z;
        
        DrawDebugBox(GetWorld(), EmitterLocation, FVector(50,50,50), FColor::Purple, false, 0.05f, 0, 5);
        
        FOpenPropagationLibraryModule& OpenPLModule = FOpenPropagationLibraryModule::Get();
        OpenPLModule.GetSystem()->SetListenerPosition(ConvertUnrealVectorToPL(ListenerLocation));
        
        Scene->Simulate(ConvertUnrealVectorToPL(ListenerLocation));
        
        float OutOcclusion;
        Scene->GetOcclusion(ConvertUnrealVectorToPL(EmitterLocation), &OutOcclusion);
        
        if (OutOcclusion > 1)
        {
            OutOcclusion = 1;
        }
        if (OutOcclusion < 0)
        {
            OutOcclusion = 0;
        }
        
        if (bUseWwise)
        {
            WwiseEvent->AkComponent->SetRTPCValue(nullptr, 1 - OutOcclusion, 0, FString("Occlusion"));
        }
        else
        {
            UFMODAudioComponent* AudioComponent = FMODEvent->AudioComponent;
            
            AudioComponent->SetParameter("Occlusion", 1 - OutOcclusion);
        }
        
    }
}

