// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"
#include "TPSProject/Character/TPSCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TPSProject/TPSComponent/CombatComponent.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	bHealing = true;
	HealingRate = HealAmount / HealingTime;
	AmountToHeal += HealAmount;
}

void UBuffComponent::ReplenishShield(float ShieldAmount, float ReplenishTime)
{
	bReplenishingShield = true;
	ShieldReplenishRate = ShieldAmount / ReplenishTime;
	ShieldReplenishAmount += ShieldAmount;
}

void UBuffComponent::HealRampUp(float DeltaTime)
{
	if (!bHealing || Character == nullptr || Character->IsElimmed()) return;

	const float HealThisFrame = HealingRate * DeltaTime;
	Character->SetHealth(FMath::Clamp(Character->GetHealth() + HealThisFrame, 0.f, Character->GetMaxHealth()));
	Character->UpdateHUDHealth();
	AmountToHeal -= HealThisFrame;

	if (AmountToHeal <= 0.f || Character->GetHealth() >= Character->GetMaxHealth())
	{
		bHealing = false;
		AmountToHeal = 0.f;
	}
}

void UBuffComponent::ShieldRampUp(float DeltaTime)
{
	if (!bReplenishingShield || Character == nullptr || Character->IsElimmed()) return;

	const float ReplenishThisFrame = ShieldReplenishRate * DeltaTime;
	Character->SetShield(FMath::Clamp(Character->GetShield() + ReplenishThisFrame, 0.f, Character->GetMaxShield()));
	Character->UpdateHUDShield();
	ShieldReplenishAmount -= ReplenishThisFrame;

	if (ShieldReplenishAmount <= 0.f || Character->GetShield() >= Character->GetMaxShield())
	{
		bReplenishingShield = false;
		ShieldReplenishAmount = 0.f;
	}
}

void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UBuffComponent::SetInitialSpeeds(float BaseSpeed, float BaseAimSpeed, float CrouchSpeed, float CrouchAimSpeed)
{
	InitialBaseSpeed = BaseSpeed;
	InitialBaseAimSpeed = BaseAimSpeed;
	InitialCrouchSpeed = CrouchSpeed;
	InitialCrouchAimSpeed = CrouchAimSpeed;
}

void UBuffComponent::SetInitialJumpVelocity(float Velocity)
{
	InitialJumpVelocity = Velocity;
}

void UBuffComponent::BuffSpeed(float BuffBaseSpeed, float BuffAimSpeed, float BuffCrouchSpeed, float BuffAimCrouchSpeed, float BuffTime)
{
	if (Character == nullptr) return;

	Character->GetWorldTimerManager().SetTimer(
		SpeedBuffTimer,
		this,
		&UBuffComponent::ResetSpeeds,
		BuffTime
	);

	if (Character->GetCharacterMovement())
	{
		UCombatComponent* Combat = Character->GetCombat();
		Combat->BaseWalkSpeed = BuffBaseSpeed;
		Combat->AimWalkSpeed = BuffAimSpeed;
		Combat->CrouchWalkSpeed = BuffCrouchSpeed;
		Combat->CrouchAimWalkSpeed = BuffAimCrouchSpeed;

		if (Combat->bAiming)
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = BuffAimSpeed;
			Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BuffAimCrouchSpeed;
		}
		else
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = BuffBaseSpeed;
			Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BuffCrouchSpeed;
		}
		
	}
	MulticastSpeedBuff(BuffBaseSpeed, BuffAimSpeed, BuffCrouchSpeed, BuffAimCrouchSpeed);
}

void UBuffComponent::ResetSpeeds()
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;

	if (Character->GetCharacterMovement())
	{
		UCombatComponent* Combat = Character->GetCombat();
		Combat->BaseWalkSpeed = InitialBaseSpeed;
		Combat->AimWalkSpeed = InitialBaseAimSpeed;
		Combat->CrouchWalkSpeed = InitialCrouchSpeed;
		Combat->CrouchAimWalkSpeed = InitialCrouchAimSpeed;

		if (Combat->bAiming)
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = InitialBaseAimSpeed;
			Character->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchAimSpeed;
		}
		else
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = InitialBaseSpeed;
			Character->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpeed;
		}

	}
	MulticastSpeedBuff(InitialBaseSpeed, InitialBaseAimSpeed,InitialCrouchSpeed, InitialCrouchAimSpeed);
}

void UBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float BaseAimSpeed, float CrouchSpeed, float CrouchAimSpeed)
{
	if (Character->GetCharacterMovement())
	{
		UCombatComponent* Combat = Character->GetCombat();
		Combat->BaseWalkSpeed = BaseSpeed;
		Combat->AimWalkSpeed = BaseAimSpeed;
		Combat->CrouchWalkSpeed = CrouchSpeed;
		Combat->CrouchAimWalkSpeed = CrouchAimSpeed;

		if (Combat->bAiming)
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = BaseAimSpeed;
			Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchAimSpeed;
		}
		else
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
			Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
		}
	}
}

void UBuffComponent::BuffJump(float BuffJumpVelocity, float BuffTime)
{
	if (Character == nullptr) return;

	Character->GetWorldTimerManager().SetTimer(
		JumpBuffTimer,
		this,
		&UBuffComponent::ResetJump,
		BuffTime
	);

	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = BuffJumpVelocity;
	}
	MulticastJumpBuff(BuffJumpVelocity);
}

void UBuffComponent::MulticastJumpBuff_Implementation(float JumpVelocity)
{
	if (Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = JumpVelocity;
	}
}

void UBuffComponent::ResetJump()
{
	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = InitialJumpVelocity;
	}
	MulticastJumpBuff(InitialJumpVelocity);
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	HealRampUp(DeltaTime);
	ShieldRampUp(DeltaTime);
}

