// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "VelvetAuthSubsystem.generated.h"

UENUM(BlueprintType)
enum class EVelvetAuthStates : uint8
{
	Initial     UMETA(DisplayName = "Initial"),
	Booting     UMETA(DisplayName = "Booting"),
	SigningIn   UMETA(DisplayName = "Signing In"),
	SignedIn    UMETA(DisplayName = "Signed In"),
	Failed      UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EVelvetPresenceStatus : uint8
{
	InMenus  UMETA(DisplayName = "In Menus"),
	InGame   UMETA(DisplayName = "In Game")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnAuthStateChanged,
	EVelvetAuthStates, NewState
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnLoginSuccess,
	int32, LocalUserNum,
	const FString&, UserId
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnLoginFailed,
	const FString&, Error
);

/**
 * 
 */
UCLASS()
class VELVETDOOR_API UVelvetAuthSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Appelé par la GameInstance pour démarrer le login
	UFUNCTION(BlueprintCallable, Category = "Velvet|Auth")
	void StartLogin();

	UFUNCTION(BlueprintCallable, Category = "Velvet|Auth")
	void SetPresence(EVelvetPresenceStatus Status);

	UFUNCTION(BlueprintCallable, Category = "Velvet|Auth")
	void SetPresenceWithProperties(EVelvetPresenceStatus Status, TMap<FString, FString> Properties);

	UFUNCTION(BlueprintCallable, Category = "Velvet|Auth")
	void ClearPresence();

	UFUNCTION(BlueprintPure, Category = "Velvet|Auth")
	EVelvetAuthStates GetAuthState() const { return CurrentAuthState; }

	UPROPERTY(BlueprintAssignable, Category = "Velvet|Auth")
	FOnAuthStateChanged OnAuthStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Velvet|Auth")
	FOnLoginSuccess OnLoginSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Velvet|Auth")
	FOnLoginFailed OnLoginFailed;

	TSharedPtr<const FUniqueNetId> GetLocalUserId() const { return LocalUserId; }

private:
	void SetAuthState(EVelvetAuthStates NewState);
	void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);
	void PopulateLocalUserInfo();

	EVelvetAuthStates CurrentAuthState = EVelvetAuthStates::Initial;
	TSharedPtr<const FUniqueNetId> LocalUserId;
	FDelegateHandle LoginCompleteHandle;

	IOnlineSubsystem* OSS;
	IOnlineIdentityPtr Identity;
};
