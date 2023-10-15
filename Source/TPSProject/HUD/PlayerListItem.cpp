// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerListItem.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/Button.h"
#include "TPSProject/GameState/LobbyGameState.h"
#include "Kismet/GameplayStatics.h"
#include "TPSProject/PlayerController/TPSController.h"
#include "GameFramework/PlayerState.h"

bool UPlayerListItem::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	//Make sure host sees kick buttons
	if (UKismetSystemLibrary::IsServer(this))
	{
		KickPlayerButton->SetVisibility(ESlateVisibility::Visible);
		KickPlayerButton->OnClicked.AddDynamic(this, &UPlayerListItem::KickPlayerButtonClicked);
	}
	return true;
}

void UPlayerListItem::KickPlayerButtonClicked()
{
	auto GameState = UGameplayStatics::GetGameState(this);

	if (GameState)
	{
		ATPSController* TPSController = Cast<ATPSController>(GameState->PlayerArray[PlayerId]->GetOwningController());
		TPSController->Kicked();
	}
}
