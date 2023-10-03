// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSController.h"
#include "TPSProject/HUD/TPSHUD.h"
#include "TPSProject/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "TPSProject/Character/TPSCharacter.h"
#include "Net/UnrealNetwork.h"
#include "TPSProject/GameMode/TPSGameMode.h"
#include "TPSProject/PlayerState/TPSPlayerState.h"
#include "TPSProject/HUD/Announcement.h"
#include "Kismet/GameplayStatics.h"
#include "TPSProject/TPSComponent/CombatComponent.h"
#include "TPSProject/Weapon/Weapon.h"
#include "TPSProject/GameState/TPSGameState.h"
#include "Components/Image.h"

void ATPSController::BeginPlay()
{
	Super::BeginPlay();

	TPSHUD = Cast<ATPSHUD>(GetHUD());
	ServerCheckMatchState();
}

void ATPSController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATPSController, MatchState);
}

void ATPSController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();
	CheckTimeSync(DeltaTime);
	PollInit();
	CheckPing(DeltaTime);
}

void ATPSController::CheckPing(float DeltaTime)
{
	HighPingRunningTime += DeltaTime;
	if (HighPingRunningTime > CheckPingFrequency)
	{
		PlayerState = PlayerState == nullptr ? GetPlayerState<APlayerState>() : PlayerState;
		if (PlayerState)
		{
			if (PlayerState->GetPing() * 4 > HighPingThreshold) // ping is compressed; it's actually ping / 4
			{
				HighPingWarning();
				PingAnimationRunningTime = 0.f;
			}
		}
		HighPingRunningTime = 0.f;
	}
	bool bHighPingAnimationPlaying =
		TPSHUD && TPSHUD->CharacterOverlay &&
		TPSHUD->CharacterOverlay->HighPingAnimation &&
		TPSHUD->CharacterOverlay->IsAnimationPlaying(TPSHUD->CharacterOverlay->HighPingAnimation);
	if (bHighPingAnimationPlaying)
	{
		PingAnimationRunningTime += DeltaTime;
		if (PingAnimationRunningTime > HighPingDuration)
		{
			StopHighPingWarning();
		}
	}
}

void ATPSController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void ATPSController::HighPingWarning()
{
	TPSHUD = TPSHUD == nullptr ? Cast<ATPSHUD>(GetHUD()) : TPSHUD;
	bool bHUDValid = TPSHUD &&
		TPSHUD->CharacterOverlay &&
		TPSHUD->CharacterOverlay->HighPingImage &&
		TPSHUD->CharacterOverlay->HighPingAnimation;
	if (bHUDValid)
	{
		TPSHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
		TPSHUD->CharacterOverlay->PlayAnimation(
			TPSHUD->CharacterOverlay->HighPingAnimation,
			0.f,
			5);
	}
}

void ATPSController::StopHighPingWarning()
{
	TPSHUD = TPSHUD == nullptr ? Cast<ATPSHUD>(GetHUD()) : TPSHUD;
	bool bHUDValid = TPSHUD &&
		TPSHUD->CharacterOverlay &&
		TPSHUD->CharacterOverlay->HighPingImage &&
		TPSHUD->CharacterOverlay->HighPingAnimation;
	if (bHUDValid)
	{
		TPSHUD->CharacterOverlay->HighPingImage->SetOpacity(0.f);
		if (TPSHUD->CharacterOverlay->IsAnimationPlaying(TPSHUD->CharacterOverlay->HighPingAnimation))
		{
			TPSHUD->CharacterOverlay->StopAnimation(TPSHUD->CharacterOverlay->HighPingAnimation);
		}
	}
}

void ATPSController::ServerCheckMatchState_Implementation()
{
	ATPSGameMode* GameMode = Cast<ATPSGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		CooldownTime = GameMode->CooldownTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartingTime);
	}
}

void ATPSController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);
	if (TPSHUD && MatchState == MatchState::WaitingToStart)
	{
		TPSHUD->AddAnnouncement();
	}
}

void ATPSController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ATPSCharacter* TPSCharacter = Cast<ATPSCharacter>(InPawn);
	if (TPSCharacter)
	{
		SetHUDHealth(TPSCharacter->GetHealth(), TPSCharacter->GetMaxHealth());
		SetHUDShield(TPSCharacter->GetShield(), TPSCharacter->GetMaxShield());
		TPSCharacter->UpdateHUDAmmo();
	}
}

void ATPSController::SetHUDHealth(float Health, float MaxHealth)
{
	TPSHUD = TPSHUD == nullptr ? Cast<ATPSHUD>(GetHUD()) : TPSHUD;

	bool bHUDValid = TPSHUD && TPSHUD->CharacterOverlay && TPSHUD->CharacterOverlay->HealthBar && TPSHUD->CharacterOverlay->HealthText;
	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		TPSHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		FMath::CeilToInt(MaxHealth);
		TPSHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void ATPSController::SetHUDShield(float Shield, float MaxShield)
{
	TPSHUD = TPSHUD == nullptr ? Cast<ATPSHUD>(GetHUD()) : TPSHUD;
	bool bHUDValid = TPSHUD &&
		TPSHUD->CharacterOverlay &&
		TPSHUD->CharacterOverlay->ShieldBar &&
		TPSHUD->CharacterOverlay->ShieldText;
	if (bHUDValid)
	{
		const float ShieldPercent = Shield / MaxShield;
		TPSHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
		FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
		TPSHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
	else
	{
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
	}
}

void ATPSController::SetHUDScore(float Score)
{
	TPSHUD = TPSHUD == nullptr ? Cast<ATPSHUD>(GetHUD()) : TPSHUD;
	bool bHUDValid = TPSHUD &&
		TPSHUD->CharacterOverlay &&
		TPSHUD->CharacterOverlay->ScoreAmount;
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		TPSHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeScore = true;
		HUDScore = Score;
	}
}

void ATPSController::SetHUDDefeats(int32 Defeats)
{
	TPSHUD = TPSHUD == nullptr ? Cast<ATPSHUD>(GetHUD()) : TPSHUD;
	bool bHUDValid = TPSHUD &&
		TPSHUD->CharacterOverlay &&
		TPSHUD->CharacterOverlay->DefeatsAmount;
	if (bHUDValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		TPSHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else
	{
		bInitializeDefeats = true;
		HUDDefeats = Defeats;
	}
}

void ATPSController::SetHUDWeaponAmmo(int32 Ammo)
{
	TPSHUD = TPSHUD == nullptr ? Cast<ATPSHUD>(GetHUD()) : TPSHUD;
	bool bHUDValid = TPSHUD &&
		TPSHUD->CharacterOverlay &&
		TPSHUD->CharacterOverlay->WeaponAmmoAmount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		TPSHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
	}
}

void ATPSController::SetHUDCarriedAmmo(int32 Ammo)
{
	TPSHUD = TPSHUD == nullptr ? Cast<ATPSHUD>(GetHUD()) : TPSHUD;
	bool bHUDValid = TPSHUD &&
		TPSHUD->CharacterOverlay &&
		TPSHUD->CharacterOverlay->CarriedAmmoAmount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		TPSHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
	}
}

void ATPSController::SetHUDMatchCountdown(float CountdownTime)
{
	TPSHUD = TPSHUD == nullptr ? Cast<ATPSHUD>(GetHUD()) : TPSHUD;
	bool bHUDValid = TPSHUD &&
		TPSHUD->CharacterOverlay &&
		TPSHUD->CharacterOverlay->MatchCountdownText;
	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			TPSHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		TPSHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void ATPSController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	TPSHUD = TPSHUD == nullptr ? Cast<ATPSHUD>(GetHUD()) : TPSHUD;
	bool bHUDValid = TPSHUD &&
		TPSHUD->Announcement &&
		TPSHUD->Announcement->WarmupTime;
	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			TPSHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		TPSHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void ATPSController::SetHUDGrenades(int32 Grenades)
{
	TPSHUD = TPSHUD == nullptr ? Cast<ATPSHUD>(GetHUD()) : TPSHUD;
	bool bHUDValid = TPSHUD &&
		TPSHUD->CharacterOverlay &&
		TPSHUD->CharacterOverlay->GrenadesText;
	if (bHUDValid)
	{
		FString GrenadesText = FString::Printf(TEXT("%d"), Grenades);
		TPSHUD->CharacterOverlay->GrenadesText->SetText(FText::FromString(GrenadesText));
	}
	else
	{
		bInitializeGrenades = true;
		HUDGrenades = Grenades;
	}
}

void ATPSController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::Cooldown) TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
	}

	CountdownInt = SecondsLeft;
}

void ATPSController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (TPSHUD && TPSHUD->CharacterOverlay)
		{
			CharacterOverlay = TPSHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				if (bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth);
				if (bInitializeShield) SetHUDShield(HUDShield, HUDMaxShield);
				if (bInitializeScore) SetHUDScore(HUDScore);
				if (bInitializeDefeats) SetHUDDefeats(HUDDefeats);
				if (bInitializeCarriedAmmo) SetHUDCarriedAmmo(HUDCarriedAmmo);
				if (bInitializeWeaponAmmo) SetHUDWeaponAmmo(HUDWeaponAmmo);

				ATPSCharacter* TPSCharacter = Cast<ATPSCharacter>(GetPawn());
				if (TPSCharacter && TPSCharacter->GetCombat())
				{
					if (bInitializeGrenades) SetHUDGrenades(TPSCharacter->GetCombat()->GetGrenades());
				}
			}
		}
	}
}

void ATPSController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void ATPSController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimeServerReceivedClientRequest + (0.5f * RoundTripTime); // 서버에 RPC가 도착한 시점이 서버시간 요청후 + 0.5 * RoundTripTime으로 가정
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float ATPSController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ATPSController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ATPSController::OnMatchStateSet(FName State)
{
	MatchState = State;

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ATPSController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ATPSController::HandleMatchHasStarted()
{
	TPSHUD = TPSHUD == nullptr ? Cast<ATPSHUD>(GetHUD()) : TPSHUD;
	if (TPSHUD)
	{
		if (TPSHUD->CharacterOverlay == nullptr)
		{
			TPSHUD->AddCharacterOverlay();
		}
		if (TPSHUD->Announcement)
		{
			TPSHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ATPSController::HandleCooldown()
{
	TPSHUD = TPSHUD == nullptr ? Cast<ATPSHUD>(GetHUD()) : TPSHUD;
	if (TPSHUD)
	{
		TPSHUD->CharacterOverlay->RemoveFromParent();
		bool bHUDValid = TPSHUD->Announcement &&
			TPSHUD->Announcement->AnnouncementText &&
			TPSHUD->Announcement->InfoText;

		if (TPSHUD)
		{
			TPSHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText("New Match Starts In:");
			TPSHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));

			ATPSGameState* TPSGameState = Cast<ATPSGameState>(UGameplayStatics::GetGameState(this));
			ATPSPlayerState* TPSPlayerState = GetPlayerState<ATPSPlayerState>();
			if (TPSGameState && TPSPlayerState)
			{
				TArray<ATPSPlayerState*> TopPlayers = TPSGameState->TopScoringPlayers;
				FString InfoTextString;
				if (TopPlayers.Num() == 0)
				{
					InfoTextString = FString("There is no winner.");
				}
				else if (TopPlayers.Num() == 1 && TopPlayers[0] == TPSPlayerState)
				{
					InfoTextString = FString("You are the winner!");
				}
				else if (TopPlayers.Num() == 1)
				{
					InfoTextString = FString::Printf(TEXT("Winner: \n%s"), *TopPlayers[0]->GetPlayerName());
				}
				else if (TopPlayers.Num() > 1)
				{
					InfoTextString = FString("Players tied for the win:\n");
					for (auto TiedPlayer : TopPlayers)
					{
						InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
					}
				}

				TPSHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}
		}
	}
	ATPSCharacter* TPSCharacter = Cast<ATPSCharacter>(GetPawn());
	if (TPSCharacter && TPSCharacter->GetCombat())
	{
		TPSCharacter->bDisableGameplay = true;
		TPSCharacter->GetCombat()->FireButtonPressed(false);
	}
}
