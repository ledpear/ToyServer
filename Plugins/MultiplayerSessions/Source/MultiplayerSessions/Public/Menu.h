// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "Menu.generated.h"

/**
 * 
 */

class UButton;
class UMultiplayerSessionsSubsystem;

UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumberOfPublicConnections = 4, const FString& TypeOfMatch = FString(TEXT("FreeForAll")), const FString& LobbyPath = FString(TEXT("/Game/ThirdPerson/Maps/Lobby")));

protected:
	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);

	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);

private:
	UFUNCTION(BlueprintCallable)
	void HostButtonClicked();

	UFUNCTION(BlueprintCallable)
	void JoinButtonClicked();

	void MenuTearDown();
	void PrintLog(FColor color, const FString& text);

private:
	FString MatchType	= TEXT("FreeForAll");
	FString PathToLobby = TEXT("");

	UPROPERTY(meta = (BindWidget))
	UButton* HostButton = nullptr;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton = nullptr;

	UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem = nullptr;
	int32 NumPublicConnections = 4;
};
