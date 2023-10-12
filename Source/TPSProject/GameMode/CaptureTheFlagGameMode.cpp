// Fill out your copyright notice in the Description page of Project Settings.


#include "CaptureTheFlagGameMode.h"
#include "TPSProject/Weapon/Flag.h"
#include "TPSProject/CaptureTheFlag/FlagZone.h"
#include "TPSProject/GameState/TPSGameState.h"

void ACaptureTheFlagGameMode::PlayerEliminated(ATPSCharacter* ElimmedCharacter, ATPSController* VictimController, ATPSController* AttackerController)
{
	ATPSGameMode::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);

}

void ACaptureTheFlagGameMode::FlagCaptured(AFlag* Flag, AFlagZone* Zone)
{
	bool bValidCapture = Flag->GetTeam() != Zone->Team;
	ATPSGameState* TGameState = Cast<ATPSGameState>(GameState);
	if (TGameState)
	{
		if (Zone->Team == ETeam::ET_BlueTeam)
		{
			TGameState->BlueTeamScores();
		}
		if (Zone->Team == ETeam::ET_RedTeam)
		{
			TGameState->RedTeamScores();
		}
	}
}
