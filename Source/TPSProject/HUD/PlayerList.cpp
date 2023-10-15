// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerList.h"
#include "TPSProject/GameState/LobbyGameState.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "TPSProject/HUD/PlayerListItem.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/Button.h"

void UPlayerList::UpdateWidget(const TArray<FPlayerStruct>& PlayerLists)
{
	PlayerListScrollBox->ClearChildren();
	int i = 0;
	for (auto& PlayerState : PlayerLists)
	{
		auto ListItem = CreateWidget<UPlayerListItem>(GetOwningPlayer(), PlayerListItemClass);
		ListItem->UsernameText->SetText(FText::FromString(PlayerState.PlayerName));
		FString StatusText;
		switch (PlayerState.PlayerReadyState)
		{
		case EPlayerReadyState::EPRS_Waiting:
			StatusText = "Waiting";
			break;
		case EPlayerReadyState::EPRS_Ready:
			StatusText = "Ready";
			break;
		case EPlayerReadyState::EPRS_Host:
			StatusText = "Host";
		{
			ListItem->KickPlayerButton->SetIsEnabled(false);
			FString TooolTip = "You cannot kick yourself";
			ListItem->KickPlayerButton->SetToolTipText(FText::FromString(TooolTip));
		}
		break;
		default:
			break;
		}
		ListItem->StatusText->SetText(FText::FromString(StatusText));
		PlayerListScrollBox->AddChild(ListItem);
		ListItem->PlayerId = i;
		i++;
	}
}

bool UPlayerList::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	ATPSController* TPSController = Cast<ATPSController>(GetOwningPlayer());
	if (TPSController)
	{
		if(UKismetSystemLibrary::IsServer(this))
		{
			ReadyButton->OnClicked.AddDynamic(TPSController, &ATPSController::TravelToBlasterMap);
		}
		else
		{
			ReadyButton->OnClicked.AddDynamic(TPSController, &ATPSController::ServerPlayerReady);
		}
	}

	return true;
}
