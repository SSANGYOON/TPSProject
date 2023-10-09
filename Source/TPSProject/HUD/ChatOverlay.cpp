// Fill out your copyright notice in the Description page of Project Settings.


#include "ChatOverlay.h"
#include "ChatBox.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"

void UChatOverlay::SetChatText(const FString& Text, const FString& PlayerName)
{
    const FString Chat = PlayerName + " : " + Text;


    OwningPlayer = OwningPlayer == nullptr ? GetOwningPlayer() : OwningPlayer;
    if (OwningPlayer)
    {
        UTextBlock* NewTextBlock = NewObject<UTextBlock>(InputScrollBox);
        if (InputScrollBox && NewTextBlock)
        {
            NewTextBlock->SetText(FText::FromString(Chat));

            InputScrollBox->AddChild(NewTextBlock);
            InputScrollBox->ScrollToEnd();
            InputScrollBox->bAnimateWheelScrolling = true;
        }
    }
}