#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FCTypes.h"
#include "FCPlayerController.generated.h"

class AFCWorld;
class AFCGameMode;
class AFCPawn;

UCLASS()
class FLEETCOMMANDER_API AFCPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AFCPlayerController();
	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

	AFCWorld* Sim() const;

	float OrbitYaw = 35.f;
	float OrbitPitch = -32.f;
	float OrbitDistance = 7200.f;
	bool bLookHeld = false;
	bool bBoost = false;
	FVector MoveInput = FVector::ZeroVector;

private:
	void OnMoveForward(float V);
	void OnMoveRight(float V);
	void OnMoveUp(float V);
	void OnLookX(float V);
	void OnLookY(float V);
	void OnZoom(float V);
	void OnLookPressed();
	void OnLookReleased();
	void OnFire();
	void OnReload();
	void OnGuard();
	void OnDodge();
	void OnBoostPressed();
	void OnBoostReleased();
	void OnPayload();
	void OnLaunch();
	void OnRecall();
	void OnPause();
	void OnNext();
	void OnPrev();
	void OnCamera();
	void OnHideHud();
	void OnFormNext();
	void OnFormPrev();
	void OnMotion();
	void OnBoids();
	void OnArena();
	void OnJoinBlue();
	void OnJoinRed();
	void OnPilot();
	void OnLeave();
	void OnShow();
	void OnWalk();
	void ApplyCamera(float Dt);
	void SyncWalker(float Dt);
};
