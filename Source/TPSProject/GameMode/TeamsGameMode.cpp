// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamsGameMode.h"
#include "TPSProject/GameState/TPSGameState.h"
#include "TPSProject/PlayerState/TPSPlayerState.h"
#include "TPSProject/PlayerController/TPSController.h"
#include "Kismet/GameplayStatics.h"

ATeamsGameMode::ATeamsGameMode()
{
	bTeamsMatch = true;
}

void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ATPSGameState* TGameState = Cast<ATPSGameState>(UGameplayStatics::GetGameState(this));
	if (TGameState)
	{
		ATPSPlayerState* TState = NewPlayer->GetPlayerState<ATPSPlayerState>();
		if (TState && TState->GetTeam() == ETeam::ET_NoTeam)
		{
			if (TGameState->BlueTeam.Num() >= TGameState->RedTeam.Num())
			{
				TGameState->RedTeam.AddUnique(TState);
				TState->SetTeam(ETeam::ET_RedTeam);
			}
			else
			{
				TGameState->BlueTeam.AddUnique(TState);
				TState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}

void ATeamsGameMode::Logout(AController* Exiting)
{
	ATPSGameState* TGameState = Cast<ATPSGameState>(UGameplayStatics::GetGameState(this));
	ATPSPlayerState* TPState = Exiting->GetPlayerState<ATPSPlayerState>();
	if (TGameState && TPState)
	{
		if (TGameState->RedTeam.Contains(TPState))
		{
			TGameState->RedTeam.Remove(TPState);
		}
		if (TGameState->BlueTeam.Contains(TPState))
		{
			TGameState->BlueTeam.Remove(TPState);
		}
	}
}

void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	ATPSGameState* TGameState = Cast<ATPSGameState>(UGameplayStatics::GetGameState(this));
	if (TGameState)
	{
		for (auto PState : TGameState->PlayerArray)
		{
			ATPSPlayerState* TPState = Cast<ATPSPlayerState>(PState.Get());
			if (TPState && TPState->GetTeam() == ETeam::ET_NoTeam)
			{
				if (TGameState->BlueTeam.Num() >= TGameState->RedTeam.Num())
				{
					TGameState->RedTeam.AddUnique(TPState);
					TPState->SetTeam(ETeam::ET_RedTeam);
				}
				else
				{
					TGameState->BlueTeam.AddUnique(TPState);
					TPState->SetTeam(ETeam::ET_BlueTeam);
				}
			}
		}
	}
}

float ATeamsGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	ATPSPlayerState *AttackerPState = Attacker->GetPlayerState<ATPSPlayerState>();
	ATPSPlayerState* VictimPState = Victim->GetPlayerState<ATPSPlayerState>();
	if (AttackerPState == nullptr || VictimPState == nullptr) return BaseDamage;
	if (VictimPState == AttackerPState)
	{
		return BaseDamage;
	}
	if (AttackerPState->GetTeam() == VictimPState->GetTeam())
	{
		return 0.f;
	}
	return BaseDamage;
}

void ATeamsGameMode::PlayerEliminated(ATPSCharacter* ElimmedCharacter, ATPSController* VictimController, ATPSController* AttackerController)
{
	Super::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);

	ATPSGameState* TGameState = Cast<ATPSGameState>(UGameplayStatics::GetGameState(this));
	ATPSPlayerState* AttackerPlayerState = AttackerController ? Cast<ATPSPlayerState>(AttackerController->PlayerState) : nullptr;
	if (TGameState && AttackerPlayerState)
	{
		if (AttackerPlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{
			TGameState->BlueTeamScores();
		}
		if (AttackerPlayerState->GetTeam() == ETeam::ET_RedTeam)
		{
			TGameState->RedTeamScores();
		}
	}
}
