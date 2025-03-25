// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GASPHeldObject.generated.h"

UCLASS()
class GASP_API AGASPHeldObject : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGASPHeldObject();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<class USkeletalMeshComponent> MeshComponent{};

public:
	UFUNCTION(BlueprintGetter)
	FORCEINLINE USkeletalMeshComponent* GetMesh()
	{
		return MeshComponent;
	}
};
