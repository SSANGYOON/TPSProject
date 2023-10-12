// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSGameMode.h"
#include "TPSProject/Character/TPSCharacter.h"
#include "TPSProject/PlayerController/TPSController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "TPSProject/PlayerState/TPSPlayerState.h"
#include "TPSProject/GameState/TPSGameState.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
	const FName VoteForNewGame = FName("VoteForNewGame");
}

ATPSGameMode::ATPSGameMode()
{
	//WaitingToStart로 시작
	bDelayedStart = true;
}

void ATPSGameMode::SendChat(const FString& Text, const FString& PlayerName)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ATPSController* TPSPlayerController = Cast<ATPSController>(*It);
		if (TPSPlayerController)
		{
			TPSPlayerController->ClientSetText(Text, PlayerName);
		}
	}
}

void ATPSGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void ATPSGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			StartMatch(); // Inprogress로 전환
		}
	}

	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::VoteForNewGame);
		}
	}

	else if (MatchState == MatchState::VoteForNewGame)
	{
		CountdownTime = VoteTime + CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			if (GetGameState<ATPSGameState>()->RestartAgree == GetWorld()->GetNumPlayerControllers())
			{
				RestartGame();
			}
			else
			{
				ReturnToMainMenuHost();
			}
		}
	}
}

void ATPSGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ATPSController* TPSController = Cast<ATPSController>(*It);
		if (TPSController)
		{
			TPSController->OnMatchStateSet(MatchState, bTeamsMatch);
		}
	}
}

void ATPSGameMode::PlayerEliminated(ATPSCharacter* ElimmedCharacter, ATPSController* VictimController, ATPSController* AttackerController)
{
	if (AttackerController == nullptr || AttackerController->PlayerState == nullptr) return;
	if (VictimController == nullptr || VictimController->PlayerState == nullptr) return;
	ATPSPlayerState* AttackerPlayerState = AttackerController ? Cast<ATPSPlayerState>(AttackerController->PlayerState) : nullptr;
	ATPSPlayerState* VictimPlayerState = VictimController ? Cast<ATPSPlayerState>(VictimController->PlayerState) : nullptr;

	ATPSGameState* TPSGameState = GetGameState<ATPSGameState>();

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && TPSGameState)
	{
		TArray<ATPSPlayerState*> PlayersCurrentlyInTheLead;
		for (auto LeadPlayer : TPSGameState->TopScoringPlayers)
		{
			PlayersCurrentlyInTheLead.Add(LeadPlayer);
		}
		AttackerPlayerState->AddToScore(1.f);
		TPSGameState->UpdateTopScore(AttackerPlayerState);
		if (TPSGameState->TopScoringPlayers.Contains(AttackerPlayerState))
		{
			ATPSCharacter* Leader = Cast<ATPSCharacter>(AttackerPlayerState->GetPawn());
			if (Leader)
			{
				Leader->MulticastGainedTheLead();
			}
		}

		for (int32 i = 0; i < PlayersCurrentlyInTheLead.Num(); i++)
		{
			if (!TPSGameState->TopScoringPlayers.Contains(PlayersCurrentlyInTheLead[i]))
			{
				ATPSCharacter* Loser = Cast<ATPSCharacter>(PlayersCurrentlyInTheLead[i]->GetPawn());
				if (Loser)
				{
					Loser->MulticastLostTheLead();
				}
			}
		}
	}
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim(false);
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ATPSController* TPSPlayer = Cast<ATPSController>(*It);
		if (TPSPlayer && AttackerPlayerState && VictimPlayerState)
		{
			TPSPlayer->BroadcastElim(AttackerPlayerState, VictimPlayerState);
		}
	}
}

void ATPSGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}

void ATPSGameMode::PlayerLeftGame(ATPSPlayerState* PlayerLeaving)
{
	if (PlayerLeaving == nullptr) return;
	ATPSGameState* TPSGameState = GetGameState<ATPSGameState>();
	if (TPSGameState && TPSGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		TPSGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}
	ATPSCharacter* CharacterLeaving = Cast<ATPSCharacter>(PlayerLeaving->GetPawn());
	if (CharacterLeaving)
	{
		CharacterLeaving->Elim(true);
	}
}

float ATPSGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	return BaseDamage;
}
