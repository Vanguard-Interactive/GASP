// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GASPPlayerControllerExample.generated.h"

/**
 *
 */
UCLASS()
class GASPEXTRAS_API AGASPPlayerControllerExample : public APlayerController
{
	GENERATED_BODY()

public:
	AGASPPlayerControllerExample() = default;

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnRep_Pawn() override;
	virtual void OnRep_Owner() override;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	TObjectPtr<class AGASPCharacterExample> PossessedPlayer{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<class UInputMappingContext> DefaultInputMapping{};

	void SetupInput() const;
};
