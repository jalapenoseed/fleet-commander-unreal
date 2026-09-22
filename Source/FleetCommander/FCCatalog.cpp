#include "FCCatalog.h"

FFCFrameProfile FFCCatalog::Frame(EFCFrame Kind)
{
	FFCFrameProfile P;
	switch (Kind)
	{
	case EFCFrame::Relay:   P.Name = TEXT("Relay");   P.Speed = 0.90f; P.Agility = 0.82f; P.Armor = 1.12f; P.Energy = 0.88f; P.Mass = 1.10f; break;
	case EFCFrame::Cargo:   P.Name = TEXT("Cargo");   P.Speed = 0.65f; P.Agility = 0.62f; P.Armor = 1.55f; P.Energy = 1.35f; P.Mass = 1.75f; break;
	case EFCFrame::Utility: P.Name = TEXT("Utility"); P.Speed = 0.85f; P.Agility = 0.90f; P.Armor = 1.25f; P.Energy = 1.08f; P.Mass = 1.28f; break;
	default:                P.Name = TEXT("Scout");   P.Speed = 1.15f; P.Agility = 1.25f; P.Armor = 0.88f; P.Energy = 0.92f; P.Mass = 0.84f; break;
	}
	return P;
}

FFCWeaponProfile FFCCatalog::Weapon(EFCWeapon Kind)
{
	FFCWeaponProfile W;
	switch (Kind)
	{
	case EFCWeapon::RapidFire:      W.Range = 3200; W.Damage = 0.40f; W.Interval = 0.27f; W.Cone = 0.986f; W.Targets = 1; W.Magazine = 60; W.Reserve = 240; W.Energy = 0.0003f; W.Heat = 0.035f; W.Reload = 2.1f; break;
	case EFCWeapon::Scatter:        W.Range = 2300; W.Damage = 0.74f; W.Interval = 1.45f; W.Cone = 0.86f;  W.Targets = 3; W.Magazine = 8;  W.Reserve = 48;  W.Energy = 0.001f;  W.Heat = 0.18f;  W.Reload = 2.5f; break;
	case EFCWeapon::Shockwave:      W.Range = 1500; W.Damage = 1.70f; W.Interval = 2.70f; W.Cone = -1.f;   W.Targets = 256; W.Magazine = 4; W.Reserve = 16; W.Energy = 0.003f; W.Heat = 0.32f; W.Reload = 3.5f; break;
	case EFCWeapon::PrecisionBeam:  W.Range = 6400; W.Damage = 2.80f; W.Interval = 2.20f; W.Cone = 0.999f; W.Targets = 1; W.Magazine = 6;  W.Reserve = 36;  W.Energy = 0.002f;  W.Heat = 0.24f;  W.Reload = 2.8f; break;
	case EFCWeapon::IonDisruptor:   W.Range = 2800; W.Damage = 0.60f; W.Interval = 1.70f; W.Cone = 0.993f; W.Targets = 1; W.Magazine = 12; W.Reserve = 60;  W.Energy = 0.002f;  W.Heat = 0.16f;  W.Reload = 2.4f; break;
	case EFCWeapon::BurstTagger:    W.Range = 3500; W.Damage = 0.62f; W.Interval = 0.38f; W.Cone = 0.99f;  W.Targets = 1; W.Magazine = 30; W.Reserve = 150; W.Energy = 0.0005f; W.Heat = 0.055f; W.Reload = 2.2f; break;
	case EFCWeapon::ArcLink:        W.Range = 2100; W.Damage = 0.55f; W.Interval = 1.40f; W.Cone = 0.90f;  W.Targets = 3; W.Magazine = 15; W.Reserve = 75;  W.Energy = 0.002f;  W.Heat = 0.20f;  W.Reload = 2.6f; break;
	case EFCWeapon::Repulsor:       W.Range = 1800; W.Damage = 0.50f; W.Interval = 1.80f; W.Cone = 0.80f;  W.Targets = 3; W.Magazine = 10; W.Reserve = 50;  W.Energy = 0.001f;  W.Heat = 0.14f;  W.Reload = 2.1f; break;
	case EFCWeapon::DrainRay:       W.Range = 2400; W.Damage = 0.30f; W.Interval = 0.60f; W.Cone = 0.992f; W.Targets = 1; W.Magazine = 40; W.Reserve = 160; W.Energy = 0.0004f; W.Heat = 0.055f; W.Reload = 2.3f; break;
	default:                       W.Range = 3800; W.Damage = 1.25f; W.Interval = 1.00f; W.Cone = 0.99f;  W.Targets = 1; break;
	}
	return W;
}

FLinearColor FFCCatalog::SkinColor(EFCSkin Kind)
{
	switch (Kind)
	{
	case EFCSkin::Arctic:     return FLinearColor(0.78f, 0.84f, 0.89f);
	case EFCSkin::Desert:     return FLinearColor(0.65f, 0.48f, 0.29f);
	case EFCSkin::Crimson:    return FLinearColor(0.55f, 0.075f, 0.065f);
	case EFCSkin::Cobalt:     return FLinearColor(0.055f, 0.20f, 0.55f);
	case EFCSkin::Industrial: return FLinearColor(0.94f, 0.57f, 0.045f);
	default:                  return FLinearColor(0.11f, 0.14f, 0.18f);
	}
}

FLinearColor FFCCatalog::TeamColor(int32 FleetId)
{
	return FleetId == 0 ? FLinearColor(0.15f, 0.78f, 0.86f) : FLinearColor(0.92f, 0.42f, 0.16f);
}

FVector FFCCatalog::FrameBodyScale(EFCFrame Kind)
{
	switch (Kind)
	{
	case EFCFrame::Relay:   return FVector(42.f, 28.f, 14.f);
	case EFCFrame::Cargo:   return FVector(58.f, 38.f, 22.f);
	case EFCFrame::Utility: return FVector(46.f, 32.f, 16.f);
	default:                return FVector(36.f, 24.f, 10.f);
	}
}

float FFCCatalog::PlanetGravity(EFCPlanet Planet)
{
	return Planet == EFCPlanet::Moon ? 162.f : Planet == EFCPlanet::Mars ? 373.f : 981.f;
}

float FFCCatalog::PlanetDensity(EFCPlanet Planet)
{
	return Planet == EFCPlanet::Moon ? 0.f : Planet == EFCPlanet::Mars ? 0.016f : 1.225f;
}

const TCHAR* FFCCatalog::FrameName(EFCFrame Kind)
{
	switch (Kind)
	{
	case EFCFrame::Relay: return TEXT("Relay");
	case EFCFrame::Cargo: return TEXT("Cargo");
	case EFCFrame::Utility: return TEXT("Utility");
	default: return TEXT("Scout");
	}
}

const TCHAR* FFCCatalog::WeaponName(EFCWeapon Kind)
{
	switch (Kind)
	{
	case EFCWeapon::RapidFire: return TEXT("Rapid Fire");
	case EFCWeapon::Scatter: return TEXT("Scatter");
	case EFCWeapon::Shockwave: return TEXT("Shockwave");
	case EFCWeapon::PrecisionBeam: return TEXT("Precision Beam");
	case EFCWeapon::IonDisruptor: return TEXT("Ion Disruptor");
	case EFCWeapon::BurstTagger: return TEXT("Burst Tagger");
	case EFCWeapon::ArcLink: return TEXT("Arc Link");
	case EFCWeapon::Repulsor: return TEXT("Repulsor");
	case EFCWeapon::DrainRay: return TEXT("Drain Ray");
	default: return TEXT("Pulse");
	}
}

const TCHAR* FFCCatalog::FormationName(EFCFormation Kind)
{
	switch (Kind)
	{
	case EFCFormation::Ring: return TEXT("Ring");
	case EFCFormation::Wedge: return TEXT("Wedge");
	case EFCFormation::Line: return TEXT("Line");
	case EFCFormation::Column: return TEXT("Column");
	case EFCFormation::DoubleOrbit: return TEXT("Double Orbit");
	case EFCFormation::Scatter: return TEXT("Scatter");
	case EFCFormation::Staggered: return TEXT("Staggered");
	case EFCFormation::HighLow: return TEXT("High / Low");
	case EFCFormation::Overwatch: return TEXT("Overwatch");
	case EFCFormation::Helix: return TEXT("Helix");
	case EFCFormation::Sphere: return TEXT("Sphere");
	case EFCFormation::Heart: return TEXT("Heart");
	case EFCFormation::Art: return TEXT("Art");
	default: return TEXT("Grid");
	}
}

const TCHAR* FFCCatalog::CameraName(EFCCamera Kind)
{
	switch (Kind)
	{
	case EFCCamera::Top: return TEXT("Top");
	case EFCCamera::Front: return TEXT("Front");
	case EFCCamera::Follow: return TEXT("Follow");
	case EFCCamera::FPV: return TEXT("FPV");
	case EFCCamera::Shoulder: return TEXT("Shoulder");
	case EFCCamera::Mounted: return TEXT("Mounted");
	case EFCCamera::Ground: return TEXT("Walk");
	case EFCCamera::Free: return TEXT("Free");
	case EFCCamera::Cinematic: return TEXT("Cinematic");
	case EFCCamera::Action: return TEXT("Action");
	case EFCCamera::BestFight: return TEXT("Best Fight");
	case EFCCamera::Survivor: return TEXT("Survivor");
	default: return TEXT("Orbit");
	}
}
