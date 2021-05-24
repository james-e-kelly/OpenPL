// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OpenPL.hpp"
#include "Templates/UniquePtr.h"
#include "FMODAmbientSound.h"
#include "AkAmbientSound.h"
#include "RuntimeOpenPL.generated.h"

UCLASS()
class OPENPROPAGATIONLIBRARY_API ARuntimeOpenPL : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARuntimeOpenPL();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
    
protected:
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bUseWwise;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bShowMeshes;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bShowAllVoxels;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector SimulationSize;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VoxelSize;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<class AStaticMeshActor*> StaticMeshes;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AFMODAmbientSound* FMODEvent;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AAkAmbientSound* WwiseEvent;

    TUniquePtr<OpenPL::PLScene> Scene;
    
    class APawn* Player;
};
