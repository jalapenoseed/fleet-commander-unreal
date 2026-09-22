#pragma once

#include "CoreMinimal.h"
#include "FCTypes.h"

struct FFCCatalog
{
	static FFCFrameProfile Frame(EFCFrame Kind);
	static FFCWeaponProfile Weapon(EFCWeapon Kind);
	static FLinearColor SkinColor(EFCSkin Kind);
	static FLinearColor TeamColor(int32 FleetId);
	static FVector FrameBodyScale(EFCFrame Kind);
	static float PlanetGravity(EFCPlanet Planet);
	static float PlanetDensity(EFCPlanet Planet);
	static const TCHAR* FrameName(EFCFrame Kind);
	static const TCHAR* WeaponName(EFCWeapon Kind);
	static const TCHAR* FormationName(EFCFormation Kind);
	static const TCHAR* CameraName(EFCCamera Kind);
};
