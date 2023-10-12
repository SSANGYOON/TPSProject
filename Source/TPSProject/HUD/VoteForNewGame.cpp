// Fill out your copyright notice in the Description page of Project Settings.


#include "VoteForNewGame.h"
#include "Components/Button.h"
#include "TPSProject/PlayerController/TPSController.h"

bool UVoteForNewGame::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (AgreeButton)
	{
		AgreeButton->OnClicked.AddDynamic(this, &UVoteForNewGame::AgreeButtonPressed);
	}
	if (DisagreeButton)
	{
		DisagreeButton->OnClicked.AddDynamic(this, &UVoteForNewGame::DisagreeButtonPressed);
	}

	return true;
}

void UVoteForNewGame::AgreeButtonPressed()
{
	AgreeButton->SetIsEnabled(false);
	DisagreeButton->SetIsEnabled(false);

	ATPSController* TPSController = Cast<ATPSController>(GetOwningPlayer());

	FInputModeGameOnly InputMode;
	TPSController->SetInputMode(InputMode);
	TPSController->SetShowMouseCursor(false);

	TPSController->ServerNewGameAgree();
}

void UVoteForNewGame::DisagreeButtonPressed()
{
	AgreeButton->SetIsEnabled(false);
	DisagreeButton->SetIsEnabled(false);

	ATPSController* TPSController = Cast<ATPSController>(GetOwningPlayer());

	FInputModeGameOnly InputMode;
	TPSController->SetInputMode(InputMode);
	TPSController->SetShowMouseCursor(false);

	TPSController->ServerNewGameDisagree();
}
