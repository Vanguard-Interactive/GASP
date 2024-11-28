// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Types/EnumTypes.h"
#include "Types/StructTypes.h"
#include "GASPCharacter.generated.h"

class UGASPCharacterMovementComponent;

UCLASS(Abstract)
class GASP_API AGASPCharacter : public ACharacter
{
	GENERATED_BODY()

protected:


	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PrevCustomMode) override;
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated)
	FGait Gait{ EGait::Run };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated)
	FRotationMode RotationMode{ ERotationMode::OrientToMovement };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated)
	FMovementState MovementState{ EMovementState::Idle };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated)
	FMovementMode MovementMode{ ECMovementMode::OnGround };

	// Sets default values for this character's properties
	explicit AGASPCharacter(const FObjectInitializer& ObjectInitializer);
	AGASPCharacter() {}


	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/****************************
	*		Input actions		*
	****************************/
	UFUNCTION(BlueprintCallable)
	void MoveAction(const FVector2D& Value);

	UFUNCTION(BlueprintCallable)
	void LookAction(const FVector2D& Value);


	UGASPCharacterMovementComponent* GetBCharacterMovement() const;

	/****************************
	*		Movement States		*
	****************************/
	void SetGait(EGait NewGait, bool bForce = false);
	UFUNCTION(Server, Reliable)
	void Server_SetGait(EGait NewGait);

	void SetRotationMode(ERotationMode NewRotationMode, bool bForce = false);
	UFUNCTION(Server, Reliable)
	void Server_SetRotationMode(ERotationMode NewRotationMode);

	void SetMovementMode(ECMovementMode NewMovementMode, bool bForce = false);
	UFUNCTION(Server, Reliable)
	void Server_SetMovementMode(ECMovementMode NewMovementMode);

	void SetMovementState(EMovementState NewMovementState, bool bForce = false);
	UFUNCTION(Server, Reliable)
	void Server_SetMovementState(EMovementState NewMovementState);
};
