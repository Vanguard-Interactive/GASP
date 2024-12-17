
#include "Utils/AnimationUtils.h"

float UAnimationUtils::CalculateDirection(const FVector& Velocity, const FRotator& ActorRotation)
{
	if (Velocity.IsNearlyZero() || ActorRotation.IsNearlyZero())
	{
		return 0.f;
	}

	// Get Forward and Right vectors from ActorRotation
	const FVector ForwardVector = FRotationMatrix(ActorRotation).GetUnitAxis(EAxis::X);
	const FVector RightVector = FRotationMatrix(ActorRotation).GetUnitAxis(EAxis::Y);

	// Projection speed vector to direction vector
	const float ForwardDot = FVector::DotProduct(ForwardVector, Velocity.GetSafeNormal2D());
	const float RightDot = FVector::DotProduct(RightVector, Velocity.GetSafeNormal2D());

	// Get angle in degrees
	return FMath::Atan2(RightDot, ForwardDot) * (180.f / PI);
}

