// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSController.h"
#include "TPSProject/HUD/TPSHUD.h"
#include "TPSProject/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "TPSProject/Character/TPSCharacter.h"

void ATPSController::BeginPlay()
{
	Super::BeginPlay();

	TPSHUD = Cast<ATPSHUD>(GetHUD());
}

void ATPSController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ATPSCharacter* TPSCharacter = Cast<ATPSCharacter>(InPawn);
	if (TPSCharacter)
	{
		SetHUDHealth(TPSCharacter->GetHealth(), TPSCharacter->GetMaxHealth());
	}
}

void ATPSController::SetHUDHealth(float Health, float MaxHealth)
{
	TPSHUD = TPSHUD == nullptr ? Cast<ATPSHUD>(GetHUD()) : TPSHUD;

	bool bHUDValid = TPSHUD && TPSHUD->CharacterOverlay && TPSHUD->CharacterOverlay->HealthBar && TPSHUD->CharacterOverlay->HealthText;
	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		TPSHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		FMath::CeilToInt(MaxHealth);
		TPSHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}

void ATPSController::SetHUDScore(float Score)
{
	TPSHUD = TPSHUD == nullptr ? Cast<ATPSHUD>(GetHUD()) : TPSHUD;
	bool bHUDValid = TPSHUD &&
		TPSHUD->CharacterOverlay &&
		TPSHUD->CharacterOverlay->ScoreAmount;
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		TPSHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
}

void ATPSController::SetHUDDefeats(int32 Defeats)
{
	TPSHUD = TPSHUD == nullptr ? Cast<ATPSHUD>(GetHUD()) : TPSHUD;
	bool bHUDValid = TPSHUD &&
		TPSHUD->CharacterOverlay &&
		TPSHUD->CharacterOverlay->DefeatsAmount;
	if (bHUDValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		TPSHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
}