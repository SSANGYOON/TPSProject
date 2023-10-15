// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerListItem.generated.h"

/**
 * 
 */
UCLASS()
class TPSPROJECT_API UPlayerListItem : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* UsernameText;
	
	UPROPERTY(meta = (BindWidget))
	class UButton* KickPlayerButton;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* StatusText;

	uint8 PlayerId;

protected:
	virtual bool Initialize() override;

private:
	UFUNCTION()
	void KickPlayerButtonClicked();
	
};
