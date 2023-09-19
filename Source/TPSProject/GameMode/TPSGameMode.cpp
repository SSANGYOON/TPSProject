// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSGameMode.h"
#include "TPSProject/Character/TPSCharacter.h"
#include "TPSProject/PlayerController/TPSController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "TPSProject/PlayerState/TPSPlayerState.h"

void ATPSGameMode::PlayerEliminated(ATPSCharacter* ElimmedCharacter, ATPSController* VictimController, ATPSController* AttackerController)
{
	if (AttackerController == nullptr || AttackerController->PlayerState == nullptr) return;
	if (VictimController == nullptr || VictimController->PlayerState == nullptr) return;
	ATPSPlayerState* AttackerPlayerState = AttackerController ? Cast<ATPSPlayerState>(AttackerController->PlayerState) : nullptr;
	ATPSPlayerState* VictimPlayerState = VictimController ? Cast<ATPSPlayerState>(VictimController->PlayerState) : nullptr;

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);
	}
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
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
