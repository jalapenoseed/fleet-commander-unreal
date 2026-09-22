#include "FCWorld.h"
#include "FCCatalog.h"
#include "FCFormation.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AFCWorld::AFCWorld()
{
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	BodyMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Bodies"));
	ArmMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Arms"));
	RotorMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Rotors"));
	BeaconMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Beacons"));
	TracerMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Tracers"));
	BodyMesh->SetupAttachment(Root);
	ArmMesh->SetupAttachment(Root);
	RotorMesh->SetupAttachment(Root);
	BeaconMesh->SetupAttachment(Root);
	TracerMesh->SetupAttachment(Root);
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BodyMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	BodyMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	BodyMesh->SetGenerateOverlapEvents(false);
	ArmMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RotorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BeaconMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TracerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodyMesh->SetCastShadow(true);
	BeaconMesh->SetCastShadow(false);
	TracerMesh->SetCastShadow(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cyl(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (Cube.Succeeded()) { BodyMesh->SetStaticMesh(Cube.Object); ArmMesh->SetStaticMesh(Cube.Object); TracerMesh->SetStaticMesh(Cube.Object); }
	if (Sphere.Succeeded()) BeaconMesh->SetStaticMesh(Sphere.Object);
	if (Cyl.Succeeded()) RotorMesh->SetStaticMesh(Cyl.Object);
}

void AFCWorld::BeginPlay()
{
	Super::BeginPlay();
	if (UMaterialInstanceDynamic* BodyMat = MakeColor(FLinearColor(0.10f, 0.12f, 0.14f), false))
	{
		BodyMesh->SetMaterial(0, BodyMat);
		ArmMesh->SetMaterial(0, BodyMat);
	}
	if (UMaterialInstanceDynamic* RotorMat = MakeColor(FLinearColor(0.04f, 0.05f, 0.055f), false))
	{
		RotorMesh->SetMaterial(0, RotorMat);
	}
	if (UMaterialInstanceDynamic* BeaconMat = MakeColor(FLinearColor(0.22f, 0.82f, 0.90f), true))
	{
		BeaconMesh->SetMaterial(0, BeaconMat);
	}
	if (UMaterialInstanceDynamic* TracerMat = MakeColor(FLinearColor(1.f, 0.82f, 0.38f), true))
	{
		TracerMesh->SetMaterial(0, TracerMat);
	}
	RebuildShow(96);
	LaunchAll();
	LastEvent = TEXT("Fleet Commander 1.3 — Unreal port ready");
}

void AFCWorld::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const float Dt = FMath::Min(DeltaSeconds, 0.1f);
	if (!bPaused)
	{
		Accumulator += Dt;
		const float StepDt = 1.f / 60.f;
		int32 Guard = 0;
		while (Accumulator >= StepDt && Guard++ < 4)
		{
			Step(StepDt);
			Accumulator -= StepDt;
		}
	}
	UpdateVisuals(Dt);
}

const FFCDroneState* AFCWorld::GetSelected() const
{
	return Drones.IsValidIndex(Selected) ? &Drones[Selected] : nullptr;
}

FVector AFCWorld::FleetCentroid() const
{
	FVector Sum = FVector::ZeroVector;
	int32 N = 0;
	for (const FFCDroneState& S : Drones)
	{
		if (!S.IsDisabled()) { Sum += S.Position; ++N; }
	}
	return N > 0 ? Sum / N : FVector(0.f, 0.f, Config.Height);
}

FVector AFCWorld::ActionFocus() const
{
	float Best = 0.f;
	FVector P = FleetCentroid();
	for (const FFCDroneState& S : Drones)
	{
		if (S.IsDisabled()) continue;
		const float Score = S.Velocity.Size() + (S.HitAge < 0.6f ? 4000.f : 0.f);
		if (Score > Best) { Best = Score; P = S.Position; }
	}
	return P;
}

int32 AFCWorld::LongestSurvivor() const
{
	int32 Best = Selected;
	float BestH = -1.f;
	for (int32 I = 0; I < Drones.Num(); ++I)
	{
		if (!Drones[I].IsDisabled() && Drones[I].Health > BestH)
		{
			BestH = Drones[I].Health;
			Best = I;
		}
	}
	return Best;
}

void AFCWorld::RebuildShow(int32 InCount)
{
	InCount = FMath::Clamp(InCount, 0, 2000);
	bBattle = false;
	bRoundEnded = false;
	Controlled = -1;
	Banner.Empty();
	Drones.Reset(InCount);
	for (int32 I = 0; I < InCount; ++I)
	{
		FFCDroneState S;
		S.Id = I;
		S.FleetId = 0;
		S.Palette = I % 9;
		S.Frame = static_cast<EFCFrame>(I % 4);
		S.Skin = static_cast<EFCSkin>(I % 6);
		S.Weapon = EFCWeapon::Pulse;
		const FVector Pad = FFCFormationMath::Grid(I, InCount, 180.f) + FVector(0.f, 0.f, 40.f);
		S.Position = Pad;
		S.Home = Pad;
		S.Target = Pad;
		S.Phase = EFCPhase::Grounded;
		S.Battery = 1.f;
		S.Health = 100.f;
		Drones.Add(S);
	}
	Selected = 0;
	LastEvent = FString::Printf(TEXT("Show fleet rebuilt: %d aircraft"), InCount);
}

void AFCWorld::LaunchAll()
{
	for (FFCDroneState& S : Drones)
	{
		if (S.Phase == EFCPhase::Grounded || S.Phase == EFCPhase::Returning)
		{
			S.Phase = EFCPhase::Flying;
			if (S.Battery < 0.05f) S.Battery = 1.f;
		}
	}
	LastEvent = TEXT("Launch");
}

void AFCWorld::RecallAll()
{
	for (FFCDroneState& S : Drones)
	{
		if (S.Phase == EFCPhase::Flying) S.Phase = EFCPhase::Returning;
	}
	LeavePilot();
	LastEvent = TEXT("Recall");
}

void AFCWorld::CycleFormation(int32 Delta)
{
	int32 V = static_cast<int32>(Config.Formation) + Delta;
	const int32 Max = static_cast<int32>(EFCFormation::Art);
	if (V < 0) V = Max;
	if (V > Max) V = 0;
	Config.Formation = static_cast<EFCFormation>(V);
	LastEvent = FString::Printf(TEXT("Formation: %s"), FFCCatalog::FormationName(Config.Formation));
}

void AFCWorld::CycleMotion()
{
	int32 V = (static_cast<int32>(Config.Pattern) + 1) % 5;
	Config.Pattern = static_cast<EFCMotion>(V);
	LastEvent = TEXT("Motion pattern cycled");
}

void AFCWorld::ToggleBoids()
{
	Config.bBoids = !Config.bBoids;
	LastEvent = Config.bBoids ? TEXT("Boids on") : TEXT("Boids off");
}

void AFCWorld::TogglePause() { bPaused = !bPaused; }

void AFCWorld::SelectNext(int32 Delta)
{
	if (Drones.Num() == 0) return;
	Selected = (Selected + Delta + Drones.Num()) % Drones.Num();
}

void AFCWorld::SelectNearest(const FVector& WorldPoint)
{
	int32 Best = Selected;
	float BestD = TNumericLimits<float>::Max();
	for (int32 I = 0; I < Drones.Num(); ++I)
	{
		if (Drones[I].IsDisabled()) continue;
		const float D = FVector::DistSquared(Drones[I].Position, WorldPoint);
		if (D < BestD) { BestD = D; Best = I; }
	}
	Selected = Best;
}

void AFCWorld::StartArena(int32 InPerTeam)
{
	PerTeam = FMath::Clamp(InPerTeam, 1, 128);
	const int32 N = PerTeam * 2;
	bBattle = true;
	bRoundEnded = false;
	RoundTime = 0.f;
	Controlled = -1;
	Banner.Empty();
	BlueKills = RedKills = 0;
	Drones.Reset(N);
	for (int32 I = 0; I < N; ++I)
	{
		FFCDroneState S;
		S.Id = I;
		S.FleetId = I < PerTeam ? 0 : 1;
		S.Frame = static_cast<EFCFrame>(I % 4);
		S.Skin = S.FleetId == 0 ? EFCSkin::Cobalt : EFCSkin::Crimson;
		S.Weapon = static_cast<EFCWeapon>(I % 4);
		S.Style = static_cast<EFCBattleStyle>((I / 2) % 16);
		const float Side = S.FleetId == 0 ? -4500.f : 4500.f;
		const FVector Slot = FFCFormationMath::ArenaSlot(S.FleetId == 0 ? BlueForm : RedForm, I % PerTeam, PerTeam, 400.f);
		S.Position = Slot + FVector(Side, 0.f, 1800.f);
		S.Home = S.Position;
		S.Target = S.Position;
		S.Phase = EFCPhase::Flying;
		S.Battery = 1.f;
		S.Health = 100.f;
		EnsureResources(S);
		Drones.Add(S);
	}
	Selected = 0;
	LastEvent = FString::Printf(TEXT("Arena: %d v %d"), PerTeam, PerTeam);
}

void AFCWorld::ReturnToShow()
{
	RebuildShow(96);
	LaunchAll();
}

void AFCWorld::JoinTeam(int32 Team)
{
	if (!bBattle) StartArena(PerTeam);
	for (int32 I = 0; I < Drones.Num(); ++I)
	{
		if (Drones[I].FleetId == Team && !Drones[I].IsDisabled() && Drones[I].Phase == EFCPhase::Flying)
		{
			Controlled = I;
			Selected = I;
			LastEvent = Team == 0 ? TEXT("Joined Blue") : TEXT("Joined Red");
			return;
		}
	}
}

void AFCWorld::PilotSelected()
{
	if (Drones.IsValidIndex(Selected) && !Drones[Selected].IsDisabled())
	{
		Controlled = Selected;
		LastEvent = TEXT("Piloting selected aircraft");
	}
}

void AFCWorld::LeavePilot()
{
	if (Controlled >= 0) LastEvent = TEXT("Returned aircraft to AI");
	Controlled = -1;
}

void AFCWorld::FireControlled()
{
	if (Controlled >= 0) TryFire(Controlled);
}

void AFCWorld::ReloadControlled()
{
	if (Drones.IsValidIndex(Controlled)) BeginReload(Drones[Controlled]);
}

void AFCWorld::UseAbility(EFCAbility Ability)
{
	if (!Drones.IsValidIndex(Controlled)) return;
	FFCDroneState& S = Drones[Controlled];
	if (S.IsDisabled() || S.AbilityCooldown > 0.f || S.Stamina < 25.f) return;
	S.Stamina -= 25.f;
	S.AbilityCooldown = Ability == EFCAbility::Guard ? 1.6f : 3.f;
	if (Ability == EFCAbility::Guard) { S.Guarding = true; S.GuardAge = 0.f; }
	else
	{
		S.Guarding = false;
		const FVector Dir = S.Rotation.Vector();
		S.Velocity += Dir * (Ability == EFCAbility::Dodge ? 1400.f : 2000.f);
	}
}

void AFCWorld::DropPayload()
{
	if (!Drones.IsValidIndex(Selected) || Drones[Selected].Payloads <= 0) return;
	FFCDroneState& S = Drones[Selected];
	S.Payloads--;
	for (int32 J = 0; J < Drones.Num(); ++J)
	{
		if (J == Selected || Drones[J].IsDisabled()) continue;
		if (FVector::Dist(Drones[J].Position, S.Position) < 1200.f)
		{
			ApplyDamage(J, Selected, 18.f, (Drones[J].Position - S.Position).GetSafeNormal());
		}
	}
	LastEvent = TEXT("Payload released");
}

void AFCWorld::ApplyPilotInput(const FVector& Move, const FRotator& Look, bool bBoost, float Dt)
{
	if (!Drones.IsValidIndex(Controlled)) return;
	FFCDroneState& S = Drones[Controlled];
	if (S.IsDisabled() || S.Phase != EFCPhase::Flying) { LeavePilot(); return; }
	const FFCFrameProfile P = FFCCatalog::Frame(S.Frame);
	S.Rotation.Yaw += Look.Yaw;
	S.Rotation.Pitch = FMath::Clamp(S.Rotation.Pitch + Look.Pitch, -50.f, 50.f);
	const FRotator YawRot(0.f, S.Rotation.Yaw, 0.f);
	const FVector Forward = YawRot.RotateVector(FVector::ForwardVector);
	const FVector Right = YawRot.RotateVector(FVector::RightVector);
	const float Speed = Config.Speed * P.Speed * (bBoost ? 1.65f : 1.f);
	S.Velocity += (Forward * Move.X + Right * Move.Y + FVector(0.f, 0.f, Move.Z)) * Speed * P.Agility * Dt * 3.f;
	S.Velocity *= FMath::Exp(-Dt * 2.4f);
	S.Position = FCClampField(S.Position + S.Velocity * Dt);
	S.AIState = TEXT("PILOT");
}

void AFCWorld::CycleCamera()
{
	int32 V = (static_cast<int32>(CameraMode) + 1) % 13;
	CameraMode = static_cast<EFCCamera>(V);
	LastEvent = FString::Printf(TEXT("Camera: %s"), FFCCatalog::CameraName(CameraMode));
}

void AFCWorld::SetSky(EFCSky Sky) { Config.Sky = Sky; }
void AFCWorld::CycleScenery()
{
	int32 V = (static_cast<int32>(Config.Scenery) + 1) % 12;
	Config.Scenery = static_cast<EFCScenery>(V);
}

void AFCWorld::Step(float Dt)
{
	SimTime += Dt;
	if (bBattle && !bRoundEnded) RoundTime += Dt;
	BlueAlive = RedAlive = 0;
	for (int32 I = 0; I < Drones.Num(); ++I)
	{
		if (I == Controlled) { UpdateResources(Drones[I], Dt); continue; }
		if (bBattle) StepBattle(I, Dt); else StepShow(I, Dt);
		StepPhysics(I, Dt);
		UpdateResources(Drones[I], Dt);
		if (!Drones[I].IsDisabled())
		{
			if (Drones[I].FleetId == 0) BlueAlive++; else RedAlive++;
		}
	}
	if (bBattle && !bRoundEnded)
	{
		if (BlueAlive == 0 || RedAlive == 0 || RoundTime >= RoundLimit)
		{
			bRoundEnded = true;
			LeavePilot();
			if (BlueAlive == RedAlive) { Draws++; Banner = TEXT("DRAW"); }
			else if (BlueAlive > RedAlive) { BlueWins++; Banner = TEXT("BLUE WINS"); }
			else { RedWins++; Banner = TEXT("RED WINS"); }
			LastEvent = Banner;
		}
	}
}

void AFCWorld::StepShow(int32 I, float Dt)
{
	FFCDroneState& S = Drones[I];
	if (S.Phase == EFCPhase::Wreck || S.Phase == EFCPhase::Falling) return;
	if (S.Phase == EFCPhase::Grounded)
	{
		if (!Config.bUnlimitedBattery) S.Battery = FMath::Min(1.f, S.Battery + Dt * 0.12f);
		S.Velocity = FVector::ZeroVector;
		return;
	}
	if (S.Phase == EFCPhase::Returning) S.Target = S.Home + FVector(0.f, 0.f, 40.f);
	else S.Target = FFCFormationMath::Target(Config, I, Drones.Num(), SimTime);
	if (Config.bBoids && S.Phase == EFCPhase::Flying) UpdateBoids(I, Dt);
	const FFCFrameProfile P = FFCCatalog::Frame(S.Frame);
	const FVector To = S.Target - S.Position;
	const float Dist = To.Size();
	FVector Desired = Dist > 1.f ? To.GetSafeNormal() * Config.Speed * P.Speed : FVector::ZeroVector;
	S.Velocity = FMath::VInterpTo(S.Velocity, Desired, Dt, Config.Acceleration * P.Agility / 2400.f);
	if (S.Phase == EFCPhase::Returning && Dist < 120.f)
	{
		S.Phase = EFCPhase::Grounded;
		S.Position = S.Home;
		S.Velocity = FVector::ZeroVector;
	}
}

void AFCWorld::StepBattle(int32 I, float Dt)
{
	FFCDroneState& S = Drones[I];
	if (S.IsDisabled() || S.Phase != EFCPhase::Flying) return;
	int32 Target = S.TargetId;
	if (!Drones.IsValidIndex(Target) || Drones[Target].IsDisabled() || Drones[Target].FleetId == S.FleetId)
	{
		Target = -1;
		float Best = TNumericLimits<float>::Max();
		for (int32 J = 0; J < Drones.Num(); ++J)
		{
			if (Drones[J].FleetId == S.FleetId || Drones[J].IsDisabled()) continue;
			const float D = FVector::DistSquared(S.Position, Drones[J].Position);
			if (D < Best) { Best = D; Target = J; }
		}
		S.TargetId = Target;
	}
	const FVector HomeSide = S.FleetId == 0 ? FVector(-4500.f, 0.f, 1800.f) : FVector(4500.f, 0.f, 1800.f);
	FVector Way = HomeSide + FFCFormationMath::ArenaSlot(S.FleetId == 0 ? BlueForm : RedForm, I % PerTeam, PerTeam, 400.f);
	FVector Perceived = Target >= 0 ? Drones[Target].Position : Way;
	FVector Away = (S.Position - Perceived).GetSafeNormal();
	if (Away.IsNearlyZero()) Away = FVector::ForwardVector;
	FVector Goal = Perceived + Away * 900.f;
	EFCBattleStyle Style = S.Style;
	if (Style == EFCBattleStyle::Adaptive) Style = static_cast<EFCBattleStyle>(8 + (static_cast<int32>(SimTime / 4.f) + I) % 7);
	const FVector Lateral = FVector::CrossProduct(Away, FVector::UpVector).GetSafeNormal();
	switch (Style)
	{
	case EFCBattleStyle::Weave: Goal += Lateral * FMath::Sin(SimTime * 1.8f + I) * 1500.f + FVector(0.f, 0.f, FMath::Cos(SimTime + I) * 600.f); break;
	case EFCBattleStyle::Strafe: Goal += Lateral * ((I % 2 == 0) ? 1600.f : -1600.f); break;
	case EFCBattleStyle::HitAndRun: Goal = Perceived + Away * ((FMath::Fmod(SimTime, 6.f) < 2.f) ? 700.f : 3000.f); break;
	case EFCBattleStyle::Screen: Goal = FMath::Lerp(Way, Perceived, 0.4f) + Lateral * ((I % 3 - 1) * 1200.f); break;
	case EFCBattleStyle::Intercept: if (Target >= 0) Goal = Perceived + Drones[Target].Velocity * 0.5f + Away * 900.f; break;
	case EFCBattleStyle::Ambush: Goal = FVector::Dist(S.Position, Perceived) > 3000.f ? Way + FVector(0.f, 0.f, 1200.f) : Perceived + Away * 500.f; break;
	case EFCBattleStyle::Regroup: Goal = Way + Away * 800.f; S.AIState = TEXT("REGROUP"); break;
	case EFCBattleStyle::Evasive: Goal += Away * 1800.f + Lateral * FMath::Sin(SimTime * 3.f + I) * 1200.f; break;
	case EFCBattleStyle::Guardian: Goal = Way; break;
	case EFCBattleStyle::FlankLeft: Goal = Perceived + Lateral * 1800.f; break;
	case EFCBattleStyle::FlankRight: Goal = Perceived - Lateral * 1800.f; break;
	case EFCBattleStyle::HighCover: Goal = Perceived + Away * 1200.f + FVector(0.f, 0.f, 1600.f); break;
	default: break;
	}
	if (S.ReloadTime > 0.f || S.Heat > 0.85f) { Goal = Perceived + Away * 3500.f; S.AIState = S.ReloadTime > 0.f ? TEXT("RELOAD") : TEXT("COOLING"); }
	else if (S.Health < 30.f) { Goal += Away * 1500.f; S.AIState = TEXT("EVADE"); }
	else S.AIState = FVector::Dist(S.Position, Perceived) > FFCCatalog::Weapon(S.Weapon).Range ? TEXT("CHASE") : TEXT("ENGAGE");
	S.Target = FCClampField(Goal);
	const FFCFrameProfile P = FFCCatalog::Frame(S.Frame);
	const FVector To = S.Target - S.Position;
	FVector Desired = To.GetSafeNormal() * Config.Speed * P.Speed * 1.15f;
	S.Velocity = FMath::VInterpTo(S.Velocity, Desired, Dt, 3.2f * P.Agility);
	if (Target >= 0) TryFire(I);
}

void AFCWorld::StepPhysics(int32 I, float Dt)
{
	FFCDroneState& S = Drones[I];
	if (S.Phase == EFCPhase::Grounded) return;
	if (S.Phase == EFCPhase::Wreck || S.Phase == EFCPhase::Falling || (!Config.bArcadeLift && Config.Planet != EFCPlanet::Earth))
	{
		S.Velocity.Z -= FFCCatalog::PlanetGravity(Config.Planet) * Dt;
		if (S.Phase == EFCPhase::Flying) S.Phase = EFCPhase::Falling;
	}
	if (!Config.bUnlimitedBattery && S.IsAirborne() && S.Phase != EFCPhase::Wreck)
	{
		const float Drain = 0.008f * Config.DrainScale * (0.4f + S.Velocity.Size() / 4000.f) * Dt;
		S.Battery = FMath::Max(0.f, S.Battery - Drain);
		if (S.Battery <= 0.f && S.Phase == EFCPhase::Flying) S.Phase = EFCPhase::Falling;
		const float DistHome = FVector::Dist(S.Position, S.Home);
		if (!bBattle && S.Phase == EFCPhase::Flying && S.Battery < 0.18f + DistHome / 200000.f) S.Phase = EFCPhase::Returning;
	}
	S.Position = FCClampField(S.Position + S.Velocity * Dt);
	if (S.Position.Z <= 40.f)
	{
		S.Position.Z = 40.f;
		if (S.Phase == EFCPhase::Falling || S.Phase == EFCPhase::Wreck)
		{
			S.Phase = EFCPhase::Wreck;
			S.Velocity = FVector::ZeroVector;
		}
		else if (S.Phase == EFCPhase::Returning)
		{
			S.Phase = EFCPhase::Grounded;
			S.Velocity = FVector::ZeroVector;
		}
	}
	if (S.Velocity.SizeSquared() > 1.f)
	{
		const FRotator Want = S.Velocity.Rotation();
		S.Rotation = FMath::RInterpTo(S.Rotation, Want, Dt, 6.f);
	}
}

void AFCWorld::UpdateBoids(int32 I, float Dt)
{
	FFCDroneState& S = Drones[I];
	FVector Sep = FVector::ZeroVector, Ali = FVector::ZeroVector, Coh = FVector::ZeroVector;
	int32 N = 0;
	const float R2 = Config.NeighborRadius * Config.NeighborRadius;
	for (int32 J = 0; J < Drones.Num() && N < 24; ++J)
	{
		if (J == I || !Drones[J].IsAirborne()) continue;
		const FVector D = S.Position - Drones[J].Position;
		const float Dist2 = D.SizeSquared();
		if (Dist2 > R2 || Dist2 < 1.f) continue;
		Sep += D / Dist2;
		Ali += Drones[J].Velocity;
		Coh += Drones[J].Position;
		++N;
	}
	if (N == 0) return;
	Sep /= N; Ali /= N; Coh = (Coh / N) - S.Position;
	S.Velocity += (Sep.GetSafeNormal() * Config.Separation * 800.f + Ali.GetSafeNormal() * Config.Alignment * 400.f + Coh.GetSafeNormal() * Config.Cohesion * 300.f) * Dt;
}

void AFCWorld::EnsureResources(FFCDroneState& S)
{
	if (S.ResourcesReady) return;
	const FFCWeaponProfile W = FFCCatalog::Weapon(S.Weapon);
	S.Ammo = W.Magazine;
	S.AmmoCapacity = W.Magazine;
	S.ReserveAmmo = W.Reserve;
	S.Stamina = 100.f;
	S.ResourcesReady = true;
	S.AIState = TEXT("READY");
}

void AFCWorld::UpdateResources(FFCDroneState& S, float Dt)
{
	EnsureResources(S);
	S.AbilityCooldown = FMath::Max(0.f, S.AbilityCooldown - Dt);
	S.Stun = FMath::Max(0.f, S.Stun - Dt);
	S.HitAge += Dt;
	S.Cooldown = FMath::Max(0.f, S.Cooldown - Dt);
	S.Heat = FMath::Max(0.f, S.Heat - Dt * 0.22f);
	if (S.Guarding)
	{
		S.GuardAge += Dt;
		S.Stamina = FMath::Max(0.f, S.Stamina - Dt * 25.f);
		if (S.Stamina <= 0.f || S.GuardAge > 1.2f) S.Guarding = false;
	}
	else S.Stamina = FMath::Min(100.f, S.Stamina + Dt * 15.f);
	if (S.ReloadTime > 0.f)
	{
		S.ReloadTime = FMath::Max(0.f, S.ReloadTime - Dt);
		if (S.ReloadTime == 0.f)
		{
			const int32 N = FMath::Min(S.AmmoCapacity - S.Ammo, S.ReserveAmmo);
			S.Ammo += N;
			S.ReserveAmmo -= N;
		}
	}
	if (S.Phase == EFCPhase::Wreck) S.DestructionAge += Dt;
}

void AFCWorld::BeginReload(FFCDroneState& S)
{
	const FFCWeaponProfile W = FFCCatalog::Weapon(S.Weapon);
	if (S.ReloadTime <= 0.f && S.ReserveAmmo > 0 && S.Ammo < S.AmmoCapacity) S.ReloadTime = W.Reload;
}

void AFCWorld::TryFire(int32 I)
{
	FFCDroneState& S = Drones[I];
	if (S.Stun > 0.f || S.Cooldown > 0.f || S.ReloadTime > 0.f || S.Ammo <= 0 || S.Heat > 0.95f) return;
	const FFCWeaponProfile W = FFCCatalog::Weapon(S.Weapon);
	const FVector Aim = S.Rotation.Vector();
	int32 Hits = 0;
	for (int32 J = 0; J < Drones.Num() && Hits < W.Targets; ++J)
	{
		if (J == I || Drones[J].FleetId == S.FleetId || Drones[J].IsDisabled()) continue;
		const FVector Delta = Drones[J].Position - S.Position;
		const float Dist = Delta.Size();
		if (Dist > W.Range || Dist < 1.f) continue;
		const FVector Dir = Delta / Dist;
		const float Cone = FVector::DotProduct(Aim, Dir);
		const bool bShock = W.Cone < 0.f && Dist <= W.Range;
		if (!bShock && Cone < W.Cone) continue;
		ApplyDamage(J, I, W.Damage * 12.f, Dir);
		SpawnTracers(S.Position, Drones[J].Position, FFCCatalog::TeamColor(S.FleetId));
		++Hits;
	}
	if (Hits > 0 || S.Weapon == EFCWeapon::Shockwave)
	{
		S.Ammo--;
		S.Cooldown = W.Interval;
		S.Heat = FMath::Min(1.f, S.Heat + W.Heat);
		S.Battery = FMath::Max(0.f, S.Battery - W.Energy);
		if (S.Ammo <= 0) BeginReload(S);
	}
}

void AFCWorld::ApplyDamage(int32 Victim, int32 Source, float Amount, const FVector& Dir)
{
	if (!Drones.IsValidIndex(Victim)) return;
	FFCDroneState& V = Drones[Victim];
	if (V.IsDisabled()) return;
	const FFCFrameProfile P = FFCCatalog::Frame(V.Frame);
	float Dmg = Amount / P.Armor;
	if (V.Guarding && Drones.IsValidIndex(Source))
	{
		const FVector ToSrc = (Drones[Source].Position - V.Position).GetSafeNormal();
		if (FVector::DotProduct(V.Rotation.Vector(), ToSrc) > 0.3f)
		{
			if (V.GuardAge <= 0.16f) { Drones[Source].Stun = FMath::Max(Drones[Source].Stun, 0.35f); Dmg = 0.f; V.AIState = TEXT("PARRY"); }
			else Dmg *= 0.25f;
		}
	}
	V.Health -= Dmg;
	V.HitAge = 0.f;
	V.HitDirection = Dir;
	V.Velocity += Dir * 400.f;
	if (Drones.IsValidIndex(Source)) Drones[Source].AIState = TEXT("HIT");
	if (V.Health <= 0.f)
	{
		V.Health = 0.f;
		V.Phase = EFCPhase::Falling;
		V.DestructionAge = 0.f;
		if (Drones.IsValidIndex(Source))
		{
			Drones[Source].Kills++;
			if (Drones[Source].FleetId == 0) BlueKills++; else RedKills++;
		}
		if (Victim == Controlled) LeavePilot();
		LastEvent = FString::Printf(TEXT("%s-%d down"), FFCCatalog::FrameName(V.Frame), V.Id);
	}
}

void AFCWorld::SpawnTracers(const FVector& From, const FVector& To, const FLinearColor& Color)
{
	FTransform T;
	T.SetLocation((From + To) * 0.5f);
	T.SetRotation((To - From).Rotation().Quaternion());
	T.SetScale3D(FVector(FVector::Dist(From, To) / 100.f, 0.08f, 0.08f));
	if (TracerMesh->GetInstanceCount() < 64) TracerMesh->AddInstance(T, false);
	else TracerMesh->UpdateInstanceTransform(FMath::RandRange(0, 63), T, true, true, true);
}

void AFCWorld::UpdateVisuals(float Dt)
{
	RotorPhase += Dt * (bPaused ? 0.f : 28.f);
	const int32 N = Drones.Num();
	if (BodyMesh->GetInstanceCount() != N)
	{
		BodyMesh->ClearInstances();
		ArmMesh->ClearInstances();
		RotorMesh->ClearInstances();
		BeaconMesh->ClearInstances();
		for (int32 I = 0; I < N; ++I)
		{
			BodyMesh->AddInstance(FTransform::Identity, false);
			ArmMesh->AddInstance(FTransform::Identity, false);
			RotorMesh->AddInstance(FTransform::Identity, false);
			BeaconMesh->AddInstance(FTransform::Identity, false);
		}
		BodyMesh->MarkRenderStateDirty();
	}
	for (int32 I = 0; I < N; ++I)
	{
		const FFCDroneState& S = Drones[I];
		const FVector Body = FFCCatalog::FrameBodyScale(S.Frame);
		const float Wreck = S.Phase == EFCPhase::Wreck ? 0.55f : 1.f;
		FTransform BT(S.Rotation, S.Position, Body * 0.01f * Wreck);
		BodyMesh->UpdateInstanceTransform(I, BT, true, false, true);
		FTransform AT(S.Rotation, S.Position, FVector(0.55f, 0.08f, 0.04f) * Wreck);
		ArmMesh->UpdateInstanceTransform(I, AT, true, false, true);
		FRotator RotorRot = S.Rotation;
		RotorRot.Yaw += RotorPhase * 70.f * (S.IsAirborne() && S.Phase != EFCPhase::Wreck ? 1.f : 0.05f);
		FTransform RT(RotorRot, S.Position + S.Rotation.RotateVector(FVector(0.f, 0.f, Body.Z * 0.4f)), FVector(0.32f, 0.32f, 0.03f));
		RotorMesh->UpdateInstanceTransform(I, RT, true, false, true);
		const FLinearColor Team = bBattle ? FFCCatalog::TeamColor(S.FleetId) : FFCCatalog::SkinColor(S.Skin);
		FTransform KT(S.Rotation, S.Position + S.Rotation.RotateVector(FVector(-Body.X * 0.2f, 0.f, Body.Z * 0.6f)), FVector(0.12f + (I == Selected ? 0.08f : 0.f)));
		if (S.Phase == EFCPhase::Wreck) KT.SetScale3D(FVector(0.04f));
		BeaconMesh->UpdateInstanceTransform(I, KT, true, false, true);
	}
	BodyMesh->MarkRenderStateDirty();
	ArmMesh->MarkRenderStateDirty();
	RotorMesh->MarkRenderStateDirty();
	BeaconMesh->MarkRenderStateDirty();
}

UMaterialInstanceDynamic* AFCWorld::MakeColor(const FLinearColor& Color, bool bEmissive)
{
	UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineDebugMaterials/DebugMeshMaterial.DebugMeshMaterial"));
	if (!Base) Base = UMaterial::GetDefaultMaterial(MD_Surface);
	UMaterialInstanceDynamic* Mid = UMaterialInstanceDynamic::Create(Base, this);
	if (Mid)
	{
		Mid->SetVectorParameterValue(TEXT("Color"), Color);
		if (bEmissive) Mid->SetScalarParameterValue(TEXT("Emissive"), 4.f);
	}
	return Mid;
}
