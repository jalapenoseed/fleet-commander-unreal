#pragma once

#include "CoreMinimal.h"

class UStaticMesh;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UTexture2D;

/** Loads Unity 1.3 FCM1 drone meshes and FCP1 GRIDRUNNER scenery from FleetAssets/. */
struct FFCLoadedProp
{
	FString Name;
	TObjectPtr<UStaticMesh> Mesh = nullptr;
};

struct FFCAssetLoader
{
	static FString RootDir();
	static UStaticMesh* LoadDrone(const FString& Name, UObject* Outer);
	static UStaticMesh* LoadProp(const FString& Name, UObject* Outer);
	static void LoadCampAndHouses(UObject* Outer, TArray<FFCLoadedProp>& Out);
	static UTexture2D* LoadPng(const FString& AbsPath, UObject* Outer);
	static UMaterialInterface* FleetMaster();
	static UMaterialInstanceDynamic* MakeDroneSurface(UObject* Outer, const FString& SurfaceName);
	static UMaterialInstanceDynamic* MakePropSurface(UObject* Outer, const FString& TexStem, const FLinearColor& Color);
};
