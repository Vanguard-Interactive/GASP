// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimationModifier.h"
#include "AnimationModifier_BakePhaseCurve.generated.h"

/**
 * 
 */
UCLASS()
class GASP_API UAnimationModifier_BakePhaseCurve : public UAnimationModifier
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere)
	FName Name{};

	UPROPERTY(EditAnywhere)
	FName LeftFootTrack{};
	UPROPERTY(EditAnywhere)
	FName RightFootTrack{};

	UPROPERTY(EditAnywhere)
	TArray<FAnimNotifyEvent> LeftFootEvents{};
	UPROPERTY(EditAnywhere)
	TArray<FAnimNotifyEvent> RightFootEvents{};

	UPROPERTY(EditAnywhere)
	FTransform LeftPhaseTransform{ FTransform::Identity };
	UPROPERTY(EditAnywhere)
	FTransform RightPhaseTransform{ FTransform::Identity };

	UPROPERTY(EditAnywhere)
	bool Looping{ false };
	UPROPERTY(EditAnywhere)
	bool StartAtZero{ false };
	UPROPERTY(EditAnywhere)
	bool LeftFirst{false};
	UPROPERTY(EditAnywhere)
	bool LeftLast{false};

public:
	virtual void OnApply_Implementation(UAnimSequence* AnimationSequence) override;

	void FootTrackPhase(UAnimSequence* AnimationSequence, FName FootTrack, TArray<FAnimNotifyEvent>& FootEvents, FTransform& PhaseTransform, bool bIsLeft);

	void AddKey(UAnimSequence* AnimationSequence, float Time, FName CurveName, FTransform Transform, bool bIsLeft);
	void ClearCurve(UAnimSequence* AnimationSequence, FName CurveName);


};
