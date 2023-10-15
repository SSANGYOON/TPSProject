// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TPSProject/Types/PlayerReadyState.h"
#include "TPSController.generated.h"

UENUM()
enum class EPlayerReadyState : uint8
{
	EPRS_Waiting UMETA(DisplayName = "Waiting"),
	EPRS_Ready UMETA(DisplayName = "Ready"),
	EPRS_Host UMETA(DisplayName = "Host"),
	EPRS_MAX UMETA(DisplayName = "DefaultMAX")
};

USTRUCT(BlueprintType)
struct FPlayerStruct
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FString PlayerName;

	UPROPERTY()
	EPlayerReadyState PlayerReadyState = EPlayerReadyState::EPRS_Waiting;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);

UCLASS()
class TPSPROJECT_API ATPSController : public APlayerController
{
	GENERATED_BODY()
public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDAnnouncementCountdown(float CountdownTime);
	void SetHUDVoteCountDown(float CountdownTime);
	void SetHUDGrenades(int32 Grenades);
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void HideTeamScores();
	void InitTeamScores();
	void SetHUDRedTeamScore(int32 RedScore);
	void SetHUDBlueTeamScore(int32 BlueScore);

	void SetHUDNewGameAgree(int32 Agree);
	void SetHUDNewGameDisagree(int32 Disagree);

	virtual float GetServerTime(); //서버 시간과 동기화
	virtual void ReceivedPlayer() override; // 플레이어 접속시 실행 서버 시간 동기화
	void OnMatchStateSet(FName State, bool bTeamsMatch = false);
	void HandleMatchHasStarted(bool bTeamsMatch = false);
	void HandleCooldown();
	void StartVote();

	float SingleTripTime = 0.f;

	FHighPingDelegate HighPingDelegate;

	void BroadcastElim(APlayerState* Attacker, APlayerState* Victim);

	UFUNCTION(Client, Reliable)
	void CreatePlayerListWidget();

	UFUNCTION(Client, Reliable)
	void UpdatePlayerList(const TArray<FPlayerStruct>& PlayerList);

	UFUNCTION(Client, Reliable)
	void Kicked();

	FPlayerStruct PlayerStateStruct;

	UFUNCTION(Server, Reliable)
	void ServerPlayerReady();

	UFUNCTION()
	void TravelToBlasterMap();

	UFUNCTION(Client, Reliable)
	void DefocusUI();

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float Cooldown, float Vote, float StartingTime);
protected:
	virtual void BeginPlay() override;
	void SetHUDTime();
	void PollInit();
	virtual void SetupInputComponent() override;
	
	/*
	서버 - 클라이언트 동기화
	*/

	/*
	* 서버에 현재시간 요청 클라이언트의 요청시간을 매개변수로 보냄
	*/
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	// 요청받은 서버시간을 클라이언트에 보냄
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	float ClientServerDelta = 0.f; // 클라이언트 서버 간 시간 차이

	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;
	void CheckTimeSync(float DeltaTime);

	void HighPingWarning();
	void StopHighPingWarning();
	void CheckPing(float DeltaTime);

	void ShowReturnToMainMenu();

	UFUNCTION(Client, Reliable)
	void ClientElimAnnouncement(APlayerState* Attacker, APlayerState* Victim);

	UPROPERTY(ReplicatedUsing = OnRep_ShowTeamScores)
	bool bShowTeamScores = false;

	UFUNCTION()
	void OnRep_ShowTeamScores();

	FString GetInfoText(const TArray<class ATPSPlayerState*>& Players);
	FString GetTeamsInfoText(class ATPSGameState* TPSGameState);

public:
	void AddChat();

	UFUNCTION()
	void EnterKeyPressed();

	UFUNCTION()
	void OnTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION(Server, Reliable)
	void ServerSetText(const FString& Text, const FString& PlayerName); //서버로 내가 쓴 채팅을 보냄
	UFUNCTION(Client, Reliable)
	void ClientSetText(const FString& Text, const FString& PlayerName); //클라이언트에 다른 사람이 쓴 채팅을 보냄

	UFUNCTION(Server, Reliable)
	void ServerNewGameAgree();

	UFUNCTION(Server, Reliable)
	void ServerNewGameDisagree();

private:
	UPROPERTY(EditAnywhere, Category = HUD)
	TSubclassOf<class UChatOverlay> ChatOverlayClass;

	UPROPERTY()
	UChatOverlay* ChatWidget;

	UPROPERTY()
	class ATPSHUD* TPSHUD;

	UPROPERTY()
	class ATPSGameMode* TPSGameMode;

	UPROPERTY(EditAnywhere, Category = HUD)
	TSubclassOf<class UUserWidget> ReturnToMainMenuWidget;

	UPROPERTY()
	class UReturnToMainMenu* ReturnToMainMenu;

	bool bReturnToMainMenuOpen = false;

	float LevelStartingTime = 0.f;
	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float CooldownTime = 0.f;
	float VoteTime = 0.f;
	uint32 CountdownInt = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	float HUDHealth;
	bool bInitializeHealth = false;
	float HUDMaxHealth;
	float HUDScore;
	bool bInitializeScore = false;
	int32 HUDDefeats;
	bool bInitializeDefeats = false;
	int32 HUDGrenades;
	bool bInitializeGrenades = false;
	float HUDShield;
	bool bInitializeShield = false;
	float HUDMaxShield;
	float HUDCarriedAmmo;
	bool bInitializeCarriedAmmo = false;
	float HUDWeaponAmmo;
	bool bInitializeWeaponAmmo = false;

	float HighPingRunningTime = 0.f;

	UPROPERTY(EditAnywhere)
	float HighPingDuration = 5.f;

	float PingAnimationRunningTime = 0.f;

	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 20.f;

	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool bHighPing);

	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 50.f;

	UPROPERTY(EditAnywhere, Category = Input)
	class UInputAction* QuitAction;

	UPROPERTY(EditAnywhere, Category = Input)
	class UInputAction* EnterKeyAction;

	UPROPERTY(VisibleAnywhere, Category = Widget)
	class UPlayerList* PlayerListWidget;

	UPROPERTY(EditAnywhere, Category = Widget)
	TSubclassOf<UPlayerList> PlayerListWidgetClass;

	bool bHudSet = false;
};
