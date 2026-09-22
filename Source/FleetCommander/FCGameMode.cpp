#include "FCGameMode.h"
#include "FCAssetLoader.h"
#include "FCWorld.h"
#include "FCPawn.h"
#include "FCPlayerController.h"
#include "FCHUD.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"

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

	TArray<FFCLoadedProp> Props;
	FFCAssetLoader::LoadCampAndHouses(this, Props);
	TMap<FString, UStaticMesh*> ByName;
	for (const FFCLoadedProp& Prop : Props)
	{
		if (Prop.Mesh) ByName.Add(Prop.Name, Prop.Mesh);
	}
	auto U3 = [](float X, float Y, float Z) { return FVector(Z, X, Y) * 100.f; };
	auto Place = [&](const FString& Name, const FVector& UnityPos, float Yaw, float Scale)
	{
		UStaticMesh** Found = ByName.Find(Name);
		if (!Found || !*Found) return;
		AStaticMeshActor* A = W->SpawnActor<AStaticMeshActor>(U3(UnityPos.X, UnityPos.Y, UnityPos.Z), FRotator(0.f, Yaw, 0.f), P);
		if (!A) return;
		UStaticMeshComponent* Mesh = A->GetStaticMeshComponent();
		Mesh->SetStaticMesh(*Found);
		Mesh->SetWorldScale3D(FVector(Scale));
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Mesh->SetMobility(EComponentMobility::Static);
	};
	static const TCHAR* Houses[] = {
		TEXT("GR05_01_UtilityHouse"), TEXT("GR05_02_RoadsideStore"), TEXT("GR05_03_TexasHouse"),
		TEXT("GR05_04_Maintenance"), TEXT("GR05_05_CreekPump")
	};
	for (int32 I = 0; I < 14; ++I)
	{
		const float X = (I / 2 - 3) * 37.f;
		const float Z = (I % 2 == 0) ? 192.f : 252.f;
		Place(Houses[I % 5], FVector(X, 0.f, Z), (I % 2 == 0) ? 0.f : 180.f, 1.5f);
	}
	const FVector Origin(-115.f, 0.f, 95.f);
	static const TCHAR* Camp[] = {
		TEXT("GR_TarpShelter_02"), TEXT("GR_Workbench_02"), TEXT("GR_FoldingChair_02"), TEXT("GR_CampCot_02"),
		TEXT("GR_SolarArray_02"), TEXT("GR_ChargingStation_02"), TEXT("GR_DroneHardCase_02"), TEXT("GR_FieldRadio_02"),
		TEXT("GR_BatteryCase_02"), TEXT("GR_FirstAidCase_02"), TEXT("GR_SupplyLocker_02"), TEXT("GR_WaterBarrel_02"),
		TEXT("GR_WaterFilter_02"), TEXT("GR_SupplyCrate_02"), TEXT("GR_Toolboard_02"), TEXT("GR_ExtensionReel_02"),
		TEXT("GR_CampStove_02"), TEXT("GR_RainCollector_02"), TEXT("GR_FieldStorageCase_02"), TEXT("GR_BikeRepairStand_02")
	};
	for (int32 I = 0; I < 20; ++I)
	{
		Place(Camp[I], Origin + FVector((I % 5) * 4.f, 0.f, (I / 5) * 5.f), (I % 2) * 90.f, 1.35f);
	}
	static const TCHAR* Ops[] = {
		TEXT("GR_FO_01"), TEXT("GR_FO_02"), TEXT("GR_FO_03"), TEXT("GR_FO_04"), TEXT("GR_FO_05"),
		TEXT("GR_FO_06"), TEXT("GR_FO_07"), TEXT("GR_FO_08"), TEXT("GR_FO_09"), TEXT("GR_FO_10")
	};
	for (int32 I = 0; I < 10; ++I)
	{
		Place(Ops[I], Origin + FVector((I % 5) * 3.f, 0.f, -6.f - (I / 5) * 3.f), 0.f, 1.4f);
	}
}
