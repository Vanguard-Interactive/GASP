// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnumTypes.generated.h"

/**
 * Movement gait
 */
UENUM(BlueprintType)
enum class EGait : uint8
{
	Walk,
	Run,
	Sprint
};

/**
 * 
 */
UENUM(BlueprintType)
enum class ERotationMode : uint8
{
	OrientToMovement,
	Strafe
};

/**
 *
 */
UENUM(BlueprintType)
enum class ECMovementMode : uint8
{
	OnGround,
	InAir
};

/**
 *
 */
UENUM(BlueprintType)
enum class EMovementState : uint8
{
	Idle,
	Moving
};

/**
 * Movement gait
 */
UENUM(BlueprintType)
enum class EStanceMode : uint8
{
	Stand,
	Crouch
};