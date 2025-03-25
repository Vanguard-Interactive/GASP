// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GASPHeldObjectInterface.generated.h"

class AGASPHeldObject;
/**
 * 
 */
UINTERFACE(MinimalAPI)
class UGASPHeldObjectInterface : public UInterface
{
	GENERATED_BODY()
};

class IGASPHeldObjectInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(Blueprintable, Category = "GASP|HeldObject")
	virtual AGASPHeldObject* GetHeldObject() = 0;
};
