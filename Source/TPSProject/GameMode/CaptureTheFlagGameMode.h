// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TeamsGameMode.h"
#include "CaptureTheFlagGameMode.generated.h"

/**
 * 
 */
UCLASS()
class TPSPROJECT_API ACaptureTheFlagGameMode : public ATeamsGameMode
{
	GENERATED_BODY()
public:
	virtual void PlayerEliminated(class ATPSCharacter* ElimmedCharacter, class ATPSController* VictimController, ATPSController* AttackerController) override;
	void FlagCaptured(class AFlag* Flag, class AFlagZone* Zone);
};
