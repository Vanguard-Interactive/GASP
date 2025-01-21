// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/Modifiers/AnimationModifier_BakePhaseCurve.h"
#include "AnimationBlueprintLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AnimationModifier_BakePhaseCurve)

void UAnimationModifier_BakePhaseCurve::OnApply_Implementation(UAnimSequence* AnimationSequence)
{
	auto GetLastIndex = [](const auto& InArray) -> int32
	{
		return (InArray.Num() > 0) ? InArray.Num() - 1 : 0;
	};

	auto GetFootTime = [&](bool bIsLeftLast, int32 Index) -> float
	{
		return bIsLeftLast ? LeftFootEvents[Index].GetTriggerTime() : RightFootEvents[Index].GetTriggerTime();
	};

	auto GetPhaseTransform = [&](bool bCondition) -> FTransform&
	{
		return bCondition ? RightPhaseTransform : LeftPhaseTransform;
	};

	ClearCurve(AnimationSequence, Name);

	//(const UAnimSequenceBase * AnimationSequenceBase, FName NotifyTrackName, TArray<FAnimNotifyEvent>&Events
	FootTrackPhase(AnimationSequence, LeftFootTrack, LeftFootEvents, LeftPhaseTransform, true);
	FootTrackPhase(AnimationSequence, RightFootTrack, RightFootEvents, RightPhaseTransform, false);

	if (LeftFootEvents.IsEmpty() || RightFootEvents.IsEmpty())
	{
		return;
	}

	LeftFirst = LeftFootEvents[0].GetTriggerTime() < RightFootEvents[0].GetTriggerTime();
	LeftLast = LeftFootEvents[GetLastIndex(LeftFootEvents)].GetTriggerTime() > RightFootEvents[GetLastIndex(RightFootEvents)].GetTriggerTime();

	if (Looping)
	{
		AddKey(AnimationSequence, GetFootTime(LeftFirst, GetLastIndex(LeftFirst ? RightFootEvents : LeftFootEvents)) 
			- AnimationSequence->GetPlayLength(), Name, GetPhaseTransform(LeftFirst), !LeftFirst);
		return;
	}

	if (StartAtZero)
	{
		AddKey(AnimationSequence,
			0.f,
			Name,
			{ GetPhaseTransform(!LeftFirst).Rotator(), FVector::ZeroVector, FVector::OneVector },
			!LeftFirst);
	}
	else
	{
		const float LeftFootTime = GetFootTime(LeftLast, 0);
		const float RightFootTime = GetFootTime(!LeftLast, 0);
		const float TriggerTime = LeftFootTime - (RightFootTime - LeftFootTime);

		AddKey(AnimationSequence, TriggerTime, Name, GetPhaseTransform(LeftLast), !LeftLast);
	}
	
	const float FinalLeftFootTime = GetFootTime(LeftLast, GetLastIndex(LeftFootEvents));
	const float FinalRightFootTime = GetFootTime(!LeftLast, GetLastIndex(RightFootEvents));
	const float FinalTriggerTime = (FinalLeftFootTime - FinalRightFootTime) + FinalLeftFootTime;

	AddKey(AnimationSequence, FinalTriggerTime, Name, GetPhaseTransform(LeftLast), !LeftLast);
}

void UAnimationModifier_BakePhaseCurve::FootTrackPhase(UAnimSequence* AnimationSequence, FName FootTrack, TArray<FAnimNotifyEvent>& FootEvents, 
	FTransform& PhaseTransform, bool bIsLeft)
{
	if (!IsValid(AnimationSequence))
		return;

	UAnimationBlueprintLibrary::GetAnimationNotifyEventsForTrack(AnimationSequence, FootTrack, FootEvents);
	if (FootEvents.IsEmpty())
		return;

	for (auto& Event : FootEvents)
	{
		AddKey(AnimationSequence, Event.GetTriggerTime(), Name, PhaseTransform, bIsLeft);
	}
}

void UAnimationModifier_BakePhaseCurve::AddKey(UAnimSequence* AnimationSequence, float Time, FName CurveName, FTransform Transform, bool bIsLeft)
{
	FAnimationCurveIdentifier CurveIdentifier;

	UAnimationCurveIdentifierExtensions::SetCurveIdentifier(CurveIdentifier, Name, ERawCurveTrackTypes::RCT_Float);

	auto& AnimController{ AnimationSequence->GetController() };

	FRichCurveKey RichCurveKey;
	RichCurveKey.InterpMode = RCIM_Linear;
	RichCurveKey.TangentMode = RCTM_Auto;
	RichCurveKey.TangentWeightMode = RCTWM_WeightedNone;
	RichCurveKey.Time = Time;
	RichCurveKey.Value = bIsLeft ? 1.f : 0.f;

	AnimController.SetCurveKey(CurveIdentifier, RichCurveKey, true);
	if (bIsLeft)
	{
		RichCurveKey.InterpMode = RCIM_Constant;
		RichCurveKey.Time = Time - .01f;
		RichCurveKey.Value = -1.f;
		AnimController.SetCurveKey(CurveIdentifier, RichCurveKey, true);
	}
}

void UAnimationModifier_BakePhaseCurve::ClearCurve(UAnimSequence* AnimationSequence, FName CurveName)
{
	if (UAnimationBlueprintLibrary::DoesCurveExist(AnimationSequence, CurveName, ERawCurveTrackTypes::RCT_Float))
	{
		UAnimationBlueprintLibrary::RemoveCurve(AnimationSequence, CurveName, false);
		return;
	}
	UAnimationBlueprintLibrary::AddCurve(AnimationSequence, CurveName, ERawCurveTrackTypes::RCT_Float, false);
}