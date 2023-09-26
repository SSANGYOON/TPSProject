#pragma once

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),
	ECS_GrenadeStart UMETA(DisplayName = "GrenadeStart"),
	ECS_GrenadeAim UMETA(DisplayName = "GrenadeStart"),
	ECS_ThrowingGrenade UMETA(DisplayName = "Throwing Grenade"),

	ECS_MAX UMETA(DisplayName = "DefaultMAX")
};
