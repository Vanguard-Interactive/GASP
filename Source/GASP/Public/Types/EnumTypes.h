// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnumTypes.generated.h"

/**
 * Movement gait
 */
UENUM(BlueprintType, meta=(ScriptName="EGait"))
enum class EGait : uint8
{
	Walk,
	Run,
	Sprint
};

/**
 * 
 */
UENUM(BlueprintType, meta=(ScriptName="ERotationMode"))
enum class ERotationMode : uint8
{
	OrientToMovement,
	Strafe
};

/**
 *
 */
UENUM(BlueprintType, meta=(ScriptName="ECMovementMode"))
enum class ECMovementMode : uint8
{
	OnGround,
	InAir
};

/**
 *
 */
UENUM(BlueprintType, meta=(ScriptName="EMovementState"))
enum class EMovementState : uint8
{
	Idle,
	Moving
};

/**
 * Movement gait
 */
UENUM(BlueprintType, meta=(ScriptName="EStanceMode"))
enum class EStanceMode : uint8
{
	Stand,
	Crouch
};

UENUM(BlueprintType, meta=(ScriptName="EMovementDirection"))
enum class EMovementDirection : uint8
{
	F UMETA(DisplayName = "Forward"),
	B UMETA(DisplayName = "Forward"),
	LL UMETA(DisplayName = "Left"),
	LR UMETA(DisplayName = "Left->Right"),
	RL UMETA(DisplayName = "Right->Left"),
	RR UMETA(DisplayName = "Right"),
};