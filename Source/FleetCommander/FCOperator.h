#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FCOperator.generated.h"

class USceneComponent;
class UStaticMeshComponent;

/** GRIDRUNNER field operator — visible body that patrols camp or walks under player control. */
UCLASS()
class FLEETCOMMANDER_API AFCOperator : public AActor
{
	GENERATED_BODY()

public:
	AFCOperator();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void SetPlayerMove(const FVector& WorldMove, float YawDelta);
	void SetControlled(bool bIn);
	bool IsControlled() const { return bControlled; }
	FVector CameraLocation() const;
	FRotator CameraRotation() const;

	float Yaw = 90.f;
	int32 PatrolIndex = 0;

	UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> Root;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Torso;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Head;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Pelvis;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> ThighL;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> ThighR;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> ShinL;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> ShinR;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> ArmL;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> ArmR;

private:
	bool bControlled = false;
	float WalkPhase = 0.f;
	float Speed = 0.f;
	FVector Home = FVector::ZeroVector;
	FVector InputMove = FVector::ZeroVector;

	void Pose(float Dt);
	UStaticMeshComponent* MakePart(const TCHAR* Name, USceneComponent* Attach);
};
