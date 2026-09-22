#include "FCAssetLoader.h"
#include "Containers/Array.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Engine/EngineTypes.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "ImageUtils.h"
#include "Logging/LogMacros.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Math/Color.h"
#include "MeshAttributeArray.h"
#include "MeshDescription.h"
#include "MeshTypes.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/MemoryReader.h"
#include "StaticMeshAttributes.h"
#include "Templates/SharedPointer.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY_STATIC(LogFCAssets, Log, All);

FString FFCAssetLoader::RootDir()
{
	return FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / TEXT("FleetAssets"));
}

static FVector3f ToUE(float X, float Y, float Z)
{
	// Unity metres, Y-up, Z-forward → Unreal cm, Z-up, X-forward
	return FVector3f(Z * 100.f, X * 100.f, Y * 100.f);
}

static bool ReadI32(FMemoryReader& Ar, int32& Out)
{
	Ar.Serialize(&Out, sizeof(int32));
	return !Ar.IsError();
}

static bool ReadF32(FMemoryReader& Ar, float& Out)
{
	Ar.Serialize(&Out, sizeof(float));
	return !Ar.IsError();
}

static UMaterialInterface* BaseMat()
{
	if (UMaterialInterface* Shape = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")))
	{
		return Shape;
	}
	if (UMaterialInterface* DebugMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineDebugMaterials/DebugMeshMaterial.DebugMeshMaterial")))
	{
		return DebugMat;
	}
	return UMaterial::GetDefaultMaterial(MD_Surface);
}

UMaterialInterface* FFCAssetLoader::FleetMaster()
{
	if (UMaterialInterface* Pbr = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Fleet/M_FleetPBR.M_FleetPBR")))
	{
		return Pbr;
	}
	return BaseMat();
}

static void Tint(UMaterialInstanceDynamic* Mid, const FLinearColor& Color)
{
	if (!Mid) return;
	Mid->SetVectorParameterValue(TEXT("Color"), Color);
	Mid->SetVectorParameterValue(TEXT("BaseColor"), Color);
}

UTexture2D* FFCAssetLoader::LoadPng(const FString& AbsPath, UObject* Outer)
{
	(void)Outer;
	if (!FPaths::FileExists(AbsPath))
	{
		return nullptr;
	}
	return FImageUtils::ImportFileAsTexture2D(AbsPath);
}

static UTexture2D* CachedTex(const FString& Rel, bool bNormal, bool bLinear)
{
	static TMap<FString, TObjectPtr<UTexture2D>> Cache;
	if (TObjectPtr<UTexture2D>* Found = Cache.Find(Rel))
	{
		return Found->Get();
	}
	UTexture2D* Tex = FFCAssetLoader::LoadPng(FFCAssetLoader::RootDir() / Rel, GetTransientPackage());
	if (Tex)
	{
		Tex->SRGB = !bLinear && !bNormal;
		if (bNormal)
		{
			Tex->CompressionSettings = TC_Normalmap;
		}
		else if (bLinear)
		{
			Tex->SRGB = false;
		}
		Tex->UpdateResource();
		Cache.Add(Rel, Tex);
	}
	return Tex;
}

static void BindMaps(UMaterialInstanceDynamic* Mid, const FString& AlbedoStem, const FString& MapStem, float Metallic, float Smooth, float Glow)
{
	if (!Mid) return;
	auto Albedo = [&](const FString& Stem) -> UTexture2D*
	{
		return CachedTex(FString::Printf(TEXT("DroneTextures/GR_%s_albedo.png"), *Stem), false, false);
	};
	auto Map = [&](const TCHAR* Suffix, bool bNormal) -> UTexture2D*
	{
		return CachedTex(FString::Printf(TEXT("DroneTextures/GR_%s_%s.png"), *MapStem, Suffix), bNormal, !bNormal);
	};
	if (UTexture2D* A = Albedo(AlbedoStem)) Mid->SetTextureParameterValue(TEXT("Albedo"), A);
	if (UTexture2D* N = Map(TEXT("normal"), true)) Mid->SetTextureParameterValue(TEXT("Normal"), N);
	if (UTexture2D* R = Map(TEXT("rough"), false))
	{
		Mid->SetTextureParameterValue(TEXT("Roughness"), R);
		Mid->SetScalarParameterValue(TEXT("RoughnessMul"), 1.f);
	}
	else
	{
		Mid->SetScalarParameterValue(TEXT("RoughnessMul"), 1.f - Smooth);
	}
	if (UTexture2D* M = Map(TEXT("metal"), false))
	{
		Mid->SetTextureParameterValue(TEXT("Metallic"), M);
		Mid->SetScalarParameterValue(TEXT("MetallicMul"), 1.f);
	}
	else
	{
		Mid->SetScalarParameterValue(TEXT("MetallicMul"), Metallic);
	}
	if (UTexture2D* O = Map(TEXT("ao"), false)) Mid->SetTextureParameterValue(TEXT("AO"), O);
	Mid->SetScalarParameterValue(TEXT("Emissive"), Glow);
	Mid->SetScalarParameterValue(TEXT("Metallic"), Metallic);
	Mid->SetScalarParameterValue(TEXT("Roughness"), 1.f - Smooth);
}

UMaterialInstanceDynamic* FFCAssetLoader::MakeDroneSurface(UObject* Outer, const FString& SurfaceName)
{
	UMaterialInstanceDynamic* Mid = UMaterialInstanceDynamic::Create(FleetMaster(), Outer);
	FLinearColor Color(1.f, 1.f, 1.f);
	float Metallic = 0.08f;
	float Smooth = 0.36f;
	float Glow = 0.f;
	FString Albedo = TEXT("01_painted_alum");
	FString Maps = TEXT("01_painted_alum");
	if (SurfaceName.StartsWith(TEXT("Panel")) || SurfaceName.StartsWith(TEXT("OffWhite")))
	{
		Albedo = TEXT("paint_offwhite");
		Maps = TEXT("01_painted_alum");
		Color = FLinearColor(0.92f, 0.93f, 0.90f);
	}
	else if (SurfaceName.StartsWith(TEXT("Accent")))
	{
		Albedo = TEXT("paint_ochre");
		Maps = TEXT("01_painted_alum");
	}
	else if (SurfaceName.StartsWith(TEXT("Rubber")))
	{
		Albedo = Maps = TEXT("05_rubber");
		Metallic = 0.08f;
		Smooth = 0.12f;
	}
	else if (SurfaceName.StartsWith(TEXT("Anodized")) || SurfaceName.StartsWith(TEXT("Prop")))
	{
		Albedo = Maps = TEXT("03_black_anodized");
		Metallic = 0.50f;
		Smooth = 0.40f;
	}
	else if (SurfaceName.StartsWith(TEXT("Galvanized")))
	{
		Albedo = Maps = TEXT("07_galvanized");
		Metallic = 0.80f;
		Smooth = 0.36f;
	}
	else if (SurfaceName.StartsWith(TEXT("Metal")))
	{
		Albedo = Maps = TEXT("02_machined_alum");
		Metallic = 0.80f;
		Smooth = 0.65f;
	}
	else if (SurfaceName.StartsWith(TEXT("Carbon")))
	{
		Albedo = Maps = TEXT("04_weave");
		Metallic = 0.08f;
	}
	else if (SurfaceName.StartsWith(TEXT("Copper")))
	{
		Albedo = Maps = TEXT("06_aged_copper");
		Metallic = 0.80f;
	}
	else if (SurfaceName.StartsWith(TEXT("Glass")) || SurfaceName.StartsWith(TEXT("Optical")))
	{
		Albedo = Maps = TEXT("09_camera_glass");
		Metallic = 0.08f;
		Smooth = 0.94f;
		Color = FLinearColor(0.08f, 0.10f, 0.12f);
	}
	else if (SurfaceName.StartsWith(TEXT("Trim")))
	{
		Albedo = Maps = TEXT("trim");
	}
	else if (SurfaceName.StartsWith(TEXT("Emission")))
	{
		Albedo = TEXT("paint_offwhite");
		Maps = TEXT("01_painted_alum");
		Glow = 1.6f;
		Color = FLinearColor(0.20f, 0.85f, 0.90f);
	}
	else if (SurfaceName.StartsWith(TEXT("MarkingLight")))
	{
		Color = FLinearColor(0.82f, 0.81f, 0.74f);
	}
	else if (SurfaceName.StartsWith(TEXT("MarkingDark")))
	{
		Color = FLinearColor(0.055f, 0.06f, 0.065f);
	}
	Tint(Mid, Color);
	BindMaps(Mid, Albedo, Maps, Metallic, Smooth, Glow);
	return Mid;
}

UMaterialInstanceDynamic* FFCAssetLoader::MakePropSurface(UObject* Outer, const FString& TexStem, const FLinearColor& Color)
{
	UMaterialInstanceDynamic* Mid = UMaterialInstanceDynamic::Create(FleetMaster(), Outer);
	Tint(Mid, TexStem.IsEmpty() ? Color : FLinearColor::White);
	if (!TexStem.IsEmpty())
	{
		if (UTexture2D* A = CachedTex(FString::Printf(TEXT("PropTextures/%s.png"), *TexStem), false, false))
		{
			Mid->SetTextureParameterValue(TEXT("Albedo"), A);
			Mid->SetTextureParameterValue(TEXT("Texture"), A);
		}
	}
	Mid->SetScalarParameterValue(TEXT("RoughnessMul"), 0.55f);
	Mid->SetScalarParameterValue(TEXT("MetallicMul"), 0.08f);
	return Mid;
}

static UStaticMesh* BuildMesh(
	UObject* Outer,
	FName Name,
	const TArray<TArray<FVector3f>>& SectionVerts,
	const TArray<TArray<FVector3f>>& SectionNormals,
	const TArray<TArray<FVector2f>>& SectionUVs,
	const TArray<TArray<int32>>& SectionIndices,
	const TArray<UMaterialInterface*>& SectionMats,
	const TArray<FString>& SectionNames)
{
	if (SectionVerts.Num() == 0)
	{
		return nullptr;
	}

	FMeshDescription Desc;
	FStaticMeshAttributes Attr(Desc);
	Attr.Register();
	TVertexAttributesRef<FVector3f> Positions = Attr.GetVertexPositions();
	TVertexInstanceAttributesRef<FVector3f> Normals = Attr.GetVertexInstanceNormals();
	TVertexInstanceAttributesRef<FVector2f> UVs = Attr.GetVertexInstanceUVs();
	TPolygonGroupAttributesRef<FName> SlotNames = Attr.GetPolygonGroupMaterialSlotNames();
	UVs.SetNumChannels(1);

	int32 BuiltSections = 0;
	for (int32 S = 0; S < SectionVerts.Num(); ++S)
	{
		const TArray<FVector3f>& V = SectionVerts[S];
		const TArray<FVector3f>& N = SectionNormals[S];
		const TArray<FVector2f>& T = SectionUVs[S];
		const TArray<int32>& Ix = SectionIndices[S];
		if (V.Num() == 0 || Ix.Num() < 3)
		{
			continue;
		}
		const FPolygonGroupID Group = Desc.CreatePolygonGroup();
		const FName SlotName = SectionNames.IsValidIndex(S) && !SectionNames[S].IsEmpty()
			? FName(*SectionNames[S])
			: FName(*FString::Printf(TEXT("Section%d"), S));
		SlotNames[Group] = SlotName;

		TArray<FVertexID> Verts;
		Verts.SetNum(V.Num());
		for (int32 I = 0; I < V.Num(); ++I)
		{
			Verts[I] = Desc.CreateVertex();
			Positions[Verts[I]] = V[I];
		}
		for (int32 I = 0; I + 2 < Ix.Num(); I += 3)
		{
			const int32 A = Ix[I];
			const int32 B = Ix[I + 2]; // flip winding Unity → Unreal
			const int32 C = Ix[I + 1];
			if (!V.IsValidIndex(A) || !V.IsValidIndex(B) || !V.IsValidIndex(C))
			{
				continue;
			}
			auto Inst = [&](int32 Idx)
			{
				const FVertexInstanceID Id = Desc.CreateVertexInstance(Verts[Idx]);
				Normals[Id] = N.IsValidIndex(Idx) ? N[Idx] : FVector3f(0.f, 0.f, 1.f);
				UVs.Set(Id, 0, T.IsValidIndex(Idx) ? T[Idx] : FVector2f(0.f, 0.f));
				return Id;
			};
			TArray<FVertexInstanceID> Poly;
			Poly.Reserve(3);
			Poly.Add(Inst(A));
			Poly.Add(Inst(B));
			Poly.Add(Inst(C));
			Desc.CreatePolygon(Group, Poly);
		}
		++BuiltSections;
	}
	if (BuiltSections == 0)
	{
		return nullptr;
	}

	UStaticMesh* Mesh = NewObject<UStaticMesh>(Outer, Name, RF_Transient);
	Mesh->NeverStream = true;
	TArray<const FMeshDescription*> Descs;
	Descs.Add(&Desc);
	UStaticMesh::FBuildMeshDescriptionsParams Params;
	Params.bBuildSimpleCollision = false;
	Params.bFastBuild = true;
	Params.bCommitMeshDescription = true;
	if (!Mesh->BuildFromMeshDescriptions(Descs, Params))
	{
		UE_LOG(LogFCAssets, Warning, TEXT("BuildFromMeshDescriptions failed for %s"), *Name.ToString());
		return nullptr;
	}

	TArray<FStaticMaterial> Mats;
	for (int32 S = 0; S < SectionMats.Num(); ++S)
	{
		const FName SlotName = SectionNames.IsValidIndex(S) && !SectionNames[S].IsEmpty()
			? FName(*SectionNames[S])
			: FName(*FString::Printf(TEXT("Section%d"), S));
		Mats.Add(FStaticMaterial(SectionMats[S] ? SectionMats[S] : BaseMat(), SlotName, SlotName));
	}
	if (Mats.Num() == 0)
	{
		Mats.Add(FStaticMaterial(BaseMat()));
	}
	Mesh->SetStaticMaterials(Mats);
	return Mesh;
}

static bool ReadFcm1(
	const TArray<uint8>& Bytes,
	TArray<TArray<FVector3f>>& Verts,
	TArray<TArray<FVector3f>>& Normals,
	TArray<TArray<FVector2f>>& UVs,
	TArray<TArray<int32>>& Indices,
	TArray<FString>& Names)
{
	TArray<uint8>& MutableBytes = const_cast<TArray<uint8>&>(Bytes);
	FMemoryReader Ar(MutableBytes, false);
	char Magic[4] = {};
	Ar.Serialize(Magic, 4);
	if (Magic[0] != 'F' || Magic[1] != 'C' || Magic[2] != 'M' || Magic[3] != '1')
	{
		return false;
	}
	int32 Surfaces = 0;
	if (!ReadI32(Ar, Surfaces) || Surfaces <= 0 || Surfaces > 32)
	{
		return false;
	}
	for (int32 S = 0; S < Surfaces; ++S)
	{
		int32 NameLen = 0;
		if (!ReadI32(Ar, NameLen) || NameLen <= 0 || NameLen > 256)
		{
			return false;
		}
		TArray<uint8> NameBytes;
		NameBytes.SetNum(NameLen + 1);
		Ar.Serialize(NameBytes.GetData(), NameLen);
		NameBytes[NameLen] = 0;
		const FString SurfName = UTF8_TO_TCHAR(reinterpret_cast<const char*>(NameBytes.GetData()));
		int32 NV = 0;
		int32 NI = 0;
		if (!ReadI32(Ar, NV) || !ReadI32(Ar, NI))
		{
			return false;
		}
		if (NV <= 0 || NV > 1000000 || NI <= 0 || NI > 3000000 || (NI % 3) != 0)
		{
			return false;
		}
		TArray<FVector3f> V;
		TArray<FVector3f> N;
		TArray<FVector2f> T;
		TArray<int32> Ix;
		V.SetNum(NV);
		N.SetNum(NV);
		T.SetNum(NV);
		for (int32 I = 0; I < NV; ++I)
		{
			float X, Y, Z, NX, NY, NZ, U, VV;
			if (!ReadF32(Ar, X) || !ReadF32(Ar, Y) || !ReadF32(Ar, Z) ||
				!ReadF32(Ar, NX) || !ReadF32(Ar, NY) || !ReadF32(Ar, NZ) ||
				!ReadF32(Ar, U) || !ReadF32(Ar, VV))
			{
				return false;
			}
			V[I] = ToUE(X, Y, Z);
			N[I] = ToUE(NX, NY, NZ).GetSafeNormal();
			T[I] = FVector2f(U, 1.f - VV); // Unity UV origin is bottom-left
		}
		Ix.SetNum(NI);
		for (int32 I = 0; I < NI; ++I)
		{
			if (!ReadI32(Ar, Ix[I]))
			{
				return false;
			}
		}
		Verts.Add(MoveTemp(V));
		Normals.Add(MoveTemp(N));
		UVs.Add(MoveTemp(T));
		Indices.Add(MoveTemp(Ix));
		Names.Add(SurfName);
	}
	return true;
}

UStaticMesh* FFCAssetLoader::LoadDrone(const FString& Name, UObject* Outer)
{
	const FString Path = RootDir() / TEXT("DroneModels") / (Name + TEXT(".fcm1"));
	TArray<uint8> Bytes;
	if (!FFileHelper::LoadFileToArray(Bytes, *Path))
	{
		UE_LOG(LogFCAssets, Warning, TEXT("Missing drone mesh %s"), *Path);
		return nullptr;
	}
	TArray<TArray<FVector3f>> V;
	TArray<TArray<FVector3f>> N;
	TArray<TArray<FVector2f>> T;
	TArray<TArray<int32>> Ix;
	TArray<FString> Names;
	if (!ReadFcm1(Bytes, V, N, T, Ix, Names))
	{
		UE_LOG(LogFCAssets, Warning, TEXT("Bad FCM1 %s"), *Path);
		return nullptr;
	}

	TArray<UMaterialInterface*> Mats;
	for (int32 S = 0; S < Names.Num(); ++S)
	{
		Mats.Add(MakeDroneSurface(Outer, Names[S]));
	}
	UE_LOG(LogFCAssets, Log, TEXT("Loaded Unity aircraft %s (%d surfaces)"), *Name, Names.Num());
	return BuildMesh(Outer, FName(*Name), V, N, T, Ix, Mats, Names);
}

UStaticMesh* FFCAssetLoader::LoadProp(const FString& Name, UObject* Outer)
{
	const FString Path = RootDir() / TEXT("ScenePacks") / (Name + TEXT(".fcp1"));
	TArray<uint8> Bytes;
	if (!FFileHelper::LoadFileToArray(Bytes, *Path))
	{
		UE_LOG(LogFCAssets, Warning, TEXT("Missing prop %s"), *Path);
		return nullptr;
	}
	FMemoryReader Ar(Bytes, false);
	char Magic[4] = {};
	Ar.Serialize(Magic, 4);
	if (Magic[0] != 'F' || Magic[1] != 'C' || Magic[2] != 'P' || Magic[3] != '1')
	{
		UE_LOG(LogFCAssets, Warning, TEXT("Bad FCP1 magic %s"), *Path);
		return nullptr;
	}
	int32 SurfaceCount = 0;
	if (!ReadI32(Ar, SurfaceCount) || SurfaceCount < 1 || SurfaceCount > 256)
	{
		return nullptr;
	}

	TArray<TArray<FVector3f>> Verts;
	TArray<TArray<FVector3f>> Normals;
	TArray<TArray<FVector2f>> UVs;
	TArray<TArray<int32>> Indices;
	TArray<UMaterialInterface*> Mats;
	TArray<FString> Names;

	for (int32 K = 0; K < SurfaceCount; ++K)
	{
		int32 HeaderBytes = 0;
		if (!ReadI32(Ar, HeaderBytes) || HeaderBytes < 1 || HeaderBytes > 65536)
		{
			return nullptr;
		}
		TArray<uint8> Header;
		Header.SetNum(HeaderBytes + 1);
		Ar.Serialize(Header.GetData(), HeaderBytes);
		Header[HeaderBytes] = 0;
		const FString JsonStr = UTF8_TO_TCHAR(reinterpret_cast<const char*>(Header.GetData()));
		TSharedPtr<FJsonObject> Obj;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonStr);
		FString TexName;
		FString SurfName = FString::Printf(TEXT("%s_%d"), *Name, K);
		FLinearColor Color(0.60f, 0.58f, 0.52f);
		if (FJsonSerializer::Deserialize(Reader, Obj) && Obj.IsValid())
		{
			Obj->TryGetStringField(TEXT("texture"), TexName);
			Obj->TryGetStringField(TEXT("name"), SurfName);
			const TArray<TSharedPtr<FJsonValue>>* Col = nullptr;
			if (Obj->TryGetArrayField(TEXT("color"), Col) && Col && Col->Num() >= 3)
			{
				Color = FLinearColor((*Col)[0]->AsNumber(), (*Col)[1]->AsNumber(), (*Col)[2]->AsNumber(), 1.f);
			}
		}
		int32 NV = 0;
		int32 NI = 0;
		if (!ReadI32(Ar, NV) || !ReadI32(Ar, NI))
		{
			return nullptr;
		}
		if (NV < 1 || NV > 4000000 || NI < 1 || NI > 12000000 || (NI % 3) != 0)
		{
			return nullptr;
		}
		TArray<FVector3f> V;
		TArray<FVector3f> Nrm;
		TArray<FVector2f> T;
		TArray<int32> Ix;
		V.SetNum(NV);
		Nrm.SetNum(NV);
		T.SetNum(NV);
		for (int32 I = 0; I < NV; ++I)
		{
			float X, Y, Z, NX, NY, NZ, U, VV;
			if (!ReadF32(Ar, X) || !ReadF32(Ar, Y) || !ReadF32(Ar, Z) ||
				!ReadF32(Ar, NX) || !ReadF32(Ar, NY) || !ReadF32(Ar, NZ) ||
				!ReadF32(Ar, U) || !ReadF32(Ar, VV))
			{
				return nullptr;
			}
			V[I] = ToUE(X, Y, Z);
			Nrm[I] = ToUE(NX, NY, NZ).GetSafeNormal();
			T[I] = FVector2f(U, 1.f - VV);
		}
		Ix.SetNum(NI);
		for (int32 I = 0; I < NI; ++I)
		{
			if (!ReadI32(Ar, Ix[I]))
			{
				return nullptr;
			}
		}
		Verts.Add(MoveTemp(V));
		Normals.Add(MoveTemp(Nrm));
		UVs.Add(MoveTemp(T));
		Indices.Add(MoveTemp(Ix));
		Names.Add(SurfName);
		Mats.Add(MakePropSurface(Outer, TexName, Color));
	}
	return BuildMesh(Outer, FName(*Name), Verts, Normals, UVs, Indices, Mats, Names);
}

void FFCAssetLoader::LoadCampAndHouses(UObject* Outer, TArray<FFCLoadedProp>& Out)
{
	static const TCHAR* Names[] = {
		TEXT("GR05_01_UtilityHouse"), TEXT("GR05_02_RoadsideStore"), TEXT("GR05_03_TexasHouse"),
		TEXT("GR05_04_Maintenance"), TEXT("GR05_05_CreekPump"),
		TEXT("GR_TarpShelter_02"), TEXT("GR_Workbench_02"), TEXT("GR_FoldingChair_02"), TEXT("GR_CampCot_02"),
		TEXT("GR_SolarArray_02"), TEXT("GR_ChargingStation_02"), TEXT("GR_DroneHardCase_02"), TEXT("GR_FieldRadio_02"),
		TEXT("GR_BatteryCase_02"), TEXT("GR_FirstAidCase_02"), TEXT("GR_SupplyLocker_02"), TEXT("GR_WaterBarrel_02"),
		TEXT("GR_WaterFilter_02"), TEXT("GR_SupplyCrate_02"), TEXT("GR_Toolboard_02"), TEXT("GR_ExtensionReel_02"),
		TEXT("GR_CampStove_02"), TEXT("GR_RainCollector_02"), TEXT("GR_FieldStorageCase_02"), TEXT("GR_BikeRepairStand_02"),
		TEXT("GR_FO_01"), TEXT("GR_FO_02"), TEXT("GR_FO_03"), TEXT("GR_FO_04"), TEXT("GR_FO_05"),
		TEXT("GR_FO_06"), TEXT("GR_FO_07"), TEXT("GR_FO_08"), TEXT("GR_FO_09"), TEXT("GR_FO_10")
	};
	int32 Loaded = 0;
	for (const TCHAR* N : Names)
	{
		FFCLoadedProp P;
		P.Name = N;
		P.Mesh = LoadProp(N, Outer);
		if (P.Mesh)
		{
			++Loaded;
		}
		Out.Add(P);
	}
	UE_LOG(LogFCAssets, Log, TEXT("GRIDRUNNER scenery loaded %d / %d packs"), Loaded, UE_ARRAY_COUNT(Names));
}
