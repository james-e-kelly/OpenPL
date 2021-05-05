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
    
    EventInstance = UFMODBlueprintStatics::PlayEventAtLocation(this, TestFMODEvent, GetActorTransform(), true);
    
    Scene->CreateVoxels(PLVector(10,10,10), 1);
    
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

            int32 AddedMeshIndex = -1;
            Scene->AddMesh(ConvertUnrealVectorToPL(MeshActor->GetActorLocation()), ConvertUnrealVectorToPL4(MeshActor->GetActorRotation().Euler()), ConvertUnrealVectorToPL(MeshActor->GetActorScale()), Vertices.GetData(), VertexCount, ActualIndices.GetData(), TriangleCount, &AddedMeshIndex);
        }
    }
    
    Scene->FillVoxelsWithGeometry();
    
    int VoxelCount;
    Scene->GetVoxelsCount(&VoxelCount);
    
    Scene->Simulate(PLVector(0,0,0));
    
    PL_RESULT DrawGraphResult = Scene->DrawGraph(PLVector(1,1,1));
    
    /*
     listenerLocation = Listener.transform.position;
     listenerEmitterLocation = new Vector3(listenerLocation.x, emitterLocation.y, listenerLocation.z);

     int VoxelIndex;
     RuntimeManager.CheckResult(SceneInstance.Encode(PLVector(0,0,0), out VoxelIndex), "Encode");
     string DesktopPath = global::System.Environment.GetFolderPath(Environment.SpecialFolder.Desktop);
     string currentIRFilePath = DesktopPath + "/" + VoxelIndex.ToString() + ".wav";
     if (global::System.IO.File.Exists(currentIRFilePath) && lastVoxel == VoxelIndex)
     {
         yield return new WaitForSeconds(1f);
         continue;
     }
     UnityEngine.Debug.Log(currentIRFilePath);

     lastVoxel = VoxelIndex;

     int channels, sampleRate;
     float[] ir = WAV.Read(currentIRFilePath, out channels, out sampleRate);

     byte[] array = new byte[ir.Length + 1];
     array[0] = (byte)channels;
     Buffer.BlockCopy(ir, 0, array, 1, ir.Length);

     reverbDSP.setParameterData((int)FMOD.DSP_CONVOLUTION_REVERB.IR, array);

     UnityEngine.Debug.Log("SENT IR");

     yield return new WaitForSeconds(0.2f);
     */
}

// Called every frame
void ARuntimeOpenPL::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

