// Fill out your copyright notice in the Description page of Project Settings.


#include "Interfaces/GASPGameplayCameraInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASPGameplayCameraInterface)

FCameraProperties UGASPGameplayCameraBlueprintFunctionLibrary::GetCameraProperties(AActor* CameraActor)
{
	if (IsValid(CameraActor) &&
		CameraActor->Implements<UGASPGameplayCameraInterface>())
	{
		return IGASPGameplayCameraInterface::Execute_GetCameraProperties(CameraActor);
	}
	return FCameraProperties();
}
