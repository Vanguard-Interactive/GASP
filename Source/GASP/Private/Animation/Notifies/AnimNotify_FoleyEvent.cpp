// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies//AnimNotify_FoleyEvent.h"

#include "BlueprintGameplayTagLibrary.h"
#include "Interfaces/FoleyAudioBankInterface.h"
#include "Kismet/GameplayStatics.h"

UAnimNotify_FoleyEvent::UAnimNotify_FoleyEvent()
{
	const TArray<FGameplayTag> MovementTagsList = {
		FGameplayTag::RequestGameplayTag(TEXT("Foley.Event.Run")),
		FGameplayTag::RequestGameplayTag(TEXT("Foley.Event.RunBackwds")),
		FGameplayTag::RequestGameplayTag(TEXT("Foley.Event.RunStrafe")),
		FGameplayTag::RequestGameplayTag(TEXT("Foley.Event.Scuff")),
		FGameplayTag::RequestGameplayTag(TEXT("Foley.Event.ScuffPivot")),
		FGameplayTag::RequestGameplayTag(TEXT("Foley.Event.Walks")),
		FGameplayTag::RequestGameplayTag(TEXT("Foley.Event.WalkBackwds"))
	};

	const TArray<FGameplayTag> ActionTagsList = {
		FGameplayTag::RequestGameplayTag(TEXT("Foley.Event.Jump")),
		FGameplayTag::RequestGameplayTag(TEXT("Foley.Event.Land"))
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

	UWorld* WorldContext = Owner->GetWorld();
	UGameplayStatics::PlaySoundAtLocation(WorldContext, DefaultBank->GetSoundFromEvent(Event),
	                                      MeshComp->GetComponentLocation(),
	                                      VolumeMultiplier, PitchMultiplier);

#if WITH_EDITOR && ALLOW_CONSOLE
	const auto DrawDebug = IConsoleManager::Get().FindConsoleVariable(TEXT("DDCvar.DrawVisLogShapesForFoleySounds"));
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
		                             : MeshComp->GetSocketLocation(
			                             Side == EFoleyEventSide::Left ? TEXT("foot_l") : TEXT("foot_r"));

	// UVisualLoggerKismetLibrary::LogSphere(WorldContext, SphereCenter, 5.f, VisLogDebugText, VisLogDebugColor,
	//                                       FName(TEXT("VisLogFoley")));
	DrawDebugSphere(WorldContext, SphereCenter, 10.f, 12, VisLogDebugColor.ToRGBE(),
	                false, 3.f, 1.f);
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

	if (!ActionTags.HasTag(Event))
	{
		return false;
	}

	if (!IsValid(Owner))
	{
		return false;
	}

	IFoleyAudioBankInterface* Interface = Cast<IFoleyAudioBankInterface>(Owner);
	if (!Interface)
	{
		return false;
	}

	DefaultBank = Interface->GetFoleyAudioBank();

	if (MovementTags.HasTag(Event))
	{
		return Interface->CanPlayFootstepSounds();
	}

	return true;
}
