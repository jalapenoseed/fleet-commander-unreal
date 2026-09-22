#include "FCHUD.h"
#include "FCGameMode.h"
#include "FCWorld.h"
#include "FCCatalog.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Engine/World.h"

namespace
{
	const FLinearColor Ink(0.85f, 0.91f, 0.93f);
	const FLinearColor Dim(0.62f, 0.73f, 0.76f);
	const FLinearColor Cyan(0.29f, 0.76f, 0.71f);
	const FLinearColor PanelBg(0.03f, 0.07f, 0.10f, 0.94f);
	const FLinearColor BtnBg(0.09f, 0.17f, 0.21f, 0.95f);
	const FLinearColor BtnHi(0.23f, 0.76f, 0.71f, 1.f);
	const FLinearColor BtnInk(0.03f, 0.10f, 0.11f);
}

void AFCHUD::Line(float X, float Y, const FString& Text, const FLinearColor& Color, float Scale)
{
	UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;
	DrawText(Text, Color, X, Y, Font, Scale, false);
}

void AFCHUD::Panel(float X, float Y, float W, float H, const FLinearColor& Color)
{
	DrawRect(Color, X, Y, W, H);
}

bool AFCHUD::Btn(const FName Id, float X, float Y, float W, float H, const FString& Label, bool bPrimary)
{
	const FBox2D Box(FVector2D(X, Y), FVector2D(X + W, Y + H));
	const int32 Index = Hits.Num();
	Hits.Add({ Box, Id });
	const bool bHot = Hover == Index;
	Panel(X, Y, W, H, bPrimary || bHot ? BtnHi : BtnBg);
	Line(X + 8.f, Y + FMath::Max(4.f, (H - 14.f) * 0.5f), Label, bPrimary || bHot ? BtnInk : Ink, 0.85f);
	return false;
}

void AFCHUD::Handle(FName Id)
{
	AFCWorld* W = nullptr;
	AFCGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AFCGameMode>() : nullptr;
	if (GM) W = GM->WorldSim;
	if (!W) return;
	const FString S = Id.ToString();
	if (S == TEXT("nav_Fleet")) Page = EFCCommandPage::Fleet;
	else if (S == TEXT("nav_Arena")) Page = EFCCommandPage::Arena;
	else if (S == TEXT("nav_Director")) Page = EFCCommandPage::Director;
	else if (S == TEXT("nav_Cameras")) Page = EFCCommandPage::Cameras;
	else if (S == TEXT("nav_Settings")) Page = EFCCommandPage::Settings;
	else if (S == TEXT("nav_Help")) Page = EFCCommandPage::Help;
	else if (S == TEXT("act_Launch")) W->LaunchAll();
	else if (S == TEXT("act_Pause")) W->TogglePause();
	else if (S == TEXT("act_Land")) W->RecallAll();
	else if (S == TEXT("act_Hide")) W->bHudHidden = !W->bHudHidden;
	else if (S == TEXT("act_Show")) W->bHudHidden = false;
	else if (S == TEXT("flt_32")) W->RebuildShow(32);
	else if (S == TEXT("flt_96")) { W->RebuildShow(96); W->LaunchAll(); }
	else if (S == TEXT("flt_256")) { W->RebuildShow(256); W->LaunchAll(); }
	else if (S == TEXT("flt_FormN")) W->CycleFormation(1);
	else if (S == TEXT("flt_FormP")) W->CycleFormation(-1);
	else if (S == TEXT("flt_Motion")) W->CycleMotion();
	else if (S == TEXT("flt_Boids")) W->ToggleBoids();
	else if (S == TEXT("arn_Start")) { W->StartArena(12); Page = EFCCommandPage::Arena; }
	else if (S == TEXT("arn_Blue")) W->JoinTeam(0);
	else if (S == TEXT("arn_Red")) W->JoinTeam(1);
	else if (S == TEXT("arn_Pilot")) W->PilotSelected();
	else if (S == TEXT("arn_Leave")) W->LeavePilot();
	else if (S == TEXT("arn_Show")) W->ReturnToShow();
	else if (S == TEXT("dir_Scenery")) W->CycleScenery();
	else if (S == TEXT("cam_Orbit")) W->CameraMode = EFCCamera::Orbit;
	else if (S == TEXT("cam_Top")) W->CameraMode = EFCCamera::Top;
	else if (S == TEXT("cam_Follow")) W->CameraMode = EFCCamera::Follow;
	else if (S == TEXT("cam_FPV")) W->CameraMode = EFCCamera::FPV;
	else if (S == TEXT("cam_Shoulder")) W->CameraMode = EFCCamera::Shoulder;
	else if (S == TEXT("cam_Walk")) { W->CameraMode = EFCCamera::Ground; W->LastEvent = TEXT("Walk the GRIDRUNNER compound"); }
	else if (S == TEXT("cam_Cine")) W->CameraMode = EFCCamera::Cinematic;
	else if (S == TEXT("cam_Next")) W->CycleCamera();
}

bool AFCHUD::TryClick(float MouseX, float MouseY)
{
	const FVector2D M(MouseX, MouseY);
	for (const FFCHit& H : Hits)
	{
		if (H.Box.IsInside(M))
		{
			Handle(H.Id);
			return true;
		}
	}
	return false;
}

void AFCHUD::DrawHUD()
{
	Super::DrawHUD();
	Hits.Reset();
	AFCWorld* W = nullptr;
	const AFCGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AFCGameMode>() : nullptr;
	if (GM) W = GM->WorldSim;
	if (!W) return;

	float MX = -1.f, MY = -1.f;
	if (PlayerOwner) PlayerOwner->GetMousePosition(MX, MY);
	const FVector2D Mouse(MX, MY);

	if (W->bHudHidden)
	{
		Btn(TEXT("act_Show"), Canvas->SizeX - 180.f, Canvas->SizeY - 46.f, 160.f, 28.f, TEXT("SHOW MENUS  [H]"), true);
		if (Mouse.X >= 0.f)
		{
			for (int32 I = 0; I < Hits.Num(); ++I) if (Hits[I].Box.IsInside(Mouse)) Hover = I;
		}
		return;
	}

	const float SX = Canvas->SizeX;
	const float SY = Canvas->SizeY;
	Panel(20.f, 16.f, SX - 40.f, 64.f, PanelBg);
	DrawRect(Cyan, 20.f, 16.f, SX - 40.f, 2.f);
	Line(36.f, 24.f, TEXT("FLEET COMMANDER"), Ink, 1.2f);
	Line(36.f, 46.f, TEXT("UNITY 1.3  /  SWARM EXPERIMENT WORKSHOP"), Dim, 0.8f);
	Line(320.f, 30.f, FString::Printf(TEXT("%d AIRCRAFT    %s    %s%s"),
		W->Count(), FFCCatalog::FormationName(W->Config.Formation),
		FFCCatalog::CameraName(W->CameraMode), W->bPaused ? TEXT("    PAUSED") : TEXT("")), Cyan, 0.9f);
	Btn(TEXT("act_Launch"), SX - 520.f, 28.f, 110.f, 32.f, TEXT("LAUNCH"), true);
	Btn(TEXT("act_Pause"), SX - 400.f, 28.f, 90.f, 32.f, W->bPaused ? TEXT("RESUME") : TEXT("PAUSE"));
	Btn(TEXT("act_Land"), SX - 300.f, 28.f, 90.f, 32.f, TEXT("LAND"));
	Btn(TEXT("act_Hide"), SX - 200.f, 28.f, 160.f, 32.f, TEXT("HIDE MENUS [H]"));

	Panel(20.f, 88.f, SX - 40.f, 36.f, FLinearColor(0.03f, 0.06f, 0.09f, 0.88f));
	const TCHAR* Nav[] = { TEXT("Fleet"), TEXT("Arena"), TEXT("Director"), TEXT("Cameras"), TEXT("Settings"), TEXT("Help") };
	const EFCCommandPage NavPage[] = { EFCCommandPage::Fleet, EFCCommandPage::Arena, EFCCommandPage::Director, EFCCommandPage::Cameras, EFCCommandPage::Settings, EFCCommandPage::Help };
	for (int32 I = 0; I < 6; ++I)
	{
		const bool bSel = Page == NavPage[I];
		Btn(*FString::Printf(TEXT("nav_%s"), Nav[I]), 28.f + I * 92.f, 92.f, 86.f, 28.f, Nav[I], bSel);
	}

	const bool bPilot = W->Controlled >= 0;
	if (!bPilot)
	{
		Panel(20.f, 132.f, 340.f, SY - 210.f, PanelBg);
		Line(36.f, 144.f, TEXT("COMMAND CENTER"), Dim, 0.75f);
		const TCHAR* Title = TEXT("FLEET");
		if (Page == EFCCommandPage::Arena) Title = TEXT("ARENA");
		else if (Page == EFCCommandPage::Director) Title = TEXT("DIRECTOR");
		else if (Page == EFCCommandPage::Cameras) Title = TEXT("CAMERAS");
		else if (Page == EFCCommandPage::Settings) Title = TEXT("SETTINGS");
		else if (Page == EFCCommandPage::Help) Title = TEXT("HELP");
		Line(36.f, 164.f, Title, Ink, 1.35f);

		float Y = 210.f;
		auto Note = [&](const TCHAR* T) { Line(36.f, Y, T, Dim, 0.8f); Y += 22.f; };
		switch (Page)
		{
		case EFCCommandPage::Fleet:
			Note(TEXT("Shape the show fleet and launch from the pads."));
			Btn(TEXT("flt_32"), 36.f, Y, 90.f, 30.f, TEXT("32")); Btn(TEXT("flt_96"), 132.f, Y, 90.f, 30.f, TEXT("96"), true); Btn(TEXT("flt_256"), 228.f, Y, 90.f, 30.f, TEXT("256")); Y += 42.f;
			Note(TEXT("BUILD FLEET applies count then keeps pads."));
			Btn(TEXT("act_Launch"), 36.f, Y, 140.f, 32.f, TEXT("LAUNCH"), true); Btn(TEXT("act_Land"), 184.f, Y, 140.f, 32.f, TEXT("LAND")); Y += 46.f;
			Line(36.f, Y, TEXT("FORMATION & MOTION"), Cyan, 0.9f); Y += 24.f;
			Btn(TEXT("flt_FormP"), 36.f, Y, 70.f, 30.f, TEXT("[")); Btn(TEXT("flt_FormN"), 112.f, Y, 70.f, 30.f, TEXT("]")); Y += 38.f;
			Line(36.f, Y, FFCCatalog::FormationName(W->Config.Formation), Ink, 0.95f); Y += 24.f;
			Btn(TEXT("flt_Motion"), 36.f, Y, 140.f, 30.f, TEXT("MOTION")); Btn(TEXT("flt_Boids"), 184.f, Y, 140.f, 30.f, W->Config.bBoids ? TEXT("BOIDS ON") : TEXT("BOIDS OFF"));
			break;
		case EFCCommandPage::Arena:
			Note(TEXT("Arcade drone arena. Cyan vs orange."));
			Line(36.f, Y, FString::Printf(TEXT("BLUE %d alive  %d kills"), W->BlueAlive, W->BlueKills), Cyan, 0.9f); Y += 20.f;
			Line(36.f, Y, FString::Printf(TEXT("RED  %d alive  %d kills"), W->RedAlive, W->RedKills), FLinearColor(0.92f, 0.42f, 0.42f), 0.9f); Y += 28.f;
			Btn(TEXT("arn_Start"), 36.f, Y, 288.f, 34.f, W->bBattle ? TEXT("APPLY & REMATCH") : TEXT("START 12v12"), true); Y += 46.f;
			Btn(TEXT("arn_Blue"), 36.f, Y, 140.f, 32.f, TEXT("JOIN BLUE")); Btn(TEXT("arn_Red"), 184.f, Y, 140.f, 32.f, TEXT("JOIN RED")); Y += 40.f;
			Btn(TEXT("arn_Pilot"), 36.f, Y, 288.f, 32.f, TEXT("PILOT SELECTED")); Y += 40.f;
			Btn(TEXT("arn_Leave"), 36.f, Y, 140.f, 30.f, TEXT("LEAVE AI")); Btn(TEXT("arn_Show"), 184.f, Y, 140.f, 30.f, TEXT("SHOW FLEET"));
			break;
		case EFCCommandPage::Director:
			Note(TEXT("Scenic compound and camera direction."));
			Btn(TEXT("dir_Scenery"), 36.f, Y, 288.f, 32.f, TEXT("CYCLE SCENERY")); Y += 42.f;
			Btn(TEXT("cam_Cine"), 36.f, Y, 288.f, 32.f, TEXT("CINEMATIC")); Y += 42.f;
			Btn(TEXT("cam_Walk"), 36.f, Y, 288.f, 32.f, TEXT("WALK THE COMPOUND"), true); Y += 40.f;
			Note(TEXT("V also puts you in the field operator."));
			break;
		case EFCCommandPage::Cameras:
			Note(TEXT("Main view plus ground operator walk."));
			Btn(TEXT("cam_Orbit"), 36.f, Y, 140.f, 30.f, TEXT("ORBIT"), W->CameraMode == EFCCamera::Orbit); Btn(TEXT("cam_Top"), 184.f, Y, 140.f, 30.f, TEXT("TOP"), W->CameraMode == EFCCamera::Top); Y += 38.f;
			Btn(TEXT("cam_Follow"), 36.f, Y, 140.f, 30.f, TEXT("FOLLOW"), W->CameraMode == EFCCamera::Follow); Btn(TEXT("cam_FPV"), 184.f, Y, 140.f, 30.f, TEXT("FPV"), W->CameraMode == EFCCamera::FPV); Y += 38.f;
			Btn(TEXT("cam_Shoulder"), 36.f, Y, 140.f, 30.f, TEXT("SHOULDER"), W->CameraMode == EFCCamera::Shoulder); Btn(TEXT("cam_Walk"), 184.f, Y, 140.f, 30.f, TEXT("WALK"), W->CameraMode == EFCCamera::Ground); Y += 38.f;
			Btn(TEXT("cam_Cine"), 36.f, Y, 140.f, 30.f, TEXT("CINEMA")); Btn(TEXT("cam_Next"), 184.f, Y, 140.f, 30.f, TEXT("CYCLE [C]"));
			break;
		case EFCCommandPage::Settings:
			Note(TEXT("Display and HUD. Sim stays 60 Hz."));
			Btn(TEXT("act_Hide"), 36.f, Y, 288.f, 32.f, TEXT("HIDE MENUS")); Y += 42.f;
			Note(TEXT("Low-end: DefaultEngine.ini 75% / Lumen off."));
			Note(TEXT("PBR maps load from FleetAssets/DroneTextures."));
			break;
		default:
			Note(TEXT("L launch  K land  [ ] formation"));
			Note(TEXT("G arena  1 blue  2 red  F pilot"));
			Note(TEXT("C camera  V walk compound"));
			Note(TEXT("Click aircraft to select. H hides UI."));
			Note(TEXT("WASD walk when on the ground."));
			break;
		}
	}

	Panel(SX - 312.f, 132.f, 292.f, 210.f, PanelBg);
	Line(SX - 296.f, 144.f, TEXT("TELEMETRY"), Dim, 0.75f);
	if (const FFCDroneState* S = W->GetSelected())
	{
		Line(SX - 296.f, 168.f, FString::Printf(TEXT("%s-%d   %s"), FFCCatalog::FrameName(S->Frame), S->Id, FFCCatalog::WeaponName(S->Weapon)), Ink, 0.95f);
		Line(SX - 296.f, 192.f, FString::Printf(TEXT("Hull %.0f   Batt %.0f%%"), S->Health, S->Battery * 100.f), Dim, 0.9f);
		Line(SX - 296.f, 214.f, FString::Printf(TEXT("Ammo %d/%d   Spd %.0f m/s"), S->Ammo, S->ReserveAmmo, S->Velocity.Size() / 100.f), Dim, 0.9f);
		Line(SX - 296.f, 236.f, FString::Printf(TEXT("%s    %s"), *S->AIState.ToString(), W->Controlled == S->Id ? TEXT("PILOT") : TEXT("AI")), Cyan, 0.85f);
	}
	else Line(SX - 296.f, 168.f, TEXT("Click an aircraft"), Dim, 0.9f);
	Line(SX - 296.f, 270.f, W->LastEvent, Dim, 0.8f);
	if (GM && GM->Walkers.Num() > 0)
	{
		Line(SX - 296.f, 292.f, FString::Printf(TEXT("Field ops  %d walking"), GM->Walkers.Num()), Cyan, 0.8f);
	}

	Panel(20.f, SY - 58.f, SX - 40.f, 42.f, PanelBg);
	Line(36.f, SY - 44.f, W->CameraMode == EFCCamera::Ground
		? TEXT("WALK  WASD move   mouse look   C cameras   Esc orbit")
		: TEXT("L launch   K land   [ ] formation   G arena   1/2 join   F pilot   V walk   C camera   H menus"), Dim, 0.8f);

	if (!W->Banner.IsEmpty())
	{
		const float BW = 640.f;
		const float BX = (SX - BW) * 0.5f;
		const float BY = SY * 0.36f;
		Panel(BX, BY, BW, 110.f, FLinearColor(0.02f, 0.05f, 0.08f, 0.96f));
		DrawRect(Cyan, BX, BY, BW, 4.f);
		Line(BX + 36.f, BY + 28.f, W->Banner, Ink, 1.7f);
		Line(BX + 36.f, BY + 72.f, TEXT("G rematch     T show fleet"), Dim, 0.9f);
	}

	if (bPilot)
	{
		const float CX = SX * 0.5f;
		const float CY = SY * 0.5f;
		DrawRect(Cyan, CX - 10.f, CY - 1.f, 20.f, 2.f);
		DrawRect(Cyan, CX - 1.f, CY - 10.f, 2.f, 20.f);
	}

	Hover = -1;
	if (Mouse.X >= 0.f)
	{
		for (int32 I = 0; I < Hits.Num(); ++I)
		{
			if (Hits[I].Box.IsInside(Mouse)) Hover = I;
		}
	}
}
