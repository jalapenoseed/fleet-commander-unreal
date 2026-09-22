#include "FCFormation.h"

float FFCFormationMath::Hash(int32 I)
{
	uint32 X = static_cast<uint32>(I + 1) * 747796405u + 2891336453u;
	X = ((X >> ((X >> 28) + 4)) ^ X) * 277803737u;
	return (static_cast<float>((X >> 22) ^ X)) / static_cast<float>(MAX_uint32);
}

FVector FFCFormationMath::Grid(int32 Index, int32 Count, float Spacing)
{
	if (Count <= 0) return FVector::ZeroVector;
	const int32 Side = FMath::CeilToInt(FMath::Sqrt(static_cast<float>(Count)));
	const int32 X = Index % Side;
	const int32 Y = Index / Side;
	const float Half = (Side - 1) * Spacing * 0.5f;
	return FVector(Y * Spacing - Half, X * Spacing - Half, 0.f);
}

FVector FFCFormationMath::Ring(int32 Index, int32 Count, float Radius)
{
	if (Count <= 0) return FVector::ZeroVector;
	const float A = (Index / static_cast<float>(Count)) * 2.f * PI;
	return FVector(FMath::Cos(A) * Radius, FMath::Sin(A) * Radius, 0.f);
}

FVector FFCFormationMath::Sphere(int32 Index, int32 Count, float Radius)
{
	if (Count <= 1) return FVector(0.f, 0.f, Radius);
	const float Golden = PI * (3.f - FMath::Sqrt(5.f));
	const float Z = 1.f - (Index / static_cast<float>(Count - 1)) * 2.f;
	const float R = FMath::Sqrt(FMath::Max(0.f, 1.f - Z * Z));
	const float Theta = Golden * Index;
	return FVector(FMath::Cos(Theta) * R, FMath::Sin(Theta) * R, Z) * Radius;
}

FVector FFCFormationMath::Point(EFCFormation Kind, int32 I, int32 N, float Spacing)
{
	if (N <= 0) return FVector::ZeroVector;
	const float T = I / static_cast<float>(FMath::Max(1, N));
	const float A = T * 2.f * PI;
	const float R = FMath::Max(Spacing * 2.f, FMath::Sqrt(static_cast<float>(N)) * Spacing * 0.8f);
	switch (Kind)
	{
	case EFCFormation::Ring: return Ring(I, N, R);
	case EFCFormation::Sphere: return Sphere(I, N, R * 0.7f);
	case EFCFormation::Line: return FVector(0.f, (I - (N - 1) * 0.5f) * FMath::Min(Spacing, 60000.f / N), 0.f);
	case EFCFormation::Column: return FVector((I - (N - 1) * 0.5f) * FMath::Min(Spacing, 60000.f / N), 0.f, 0.f);
	case EFCFormation::Wedge:
	{
		const int32 Row = FMath::FloorToInt(FMath::Sqrt(static_cast<float>(I)));
		const int32 Col = I - Row * Row;
		return FVector(Row * Spacing - R * 0.5f, (Col - Row) * Spacing, 0.f);
	}
	case EFCFormation::DoubleOrbit: return Ring(I / 2, (N + 1) / 2, R) + FVector(0.f, ((I % 2) == 0 ? -1.f : 1.f) * R * 0.65f, (I % 2) * Spacing * 2.f);
	case EFCFormation::Scatter: return FVector(Hash(I * 3) - 0.5f, Hash(I * 3 + 2) - 0.5f, (Hash(I * 3 + 1) - 0.5f) * 0.4f) * R * 2.f;
	case EFCFormation::Staggered: return Grid(I, N, Spacing) + FVector(0.f, (I % 2) * Spacing * 0.5f, (I % 3) * Spacing);
	case EFCFormation::HighLow: return Ring(I, N, R) + FVector(0.f, 0.f, FMath::Sin(A * 4.f) * R * 0.25f);
	case EFCFormation::Overwatch: return Grid(I, N, Spacing) + FVector(0.f, 0.f, (I % 3) * Spacing * 2.f);
	case EFCFormation::Helix: return FVector(FMath::Cos(A * 3.f) * R * 0.6f, FMath::Sin(A * 3.f) * R * 0.6f, (T - 0.5f) * R);
	case EFCFormation::Heart:
	{
		const float X = 16.f * FMath::Pow(FMath::Sin(A), 3.f);
		const float Z = 13.f * FMath::Cos(A) - 5.f * FMath::Cos(2.f * A) - 2.f * FMath::Cos(3.f * A) - FMath::Cos(4.f * A);
		return FVector(0.f, X, Z) * R / 18.f;
	}
	default: return Grid(I, N, Spacing);
	}
}

FVector FFCFormationMath::ArenaSlot(EFCArenaFormation Kind, int32 Slot, int32 Count, float Spacing)
{
	const float A = Slot * 2.f * PI / FMath::Max(1, Count);
	const float R = FMath::Max(Spacing, Count * Spacing / 6.28f);
	switch (Kind)
	{
	case EFCArenaFormation::Line: return FVector(0.f, (Slot - (Count - 1) * 0.5f) * Spacing, 0.f);
	case EFCArenaFormation::Grid: { const FVector G = Grid(Slot, Count, Spacing); return FVector(G.Y, G.X, 0.f); }
	case EFCArenaFormation::Ring: return Ring(Slot, Count, FMath::Max(Spacing, Count * Spacing / 6.28f));
	case EFCArenaFormation::Stack: return FVector((Slot / 5) * -Spacing, 0.f, (Slot % 5) * Spacing);
	case EFCArenaFormation::Echelon: return FVector(-Slot * Spacing * 0.7f, (Slot - (Count - 1) * 0.5f) * Spacing * 0.7f, 0.f);
	case EFCArenaFormation::Diamond: return FVector(FMath::Cos(A), FMath::Sin(A), 0.f) * R / (FMath::Abs(FMath::Cos(A)) + FMath::Abs(FMath::Sin(A)));
	case EFCArenaFormation::DoubleWedge: return FVector(-(Slot / 4) * Spacing, (Slot % 4 - 1.5f) * Spacing + ((Slot % 2) == 0 ? -1.f : 1.f) * (Slot / 4.f) * Spacing, 0.f);
	case EFCArenaFormation::Box: return FVector((Slot % 4 < 2 ? -1.f : 1.f) * R * 0.5f, (Slot % 2 == 0 ? -1.f : 1.f) * R * 0.5f, (Slot / 4) * Spacing);
	case EFCArenaFormation::Sphere: return Sphere(Slot, Count, FMath::Max(Spacing, R * 0.65f));
	case EFCArenaFormation::Helix: return FVector(FMath::Cos(A * 2.f) * R * 0.5f, FMath::Sin(A * 2.f) * R * 0.5f, (Slot - (Count - 1) * 0.5f) * Spacing * 0.4f);
	case EFCArenaFormation::Arc: return FVector(FMath::Cos(A * 0.45f) * R, FMath::Sin(A * 0.45f) * R - R * 0.5f, 0.f);
	case EFCArenaFormation::Crescent: return FVector(FMath::Cos(A * 0.7f) * R, FMath::Sin(A * 0.7f) * R, FMath::Sin(A) * Spacing);
	case EFCArenaFormation::Cross: return (Slot % 2 == 0) ? FVector((Slot / 2 - Count * 0.25f) * Spacing, 0.f, 0.f) : FVector(0.f, (Slot / 2 - Count * 0.25f) * Spacing, 0.f);
	case EFCArenaFormation::Staggered: return FVector(-(Slot / 2) * Spacing, (Slot % 2 == 0 ? -1.f : 1.f) * Spacing, (Slot % 2) * Spacing * 0.5f);
	case EFCArenaFormation::Columns: return FVector(-(Slot / 3) * Spacing, (Slot % 3 - 1) * Spacing * 1.5f, 0.f);
	case EFCArenaFormation::LooseCloud: return FVector(Hash(Slot * 3) * 2.f - 1.f, Hash(Slot * 3 + 2) * 2.f - 1.f, Hash(Slot * 3 + 1) - 0.5f) * R;
	case EFCArenaFormation::HighLow: return FVector(-(Slot / 2) * Spacing, (Slot % 2 == 0 ? -1.f : 1.f) * Spacing * 1.5f, (Slot % 2 == 0 ? -1.f : 1.f) * Spacing);
	case EFCArenaFormation::Pincer: return FVector(-FMath::Abs(Slot - Count * 0.5f) * Spacing * 0.4f, (Slot < Count / 2 ? -1.f : 1.f) * (Spacing * 2.f + (Slot % FMath::Max(1, Count / 2)) * Spacing), 0.f);
	case EFCArenaFormation::Escort: return Slot == 0 ? FVector::ZeroVector : Ring(Slot - 1, FMath::Max(1, Count - 1), Spacing * 2.f);
	default:
	{
		const float Row = FMath::CeilToFloat(Slot / 2.f);
		return FVector(-Row * Spacing * 0.65f, (Slot % 2 == 0 ? 1.f : -1.f) * Row * Spacing * 0.65f, 0.f);
	}
	}
}

FVector FFCFormationMath::Influence(const FFCInfluenceLayer& L, const FVector& P, int32 I, float Time)
{
	if (L.Kind == EFCInfluence::None) return FVector::ZeroVector;
	const float A = Time * L.Frequency + L.Phase;
	const float X = P.X / 2000.f;
	const float Y = P.Y / 2000.f;
	FVector V = FVector::ZeroVector;
	switch (L.Kind)
	{
	case EFCInfluence::Vortex: V = FVector(-Y, X, FMath::Sin(A + X)); V.Normalize(); break;
	case EFCInfluence::Attract: V = -P.GetSafeNormal(); break;
	case EFCInfluence::Repel: V = P.GetSafeNormal(); break;
	case EFCInfluence::Wave: V = FVector(0.f, 0.f, FMath::Sin(A + X)); break;
	case EFCInfluence::Lissajous: V = FVector(FMath::Sin(A * 2.f + I * 0.1f), FMath::Cos(A), FMath::Sin(A * 3.f + I * 0.1f)); break;
	case EFCInfluence::Spiral: V = FVector(FMath::Cos(A + I * 0.15f), FMath::Sin(A + I * 0.15f), FMath::Sin(A + X)); break;
	case EFCInfluence::Braid: V = FVector(FMath::Sin(A + Y), 0.f, FMath::Cos(A + (I % 3) * 2.094f)); break;
	case EFCInfluence::Twin: V = FVector(FMath::Sin(A) * ((I % 2) == 0 ? -1.f : 1.f), 0.f, FMath::Cos(A + X)); break;
	case EFCInfluence::Square: V = FVector(FMath::Sin(A + X) >= 0.f ? 1.f : -1.f, FMath::Sin(A + Y) >= 0.f ? 1.f : -1.f, 0.f); break;
	case EFCInfluence::Riemann: V = FVector(FMath::Sin(X * X - Y * Y + A), FMath::Cos(X * X + Y * Y - A), FMath::Sin(2.f * X * Y + A)); break;
	default: break;
	}
	return V * L.Strength * L.Blend;
}

FVector FFCFormationMath::Target(const FFCFleetConfig& C, int32 I, int32 N, float Time)
{
	FVector P = Point(C.Formation, I, N, C.Spacing);
	const float Beat = Time * C.Bpm / 60.f * 2.f * PI;
	switch (C.Pattern)
	{
	case EFCMotion::Orbit: P = FRotator(0.f, Time * 10.f, 0.f).RotateVector(P); break;
	case EFCMotion::Wave: P.Z += FMath::Sin(Time * 1.5f + P.X * 0.0009f) * 800.f; break;
	case EFCMotion::Pulse: P *= 1.f + FMath::Sin(Time) * 0.22f; break;
	case EFCMotion::Dance:
		P.Z += (FMath::Sin(Beat + I * 0.12f) + 1.f) * 300.f;
		P.Y += FMath::Sin(Beat * 0.25f + I * 0.1f) * 300.f;
		break;
	default: break;
	}
	P = FRotator(0.f, C.RotationDeg, 0.f).RotateVector(P) * C.Scale;
	FVector Field = FVector::ZeroVector;
	for (int32 L = 0; L < 4; ++L) Field += Influence(C.Layers[L], P, I, Time);
	if (Field.SizeSquared() > 3200.f * 3200.f) Field = Field.GetSafeNormal() * 3200.f;
	P += Field + C.Origin + FVector(0.f, 0.f, C.Height);
	return FCClampField(P);
}
