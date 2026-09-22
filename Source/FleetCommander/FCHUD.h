#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "FCHUD.generated.h"

UCLASS()
class FLEETCOMMANDER_API AFCHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
	void Line(float X, float Y, const FString& Text, const FLinearColor& Color, float Scale = 1.f);
};
