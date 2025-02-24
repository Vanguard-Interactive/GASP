// Fill out your copyright notice in the Description page of Project Settings.


#include "Interfaces/GASPInteractionTransformInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASPInteractionTransformInterface)

bool UInteractionTransformBlueprintFunctionLibrary::SetInteractionTransform(UObject* InteractionTransformObject,
                                                                            const FTransform& InteractionTransform)
{
	if (IsValid(InteractionTransformObject) &&
		InteractionTransformObject->Implements<UGASPInteractionTransformInterface>())
	{
		IGASPInteractionTransformInterface::Execute_SetInteractionTransform(InteractionTransformObject,
		                                                                    InteractionTransform);
		return true;
	}
	return false;
}

bool UInteractionTransformBlueprintFunctionLibrary::GetInteractionTransform(UObject* InteractionTransformObject,
                                                                            FTransform& OutInteractionTransform)
{
	if (IsValid(InteractionTransformObject) &&
		InteractionTransformObject->Implements<UGASPInteractionTransformInterface>())
	{
		IGASPInteractionTransformInterface::Execute_GetInteractionTransform(InteractionTransformObject,
		                                                                    OutInteractionTransform);
		return true;
	}
	return false;
}
