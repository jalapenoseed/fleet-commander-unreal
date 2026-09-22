#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "FCPawn.generated.h"

class UCameraComponent;
class USpringArmComponent;

UCLASS()
class FLEETCOMMANDER_API AFCPawn : public APawn
{
	GENERATED_BODY()

public:
	AFCPawn();

	UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> Root;
	UPROPERTY(VisibleAnywhere) TObjectPtr<USpringArmComponent> Boom;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UCameraComponent> Camera;
};
