#include "FCOperator.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/EngineTypes.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"

AFCOperator::AFCOperator()
{
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	Torso = MakePart(TEXT("Torso"), Root);
	Head = MakePart(TEXT("Head"), Torso);
	Pelvis = MakePart(TEXT("Pelvis"), Root);
	ThighL = MakePart(TEXT("ThighL"), Pelvis);
	ThighR = MakePart(TEXT("ThighR"), Pelvis);
	ShinL = MakePart(TEXT("ShinL"), ThighL);
	ShinR = MakePart(TEXT("ShinR"), ThighR);
	ArmL = MakePart(TEXT("ArmL"), Torso);
	ArmR = MakePart(TEXT("ArmR"), Torso);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (Cube.Succeeded())
	{
		Torso->SetStaticMesh(Cube.Object);
		Pelvis->SetStaticMesh(Cube.Object);
		ThighL->SetStaticMesh(Cube.Object);
		ThighR->SetStaticMesh(Cube.Object);
		ShinL->SetStaticMesh(Cube.Object);
		ShinR->SetStaticMesh(Cube.Object);
		ArmL->SetStaticMesh(Cube.Object);
		ArmR->SetStaticMesh(Cube.Object);
	}
	if (Sphere.Succeeded())
	{
		Head->SetStaticMesh(Sphere.Object);
	}
}

UStaticMeshComponent* AFCOperator::MakePart(const TCHAR* Name, USceneComponent* Attach)
{
	UStaticMeshComponent* C = CreateDefaultSubobject<UStaticMeshComponent>(Name);
	C->SetupAttachment(Attach);
	C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	C->SetCastShadow(true);
	C->SetMobility(EComponentMobility::Movable);
	return C;
}

void AFCOperator::BeginPlay()
{
	Super::BeginPlay();
	Home = GetActorLocation();
	UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (!Base) Base = UMaterial::GetDefaultMaterial(MD_Surface);
	auto Paint = [&](UStaticMeshComponent* C, const FLinearColor& Color, const FVector& Rel, const FVector& Scale)
	{
		if (!C) return;
		C->SetRelativeLocation(Rel);
		C->SetRelativeScale3D(Scale);
		if (UMaterialInstanceDynamic* Mid = UMaterialInstanceDynamic::Create(Base, this))
		{
			Mid->SetVectorParameterValue(TEXT("Color"), Color);
			Mid->SetVectorParameterValue(TEXT("BaseColor"), Color);
			C->SetMaterial(0, Mid);
		}
	};
	const FLinearColor Suit(0.07f, 0.16f, 0.18f);
	const FLinearColor Strap(0.12f, 0.72f, 0.70f);
	const FLinearColor Skin(0.62f, 0.46f, 0.34f);
	const FLinearColor Boot(0.05f, 0.05f, 0.05f);
	Paint(Torso, Suit, FVector(0.f, 0.f, 118.f), FVector(0.28f, 0.38f, 0.48f));
	Paint(Head, Skin, FVector(0.f, 0.f, 42.f), FVector(0.22f, 0.22f, 0.24f));
	Paint(Pelvis, Strap, FVector(0.f, 0.f, 82.f), FVector(0.26f, 0.34f, 0.16f));
	Paint(ThighL, Suit, FVector(0.f, -10.f, -22.f), FVector(0.14f, 0.14f, 0.32f));
	Paint(ThighR, Suit, FVector(0.f, 10.f, -22.f), FVector(0.14f, 0.14f, 0.32f));
	Paint(ShinL, Boot, FVector(0.f, 0.f, -28.f), FVector(0.12f, 0.12f, 0.28f));
	Paint(ShinR, Boot, FVector(0.f, 0.f, -28.f), FVector(0.12f, 0.12f, 0.28f));
	Paint(ArmL, Suit, FVector(0.f, -24.f, 4.f), FVector(0.10f, 0.10f, 0.38f));
	Paint(ArmR, Suit, FVector(0.f, 24.f, 4.f), FVector(0.10f, 0.10f, 0.38f));
}

void AFCOperator::SetControlled(bool bIn)
{
	bControlled = bIn;
	if (!bIn) InputMove = FVector::ZeroVector;
}

void AFCOperator::SetPlayerMove(const FVector& WorldMove, float YawDelta)
{
	InputMove = WorldMove;
	Yaw += YawDelta;
}

FVector AFCOperator::CameraLocation() const
{
	const FRotator R(0.f, Yaw, 0.f);
	return GetActorLocation() + FVector(0.f, 0.f, 168.f) - R.RotateVector(FVector(220.f, 0.f, -40.f));
}

FRotator AFCOperator::CameraRotation() const
{
	return (GetActorLocation() + FVector(0.f, 0.f, 150.f) - CameraLocation()).Rotation();
}

void AFCOperator::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	FVector Delta = FVector::ZeroVector;
	if (bControlled)
	{
		const FRotator Facing(0.f, Yaw, 0.f);
		Delta = Facing.RotateVector(FVector(InputMove.X, InputMove.Y, 0.f)) * 320.f;
	}
	else
	{
		const float T = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
		const float Orbit = 420.f;
		const FVector Center = Home;
		const FVector Goal = Center + FVector(FMath::Sin(T * 0.18f + PatrolIndex) * Orbit, FMath::Cos(T * 0.18f + PatrolIndex * 1.7f) * Orbit, 0.f);
		FVector To = Goal - GetActorLocation();
		To.Z = 0.f;
		if (To.SizeSquared() > 4.f)
		{
			Delta = To.GetSafeNormal() * 90.f;
			Yaw = FMath::RadiansToDegrees(FMath::Atan2(Delta.Y, Delta.X));
		}
	}
	Speed = FMath::FInterpTo(Speed, Delta.Size(), DeltaSeconds, 6.f);
	FVector Next = GetActorLocation() + Delta * DeltaSeconds;
	Next.Z = Home.Z;
	SetActorLocation(Next);
	SetActorRotation(FRotator(0.f, Yaw, 0.f));
	Pose(DeltaSeconds);
}

void AFCOperator::Pose(float Dt)
{
	WalkPhase += Dt * (Speed > 8.f ? Speed * 0.045f : 1.6f);
	const float Swing = Speed > 8.f ? 28.f : 4.f;
	const float A = FMath::Sin(WalkPhase) * Swing;
	const float B = FMath::Sin(WalkPhase + PI) * Swing;
	if (ThighL) ThighL->SetRelativeRotation(FRotator(A, 0.f, 0.f));
	if (ThighR) ThighR->SetRelativeRotation(FRotator(B, 0.f, 0.f));
	if (ShinL) ShinL->SetRelativeRotation(FRotator(FMath::Max(0.f, -A) * 0.6f, 0.f, 0.f));
	if (ShinR) ShinR->SetRelativeRotation(FRotator(FMath::Max(0.f, -B) * 0.6f, 0.f, 0.f));
	if (ArmL) ArmL->SetRelativeRotation(FRotator(B * 0.7f, 0.f, 8.f));
	if (ArmR) ArmR->SetRelativeRotation(FRotator(A * 0.7f, 0.f, -8.f));
	if (Torso) Torso->SetRelativeLocation(FVector(0.f, 0.f, 118.f + FMath::Abs(FMath::Sin(WalkPhase * 2.f)) * (Speed > 8.f ? 4.f : 1.f)));
}
