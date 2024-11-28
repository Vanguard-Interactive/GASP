
#include "Utils/AnimationUtils.h"

float UAnimationUtils::CalculateDirection(const FVector& Velocity, const FTransform& ActorTransform)
{
	if (Velocity.IsNearlyZero())
	{
		return 0.f;
	}

	// Получаем Forward и Right векторы из ActorTransform
	const FVector ForwardVector = ActorTransform.GetRotation().GetForwardVector();
	const FVector RightVector = ActorTransform.GetRotation().GetRightVector();

	// Проецируем вектор скорости на направляющие векторы
	const float ForwardDot = FVector::DotProduct(ForwardVector, Velocity.GetSafeNormal2D());
	const float RightDot = FVector::DotProduct(RightVector, Velocity.GetSafeNormal2D());

	// Вычисляем угол в градусах и возвращаем его
	return FMath::Atan2(RightDot, ForwardDot) * (180.f / PI);
}

