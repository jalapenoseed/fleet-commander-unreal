#include "FCGameMode.h"
#include "FCWorld.h"
#include "FCPawn.h"
#include "FCPlayerController.h"
#include "FCHUD.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

AFCGameMode::AFCGameMode()
{
	DefaultPawnClass = AFCPawn::StaticClass();
	PlayerControllerClass = AFCPlayerController::StaticClass();
	HUDClass = AFCHUD::StaticClass();
}

void AFCGameMode::StartPlay()
{
	Super::StartPlay();
	BuildArena();
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	WorldSim = GetWorld()->SpawnActor<AFCWorld>(AFCWorld::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);
}

void AFCGameMode::BuildArena()
{
	UWorld* W = GetWorld();
	if (!W) return;

	FActorSpawnParameters P;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ADirectionalLight* Sun = W->SpawnActor<ADirectionalLight>(FVector(0.f, 0.f, 8000.f), FRotator(-48.f, 35.f, 0.f), P);
	if (Sun && Sun->GetLightComponent())
	{
		Sun->GetLightComponent()->SetIntensity(8.f);
		Sun->GetLightComponent()->SetLightColor(FLinearColor(1.f, 0.92f, 0.82f));
		Sun->GetLightComponent()->SetCastShadows(true);
	}

	ASkyLight* Sky = W->SpawnActor<ASkyLight>(FVector::ZeroVector, FRotator::ZeroRotator, P);
	if (Sky && Sky->GetLightComponent())
	{
		Sky->GetLightComponent()->SetIntensity(1.4f);
		Sky->GetLightComponent()->SetLowerHemisphereColor(FLinearColor(0.02f, 0.03f, 0.05f));
		Sky->GetLightComponent()->bRealTimeCapture = true;
	}

	AExponentialHeightFog* Fog = W->SpawnActor<AExponentialHeightFog>(FVector::ZeroVector, FRotator::ZeroRotator, P);
	if (Fog && Fog->GetComponent())
	{
		Fog->GetComponent()->SetFogDensity(0.012f);
		Fog->GetComponent()->SetFogHeightFalloff(0.12f);
		Fog->GetComponent()->SetFogInscatteringColor(FLinearColor(0.05f, 0.07f, 0.10f));
		Fog->GetComponent()->SetVolumetricFog(true);
	}

	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	auto SpawnBox = [&](const FVector& Loc, const FVector& Scale, const FLinearColor& Color)
	{
		AStaticMeshActor* A = W->SpawnActor<AStaticMeshActor>(Loc, FRotator::ZeroRotator, P);
		if (!A) return;
		UStaticMeshComponent* Mesh = A->GetStaticMeshComponent();
		Mesh->SetStaticMesh(Cube);
		Mesh->SetWorldScale3D(Scale);
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		if (UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineDebugMaterials/DebugMeshMaterial.DebugMeshMaterial")))
		{
			if (UMaterialInstanceDynamic* Mid = UMaterialInstanceDynamic::Create(Base, A))
			{
				Mid->SetVectorParameterValue(TEXT("Color"), Color);
				Mesh->SetMaterial(0, Mid);
			}
		}
	};

	// Pitch
	SpawnBox(FVector(0.f, 0.f, 0.f), FVector(220.f, 220.f, 0.4f), FLinearColor(0.07f, 0.09f, 0.06f));
	// Center stripe
	SpawnBox(FVector(0.f, 0.f, 22.f), FVector(220.f, 0.6f, 0.05f), FLinearColor(0.75f, 0.78f, 0.70f));
	// Stadium bowls
	SpawnBox(FVector(0.f, 12000.f, 1800.f), FVector(180.f, 12.f, 36.f), FLinearColor(0.12f, 0.13f, 0.14f));
	SpawnBox(FVector(0.f, -12000.f, 1800.f), FVector(180.f, 12.f, 36.f), FLinearColor(0.12f, 0.13f, 0.14f));
	SpawnBox(FVector(12000.f, 0.f, 1400.f), FVector(12.f, 160.f, 28.f), FLinearColor(0.11f, 0.12f, 0.13f));
	SpawnBox(FVector(-12000.f, 0.f, 1400.f), FVector(12.f, 160.f, 28.f), FLinearColor(0.11f, 0.12f, 0.13f));
	// Floodlight towers
	SpawnBox(FVector(9000.f, 9000.f, 2500.f), FVector(1.2f, 1.2f, 50.f), FLinearColor(0.2f, 0.2f, 0.18f));
	SpawnBox(FVector(-9000.f, 9000.f, 2500.f), FVector(1.2f, 1.2f, 50.f), FLinearColor(0.2f, 0.2f, 0.18f));
	SpawnBox(FVector(9000.f, -9000.f, 2500.f), FVector(1.2f, 1.2f, 50.f), FLinearColor(0.2f, 0.2f, 0.18f));
	SpawnBox(FVector(-9000.f, -9000.f, 2500.f), FVector(1.2f, 1.2f, 50.f), FLinearColor(0.2f, 0.2f, 0.18f));
	// Team pads
	SpawnBox(FVector(-4500.f, 0.f, 30.f), FVector(18.f, 18.f, 0.3f), FLinearColor(0.1f, 0.45f, 0.5f));
	SpawnBox(FVector(4500.f, 0.f, 30.f), FVector(18.f, 18.f, 0.3f), FLinearColor(0.5f, 0.22f, 0.1f));
	// Control station
	SpawnBox(FVector(0.f, -4000.f, 250.f), FVector(8.f, 6.f, 5.f), FLinearColor(0.16f, 0.17f, 0.18f));
}
