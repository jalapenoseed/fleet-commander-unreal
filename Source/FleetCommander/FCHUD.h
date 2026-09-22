#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Math/Box2D.h"
#include "FCHUD.generated.h"

UENUM()
enum class EFCCommandPage : uint8 { Fleet, Arena, Director, Cameras, Settings, Help };

UCLASS()
class FLEETCOMMANDER_API AFCHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;
	bool TryClick(float MouseX, float MouseY);
	EFCCommandPage Page = EFCCommandPage::Fleet;

private:
	struct FFCHit
	{
		FBox2D Box;
		FName Id;
	};
	TArray<FFCHit> Hits;
	int32 Hover = -1;

	void Line(float X, float Y, const FString& Text, const FLinearColor& Color, float Scale = 1.f);
	void Panel(float X, float Y, float W, float H, const FLinearColor& Color);
	bool Btn(const FName Id, float X, float Y, float W, float H, const FString& Label, bool bPrimary = false);
	void Handle(FName Id);
};
