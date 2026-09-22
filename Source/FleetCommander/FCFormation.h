#pragma once

#include "CoreMinimal.h"
#include "FCTypes.h"

struct FFCFormationMath
{
	static float Hash(int32 I);
	static FVector Grid(int32 Index, int32 Count, float Spacing);
	static FVector Ring(int32 Index, int32 Count, float Radius);
	static FVector Sphere(int32 Index, int32 Count, float Radius);
	static FVector Point(EFCFormation Kind, int32 I, int32 N, float Spacing);
	static FVector ArenaSlot(EFCArenaFormation Kind, int32 Slot, int32 Count, float Spacing);
	static FVector Influence(const FFCInfluenceLayer& L, const FVector& P, int32 I, float Time);
	static FVector Target(const FFCFleetConfig& C, int32 I, int32 N, float Time);
};
