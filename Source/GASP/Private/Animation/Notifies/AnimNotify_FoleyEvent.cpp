// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies//AnimNotify_FoleyEvent.h"
#include "BlueprintGameplayTagLibrary.h"
#include "Interfaces/GASPFoleyAudioBankInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Types/GameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AnimNotify_FoleyEvent)

#if WITH_EDITOR && ALLOW_CONSOLE
static IConsoleVariable* DrawDebug = IConsoleManager::Get().FindConsoleVariable(
	TEXT("DDCvar.DrawVisLogShapesForFoleySounds"));
#endif

UAnimNotify_FoleyEvent::UAnimNotify_FoleyEvent()
{
	const TArray<FGameplayTag> MovementTagsList = {
		FoleyTags::Run,
		FoleyTags::RunBackwds,
		FoleyTags::RunStrafe,
		FoleyTags::Scuff,
		FoleyTags::ScuffPivot,
		FoleyTags::Walk,
		FoleyTags::WalkBackwds
	};

	const TArray<FGameplayTag> ActionTagsList = {
		FoleyTags::Jump,
		FoleyTags::Land
	};

	MovementTags = FGameplayTagContainer::CreateFromArray(MovementTagsList);
	ActionTags = FGameplayTagContainer::CreateFromArray(ActionTagsList);
}

void UAnimNotify_FoleyEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                    const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!GetFoleyAudioBank(MeshComp) && !IsValid(DefaultBank))
	{
		return;
	}

	AActor* Owner = MeshComp->GetOwner();
	if (!IsValid(Owner))
	{
		return;
	}

	const UWorld* WorldContext = Owner->GetWorld();

	FName SocketName{Side == EFoleyEventSide::Left ? TEXT("foot_l") : TEXT("foot_r")};

	FHitResult Hit;
	const FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);

	WorldContext->LineTraceSingleByChannel(Hit, SocketLocation, SocketLocation - FVector::ZAxisVector * TraceLength,
	                                       ECC_Visibility, QueryParams);
	if (!Hit.bBlockingHit)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(WorldContext, DefaultBank->GetSoundFromEvent(Event),
	                                      MeshComp->GetComponentLocation(),
	                                      VolumeMultiplier, PitchMultiplier);

#if WITH_EDITOR && ALLOW_CONSOLE

	if (!DrawDebug)
	{
		return;
	}
	const bool bDebug = DrawDebug->GetBool();
	if (!bDebug)
	{
		return;
	}

	const FVector SphereCenter = Side == EFoleyEventSide::None
		                             ? MeshComp->GetComponentLocation()
		                             : SocketLocation;

	// UVisualLoggerKismetLibrary::LogSphere(WorldContext, SphereCenter, 5.f, VisLogDebugText, VisLogDebugColor,
	//                                       FName(TEXT("VisLogFoley")));
	DrawDebugSphere(WorldContext, SphereCenter, 10.f, 12, VisLogDebugColor.ToRGBE(),
	                false, 4.f);
#endif
}

FString UAnimNotify_FoleyEvent::GetNotifyName_Implementation() const
{
	const FString TagName = Event.ToString();
	FString RightPart;
	TagName.Split(TEXT("."), nullptr, &RightPart, ESearchCase::IgnoreCase, ESearchDir::FromEnd);

	return FString::Printf(TEXT("FoleyEvent: %s"), RightPart.IsEmpty() ? *TagName : *RightPart);
}

bool UAnimNotify_FoleyEvent::GetFoleyAudioBank(const USkeletalMeshComponent* MeshComponent)
{
	AActor* Owner = MeshComponent->GetOwner();
	if (!IsValid(Owner))
	{
		return false;
	}

	if (!ActionTags.HasTag(Event))
	{
		return false;
	}

	IGASPFoleyAudioBankInterface* Interface = Cast<IGASPFoleyAudioBankInterface>(Owner);
	if (!Interface)
	{
		return false;
	}

	UGASPFoleyAudioBankPrimaryDataAsset* FoleyBank = Interface->GetFoleyAudioBank();
	if (IsValid(FoleyBank))
	{
		DefaultBank = TObjectPtr<UGASPFoleyAudioBankPrimaryDataAsset>(FoleyBank);
	}

	if (MovementTags.HasTag(Event))
	{
		return Interface->CanPlayFootstepSounds();
	}

	return true;
}
