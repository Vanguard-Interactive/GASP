
#include "Utils/AnimationUtils.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AnimationUtils)

float UAnimationUtils::CalculateDirection(const FVector& Vector, const FRotator& Rotation)
{
	if (Vector.IsNearlyZero() || Rotation.IsNearlyZero())
		return 0.f;

	const FVector NormalizedVelocity{ Vector.GetSafeNormal2D() };

	const float YawRad = FMath::DegreesToRadians(Rotation.Yaw);

	// Get Forward and Right vectors from ActorRotation
	const FVector ForwardVector{ FMath::Cos(YawRad), FMath::Sin(YawRad), 0.f };
	const FVector RightVector{ -ForwardVector.Y, ForwardVector.X, 0.f };

	// Projection speed vector to direction vector
	const float ForwardDot = FVector::DotProduct(ForwardVector, NormalizedVelocity);
	const float RightDot = FVector::DotProduct(RightVector, NormalizedVelocity);

	return FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));
}
