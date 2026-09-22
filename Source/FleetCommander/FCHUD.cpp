#include "FCHUD.h"
#include "FCGameMode.h"
#include "FCWorld.h"
#include "FCCatalog.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Engine/World.h"

void AFCHUD::Line(float X, float Y, const FString& Text, const FLinearColor& Color, float Scale)
{
	UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;
	DrawText(Text, Color, X, Y, Font, Scale, false);
}

void AFCHUD::DrawHUD()
{
	Super::DrawHUD();
	AFCWorld* W = nullptr;
	if (const AFCGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AFCGameMode>() : nullptr) W = GM->WorldSim;
	if (!W || W->bHudHidden) return;

	const float M = 28.f;
	const FLinearColor Ink(0.93f, 0.94f, 0.95f);
	const FLinearColor Dim(0.62f, 0.64f, 0.67f);
	const FLinearColor Cyan(0.49f, 0.78f, 0.83f);
	const FLinearColor Orange(0.82f, 0.48f, 0.29f);

	DrawRect(FLinearColor(0.04f, 0.045f, 0.05f, 0.72f), M - 12.f, M - 12.f, 520.f, 168.f);
	Line(M, M, TEXT("FLEET COMMANDER  1.3"), Ink, 1.15f);
	Line(M, M + 28.f, TEXT("UNREAL ENGINE 5.8  ·  UNITY PORT"), Dim, 0.85f);
	Line(M, M + 56.f, FString::Printf(TEXT("Aircraft  %d    Formation  %s    Camera  %s"),
		W->Count(), FFCCatalog::FormationName(W->Config.Formation), FFCCatalog::CameraName(W->CameraMode)), Ink, 0.9f);
	Line(M, M + 80.f, FString::Printf(TEXT("%s%s"), W->bPaused ? TEXT("PAUSED  ·  ") : TEXT(""), *W->LastEvent), Dim, 0.85f);

	if (W->bBattle)
	{
		DrawRect(FLinearColor(0.04f, 0.05f, 0.06f, 0.7f), Canvas->SizeX - 360.f, M - 12.f, 332.f, 120.f);
		Line(Canvas->SizeX - 340.f, M, FString::Printf(TEXT("BLUE  %d  alive   %d  kills"), W->BlueAlive, W->BlueKills), Cyan, 0.95f);
		Line(Canvas->SizeX - 340.f, M + 28.f, FString::Printf(TEXT("RED   %d  alive   %d  kills"), W->RedAlive, W->RedKills), Orange, 0.95f);
		Line(Canvas->SizeX - 340.f, M + 56.f, FString::Printf(TEXT("Series  B %d   R %d   D %d    %.0fs"), W->BlueWins, W->RedWins, W->Draws, W->RoundTime), Dim, 0.85f);
	}

	if (const FFCDroneState* S = W->GetSelected())
	{
		DrawRect(FLinearColor(0.04f, 0.045f, 0.05f, 0.7f), M - 12.f, Canvas->SizeY - 188.f, 560.f, 164.f);
		Line(M, Canvas->SizeY - 176.f, FString::Printf(TEXT("SEL  %s-%d    %s    %s"),
			FFCCatalog::FrameName(S->Frame), S->Id, FFCCatalog::WeaponName(S->Weapon), *S->AIState.ToString()), Ink, 0.95f);
		Line(M, Canvas->SizeY - 148.f, FString::Printf(TEXT("Hull %.0f    Batt %.0f%%    Ammo %d/%d    Spd %.0f"),
			S->Health, S->Battery * 100.f, S->Ammo, S->ReserveAmmo, S->Velocity.Size() / 100.f), Dim, 0.9f);
		Line(M, Canvas->SizeY - 120.f, FString::Printf(TEXT("Phase %d    Payloads %d    Kills %d    %s"),
			static_cast<int32>(S->Phase), S->Payloads, S->Kills, W->Controlled == S->Id ? TEXT("PILOT") : TEXT("AI")), Dim, 0.85f);
	}

	Line(M, Canvas->SizeY - 52.f, TEXT("L launch   K recall   [ ] formation   M motion   G arena   1 blue  2 red   F pilot   C camera   H hud   P pause"), Dim, 0.75f);
	Line(M, Canvas->SizeY - 32.f, TEXT("Pilot: WASD fly  Space/Ctrl altitude  Shift boost  LMB fire  R reload  RMB guard  E dodge  Q payload  Esc AI"), Dim, 0.75f);

	if (!W->Banner.IsEmpty())
	{
		const float BW = 640.f;
		const float BX = (Canvas->SizeX - BW) * 0.5f;
		const float BY = Canvas->SizeY * 0.38f;
		DrawRect(FLinearColor(0.02f, 0.02f, 0.025f, 0.82f), BX, BY, BW, 96.f);
		Line(BX + 40.f, BY + 28.f, W->Banner, Ink, 1.6f);
		Line(BX + 40.f, BY + 62.f, TEXT("G rematch    T return to show"), Dim, 0.9f);
	}

	if (W->Controlled >= 0)
	{
		const float CX = Canvas->SizeX * 0.5f;
		const float CY = Canvas->SizeY * 0.5f;
		DrawRect(Ink, CX - 10.f, CY - 1.f, 20.f, 2.f);
		DrawRect(Ink, CX - 1.f, CY - 10.f, 2.f, 20.f);
	}
}
