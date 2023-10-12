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
#include "TPSProject/GameState/TPSGameState.h"
#include "Components/Image.h"
#include "TPSProject/HUD/ReturnToMainMenu.h"
#include "TPSProject/Types/Announcement.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "TPSProject/HUD/ChatOverlay.h"
#include "TPSProject/HUD/VoteForNewGame.h"
#include "Components/EditableText.h"

void ATPSController::BeginPlay()
{
	Super::BeginPlay();

	TPSHUD = Cast<ATPSHUD>(GetHUD());
	ServerCheckMatchState();

	AddChat();
}

void ATPSController::AddChat()
{
	if (!IsLocalPlayerController()) return;
	if (ChatOverlayClass)
	{
		ChatWidget = ChatWidget == nullptr ? CreateWidget<UChatOverlay>(this, ChatOverlayClass) : ChatWidget;
		if (ChatWidget)
		{
			ChatWidget->AddToViewport();
			ChatWidget->InputTextBox->SetVisibility(ESlateVisibility::Collapsed);
			ChatWidget->InputTextBox->OnTextCommitted.AddDynamic(this, &ATPSController::OnTextCommitted);
		}
	}
}

void ATPSController::EnterKeyPressed()
{
	if (ChatWidget && ChatWidget->InputTextBox)
	{
		if (ChatWidget->InputTextBox->GetVisibility() == ESlateVisibility::Collapsed)
		{
			ChatWidget->InputTextBox->SetVisibility(ESlateVisibility::Visible);
			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(ChatWidget->InputTextBox->TakeWidget());
			SetInputMode(InputMode);
			SetShowMouseCursor(true);
		}
		else
		{
			ChatWidget->InputTextBox->SetVisibility(ESlateVisibility::Collapsed);
			FInputModeGameOnly InputMode;
			SetInputMode(InputMode);
			SetShowMouseCursor(false);
		}
	}
}

void ATPSController::OnTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod != ETextCommit::OnEnter) return;

	PlayerState = PlayerState == nullptr ? GetPlayerState<APlayerState>() : PlayerState;
	FString PlayerName("");
	if (PlayerState)
	{
		PlayerName = PlayerState->GetPlayerName();
	}
	if (ChatWidget)
	{
		if (!Text.IsEmpty())
		{
			ServerSetText(Text.ToString(), PlayerName);
		}
		ChatWidget->InputTextBox->SetText(FText());
		ChatWidget->InputTextBox->SetVisibility(ESlateVisibility::Collapsed);
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		SetShowMouseCursor(false);
	}
}

void ATPSController::ServerSetText_Implementation(const FString& Text, const FString& PlayerName)
{
	TPSGameMode = TPSGameMode == nullptr? Cast<ATPSGameMode>(UGameplayStatics::GetGameMode(this)) : TPSGameMode;
	if (TPSGameMode)
	{
		TPSGameMode->SendChat(Text, PlayerName);
	}
}

void ATPSController::ClientSetText_Implementation(const FString& Text, const FString& PlayerName)
{
	PlayerState = PlayerState == nullptr ? GetPlayerState<APlayerState>() : PlayerState;
	if (ChatWidget && PlayerState)
	{
		if (PlayerName == PlayerState->GetPlayerName())
		{
			ChatWidget->SetChatText(Text, "You");
		}
		else
		{
			ChatWidget->SetChatText(Text, PlayerName);
		}
	}
}

void ATPSController::ServerNewGameAgree_Implementation()
{
	auto GameMode = UGameplayStatics::GetGameMode(this);
	if (GameMode)
	{
		ATPSGameState* GameState =  GameMode->GetGameState<ATPSGameState>();
		GameState->NewGameAgree();
	}

}

void ATPSController::ServerNewGameDisagree_Implementation()
{
	auto GameMode = UGameplayStatics::GetGameMode(this);
	if (GameMode)
	{
		ATPSGameState* GameState = GameMode->GetGameState<ATPSGameState>();
		GameState->NewGameDisagree();
	}
}


void ATPSController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATPSController, MatchState);
	DOREPLIFETIME(ATPSController, bShowTeamScores);
}

void ATPSController::HideTeamScores()
{
	TPSHUD = TPSHUD == nullptr ? Cast<ATPSHUD>(GetHUD()) : TPSHUD;
	bool bHUDValid = TPSHUD &&
		TPSHUD->CharacterOverlay &&
		TPSHUD->CharacterOverlay->RedTeamScore &&
		TPSHUD->CharacterOverlay->BlueTeamScore &&
		TPSHUD->CharacterOverlay->ScoreSpacerText;
	if (bHUDValid)
	{
		TPSHUD->CharacterOverlay->RedTeamScore->SetText(FText());
		TPSHUD->CharacterOverlay->BlueTeamScore->SetText(FText());
		TPSHUD->CharacterOverlay->ScoreSpacerText->SetText(FText());
	}
}

void ATPSController::InitTeamScores()
{
	TPSHUD = TPSHUD == nullptr ? Cast<ATPSHUD>(GetHUD()) : TPSHUD;
	bool bHUDValid = TPSHUD &&
		TPSHUD->CharacterOverlay &&
		TPSHUD->CharacterOverlay->RedTeamScore &&
		TPSHUD->CharacterOverlay->BlueTeamScore &&
		TPSHUD->CharacterOverlay->ScoreSpacerText;
	if (bHUDValid)
	{
		FString Zero("0");
		FString Spacer("|");
		TPSHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(Zero));
		TPSHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(Zero));
		TPSHUD->CharacterOverlay->ScoreSpacerText->SetText(FText::FromString(Spacer));
	}
}

void ATPSController::SetHUDRedTeamScore(int32 RedScore)
{
	TPSHUD = TPSHUD == nullptr ? Cast<ATPSHUD>(GetHUD()) : TPSHUD;
	bool bHUDValid = TPSHUD &&
		TPSHUD->CharacterOverlay &&
		TPSHUD->CharacterOverlay->RedTeamScore;
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), RedScore);
		TPSHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(ScoreText));
	}
}

void ATPSController::SetHUDBlueTeamScore(int32 BlueScore)
{
	TPSHUD = TPSHUD == nullptr ? Cast<ATPSHUD>(GetHUD()) : TPSHUD;
	bool bHUDValid = TPSHUD &&
		TPSHUD->CharacterOverlay &&
		TPSHUD->CharacterOverlay->BlueTeamScore;
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), BlueScore);
		TPSHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(ScoreText));
	}
}

void ATPSController::SetHUDNewGameAgree(int32 Agree)
{
	TPSHUD = TPSHUD == nullptr ? Cast<ATPSHUD>(GetHUD()) : TPSHUD;
	bool bHUDValid = TPSHUD &&
		TPSHUD->VoteForNewGameWidget &&
		TPSHUD->VoteForNewGameWidget->AgreeNum;
	if (bHUDValid)
	{
		FString AgreeText = FString::Printf(TEXT("%d"), Agree);
		TPSHUD->VoteForNewGameWidget->AgreeNum->SetText(FText::FromString(AgreeText));
	}
}

void ATPSController::SetHUDNewGameDisagree(int32 Disagree)
{
	TPSHUD = TPSHUD == nullptr ? Cast<ATPSHUD>(GetHUD()) : TPSHUD;
	bool bHUDValid = TPSHUD &&
		TPSHUD->VoteForNewGameWidget &&
		TPSHUD->VoteForNewGameWidget->DisagreeNum;
	if (bHUDValid)
	{
		FString DisagreeText = FString::Printf(TEXT("%d"), Disagree);
		TPSHUD->VoteForNewGameWidget->DisagreeNum->SetText(FText::FromString(DisagreeText));
	}
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
	if (HasAuthority()) return;
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

				ServerReportPingStatus(true);
			}
			else
			{
				ServerReportPingStatus(false);
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

void ATPSController::ShowReturnToMainMenu()
{
	if (ReturnToMainMenuWidget == nullptr) return;
	if (ReturnToMainMenu == nullptr)
	{
		ReturnToMainMenu = CreateWidget<UReturnToMainMenu>(this, ReturnToMainMenuWidget);
	}
	if (ReturnToMainMenu)
	{
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		if (bReturnToMainMenuOpen)
		{
			ReturnToMainMenu->MenuSetup();
		}
		else
		{
			ReturnToMainMenu->MenuTearDown();
		}
	}
}

void ATPSController::OnRep_ShowTeamScores()
{
	if (bShowTeamScores)
	{
		InitTeamScores();
	}
	else
	{
		HideTeamScores();
	}
}

FString ATPSController::GetInfoText(const TArray<class ATPSPlayerState*>& Players)
{
	ATPSPlayerState* TPSPlayerState = GetPlayerState<ATPSPlayerState>();
	if (TPSPlayerState == nullptr) return FString();
	FString InfoTextString;
	if (Players.Num() == 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if (Players.Num() == 1 && Players[0] == TPSPlayerState)
	{
		InfoTextString = Announcement::YouAreTheWinner;
	}
	else if (Players.Num() == 1)
	{
		InfoTextString = FString::Printf(TEXT("Winner: \n%s"), *Players[0]->GetPlayerName());
	}
	else if (Players.Num() > 1)
	{
		InfoTextString = Announcement::PlayersTiedForTheWin;
		InfoTextString.Append(FString("\n"));
		for (auto TiedPlayer : Players)
		{
			InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
		}
	}

	return InfoTextString;
}

FString ATPSController::GetTeamsInfoText(ATPSGameState* TPSGameState)
{
	if (TPSGameState == nullptr) return FString();
	FString InfoTextString;

	const int32 RedTeamScore = TPSGameState->RedTeamScore;
	const int32 BlueTeamScore = TPSGameState->BlueTeamScore;

	if (RedTeamScore == 0 && BlueTeamScore == 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if (RedTeamScore == BlueTeamScore)
	{
		InfoTextString = FString::Printf(TEXT("%s\n"), *Announcement::TeamsTiedForTheWin);
		InfoTextString.Append(Announcement::RedTeam);
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(Announcement::BlueTeam);
		InfoTextString.Append(TEXT("\n"));
	}
	else if (RedTeamScore > BlueTeamScore)
	{
		InfoTextString = Announcement::RedTeamWins;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::RedTeam, RedTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::BlueTeam, BlueTeamScore));
	}
	else if (BlueTeamScore > RedTeamScore)
	{
		InfoTextString = Announcement::BlueTeamWins;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::BlueTeam, BlueTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::RedTeam, RedTeamScore));
	}

	return InfoTextString;
}

void ATPSController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
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
		VoteTime = GameMode->VoteTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, CooldownTime, VoteTime, LevelStartingTime);
	}
}

void ATPSController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match, float Cooldown, float Vote, float StartingTime)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;
	VoteTime = Vote;
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

void ATPSController::SetHUDVoteCountDown(float CountdownTime)
{
	TPSHUD = TPSHUD == nullptr ? Cast<ATPSHUD>(GetHUD()) : TPSHUD;
	bool bHUDValid = TPSHUD &&
		TPSHUD->VoteForNewGameWidget &&
		TPSHUD->VoteForNewGameWidget->VoteTime;
	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			TPSHUD->VoteForNewGameWidget->VoteTime->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		TPSHUD->VoteForNewGameWidget->VoteTime->SetText(FText::FromString(CountdownText));
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
	else if (MatchState == MatchState::VoteForNewGame) TimeLeft = VoteTime + CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;

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
		if (MatchState == MatchState::VoteForNewGame)
		{
			SetHUDVoteCountDown(TimeLeft);
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

void ATPSController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent == nullptr) return;
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(QuitAction, ETriggerEvent::Completed, this, &ATPSController::ShowReturnToMainMenu);
		EnhancedInputComponent->BindAction(EnterKeyAction, ETriggerEvent::Triggered, this, &ATPSController::EnterKeyPressed);
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
	SingleTripTime = 0.5f * RoundTripTime;
	float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;
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

void ATPSController::OnMatchStateSet(FName State, bool bTeamsMatch)
{
	MatchState = State;

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted(bTeamsMatch);
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}

	else if (MatchState == MatchState::VoteForNewGame)
	{
		StartVote();
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
	else if (MatchState == MatchState::VoteForNewGame)
	{
		StartVote();
	}
}

void ATPSController::HandleMatchHasStarted(bool bTeamsMatch)
{
	if (HasAuthority()) bShowTeamScores = bTeamsMatch;
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
		if (!HasAuthority()) return;
		if (bTeamsMatch)
		{
			InitTeamScores();
		}
		else
		{
			HideTeamScores();
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
			FString AnnouncementText = Announcement::NewMatchStartsIn;
			TPSHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));

			ATPSGameState* TPSGameState = Cast<ATPSGameState>(UGameplayStatics::GetGameState(this));
			ATPSPlayerState* TPSPlayerState = GetPlayerState<ATPSPlayerState>();
			if (TPSGameState && TPSPlayerState)
			{
				TArray<ATPSPlayerState*> TopPlayers = TPSGameState->TopScoringPlayers;
				FString InfoTextString = bShowTeamScores ? GetTeamsInfoText(TPSGameState) : GetInfoText(TopPlayers);

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

void ATPSController::StartVote()
{
	TPSHUD = TPSHUD == nullptr ? Cast<ATPSHUD>(GetHUD()) : TPSHUD;
	if (TPSHUD)
	{
		if (TPSHUD->Announcement)
		{
			TPSHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
		TPSHUD->AddVoteWidget();

		if (TPSHUD->VoteForNewGameWidget)
		{
			TPSHUD->VoteForNewGameWidget->SetVisibility(ESlateVisibility::Visible);

			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(TPSHUD->VoteForNewGameWidget->TakeWidget());
			SetInputMode(InputMode);
			SetShowMouseCursor(true);
		}
	}
}

void ATPSController::BroadcastElim(APlayerState* Attacker, APlayerState* Victim)
{
	ClientElimAnnouncement(Attacker, Victim);
}

void ATPSController::ClientElimAnnouncement_Implementation(APlayerState* Attacker, APlayerState* Victim)
{
	APlayerState* Self = GetPlayerState<APlayerState>();
	if (Attacker && Victim && Self)
	{
		TPSHUD = TPSHUD == nullptr ? Cast<ATPSHUD>(GetHUD()) : TPSHUD;
		if (TPSHUD)
		{
			if (Attacker == Self && Victim != Self)
			{
				TPSHUD->AddElimAnnouncement("You", Victim->GetPlayerName());
				return;
			}
			if (Attacker != Self && Victim == Self)
			{
				TPSHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "you");
				return;
			}
			if (Attacker == Victim && Attacker == Self)
			{
				TPSHUD->AddElimAnnouncement("You", "yourself");
				return;
			}
			if (Attacker == Victim && Attacker != Self)
			{
				TPSHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "themselves");
				return;
			}
			TPSHUD->AddElimAnnouncement(Attacker->GetPlayerName(), Victim->GetPlayerName());
		}
	}
}
