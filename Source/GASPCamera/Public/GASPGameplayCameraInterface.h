// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayTagContainer.h"
#include "GASPGameplayCameraInterface.generated.h"

UENUM(BlueprintType, meta = (ScriptName = "ECameraMode"))
enum class ECameraMode : uint8
{
	FreeCam,
	Strafe,
	Aim
};

UENUM(BlueprintType, meta = (ScriptName = "ECameraStyle"))
enum class ECameraStyle : uint8
{
	Far,
	Balanced,
	Close
};

UENUM(BlueprintType, meta = (ScriptName = "EViewMode"))
enum class EViewMode : uint8
{
	FirstPerson,
	ThirdPerson
};

USTRUCT(BlueprintType)
struct FCameraProperties
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECameraStyle CameraStyle{0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECameraMode CameraMode{0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag StanceMode{FGameplayTag::EmptyTag};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EViewMode ViewMode{0};
};

/**
 * 
 */
UINTERFACE()
class UGASPGameplayCameraInterface : public UInterface
{
	GENERATED_BODY()
};

class GASPCAMERA_API IGASPGameplayCameraInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Camera|Properties")
	FCameraProperties GetCameraProperties();
};

UCLASS(meta = (BlueprintThreadSafe))
class GASPCAMERA_API UGASPGameplayCameraBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Camera|Helpers")
	static FCameraProperties GetCameraProperties(AActor* CameraActor);
};
