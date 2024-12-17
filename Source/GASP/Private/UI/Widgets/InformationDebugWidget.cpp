// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/InformationDebugWidget.h"
#include "Actors/GASPCharacter.h"
#include "Animation/GASPAnimInstance.h"

void UInformationDebugWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CachedCharacter = StaticCast<AGASPCharacter*>(GetOwningPlayerPawn());
	if (!CachedCharacter.IsValid()) return;
	
	AnimInstance = StaticCast<UGASPAnimInstance*>(CachedCharacter->GetMesh()->GetAnimInstance());
	if (!AnimInstance.IsValid()) return;
}

void UInformationDebugWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	UpdateCharacterInfo();
}

void UInformationDebugWidget::UpdateCharacterInfo()
{
	if (!CachedCharacter.IsValid() || !AnimInstance.IsValid()) return;
	CharacterInfo = AnimInstance->GetCharacterInfo();
}
