// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "TPSGameState.generated.h"

/**
 * 
 */
UCLASS()
class TPSPROJECT_API ATPSGameState : public AGameState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateTopScore(class ATPSPlayerState* ScoringPlayer);

	UPROPERTY(Replicated)
	TArray<ATPSPlayerState*> TopScoringPlayers;

	void RedTeamScores();
	void BlueTeamScores();

	TArray<ATPSPlayerState*> RedTeam;
	TArray<ATPSPlayerState*> BlueTeam;

	UPROPERTY(ReplicatedUsing = OnRep_RedTeamScore)
	float RedTeamScore = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
	float BlueTeamScore = 0.f;

	UFUNCTION()
	void OnRep_RedTeamScore();

	UFUNCTION()
	void OnRep_BlueTeamScore();

	void NewGameAgree();
	void NewGameDisagree();

	UPROPERTY(ReplicatedUsing = OnRep_Agree)
	float RestartAgree = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_Disagree)
	float RestartDisagree = 0.f;

	UFUNCTION()
	void OnRep_Agree();

	UFUNCTION()
	void OnRep_Disagree();


private:
	float TopScore = 0.f;
};
