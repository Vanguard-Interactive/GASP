// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Types/EnumTypes.h"
#include "Types/StructTypes.h"
#include "GASPCharacterMovementComponent.generated.h"

/**
 *
 */
UCLASS()
class GASP_API UGASPCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	class GASP_API FGASPCharacterNetworkMoveData final : public FCharacterNetworkMoveData
	{
		using Super = FCharacterNetworkMoveData;

	public:
		uint8 bSavedRotationModeUpdate : 1;
		EGait SavedGait{EGait::Walk};
		ERotationMode SavedRotationMode{ERotationMode::Strafe};

		virtual void ClientFillNetworkMoveData(const FSavedMove_Character& Move, ENetworkMoveType MoveType) override;
	};

	class GASP_API FGASPSavedMove final : public FSavedMove_Character
	{
		using Super = FSavedMove_Character;

	public:
		// Flags
		uint8 bSavedRotationModeUpdate : 1;
		EGait SavedGait{EGait::Walk};
		ERotationMode SavedRotationMode{ERotationMode::Strafe};

		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter,
		                            float MaxDelta) const override;
		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, const FVector& NewAccel,
		                        FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(ACharacter* C) override;
		virtual void CombineWith(const FSavedMove_Character* OldMove, ACharacter* InCharacter, APlayerController* PC,
		                         const FVector& OldStartLocation) override;
	};

	class GASP_API FNetworkPredictionData_Client_Base : public FNetworkPredictionData_Client_Character
	{
	public:
		FNetworkPredictionData_Client_Base(const UCharacterMovementComponent& ClientMovement);

		using Super = FNetworkPredictionData_Client_Character;

		virtual FSavedMovePtr AllocateNewMove() override;
	};

	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float InAirRotationYaw{200.f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float OnGroundRotationYaw{-1.f};

	EGait SafeGait{EGait::Walk};
	ERotationMode SafeRotationMode{ERotationMode::OrientToMovement};
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGaitSettings GaitSettings;

	virtual void UpdateFromCompressedFlags(uint8 Flags) override;

	bool bSafeRotationModeUpdate{true};

	virtual float GetMappedSpeed() const;
	virtual bool IsInAir() const;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SpeedMultiplier{1.f};

	UGASPCharacterMovementComponent();

	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	virtual void PhysicsRotation(float DeltaTime) override;
	virtual void PhysNavWalking(float deltaTime, int32 Iterations) override;
	virtual void PhysWalking(float deltaTime, int32 Iterations) override;

	virtual void MoveSmooth(const FVector& InVelocity, const float DeltaSeconds,
	                        FStepDownResult* OutStepDownResult = 0) override;
	virtual float GetMaxAcceleration() const override;
	virtual float GetMaxBrakingDeceleration() const override;
	virtual bool HasMovementInputVector() const;
	void UpdateRotationMode();

	void SetGait(const EGait NewGait);
	UFUNCTION(Server, Reliable)
	void Server_SetGait(const EGait NewGait);

	void SetRotationMode(const ERotationMode NewRotationMode);
	UFUNCTION(Server, Reliable)
	void Server_SetRotationMode(const ERotationMode NewRotationMode);

	FORCEINLINE void SetGaitSettings(const FGaitSettings& NewGaitSettings)
	{
		GaitSettings = NewGaitSettings;
	}

	FORCEINLINE EGait GetGait() const
	{
		return SafeGait;
	}

	FORCEINLINE ERotationMode GetRotationMode() const
	{
		return SafeRotationMode;
	}

	FORCEINLINE FGaitSettings GetGaitSettings() const
	{
		return GaitSettings;
	}
};
