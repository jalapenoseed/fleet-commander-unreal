#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FCGameMode.generated.h"

class AFCWorld;
class AFCOperator;

UCLASS()
class FLEETCOMMANDER_API AFCGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFCGameMode();
	virtual void StartPlay() override;

	UPROPERTY() TObjectPtr<AFCWorld> WorldSim;
	UPROPERTY() TArray<TObjectPtr<AFCOperator>> Walkers;

	AFCOperator* PlayerWalker() const { return Walkers.Num() > 0 ? Walkers[0].Get() : nullptr; }

	void BuildArena();
	void SpawnOperators();
};
