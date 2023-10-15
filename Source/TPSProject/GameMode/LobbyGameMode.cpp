// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "TPSProject/GameState/LobbyGameState.h"
#include "MultiplayerSessionsSubsystem.h"
#include "TPSProject/PlayerController/TPSController.h"
#include "TPSProject/PlayerState/TPSPlayerState.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UMultiplayerSessionsSubsystem* Subsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		check(Subsystem);

		ATPSController* TPSController = Cast<ATPSController>(NewPlayer);
		TPSController->CreatePlayerListWidget();

		FTimerHandle PlayerUpdateHandle;
		FTimerDelegate PlayerUpdateDelegate;
		PlayerUpdateDelegate.BindUFunction(this, FName("UpdatePlayerList"));
		GetWorldTimerManager().SetTimer(PlayerUpdateHandle, PlayerUpdateDelegate, 0.3f, false);
	}
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	FTimerHandle PlayerUpdateHandle;
	FTimerDelegate PlayerUpdateDelegate;
	PlayerUpdateDelegate.BindUFunction(this, FName("UpdatePlayerList"));
	GetWorldTimerManager().SetTimer(PlayerUpdateHandle, PlayerUpdateDelegate, 0.3f, false);
}

void ALobbyGameMode::SendChat(const FString& Text, const FString& PlayerName)
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

void ALobbyGameMode::DefocusFromUI()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ATPSController* TPSPlayerController = Cast<ATPSController>(*It);
		if (TPSPlayerController)
		{
			TPSPlayerController->DefocusUI();
		}
	}
}

void ALobbyGameMode::UpdatePlayerList()
{
	TArray<FPlayerStruct> PlayerList;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ATPSController* TPSPlayerController = Cast<ATPSController>(*It);
		if (TPSPlayerController)
		{
			if (TPSPlayerController->GetRemoteRole() == ENetRole::ROLE_SimulatedProxy)
			{
				TPSPlayerController->PlayerStateStruct.PlayerReadyState = EPlayerReadyState::EPRS_Host;
			}
			ATPSPlayerState* PlayerState = TPSPlayerController->GetPlayerState<ATPSPlayerState>();

			if (PlayerState)
			{
				TPSPlayerController->PlayerStateStruct.PlayerName = PlayerState->GetPlayerName();
			}

			PlayerList.Add(TPSPlayerController->PlayerStateStruct);
		}
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ATPSController* TPSPlayerController = Cast<ATPSController>(*It);
		if (TPSPlayerController)
		{
			TPSPlayerController->UpdatePlayerList(PlayerList);
		}
	}
}

