// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatOverlay.generated.h"

/**
 * 
 */
UCLASS()
class TPSPROJECT_API UChatOverlay : public UUserWidget
{
    GENERATED_BODY()
public:
    void SetChatText(const FString& Text, const FString& PlayerName);
    UPROPERTY(meta = (BindWidget))
    class UScrollBox* InputScrollBox;
    UPROPERTY(meta = (BindWidget))
    class UEditableText* InputTextBox;

private:
    UPROPERTY()
    class APlayerController* OwningPlayer;
};
