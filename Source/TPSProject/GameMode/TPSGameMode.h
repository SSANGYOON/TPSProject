// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TPSGameMode.generated.h"

/**
 * 
 */
UCLASS()
class TPSPROJECT_API ATPSGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	virtual void PlayerEliminated(class ATPSCharacter* ElimmedCharacter, class ATPSController* VictimController, ATPSController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);
};
