// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VoteForNewGame.generated.h"

/**
 * 
 */
UCLASS()
class TPSPROJECT_API UVoteForNewGame : public UUserWidget
{
	GENERATED_BODY()

public:

	virtual bool Initialize() override;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* VoteTime;

	UPROPERTY(meta = (BindWidget))
	class UButton* AgreeButton;

	UFUNCTION()
	void AgreeButtonPressed();

	UPROPERTY(meta = (BindWidget))
	UButton* DisagreeButton;

	UFUNCTION()
	void DisagreeButtonPressed();

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AgreeNum;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DisagreeNum;
};
