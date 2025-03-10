// Fill out your copyright notice in the Description page of Project Settings.


#include "Audio/GASPFoleyAudioBankPrimaryDataAsset.h"
#include "Engine/StreamableManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASPFoleyAudioBankPrimaryDataAsset)

TSoftObjectPtr<USoundBase> UGASPFoleyAudioBankPrimaryDataAsset::GetSoundFromEvent(const FGameplayTag Event)
{
	const TSoftObjectPtr<USoundBase>& SoundEffect = FoleyPrimaryData.FindRef(Event);

	return SoundEffect;
}

void UGASPFoleyAudioBankPrimaryDataAsset::PreloadSoundsAsync()
{
	TArray<FSoftObjectPath> SoundPaths;
	for (const auto& [Key, Value] : FoleyPrimaryData)
	{
		if (Value.IsValid())
		{
			SoundPaths.Add(Value.ToSoftObjectPath());
		}
	}

	if (SoundPaths.Num() > 0)
	{
		StreamableHandle = StreamableManager.RequestAsyncLoad(
			SoundPaths,
			FStreamableDelegate::CreateUObject(this, &UGASPFoleyAudioBankPrimaryDataAsset::HandleAssetsLoaded)
		);
	}
}

void UGASPFoleyAudioBankPrimaryDataAsset::HandleAssetsLoaded()
{
	UE_LOG(LogTemp, Warning, TEXT("Loaded"))
	StreamableHandle.Reset();
	OnSoundsPreloaded.Broadcast();
}
