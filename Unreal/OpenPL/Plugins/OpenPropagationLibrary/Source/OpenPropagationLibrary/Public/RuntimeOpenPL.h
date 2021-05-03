// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OpenPL.hpp"
#include "Templates/UniquePtr.h"
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
    TArray<class AStaticMeshActor*> StaticMeshes;

    TUniquePtr<OpenPL::PLScene> Scene;
};
