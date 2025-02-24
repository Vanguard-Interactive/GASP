// Fill out your copyright notice in the Description page of Project Settings.


#include "Audio/GASPFoleyAudioBankPrimaryDataAsset.h"
#include "Engine/StreamableManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASPFoleyAudioBankPrimaryDataAsset)

USoundBase* UGASPFoleyAudioBankPrimaryDataAsset::GetSoundFromEvent(const FGameplayTag Event) const
{
	const TSoftObjectPtr<USoundBase>& SoundEffect = FoleyPrimaryData.FindRef(Event);

	return SoundEffect.LoadSynchronous();
}

void UGASPFoleyAudioBankPrimaryDataAsset::PreloadSoundsAsync()
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
			FStreamableDelegate::CreateUObject(this, &UGASPFoleyAudioBankPrimaryDataAsset::HandleAssetsLoaded)
		);
	}
}

void UGASPFoleyAudioBankPrimaryDataAsset::HandleAssetsLoaded()
{
	StreamableHandle.Reset();
	OnSoundsPreloaded.Broadcast();
}
