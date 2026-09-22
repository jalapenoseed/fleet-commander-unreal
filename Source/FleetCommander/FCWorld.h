#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FCTypes.h"
#include "FCWorld.generated.h"

class UInstancedStaticMeshComponent;
class UStaticMesh;
class USceneComponent;
class UMaterialInstanceDynamic;

UCLASS()
class FLEETCOMMANDER_API AFCWorld : public AActor
{
	GENERATED_BODY()

public:
	AFCWorld();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> Root;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UInstancedStaticMeshComponent> BodyMesh;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UInstancedStaticMeshComponent> ArmMesh;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UInstancedStaticMeshComponent> RotorMesh;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UInstancedStaticMeshComponent> BeaconMesh;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UInstancedStaticMeshComponent> TracerMesh;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UInstancedStaticMeshComponent> ScoutMesh;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UInstancedStaticMeshComponent> RelayMesh;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UInstancedStaticMeshComponent> CargoMesh;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UInstancedStaticMeshComponent> UtilityMesh;
	bool bUnityMeshes = false;

	FFCFleetConfig Config;
	TArray<FFCDroneState> Drones;
	TArray<FTransform> TracerPool;

	bool bPaused = false;
	bool bBattle = false;
	bool bRoundEnded = false;
	bool bHudHidden = false;
	float SimTime = 0.f;
	float RoundTime = 0.f;
	float RoundLimit = 180.f;
	int32 Selected = 0;
	int32 Controlled = -1;
	int32 BlueAlive = 0;
	int32 RedAlive = 0;
	int32 BlueKills = 0;
	int32 RedKills = 0;
	int32 BlueWins = 0;
	int32 RedWins = 0;
	int32 Draws = 0;
	int32 PerTeam = 12;
	EFCArenaFormation BlueForm = EFCArenaFormation::Wedge;
	EFCArenaFormation RedForm = EFCArenaFormation::Wedge;
	EFCCamera CameraMode = EFCCamera::Orbit;
	FString Banner;
	FString LastEvent;

	int32 Count() const { return Drones.Num(); }
	const FFCDroneState* GetSelected() const;
	FVector FleetCentroid() const;
	FVector ActionFocus() const;
	int32 LongestSurvivor() const;

	void RebuildShow(int32 InCount);
	void LaunchAll();
	void RecallAll();
	void CycleFormation(int32 Delta);
	void CycleMotion();
	void ToggleBoids();
	void TogglePause();
	void SelectNext(int32 Delta);
	void SelectNearest(const FVector& WorldPoint);
	void StartArena(int32 InPerTeam);
	void ReturnToShow();
	void JoinTeam(int32 Team);
	void PilotSelected();
	void LeavePilot();
	void FireControlled();
	void ReloadControlled();
	void UseAbility(EFCAbility Ability);
	void DropPayload();
	void ApplyPilotInput(const FVector& Move, const FRotator& Look, bool bBoost, float Dt);
	void CycleCamera();
	void SetSky(EFCSky Sky);
	void CycleScenery();

private:
	float Accumulator = 0.f;
	float RotorPhase = 0.f;
	TArray<int32> Neighbors;

	UInstancedStaticMeshComponent* FrameComp(int32 Index) const;
	void Step(float Dt);
	void StepShow(int32 I, float Dt);
	void StepBattle(int32 I, float Dt);
	void StepPhysics(int32 I, float Dt);
	void UpdateBoids(int32 I, float Dt);
	void EnsureResources(FFCDroneState& S);
	void UpdateResources(FFCDroneState& S, float Dt);
	void TryFire(int32 I);
	void ApplyDamage(int32 Victim, int32 Source, float Amount, const FVector& Dir);
	void BeginReload(FFCDroneState& S);
	void UpdateVisuals(float Dt);
	void SpawnTracers(const FVector& From, const FVector& To, const FLinearColor& Color);
	UMaterialInstanceDynamic* MakeColor(const FLinearColor& Color, bool bEmissive);
};
