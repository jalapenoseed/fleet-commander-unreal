#include "FCAssetLoader.h"
#include "Containers/Array.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Engine/EngineTypes.h"
#include "Engine/StaticMesh.h"
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
#include "UObject/Object.h"
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

UTexture2D* FFCAssetLoader::LoadPng(const FString& AbsPath, UObject* Outer)
{
	if (!FPaths::FileExists(AbsPath))
	{
		return nullptr;
	}
	return FImageUtils::ImportFileAsTexture2D(AbsPath);
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

static void Tint(UMaterialInstanceDynamic* Mid, const FLinearColor& Color)
{
	if (!Mid)
	{
		return;
	}
	Mid->SetVectorParameterValue(TEXT("Color"), Color);
	Mid->SetVectorParameterValue(TEXT("BaseColor"), Color);
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
	UMaterialInterface* Base = BaseMat();
	for (int32 S = 0; S < Names.Num(); ++S)
	{
		UMaterialInstanceDynamic* Mid = UMaterialInstanceDynamic::Create(Base, Outer);
		const FString& Surf = Names[S];
		FLinearColor Color(0.18f, 0.20f, 0.22f);
		if (Surf.StartsWith(TEXT("Panel"))) Color = FLinearColor(0.75f, 0.76f, 0.72f);
		else if (Surf.StartsWith(TEXT("Rubber"))) Color = FLinearColor(0.08f, 0.08f, 0.08f);
		else if (Surf.StartsWith(TEXT("Anodized"))) Color = FLinearColor(0.12f, 0.12f, 0.13f);
		else if (Surf.StartsWith(TEXT("Galvanized"))) Color = FLinearColor(0.55f, 0.56f, 0.54f);
		else if (Surf.StartsWith(TEXT("Optical")) || Surf.StartsWith(TEXT("Glass"))) Color = FLinearColor(0.05f, 0.08f, 0.10f);
		else if (Surf.StartsWith(TEXT("Emission"))) Color = FLinearColor(0.20f, 0.85f, 0.90f);
		else if (Surf.StartsWith(TEXT("Carbon"))) Color = FLinearColor(0.06f, 0.06f, 0.07f);
		else if (Surf.StartsWith(TEXT("Copper"))) Color = FLinearColor(0.45f, 0.22f, 0.10f);
		else if (Surf.StartsWith(TEXT("Prop"))) Color = FLinearColor(0.10f, 0.10f, 0.11f);
		else if (Surf.StartsWith(TEXT("Trim"))) Color = FLinearColor(0.70f, 0.70f, 0.68f);
		Tint(Mid, Color);
		Mats.Add(Mid);
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
	UMaterialInterface* Base = BaseMat();

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

		UMaterialInstanceDynamic* Mid = UMaterialInstanceDynamic::Create(Base, Outer);
		UTexture2D* Tex = nullptr;
		if (!TexName.IsEmpty())
		{
			Tex = LoadPng(RootDir() / TEXT("PropTextures") / (TexName + TEXT(".png")), Outer);
		}
		Tint(Mid, Tex ? FLinearColor::White : Color);
		if (Mid && Tex)
		{
			Mid->SetTextureParameterValue(TEXT("Texture"), Tex);
			Mid->SetTextureParameterValue(TEXT("BaseColorTexture"), Tex);
		}
		Mats.Add(Mid);
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
