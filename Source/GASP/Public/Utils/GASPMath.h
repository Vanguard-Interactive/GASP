#pragma once

DECLARE_STATS_GROUP(TEXT("GASP"), STATGROUP_GASP, STATCAT_Advanced)

struct GASP_API FGASPMath
{
	FGASPMath() = default;

	static float CalculateDirection(const FVector& Velocity, const FRotator& ActorRotation);
};
