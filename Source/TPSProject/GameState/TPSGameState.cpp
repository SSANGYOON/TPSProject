// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSGameState.h"
#include "Net/UnrealNetwork.h"
#include "TPSProject/PlayerState/TPSPlayerState.h"
#include "TPSProject/PlayerController/TPSController.h"

void ATPSGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATPSGameState, TopScoringPlayers);
	DOREPLIFETIME(ATPSGameState, RedTeamScore);
	DOREPLIFETIME(ATPSGameState, BlueTeamScore);
}

void ATPSGameState::UpdateTopScore(class ATPSPlayerState* ScoringPlayer)
{
	if (TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}

void ATPSGameState::RedTeamScores()
{
	RedTeamScore++;
	ATPSController* TPlayer = Cast<ATPSController>(GetWorld()->GetFirstPlayerController());
	if (TPlayer)
	{
		TPlayer->SetHUDRedTeamScore(RedTeamScore);
	}
}

void ATPSGameState::BlueTeamScores()
{
	BlueTeamScore++;
	ATPSController* TPlayer = Cast<ATPSController>(GetWorld()->GetFirstPlayerController());
	if (TPlayer)
	{
		TPlayer->SetHUDBlueTeamScore(BlueTeamScore);
	}
}

void ATPSGameState::OnRep_RedTeamScore()
{
	ATPSController* TPlayer = Cast<ATPSController>(GetWorld()->GetFirstPlayerController());
	if (TPlayer)
	{
		TPlayer->SetHUDRedTeamScore(RedTeamScore);
	}
}

void ATPSGameState::OnRep_BlueTeamScore()
{
	ATPSController* TPlayer = Cast<ATPSController>(GetWorld()->GetFirstPlayerController());
	if (TPlayer)
	{
		TPlayer->SetHUDBlueTeamScore(BlueTeamScore);
	}
}
