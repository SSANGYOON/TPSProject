// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TPSGameMode.generated.h"

namespace MatchState
{
	extern TPSPROJECT_API const FName Cooldown; // 매치시간 종료 후 도달하는 state
}

/**
 * 
 */
UCLASS()
class TPSPROJECT_API ATPSGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	ATPSGameMode();
	virtual void Tick(float DeltaTime) override;
	virtual void PlayerEliminated(class ATPSCharacter* ElimmedCharacter, class ATPSController* VictimController, ATPSController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);

	/*매치 시작 전 대기시간*/
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

	float LevelStartingTime = 0.f;

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

private:
	float CountdownTime = 0.f;
};
