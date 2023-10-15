// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TPSProject/Types/PlayerReadyState.h"
#include "TPSProject/PlayerController/TPSController.h"
#include "PlayerList.generated.h"

/**
 * 
 */
UCLASS()
class TPSPROJECT_API UPlayerList : public UUserWidget
{
	GENERATED_BODY()

public:
	void UpdateWidget(const TArray<FPlayerStruct>& PlayerLists);
	
	UPROPERTY(meta = (BindWidget))
	class UScrollBox* PlayerListScrollBox;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UPlayerListItem> PlayerListItemClass;

	UPROPERTY(meta = (BindWidget))
	class UButton* ReadyButton;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ReadyText;
protected:
	virtual bool Initialize() override;
};
