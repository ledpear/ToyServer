// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"

void UMenu::MenuSetup(int32 NumberOfPublicConnections, const FString& TypeOfMatch, const FString& LobbyPath)
{
	NumPublicConnections = NumberOfPublicConnections;
	MatchType = TypeOfMatch;
	PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);

	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	//Access Subsystem
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();

	if (MultiplayerSessionsSubsystem == nullptr)
	{
		PrintLog(FColor::Red, FString(TEXT("[MenuSetup] MultiplayerSessionsSubsystem is Nullptr!!")));
		return;
	}

	MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
	MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
	MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
	MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
	MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
}

bool UMenu::Initialize()
{
	if (Super::Initialize() == false)
		return false;

	if (HostButton)
		HostButton->OnClicked.AddDynamic(this, &UMenu::HostButtonClicked);

	if (JoinButton)
		JoinButton->OnClicked.AddDynamic(this, &UMenu::JoinButtonClicked);

	return true;
}

void UMenu::NativeDestruct()
{
	MenuTearDown();
	Super::NativeDestruct();
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		PrintLog(FColor::Cyan, FString(TEXT("[OnCreateSession] Session created successfull")));

		UWorld* World = GetWorld();
		if (World == nullptr)
		{
			PrintLog(FColor::Red, FString(TEXT("[OnCreateSession]World is Nullptr!!")));
			return;
		}

		if (World->ServerTravel(PathToLobby) == false)
			PrintLog(FColor::Red, FString(TEXT("[OnCreateSession] ServerTravel fail")));
	}
	else
	{
		PrintLog(FColor::Red, FString(TEXT("[OnCreateSession] Session created fail")));
		HostButton->SetIsEnabled(true);
	}
}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
}

void UMenu::OnStartSession(bool bWasSuccessful)
{
}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccessful)
{
	if (MultiplayerSessionsSubsystem == nullptr)
	{
		PrintLog(FColor::Red, FString(TEXT("[OnFindSessions] MultiplayerSessionsSubsystem is Nullptr!!")));
		return;
	}

	for (auto& Result : SearchResults)
	{
		FString SettingValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingValue);
		if (SettingValue != MatchType)
			continue;

		MultiplayerSessionsSubsystem->JoinSession(Result);
		break;
	}

	if ((bWasSuccessful == false) || SearchResults.IsEmpty())
	{
		PrintLog(FColor::Red, FString(TEXT("[OnFindSessions] Session Join fail")));
		JoinButton->SetIsEnabled(true);
	}
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem == nullptr)
	{
		PrintLog(FColor::Red, FString(TEXT("[OnJoinSession] IOnlineSubsystem is Nullptr!!")));
		return;
	}

	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (SessionInterface == nullptr)
	{
		PrintLog(FColor::Red, FString(TEXT("[OnFindSessions] IOnlineSessionPtr is Nullptr!!")));
		return;
	}

	FString Address;
	if (SessionInterface->GetResolvedConnectString(NAME_GameSession, Address))
	{
		PrintLog(FColor::Yellow, FString::Printf(TEXT("[OnJoinSession] Connext string: %s"), *Address));
		APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
		if (PlayerController)
		{
			PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
		}
	}
	else
		PrintLog(FColor::Red, FString::Printf(TEXT("[OnJoinSession] GetResolvedConnectString Failed")));

	if( Result != EOnJoinSessionCompleteResult::Success)
		JoinButton->SetIsEnabled(true);
}

void UMenu::HostButtonClicked()
{
	PrintLog(FColor::Yellow, FString(TEXT("Host Button Clicked")));
	HostButton->SetIsEnabled(false);
	if (MultiplayerSessionsSubsystem == nullptr)
	{
		PrintLog(FColor::Red, FString(TEXT("MultiplayerSessionsSubsystem is Nullptr!!")));
		return;
	}

	MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
}

void UMenu::JoinButtonClicked()
{
	PrintLog(FColor::Yellow, FString(TEXT("Join Button Clicked")));
	JoinButton->SetIsEnabled(false);
	if (MultiplayerSessionsSubsystem == nullptr)
	{
		PrintLog(FColor::Red, FString(TEXT("MultiplayerSessionsSubsystem is Nullptr!!")));
		return;
	}

	MultiplayerSessionsSubsystem->FindSessions(10000);
}

void UMenu::MenuTearDown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		PrintLog(FColor::Red, FString(TEXT("[MenuTearDown] World is Nullptr!!")));
		return;
	}

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (PlayerController == nullptr)
	{
		PrintLog(FColor::Red, FString(TEXT("[MenuTearDown] PlayerController is Nullptr!!")));
		return;
	}

	FInputModeGameOnly InputModeData;
	PlayerController->SetInputMode(InputModeData);
	PlayerController->SetShowMouseCursor(false);
}

void UMenu::PrintLog(FColor color, const FString& text)
{
	if (GEngine == nullptr)
		return;

	GEngine->AddOnScreenDebugMessage(-1, 15.f, color, text);
}