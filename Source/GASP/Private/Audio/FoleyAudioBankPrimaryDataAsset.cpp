// Fill out your copyright notice in the Description page of Project Settings.


#include "Audio/FoleyAudioBankPrimaryDataAsset.h"
#include "GameplayTagContainer.h"
#include "Engine/StreamableManager.h"

USoundBase* UFoleyAudioBankPrimaryDataAsset::GetSoundFromEvent(const FGameplayTag Event) const
{
	const TSoftObjectPtr<USoundBase>& SoundEffect = FoleyPrimaryData.FindRef(Event);

	return SoundEffect.LoadSynchronous();
}

void UFoleyAudioBankPrimaryDataAsset::PreloadSoundsAsync()
{
	TArray<FSoftObjectPath> SoundPaths;
	for (const auto& Pair : FoleyPrimaryData)
	{
		if (Pair.Value.IsValid())
		{
			SoundPaths.Add(Pair.Value.ToSoftObjectPath());
		}
	}

	if (SoundPaths.Num() > 0)
	{
		StreamableHandle = StreamableManager.RequestAsyncLoad(
			SoundPaths,
			FStreamableDelegate::CreateUObject(this, &UFoleyAudioBankPrimaryDataAsset::HandleAssetsLoaded)
		);
	}
}

void UFoleyAudioBankPrimaryDataAsset::HandleAssetsLoaded()
{
	StreamableHandle.Reset();
	OnSoundsPreloaded.Broadcast();
}
