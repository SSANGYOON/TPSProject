// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSPlayerState.h"
#include "TPSProject/Character/TPSCharacter.h"
#include "TPSProject/PlayerController/TPSController.h"
#include "Net/UnrealNetwork.h"

void ATPSPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATPSPlayerState, Defeats);
}

void ATPSPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);
	Character = Character == nullptr ? Cast<ATPSCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Character->GetController();
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void ATPSPlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	Character = Character == nullptr ? Cast<ATPSCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Character->GetController();
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void ATPSPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;
	Character = Character == nullptr ? Cast<ATPSCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Character->GetController();
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void ATPSPlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<ATPSCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Character->GetController();
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}