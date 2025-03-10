// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/GASPTraversalComponent.h"
#include "Actors/GASPCharacter.h"
#include "AnimationWarpingLibrary.h"
#include "Animation/GASPAnimInstance.h"
#include "ChooserFunctionLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Components/GASPCharacterMovementComponent.h"
#include "Components/GASPTraversableObstacleComponent.h"
#include "Interfaces/GASPInteractionTransformInterface.h"
#include "IObjectChooser.h"
#include "MotionWarpingComponent.h"
#include "Net/UnrealNetwork.h"
#include "PlayMontageCallbackProxy.h"
#include "Engine/AssetManager.h"
#include "PoseSearch/PoseSearchLibrary.h"
#include "PoseSearch/PoseSearchResult.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASPTraversalComponent)

static FName NAME_FrontLedge{TEXT("FrontLedge")};
static FName NAME_BackLedge{TEXT("BackLedge")};
static FName NAME_BackFloor{TEXT("BackFloor")};
static FName NAME_DistanceFromLedge{TEXT("Distance_From_Ledge")};
static FName NAME_PoseHistory = TEXT("PoseHistory");

#if WITH_EDITOR && ALLOW_CONSOLE
static IConsoleVariable* DrawDebugLevelVar = IConsoleManager::Get().FindConsoleVariable(
	TEXT("DDCvar.Traversal.DrawDebugLevel"));
static IConsoleVariable* DrawDebugDurationVar = IConsoleManager::Get().FindConsoleVariable(
	TEXT("DDCvar.Traversal.DrawDebugDuration"));
#endif


// Sets default values for this component's properties
UGASPTraversalComponent::UGASPTraversalComponent()
{
	SetIsReplicatedByDefault(true);
}

void UGASPTraversalComponent::AsyncChooserLoaded()
{
	StreamableHandle.Reset();
}

void UGASPTraversalComponent::BeginPlay()
{
	Super::BeginPlay();

	CharacterOwner = Cast<AGASPCharacter>(GetOwner());
	if (!CharacterOwner.IsValid())
	{
		return;
	}

	MovementComponent = CharacterOwner->FindComponentByClass<UGASPCharacterMovementComponent>();
	MotionWarpingComponent = CharacterOwner->FindComponentByClass<UMotionWarpingComponent>();
	CapsuleComponent = CharacterOwner->GetCapsuleComponent();
	MeshComponent = CharacterOwner->GetMesh();
	if (MeshComponent.IsValid())
	{
		AnimInstance = Cast<UGASPAnimInstance>(MeshComponent->GetAnimInstance());
	}

	StreamableHandle = StreamableManager.RequestAsyncLoad(TraversalAnimationsChooserTable.ToSoftObjectPath(),
	                                                      FStreamableDelegate::CreateUObject(
		                                                      this, &ThisClass::AsyncChooserLoaded),
	                                                      FStreamableManager::DefaultAsyncLoadPriority, false);
}

void UGASPTraversalComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	Params.Condition = COND_SimulatedOnly;
	Params.RepNotifyCondition = REPNOTIFY_OnChanged;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, TraversalCheckResult, Params);
}

void UGASPTraversalComponent::ProccessHurdle(const FName CurveName, const FName WarpTarget, float& Value) const
{
	TArray<FMotionWarpingWindowData> Montages;
	UMotionWarpingUtilities::GetMotionWarpingWindowsForWarpTargetFromAnimation(
		TraversalCheckResult.ChosenMontage, WarpTarget, Montages);

	if (!Montages.IsEmpty())
	{
		UAnimationWarpingLibrary::GetCurveValueFromAnimation(TraversalCheckResult.ChosenMontage, CurveName,
		                                                     Montages[0].EndTime, Value);
		return;
	}
	MotionWarpingComponent->RemoveWarpTarget(WarpTarget);
}

void UGASPTraversalComponent::UpdateWarpTargets()
{
	if (!MotionWarpingComponent.IsValid())
	{
		return;
	}

	float DistanceFromFrontLedgeToBackLedge{0.f};

	MotionWarpingComponent->AddOrUpdateWarpTargetFromLocationAndRotation(
		NAME_FrontLedge, TraversalCheckResult.FrontLedgeLocation + FVector(0.f, 0.f, .5f),
		FRotationMatrix::MakeFromX(-TraversalCheckResult.FrontLedgeNormal).Rotator()
	);

	if (TraversalCheckResult.ActionType == LocomotionActionTags::Hurdle || TraversalCheckResult.ActionType ==
		LocomotionActionTags::Vault)
	{
		ProccessHurdle(NAME_DistanceFromLedge, NAME_BackLedge, DistanceFromFrontLedgeToBackLedge);
		MotionWarpingComponent->AddOrUpdateWarpTargetFromLocationAndRotation(
			NAME_BackLedge, TraversalCheckResult.BackLedgeLocation, FRotator::ZeroRotator);
	}
	else
	{
		MotionWarpingComponent->RemoveWarpTarget(NAME_BackLedge);
	}

	if (TraversalCheckResult.ActionType == LocomotionActionTags::Hurdle)
	{
		float DistanceFromFrontLedgeToBackFloor{0.f};
		ProccessHurdle(NAME_DistanceFromLedge, NAME_BackFloor, DistanceFromFrontLedgeToBackFloor);

		FVector NormalVector = TraversalCheckResult.BackLedgeNormal * FMath::Abs(
			DistanceFromFrontLedgeToBackLedge - DistanceFromFrontLedgeToBackFloor);

		FVector Result = TraversalCheckResult.BackLedgeLocation + NormalVector;
		Result.Z = TraversalCheckResult.BackFloorLocation.Z;

		MotionWarpingComponent->AddOrUpdateWarpTargetFromLocationAndRotation(
			NAME_BackFloor, Result, FRotator::ZeroRotator);
	}
	else
	{
		MotionWarpingComponent->RemoveWarpTarget(NAME_BackFloor);
	}
}

FTraversalResult UGASPTraversalComponent::TryTraversalAction(FTraversalCheckInputs CheckInputs)
{
	if (!CharacterOwner.IsValid())
	{
		return {true, false};
	}

	// Step 1: Cache some important values for use later in the function.
	const double StartTime = FPlatformTime::Seconds();
	const FVector& ActorLocation = CharacterOwner->GetActorLocation();
	const float& CapsuleRadius = CapsuleComponent.IsValid() ? CapsuleComponent->GetScaledCapsuleRadius() : 30.f;
	const float& CapsuleHalfHeight = CapsuleComponent.IsValid() ? CapsuleComponent->GetScaledCapsuleHalfHeight() : 60.f;

#if WITH_EDITOR && ALLOW_CONSOLE
	const int32& DrawDebugLevel = DrawDebugLevelVar ? DrawDebugLevelVar->GetInt() : 0;
	const float& DrawDebugDuration = DrawDebugDurationVar ? DrawDebugDurationVar->GetFloat() : 0;
#endif

	FTraversalCheckResult NewTraversalCheckResult;

	// Step 2.1: Perform a trace in the actor's forward direction to find a Traversable Level Block. If found, set the Hit Component, if not, exit the function.
	FHitResult Hit;
	FCollisionQueryParams QueryParams{GetQueryParams()};

	UWorld* World = CharacterOwner->GetWorld();
	FVector StartLocation = ActorLocation + CheckInputs.TraceOriginOffset;
	FVector EndLocation = StartLocation + CheckInputs.TraceForwardDirection * CheckInputs.TraceForwardDistance;
	World->SweepSingleByChannel(Hit, StartLocation,
	                            EndLocation,
	                            FQuat::Identity, ECC_Visibility,
	                            FCollisionShape::MakeCapsule(CheckInputs.TraceRadius, CheckInputs.TraceHalfHeight),
	                            QueryParams);
#if WITH_EDITOR && ALLOW_CONSOLE
	const float& LifeTime = DrawDebugDuration;
	if (DrawDebugLevel >= 2)
	{
		DrawDebugCapsule(World, StartLocation, CheckInputs.TraceHalfHeight, CheckInputs.TraceRadius, FQuat::Identity,
		                 FColor::Black, false, LifeTime);
		if (Hit.bBlockingHit)
		{
			DrawDebugCapsule(World, Hit.Location, CheckInputs.TraceHalfHeight, CheckInputs.TraceRadius, FQuat::Identity,
			                 FColor::Black, false, LifeTime);
		}

		DrawDebugLine(World, StartLocation, Hit.Location, FColor::Black, false, LifeTime);
	}
#endif

	IGASPTraversableObstacleInterface* Obstacle = Cast<IGASPTraversableObstacleInterface>(Hit.GetActor());

	if (!Hit.bBlockingHit || (Obstacle == nullptr && (!Hit.GetActor() || !(Hit.GetActor() && Hit.GetActor()->Implements<
		UGASPTraversableObstacleInterface>()))))
	{
		return {true, false};
	}

	NewTraversalCheckResult.HitComponent = Hit.GetComponent();

	// Step 2.2: If a traversable level block was found, get the front and back ledge transforms from it (using its own internal function).
	if (Obstacle)
	{
		Obstacle->Execute_GetLedgeTransforms(Hit.GetActor(), Hit.ImpactPoint, ActorLocation, NewTraversalCheckResult);
	}
	else
	{
		IGASPTraversableObstacleInterface::Execute_GetLedgeTransforms(Hit.GetActor(), Hit.ImpactPoint, ActorLocation,
		                                                              NewTraversalCheckResult);
	}

#if WITH_EDITOR && ALLOW_CONSOLE
	// DEBUG: Draw Debug shapes at ledge locations.
	if (DrawDebugLevel >= 1)
	{
		if (NewTraversalCheckResult.bHasFrontLedge)
		{
			DrawDebugSphere(World, NewTraversalCheckResult.FrontLedgeLocation, 10.0f, 12,
			                FLinearColor::Green.ToFColor(true), false, DrawDebugDuration,
			                SDPG_World, 1.0f);
		}

		if (NewTraversalCheckResult.bHasBackLedge)
		{
			DrawDebugSphere(World, NewTraversalCheckResult.BackLedgeLocation, 10.0f, 12,
			                FLinearColor::Blue.ToFColor(true), false, DrawDebugDuration,
			                SDPG_World, 1.0f);
		}
	}
#endif

	// Step 3.1 If the traversable level block has a valid front ledge, continue the function. If not, exit early.
	if (!NewTraversalCheckResult.bHasFrontLedge)
	{
		return {true, false};
	}

	/** Step 3.2: Perform a trace from the actors location up to the front ledge location to determine if there is
	 * room for the actor to move up to it. If so, continue the function. If not, exit early. */
	const FVector HasRoomCheckFrontLedgeLocation = NewTraversalCheckResult.FrontLedgeLocation +
		NewTraversalCheckResult.FrontLedgeNormal * (CapsuleRadius + 2.0f) +
		FVector::ZAxisVector * (CapsuleHalfHeight + 2.0f);

	World->SweepSingleByChannel(Hit, ActorLocation, HasRoomCheckFrontLedgeLocation, FQuat::Identity,
	                            ECC_Visibility, FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight),
	                            QueryParams);
#if WITH_EDITOR && ALLOW_CONSOLE
	if (DrawDebugLevel >= 2)
	{
		DrawDebugCapsule(World, ActorLocation, CapsuleHalfHeight, CapsuleRadius, FQuat::Identity, FColor::Red,
		                 false,
		                 LifeTime);
	}
#endif

	if (Hit.bBlockingHit || Hit.bStartPenetrating)
	{
		NewTraversalCheckResult.bHasFrontLedge = false;
		return {true, false};
	}

	// Step 3.3: save the height of the obstacle using the delta between the actor and front ledge transform.
	NewTraversalCheckResult.ObstacleHeight = FMath::Abs(
		(ActorLocation - FVector::ZAxisVector * CapsuleHalfHeight - NewTraversalCheckResult.FrontLedgeLocation).Z);

	/** Step 3.4: Perform a trace across the top of the obstacle from the front ledge to the back ledge to see if
	 * there's room for the actor to move across it.*/
	const FVector HasRoomCheckBackLedgeLocation = NewTraversalCheckResult.BackLedgeLocation +
		NewTraversalCheckResult.BackLedgeNormal * (CapsuleRadius + 2.0f) +
		FVector::ZAxisVector * (CapsuleHalfHeight + 2.0f);


	bool bHit = World->SweepSingleByChannel(Hit, HasRoomCheckFrontLedgeLocation, HasRoomCheckBackLedgeLocation,
	                                        FQuat::Identity,
	                                        ECC_Visibility,
	                                        FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight),
	                                        QueryParams);
#if WITH_EDITOR && ALLOW_CONSOLE
	if (DrawDebugLevel >= 1)
	{
		DrawDebugCapsule(World, HasRoomCheckFrontLedgeLocation, CapsuleHalfHeight, CapsuleRadius, FQuat::Identity,
		                 FColor::Red, false,
		                 LifeTime);
	}
#endif

	if (bHit)
	{
		/** Step 3.5: If there is not room, save the obstacle depth using the difference between the front ledge and the
		 * trace impact point, and invalidate the back ledge. */
		NewTraversalCheckResult.ObstacleDepth = (Hit.ImpactPoint - NewTraversalCheckResult.FrontLedgeLocation).Size2D();
		NewTraversalCheckResult.bHasBackLedge = false;
	}
	else
	{
		// Step 3.5: If there is room, save the obstacle depth using the difference between the front and back ledge locations.
		NewTraversalCheckResult.ObstacleDepth =
			(NewTraversalCheckResult.FrontLedgeLocation - NewTraversalCheckResult.BackLedgeLocation).Size2D();

		/** Step 3.6: Trace downward from the back ledge location (using the height of the obstacle for the distance)
		 * to find the floor. If there is a floor, save its location and the back ledges height (using the distance
		 * between the back ledge and the floor). If no floor was found, invalidate the back floor.*/
		const FVector EndTraceLocation = NewTraversalCheckResult.BackLedgeLocation +
			NewTraversalCheckResult.BackLedgeNormal * (CapsuleRadius + 2.0f) -
			FVector::ZAxisVector * (NewTraversalCheckResult.ObstacleHeight - CapsuleHalfHeight + 50.0f);

		World->SweepSingleByChannel(Hit, HasRoomCheckBackLedgeLocation, EndTraceLocation,
		                            FQuat::Identity,
		                            ECC_Visibility,
		                            FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight),
		                            QueryParams);
#if WITH_EDITOR && ALLOW_CONSOLE
		if (DrawDebugLevel >= 1)
		{
			DrawDebugCapsule(World, HasRoomCheckBackLedgeLocation, CapsuleHalfHeight, CapsuleRadius, FQuat::Identity,
			                 FColor::Red, false, LifeTime);
		}
#endif

		if (Hit.bBlockingHit)
		{
			NewTraversalCheckResult.bHasBackFloor = true;
			NewTraversalCheckResult.BackFloorLocation = Hit.ImpactPoint;
			NewTraversalCheckResult.BackLedgeHeight = FMath::Abs(
				(Hit.ImpactPoint - NewTraversalCheckResult.BackLedgeLocation).Z);
		}
		else
		{
			NewTraversalCheckResult.bHasBackFloor = false;
		}
	}

	UChooserTable* ChooserTable{TraversalAnimationsChooserTable.Get()};
	// Step 5.3: Evaluate a chooser to select all montages that match the conditions of the traversal check.
	FTraversalChooserInput ChooserParameters;
	ChooserParameters.ActionType = NewTraversalCheckResult.ActionType;
	ChooserParameters.Gait = CharacterOwner->GetGait();
	ChooserParameters.Speed = CharacterOwner->GetVelocity().Size2D();
	ChooserParameters.MovementMode = MovementComponent->MovementMode;
	ChooserParameters.bHasBackFloor = NewTraversalCheckResult.bHasBackFloor;
	ChooserParameters.bHasBackLedge = NewTraversalCheckResult.bHasBackLedge;
	ChooserParameters.bHasFrontLedge = NewTraversalCheckResult.bHasFrontLedge;
	ChooserParameters.ObstacleHeight = NewTraversalCheckResult.ObstacleHeight;
	ChooserParameters.ObstacleDepth = NewTraversalCheckResult.ObstacleDepth;
	ChooserParameters.BackLedgeHeight = NewTraversalCheckResult.BackLedgeHeight;
	FTraversalChooserOutput ChooserOutput;

	FChooserEvaluationContext Context = UChooserFunctionLibrary::MakeChooserEvaluationContext();
	Context.AddStructParam(ChooserParameters);
	Context.AddStructParam(ChooserOutput);
	auto AnimationAssets{
		UChooserFunctionLibrary::EvaluateObjectChooserBaseMulti(
			Context, UChooserFunctionLibrary::MakeEvaluateChooser(ChooserTable), UAnimMontage::StaticClass())
	};
	NewTraversalCheckResult.ActionType = ChooserOutput.ActionType;

	/* Step 5.1: Continue if there is a valid action type. If none of the conditions were met, no action can be
	 * performed, therefore exit the function. */
	if (NewTraversalCheckResult.ActionType == FGameplayTag::EmptyTag)
	{
		return {true, false};
	}

	/** Step 5.2: Send the front ledge location to the Anim BP using an interface. This transform will be used for a
	 * custom channel within the following Motion Matching search. */
	if (!AnimInstance.IsValid())
	{
		return {true, false};
	}
	IGASPInteractionTransformInterface* InteractableObject =
		Cast<IGASPInteractionTransformInterface>(AnimInstance);
	if (InteractableObject == nullptr && !AnimInstance->Implements<UGASPInteractionTransformInterface>())
	{
		return {true, false};
	}

	const FTransform InteractionTransform =
		FTransform(FRotationMatrix::MakeFromZ(NewTraversalCheckResult.FrontLedgeNormal).ToQuat(),
		           NewTraversalCheckResult.FrontLedgeLocation, FVector::OneVector);
	if (InteractableObject != nullptr)
	{
		InteractableObject->Execute_SetInteractionTransform(AnimInstance.Get(), InteractionTransform);
	}
	else
	{
		IGASPInteractionTransformInterface::Execute_SetInteractionTransform(AnimInstance.Get(), InteractionTransform);
	}

	/** Step 5.4: Perform a Motion Match on all the montages that were chosen by the chooser to find the best result.
	 * This match will elect the best montage AND the best entry frame (start time) based on the distance to the ledge,
	 * and the current characters pose. If for some reason no montage was found (motion matching failed, perhaps due to
	 * an invalid database or issue with the schema), print a warning and exit the function. */
	FPoseSearchBlueprintResult Result;
	UPoseSearchLibrary::MotionMatch(AnimInstance.Get(), AnimationAssets, NAME_PoseHistory,
	                                FPoseSearchContinuingProperties(), FPoseSearchFutureProperties(), Result);
	const UAnimMontage* AnimationMontage = Cast<UAnimMontage>(Result.SelectedAnimation);
	if (!IsValid(AnimationMontage))
	{
#if WITH_EDITOR && ALLOW_CONSOLE
		GEngine->AddOnScreenDebugMessage(NULL, DrawDebugDuration, FColor::Red,
		                                 FString::Printf(TEXT("Failed To Find Montage!")));
#endif
		return {true, false};
	}
	NewTraversalCheckResult.ChosenMontage = AnimationMontage;
	NewTraversalCheckResult.StartTime = Result.SelectedTime;
	NewTraversalCheckResult.PlayRate = Result.WantedPlayRate;

	TraversalCheckResult = NewTraversalCheckResult;
	PerformTraversalAction();
	Server_Traversal(TraversalCheckResult);

#if WITH_EDITOR
	if (DrawDebugLevel >= 1)
	{
		GEngine->AddOnScreenDebugMessage(-1, DrawDebugDuration, FLinearColor(0.0f, 0.66f, 1.0f).ToFColor(true),
		                                 FString::Printf(TEXT("%s"), *NewTraversalCheckResult.ToString()));
		GEngine->AddOnScreenDebugMessage(-1, DrawDebugDuration,
		                                 FLinearColor(1.0f, 0.0f, 0.824021f).ToFColor(true),
		                                 FString::Printf(
			                                 TEXT("%s"), *NewTraversalCheckResult.ActionType.ToString()));

		const FString PerfString = FString::Printf(TEXT("Execution Time: %f seconds"),
		                                           FPlatformTime::Seconds() - StartTime);
		GEngine->AddOnScreenDebugMessage(-1, DrawDebugDuration, FLinearColor(1.0f, 0.5f, 0.15f).ToFColor(true),
		                                 FString::Printf(TEXT("%s"), *PerfString));
	}
#endif
	return FTraversalResult();
}

bool UGASPTraversalComponent::IsDoingTraversal() const
{
	return bDoingTraversalAction;
}

void UGASPTraversalComponent::Traversal_ServerImplementation(const FTraversalCheckResult TraversalRep)
{
	TraversalCheckResult = TraversalRep;
	PerformTraversalAction();
}

void UGASPTraversalComponent::OnTraversalStart()
{
	MovementComponent->bIgnoreClientMovementErrorChecksAndCorrection = true;
	MovementComponent->bServerAcceptClientAuthoritativePosition = true;
}

void UGASPTraversalComponent::OnRep_TraversalResult()
{
	PerformTraversalAction();
}

void UGASPTraversalComponent::OnTraversalEnd() const
{
	MovementComponent->bIgnoreClientMovementErrorChecksAndCorrection = false;
	MovementComponent->bServerAcceptClientAuthoritativePosition = false;
}

void UGASPTraversalComponent::OnCompleteTraversal(FName NotifyName)
{
	bDoingTraversalAction = false;
	CapsuleComponent->IgnoreComponentWhenMoving(TraversalCheckResult.HitComponent, false);

	const EMovementMode MovementMode{
		TraversalCheckResult.ActionType == LocomotionActionTags::Vault
			? MOVE_Falling
			: MOVE_Walking
	};
	MovementComponent->SetMovementMode(MovementMode);

	GetWorld()->GetTimerManager().SetTimer(TraversalEndHandle, [this]()
	{
		OnTraversalEnd();
	}, IgnoreCorrectionDelay, false);
}

void UGASPTraversalComponent::PerformTraversalAction_Implementation()
{
	UpdateWarpTargets();

	OnTraversalStart();

	UPlayMontageCallbackProxy* MontageProxy{
		UPlayMontageCallbackProxy::CreateProxyObjectForPlayMontage(
			MeshComponent.Get(), const_cast<UAnimMontage*>(TraversalCheckResult.ChosenMontage.Get()),
			TraversalCheckResult.PlayRate, TraversalCheckResult.StartTime)
	};

	MontageProxy->OnCompleted.AddUniqueDynamic(this, &ThisClass::OnCompleteTraversal);
	MontageProxy->OnInterrupted.AddUniqueDynamic(this, &ThisClass::OnCompleteTraversal);

	bDoingTraversalAction = true;
	CapsuleComponent->IgnoreComponentWhenMoving(TraversalCheckResult.HitComponent, true);

	MovementComponent->SetMovementMode(MOVE_Flying);
}

void UGASPTraversalComponent::Server_Traversal_Implementation(FTraversalCheckResult TraversalRep)
{
	Traversal_ServerImplementation(TraversalRep);
}

FCollisionQueryParams UGASPTraversalComponent::GetQueryParams() const
{
	TArray<AActor*> IgnoredActors;
	CharacterOwner->GetAllChildActors(IgnoredActors);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(CharacterOwner.Get());
	QueryParams.AddIgnoredActors(IgnoredActors);

	return QueryParams;
}
