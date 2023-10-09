// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TPSGameMode.h"
#include "TeamsGameMode.generated.h"

/**
 * 
 */
UCLASS()
class TPSPROJECT_API ATeamsGameMode : public ATPSGameMode
{
	GENERATED_BODY()
public:
	ATeamsGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage) override;
	virtual void PlayerEliminated(class ATPSCharacter* ElimmedCharacter, class ATPSController* VictimController, ATPSController* AttackerController) override;

protected:
	virtual void HandleMatchHasStarted() override;
	
};
