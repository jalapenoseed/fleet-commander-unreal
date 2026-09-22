#pragma once

#include "CoreMinimal.h"
#include "FCTypes.generated.h"

UENUM(BlueprintType)
enum class EFCFrame : uint8 { Scout, Relay, Cargo, Utility };

UENUM(BlueprintType)
enum class EFCSkin : uint8 { Graphite, Arctic, Desert, Crimson, Cobalt, Industrial };

UENUM(BlueprintType)
enum class EFCWeapon : uint8
{
	Pulse, RapidFire, Scatter, Shockwave, PrecisionBeam,
	IonDisruptor, BurstTagger, ArcLink, Repulsor, DrainRay
};

UENUM(BlueprintType)
enum class EFCFormation : uint8
{
	Grid, Ring, Wedge, Line, Column, DoubleOrbit, Scatter, Staggered,
	HighLow, Overwatch, Helix, Sphere, Heart, Art
};

UENUM(BlueprintType)
enum class EFCArenaFormation : uint8
{
	Line, Wedge, Grid, Ring, Stack, Echelon, Diamond, DoubleWedge, Box, Sphere,
	Helix, Arc, Cross, Staggered, Columns, LooseCloud, HighLow, Crescent, Pincer, Escort
};

UENUM(BlueprintType)
enum class EFCMotion : uint8 { None, Orbit, Wave, Pulse, Dance };

UENUM(BlueprintType)
enum class EFCInfluence : uint8
{
	None, Vortex, Attract, Repel, Wave, Lissajous, Spiral, Braid, Twin, Square, Riemann
};

UENUM(BlueprintType)
enum class EFCPhase : uint8 { Grounded, Flying, Returning, Falling, Wreck };

UENUM(BlueprintType)
enum class EFCPlanet : uint8 { Earth, Moon, Mars };

UENUM(BlueprintType)
enum class EFCSky : uint8 { Day, Golden, Dusk, Night, MilkyWay, Moonlit, Overcast };

UENUM(BlueprintType)
enum class EFCScenery : uint8
{
	Stadium, Coast, Alpine, City, Meadow, Creek, Overlook, RuralTown, Metro, Harbor, Desert, ForestLake
};

UENUM(BlueprintType)
enum class EFCWeather : uint8 { Clear, Rain, Storm, Snow };

UENUM(BlueprintType)
enum class EFCBattleStyle : uint8
{
	Balanced, Pursuit, Evasive, Guardian, FlankLeft, FlankRight, HighCover, OrbitStyle,
	Weave, Strafe, HitAndRun, Screen, Intercept, Ambush, Regroup, Adaptive
};

UENUM(BlueprintType)
enum class EFCCamera : uint8 { Orbit, Top, Front, Follow, FPV, Shoulder, Mounted, Ground, Free, Cinematic, Action, BestFight, Survivor };

UENUM(BlueprintType)
enum class EFCAbility : uint8 { Guard, Dodge, Boost };

USTRUCT(BlueprintType)
struct FFCWeaponProfile
{
	GENERATED_BODY()
	float Range = 3800.f;
	float Damage = 1.25f;
	float Interval = 1.f;
	float Cone = 0.99f;
	int32 Targets = 1;
	int32 Magazine = 24;
	int32 Reserve = 120;
	float Energy = 0.0006f;
	float Heat = 0.08f;
	float Reload = 1.8f;
};

USTRUCT(BlueprintType)
struct FFCFrameProfile
{
	GENERATED_BODY()
	FString Name = TEXT("Scout");
	float Speed = 1.15f;
	float Agility = 1.25f;
	float Armor = 0.88f;
	float Energy = 0.92f;
	float Mass = 0.84f;
};

USTRUCT(BlueprintType)
struct FFCInfluenceLayer
{
	GENERATED_BODY()
	EFCInfluence Kind = EFCInfluence::None;
	float Strength = 800.f;
	float Frequency = 0.6f;
	float Phase = 0.f;
	float Blend = 1.f;
};

USTRUCT(BlueprintType)
struct FFCDroneState
{
	GENERATED_BODY()

	int32 Id = 0;
	int32 FleetId = 0;
	int32 Palette = 0;
	EFCFrame Frame = EFCFrame::Scout;
	EFCSkin Skin = EFCSkin::Graphite;
	EFCWeapon Weapon = EFCWeapon::Pulse;
	EFCBattleStyle Style = EFCBattleStyle::Balanced;
	EFCPhase Phase = EFCPhase::Grounded;
	FVector Position = FVector::ZeroVector;
	FVector Velocity = FVector::ZeroVector;
	FVector Home = FVector::ZeroVector;
	FVector Target = FVector::ZeroVector;
	FVector HitDirection = FVector::ZeroVector;
	FRotator Rotation = FRotator::ZeroRotator;
	float Battery = 1.f;
	float Health = 100.f;
	float Cooldown = 0.f;
	float Heat = 0.f;
	float Stamina = 100.f;
	float Stun = 0.f;
	float ReloadTime = 0.f;
	float AbilityCooldown = 0.f;
	float GuardAge = 0.f;
	float HitAge = 10.f;
	float DestructionAge = 0.f;
	int32 Ammo = 24;
	int32 AmmoCapacity = 24;
	int32 ReserveAmmo = 120;
	int32 Payloads = 3;
	int32 Kills = 0;
	int32 TargetId = -1;
	bool Guarding = false;
	bool ResourcesReady = false;
	FName AIState = TEXT("READY");

	bool IsAirborne() const { return Phase == EFCPhase::Flying || Phase == EFCPhase::Returning || Phase == EFCPhase::Falling; }
	bool IsDisabled() const { return Phase == EFCPhase::Wreck || Health <= 0.f; }
};

USTRUCT(BlueprintType)
struct FFCFleetConfig
{
	GENERATED_BODY()

	FFCFleetConfig()
	{
		Layers.SetNum(4);
	}

	EFCFormation Formation = EFCFormation::Ring;
	EFCMotion Pattern = EFCMotion::Orbit;
	EFCPlanet Planet = EFCPlanet::Earth;
	EFCScenery Scenery = EFCScenery::Stadium;
	EFCSky Sky = EFCSky::Night;
	EFCWeather Weather = EFCWeather::Clear;
	float Spacing = 300.f;
	float Height = 3500.f;
	float Scale = 1.f;
	float RotationDeg = 0.f;
	float Speed = 2400.f;
	float Acceleration = 2400.f;
	FVector Origin = FVector::ZeroVector;
	bool bBoids = true;
	bool bUnlimitedBattery = false;
	bool bArcadeLift = true;
	float Separation = 1.5f;
	float Alignment = 0.7f;
	float Cohesion = 0.1f;
	float NeighborRadius = 800.f;
	float BatteryWh = 45.f;
	float DrainScale = 1.f;
	float Bpm = 120.f;
	TArray<FFCInfluenceLayer> Layers;
};

inline float FCMeters(float Meters) { return Meters * 100.f; }
inline FVector FCClampField(const FVector& P)
{
	return FVector(
		FMath::Clamp(P.X, -95000.f, 95000.f),
		FMath::Clamp(P.Y, -95000.f, 95000.f),
		FMath::Clamp(P.Z, 80.f, 31000.f));
}
