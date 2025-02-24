// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/Interface.h"
#include "GASPInteractionTransformInterface.generated.h"


UINTERFACE()
class UGASPInteractionTransformInterface : public UInterface
{
	GENERATED_BODY()
};

class GASP_API IGASPInteractionTransformInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Traversal")
	void SetInteractionTransform(const FTransform& InteractionTransform);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Traversal")
	void GetInteractionTransform(FTransform& OutInteractionTransform) const;
};

UCLASS(meta = (BlueprintThreadSafe))
class GASP_API UInteractionTransformBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Traversal")
	static UPARAM(DisplayName = "Success") bool SetInteractionTransform(UObject* InteractionTransformObject,
	                                                                    const FTransform& InteractionTransform);
	UFUNCTION(BlueprintCallable, Category="Traversal")
	static UPARAM(DisplayName = "Success") bool GetInteractionTransform(UObject* InteractionTransformObject,
	                                                                    UPARAM(DisplayName = "InteractionTransform")
	                                                                    FTransform& OutInteractionTransform);
};
