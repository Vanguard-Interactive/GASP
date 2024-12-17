// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Types/EnumTypes.h"
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

	UPROPERTY()
	TObjectPtr<UGASPCharacterMovementComponent> MovementComponent{}; 
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ERotationMode DesiredRotationMode{ERotationMode::Strafe};
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EGait DesiredGait{ EGait::Run };
	
	UPROPERTY(BlueprintReadOnly, Replicated)
	EGait Gait{};
	UPROPERTY(BlueprintReadOnly, Replicated)
	ERotationMode RotationMode{};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated)
	EMovementState MovementState{ EMovementState::Idle };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated)
	ECMovementMode MovementMode{ ECMovementMode::OnGround };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated)
	EStanceMode StanceMode;

public:
	
	// Sets default values for this character's properties
	explicit AGASPCharacter(const FObjectInitializer& ObjectInitializer);
	AGASPCharacter() = default;
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void PostRegisterAllComponents() override;

	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	
	/****************************
	*		Input actions		*
	****************************/
	UFUNCTION(BlueprintCallable)
	void MoveAction(const FVector2D& Value);
	UFUNCTION(BlueprintCallable)
	void LookAction(const FVector2D& Value);

	/****************************
	*		Movement States		*
	****************************/
	void SetGait(EGait NewGait, bool bForce = false);
	UFUNCTION(Server, Reliable)
	void Server_SetGait(EGait NewGait);
	UFUNCTION(BlueprintImplementableEvent)
	void OnGaitChanged(EGait PreviousGait);
	
	void SetRotationMode(ERotationMode NewRotationMode, bool bForce = false);
	UFUNCTION(Server, Reliable)
	void Server_SetRotationMode(ERotationMode NewRotationMode);
	UFUNCTION(BlueprintImplementableEvent)
	void OnRotationModeChanged(ERotationMode PreviousRotationMode);
	
	void SetMovementMode(ECMovementMode NewMovementMode, bool bForce = false);
	UFUNCTION(Server, Reliable)
	void Server_SetMovementMode(ECMovementMode NewMovementMode);
	
	void SetMovementState(EMovementState NewMovementState, bool bForce = false);
	UFUNCTION(Server, Reliable)
	void Server_SetMovementState(EMovementState NewMovementState);
	UFUNCTION(BlueprintImplementableEvent)
	void OnMovementStateChanged(EMovementState PreviousMovementState);

	void SetStanceMode(EStanceMode NewStanceMode, bool bForce = false);
	UFUNCTION(Server, Reliable)
	void Server_SetStanceMode(EStanceMode NewStanceMode);
	UFUNCTION(BlueprintImplementableEvent)
	void OnStanceModeChanged(EStanceMode PreviousStanceMode);
	
	virtual bool CanSprint();
	UGASPCharacterMovementComponent* GetBCharacterMovement() const;

	UFUNCTION(BlueprintGetter)
	FORCEINLINE EGait GetGait() const { return Gait; }
	UFUNCTION(BlueprintGetter)
	FORCEINLINE ERotationMode GetRotationMode() const { return RotationMode; }
	UFUNCTION(BlueprintGetter)
	FORCEINLINE ECMovementMode GetMovementMode() const { return MovementMode; }
	UFUNCTION(BlueprintGetter)
	FORCEINLINE EMovementState GetMovementState() const { return MovementState; }
	
	UFUNCTION(BlueprintGetter)
	FORCEINLINE EStanceMode GetStanceMode() const { return StanceMode; }
};
