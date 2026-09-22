#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FCGameMode.generated.h"

class AFCWorld;

UCLASS()
class FLEETCOMMANDER_API AFCGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFCGameMode();
	virtual void StartPlay() override;

	UPROPERTY() TObjectPtr<AFCWorld> WorldSim;

	void BuildArena();
};
