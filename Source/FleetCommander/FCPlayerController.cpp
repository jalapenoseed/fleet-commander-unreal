#include "FCPlayerController.h"
#include "FCGameMode.h"
#include "FCWorld.h"
#include "FCPawn.h"
#include "FCHUD.h"
#include "FCOperator.h"
#include "FCCatalog.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/EngineBaseTypes.h"
#include "Engine/HitResult.h"
#include "Engine/World.h"

AFCPlayerController::AFCPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	PrimaryActorTick.bCanEverTick = true;
}

void AFCPlayerController::BeginPlay()
{
	Super::BeginPlay();
	FInputModeGameAndUI Mode;
	Mode.SetHideCursorDuringCapture(false);
	SetInputMode(Mode);
}

AFCWorld* AFCPlayerController::Sim() const
{
	if (const AFCGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AFCGameMode>() : nullptr)
	{
		return GM->WorldSim;
	}
	return nullptr;
}

void AFCPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (!InputComponent) return;
	InputComponent->BindAxis(TEXT("MoveForward"), this, &AFCPlayerController::OnMoveForward);
	InputComponent->BindAxis(TEXT("MoveRight"), this, &AFCPlayerController::OnMoveRight);
	InputComponent->BindAxis(TEXT("MoveUp"), this, &AFCPlayerController::OnMoveUp);
	InputComponent->BindAxis(TEXT("Turn"), this, &AFCPlayerController::OnLookX);
	InputComponent->BindAxis(TEXT("LookUp"), this, &AFCPlayerController::OnLookY);
	InputComponent->BindAxis(TEXT("Zoom"), this, &AFCPlayerController::OnZoom);
	InputComponent->BindAction(TEXT("Look"), IE_Pressed, this, &AFCPlayerController::OnLookPressed);
	InputComponent->BindAction(TEXT("Look"), IE_Released, this, &AFCPlayerController::OnLookReleased);
	InputComponent->BindAction(TEXT("Fire"), IE_Pressed, this, &AFCPlayerController::OnFire);
	InputComponent->BindAction(TEXT("Reload"), IE_Pressed, this, &AFCPlayerController::OnReload);
	InputComponent->BindAction(TEXT("Guard"), IE_Pressed, this, &AFCPlayerController::OnGuard);
	InputComponent->BindAction(TEXT("Dodge"), IE_Pressed, this, &AFCPlayerController::OnDodge);
	InputComponent->BindAction(TEXT("Boost"), IE_Pressed, this, &AFCPlayerController::OnBoostPressed);
	InputComponent->BindAction(TEXT("Boost"), IE_Released, this, &AFCPlayerController::OnBoostReleased);
	InputComponent->BindAction(TEXT("Payload"), IE_Pressed, this, &AFCPlayerController::OnPayload);
	InputComponent->BindAction(TEXT("Launch"), IE_Pressed, this, &AFCPlayerController::OnLaunch);
	InputComponent->BindAction(TEXT("Recall"), IE_Pressed, this, &AFCPlayerController::OnRecall);
	InputComponent->BindAction(TEXT("PauseGame"), IE_Pressed, this, &AFCPlayerController::OnPause);
	InputComponent->BindAction(TEXT("NextDrone"), IE_Pressed, this, &AFCPlayerController::OnNext);
	InputComponent->BindAction(TEXT("PrevDrone"), IE_Pressed, this, &AFCPlayerController::OnPrev);
	InputComponent->BindAction(TEXT("CycleCamera"), IE_Pressed, this, &AFCPlayerController::OnCamera);
	InputComponent->BindAction(TEXT("HideHud"), IE_Pressed, this, &AFCPlayerController::OnHideHud);
	InputComponent->BindAction(TEXT("FormNext"), IE_Pressed, this, &AFCPlayerController::OnFormNext);
	InputComponent->BindAction(TEXT("FormPrev"), IE_Pressed, this, &AFCPlayerController::OnFormPrev);
	InputComponent->BindAction(TEXT("Motion"), IE_Pressed, this, &AFCPlayerController::OnMotion);
	InputComponent->BindAction(TEXT("Boids"), IE_Pressed, this, &AFCPlayerController::OnBoids);
	InputComponent->BindAction(TEXT("Arena"), IE_Pressed, this, &AFCPlayerController::OnArena);
	InputComponent->BindAction(TEXT("JoinBlue"), IE_Pressed, this, &AFCPlayerController::OnJoinBlue);
	InputComponent->BindAction(TEXT("JoinRed"), IE_Pressed, this, &AFCPlayerController::OnJoinRed);
	InputComponent->BindAction(TEXT("Pilot"), IE_Pressed, this, &AFCPlayerController::OnPilot);
	InputComponent->BindAction(TEXT("LeavePilot"), IE_Pressed, this, &AFCPlayerController::OnLeave);
	InputComponent->BindAction(TEXT("ShowFleet"), IE_Pressed, this, &AFCPlayerController::OnShow);
	InputComponent->BindAction(TEXT("Walk"), IE_Pressed, this, &AFCPlayerController::OnWalk);
}

void AFCPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	AFCWorld* W = Sim();
	if (!W) return;
	const float Dt = FMath::Min(DeltaTime, 0.1f);
	if (W->Controlled >= 0)
	{
		FRotator Look = FRotator::ZeroRotator;
		if (bLookHeld)
		{
			float X = 0.f, Y = 0.f;
			GetInputMouseDelta(X, Y);
			Look.Yaw = X * 1.6f;
			Look.Pitch = Y * 1.6f;
		}
		W->ApplyPilotInput(MoveInput, Look, bBoost, Dt);
	}
	ApplyCamera(Dt);
	SyncWalker(Dt);
	MoveInput.X = 0.f;
	MoveInput.Y = 0.f;
}

void AFCPlayerController::ApplyCamera(float Dt)
{
	AFCWorld* W = Sim();
	AFCPawn* CamPawn = GetPawn<AFCPawn>();
	if (!W || !CamPawn) return;

	FVector Focus = W->FleetCentroid();
	if (W->Drones.IsValidIndex(W->Selected)) Focus = W->Drones[W->Selected].Position;
	if (W->CameraMode == EFCCamera::Action) Focus = W->ActionFocus();
	if (W->CameraMode == EFCCamera::Survivor)
	{
		const int32 S = W->LongestSurvivor();
		if (W->Drones.IsValidIndex(S)) Focus = W->Drones[S].Position;
	}

	FVector Desired = Focus + FVector(-OrbitDistance, 0.f, OrbitDistance * 0.45f);
	FRotator DesiredRot = (-(Desired - Focus)).Rotation();

	if (W->Controlled >= 0 && W->Drones.IsValidIndex(W->Controlled) &&
		(W->CameraMode == EFCCamera::FPV || W->CameraMode == EFCCamera::Shoulder || W->CameraMode == EFCCamera::Mounted || W->CameraMode == EFCCamera::Follow))
	{
		const FFCDroneState& S = W->Drones[W->Controlled];
		const FVector Fwd = S.Rotation.Vector();
		const FVector Up = FVector::UpVector;
		if (W->CameraMode == EFCCamera::FPV)
		{
			Desired = S.Position + Fwd * 30.f + Up * 12.f;
			DesiredRot = S.Rotation;
		}
		else if (W->CameraMode == EFCCamera::Mounted)
		{
			Desired = S.Position + Up * 80.f - Fwd * 40.f;
			DesiredRot = S.Rotation;
		}
		else
		{
			Desired = S.Position - Fwd * 220.f + Up * 90.f;
			DesiredRot = (Focus - Desired).Rotation();
		}
	}
	else
	{
		switch (W->CameraMode)
		{
		case EFCCamera::Top:
			Desired = Focus + FVector(0.f, 0.f, OrbitDistance);
			DesiredRot = FRotator(-89.f, OrbitYaw, 0.f);
			break;
		case EFCCamera::Front:
			Desired = Focus + FVector(OrbitDistance, 0.f, 800.f);
			DesiredRot = (Focus - Desired).Rotation();
			break;
		case EFCCamera::Ground:
			if (const AFCGameMode* GM = GetWorld()->GetAuthGameMode<AFCGameMode>())
			{
				if (AFCOperator* Op = GM->PlayerWalker())
				{
					Desired = Op->CameraLocation();
					DesiredRot = Op->CameraRotation();
					break;
				}
			}
			Desired = FVector(Focus.X, Focus.Y, 170.f) - FVector(400.f, 0.f, 0.f);
			DesiredRot = (Focus - Desired).Rotation();
			break;
		case EFCCamera::Cinematic:
			OrbitYaw += Dt * 8.f;
			Desired = Focus + FRotator(0.f, OrbitYaw, 0.f).RotateVector(FVector(-OrbitDistance * 0.8f, 0.f, OrbitDistance * 0.35f));
			DesiredRot = (Focus - Desired).Rotation();
			break;
		default:
			Desired = Focus + FRotator(0.f, OrbitYaw, 0.f).RotateVector(FVector(-OrbitDistance, 0.f, 0.f));
			Desired.Z += FMath::Abs(FMath::Sin(FMath::DegreesToRadians(OrbitPitch))) * OrbitDistance * 0.9f + 600.f;
			DesiredRot = (Focus - Desired).Rotation();
			break;
		}
	}

	CamPawn->SetActorLocation(FMath::VInterpTo(CamPawn->GetActorLocation(), Desired, Dt, 6.f));
	CamPawn->SetActorRotation(FMath::RInterpTo(CamPawn->GetActorRotation(), DesiredRot, Dt, 8.f));
	if (CamPawn->Boom)
	{
		CamPawn->Boom->TargetArmLength = 0.f;
		CamPawn->Boom->SetRelativeRotation(FRotator::ZeroRotator);
	}
}

void AFCPlayerController::OnMoveForward(float V)
{
	AFCWorld* W = Sim();
	if (!W || FMath::Abs(V) < 0.01f) return;
	if (W->Controlled >= 0 || W->CameraMode == EFCCamera::Ground) MoveInput.X = V;
	else
	{
		const FRotator Yaw(0.f, OrbitYaw, 0.f);
		W->Config.Origin += Yaw.RotateVector(FVector(V * 80.f, 0.f, 0.f));
	}
}
void AFCPlayerController::OnMoveRight(float V)
{
	AFCWorld* W = Sim();
	if (!W || FMath::Abs(V) < 0.01f) return;
	if (W->Controlled >= 0 || W->CameraMode == EFCCamera::Ground) MoveInput.Y = V;
	else
	{
		const FRotator Yaw(0.f, OrbitYaw, 0.f);
		W->Config.Origin += Yaw.RotateVector(FVector(0.f, V * 80.f, 0.f));
	}
}
void AFCPlayerController::OnMoveUp(float V)
{
	MoveInput.Z = V;
}
void AFCPlayerController::OnLookX(float V)
{
	AFCWorld* W = Sim();
	if (!bLookHeld || !W || W->Controlled >= 0) return;
	if (W->CameraMode == EFCCamera::Ground)
	{
		if (AFCGameMode* GM = GetWorld()->GetAuthGameMode<AFCGameMode>())
		{
			if (AFCOperator* Op = GM->PlayerWalker()) Op->Yaw += V * 1.8f;
		}
		return;
	}
	OrbitYaw += V;
}
void AFCPlayerController::OnLookY(float V)
{
	if (bLookHeld && Sim() && Sim()->Controlled < 0) OrbitPitch = FMath::Clamp(OrbitPitch + V, -80.f, -8.f);
}
void AFCPlayerController::OnZoom(float V)
{
	if (FMath::Abs(V) > 0.01f) OrbitDistance = FMath::Clamp(OrbitDistance - V * 420.f, 400.f, 50000.f);
}
void AFCPlayerController::OnLookPressed()
{
	bLookHeld = true;
}
void AFCPlayerController::OnLookReleased() { bLookHeld = false; }
void AFCPlayerController::OnFire()
{
	AFCWorld* W = Sim();
	if (!W) return;
	if (AFCHUD* Hud = Cast<AFCHUD>(GetHUD()))
	{
		float MX = 0.f, MY = 0.f;
		if (GetMousePosition(MX, MY) && Hud->TryClick(MX, MY)) return;
	}
	if (W->Controlled >= 0)
	{
		W->FireControlled();
		return;
	}
	FHitResult Hit;
	if (GetHitResultUnderCursor(ECC_Visibility, true, Hit))
	{
		W->SelectNearest(Hit.ImpactPoint);
	}
}
void AFCPlayerController::OnReload() { if (AFCWorld* W = Sim()) W->ReloadControlled(); }
void AFCPlayerController::OnGuard() { if (AFCWorld* W = Sim()) W->UseAbility(EFCAbility::Guard); }
void AFCPlayerController::OnDodge() { if (AFCWorld* W = Sim()) W->UseAbility(EFCAbility::Dodge); }
void AFCPlayerController::OnBoostPressed() { bBoost = true; }
void AFCPlayerController::OnBoostReleased() { bBoost = false; }
void AFCPlayerController::OnPayload() { if (AFCWorld* W = Sim()) W->DropPayload(); }
void AFCPlayerController::OnLaunch() { if (AFCWorld* W = Sim()) W->LaunchAll(); }
void AFCPlayerController::OnRecall() { if (AFCWorld* W = Sim()) W->RecallAll(); }
void AFCPlayerController::OnPause() { if (AFCWorld* W = Sim()) W->TogglePause(); }
void AFCPlayerController::OnNext() { if (AFCWorld* W = Sim()) W->SelectNext(1); }
void AFCPlayerController::OnPrev() { if (AFCWorld* W = Sim()) W->SelectNext(-1); }
void AFCPlayerController::OnCamera() { if (AFCWorld* W = Sim()) W->CycleCamera(); }
void AFCPlayerController::OnHideHud() { if (AFCWorld* W = Sim()) W->bHudHidden = !W->bHudHidden; }
void AFCPlayerController::OnFormNext() { if (AFCWorld* W = Sim()) W->CycleFormation(1); }
void AFCPlayerController::OnFormPrev() { if (AFCWorld* W = Sim()) W->CycleFormation(-1); }
void AFCPlayerController::OnMotion() { if (AFCWorld* W = Sim()) W->CycleMotion(); }
void AFCPlayerController::OnBoids() { if (AFCWorld* W = Sim()) W->ToggleBoids(); }
void AFCPlayerController::OnArena() { if (AFCWorld* W = Sim()) W->StartArena(12); }
void AFCPlayerController::OnJoinBlue() { if (AFCWorld* W = Sim()) { W->JoinTeam(0); W->CameraMode = EFCCamera::Shoulder; } }
void AFCPlayerController::OnJoinRed() { if (AFCWorld* W = Sim()) { W->JoinTeam(1); W->CameraMode = EFCCamera::Shoulder; } }
void AFCPlayerController::OnPilot() { if (AFCWorld* W = Sim()) { W->PilotSelected(); W->CameraMode = EFCCamera::Shoulder; } }
void AFCPlayerController::OnLeave()
{
	AFCWorld* W = Sim();
	if (!W) return;
	if (W->Controlled >= 0) W->LeavePilot();
	else if (W->CameraMode == EFCCamera::Ground) W->CameraMode = EFCCamera::Orbit;
}
void AFCPlayerController::OnShow() { if (AFCWorld* W = Sim()) W->ReturnToShow(); }
void AFCPlayerController::OnWalk()
{
	AFCWorld* W = Sim();
	if (!W) return;
	W->LeavePilot();
	W->CameraMode = EFCCamera::Ground;
	W->LastEvent = TEXT("Walk the GRIDRUNNER compound");
	if (AFCGameMode* GM = GetWorld()->GetAuthGameMode<AFCGameMode>())
	{
		if (AFCOperator* Op = GM->PlayerWalker()) Op->SetControlled(true);
	}
}
void AFCPlayerController::SyncWalker(float Dt)
{
	(void)Dt;
	AFCWorld* W = Sim();
	AFCGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AFCGameMode>() : nullptr;
	if (!W || !GM) return;
	const bool bWalk = W->CameraMode == EFCCamera::Ground && W->Controlled < 0;
	for (int32 I = 0; I < GM->Walkers.Num(); ++I)
	{
		if (AFCOperator* Op = GM->Walkers[I].Get())
		{
			Op->SetControlled(bWalk && I == 0);
			if (bWalk && I == 0) Op->SetPlayerMove(MoveInput, 0.f);
		}
	}
}
