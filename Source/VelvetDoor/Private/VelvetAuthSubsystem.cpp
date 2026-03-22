// Fill out your copyright notice in the Description page of Project Settings.


#include "VelvetAuthSubsystem.h"
#include "VelvetMVVMSubsystem.h"
#include "OnlineSubsystem.h"
#include "VelvetSocialViewModel.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlinePresenceInterface.h"


void UVelvetAuthSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    if (CurrentAuthState != EVelvetAuthStates::Initial)
    {
        return;
    }

    SetAuthState(EVelvetAuthStates::Booting);

    OSS = IOnlineSubsystem::Get();
    if (!OSS)
    {
        SetAuthState(EVelvetAuthStates::Failed);
        return;
    }

    Identity = OSS->GetIdentityInterface();
    if (!Identity.IsValid())
    {
        SetAuthState(EVelvetAuthStates::Failed);
        return;
    }

    // Bind Events
    ////////////////////
    
	// Bind on login complete event to handle login results
    FOnLoginCompleteDelegate LoginDelegate = FOnLoginCompleteDelegate::CreateUObject(
        this, &UVelvetAuthSubsystem::OnLoginComplete
    );
    LoginCompleteHandle = Identity->AddOnLoginCompleteDelegate_Handle(0, LoginDelegate);

    
}

void UVelvetAuthSubsystem::Deinitialize()
{
    // Se désabonner proprement
    if (Identity && Identity.IsValid())
    {
        Identity->ClearOnLoginCompleteDelegate_Handle(0, LoginCompleteHandle);
    }

	ClearPresence();

    Identity = nullptr;
    OSS = nullptr;
    LocalUserId.Reset();

    Super::Deinitialize();
}

void UVelvetAuthSubsystem::SetAuthState(EVelvetAuthStates NewState)
{
    CurrentAuthState = NewState;

    // Pousser vers la VM
    if (UVelvetMVVMSubsystem* MVVM = GetGameInstance()->GetSubsystem<UVelvetMVVMSubsystem>())
    {
        MVVM->GetViewModelTyped<UVelvetSocialViewModel>(FName("Social"))->SetLocalAuthState(CurrentAuthState);
    }

    // Notifier les abonnés (FriendsSubsystem, etc.)
    OnAuthStateChanged.Broadcast(NewState);
}

void UVelvetAuthSubsystem::StartLogin()
{
	UKismetSystemLibrary::PrintString(this, TEXT("VelvetAuth: StartLogin called"));

    if (CurrentAuthState != EVelvetAuthStates::Booting)
    {
		UKismetSystemLibrary::PrintString(this, TEXT("VelvetAuth: StartLogin called but auth state is not Booting"));
        
        return;
	}

    SetAuthState(EVelvetAuthStates::SigningIn);

    // Déclencher le login automatique
    Identity->AutoLogin(0);
}

void UVelvetAuthSubsystem::OnLoginComplete(
    int32 LocalUserNum,
    bool bWasSuccessful,
    const FUniqueNetId& UserId,
    const FString& Error)
{
    if (CurrentAuthState != EVelvetAuthStates::SigningIn)
    {
		UKismetSystemLibrary::PrintString(this, TEXT("VelvetAuth: OnLoginComplete called but auth state is not SigningIn"));

        return;
	}

    if (bWasSuccessful)
    {
		LocalUserId = UserId.AsShared();
        SetAuthState(EVelvetAuthStates::SignedIn);

		PopulateLocalUserInfo();

		OnLoginSuccess.Broadcast(LocalUserNum, UserId.ToString());
    }
    else
    {
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("VelvetAuth: Login failed — %s"), *Error));

        SetAuthState(EVelvetAuthStates::Failed);
    }
}

void UVelvetAuthSubsystem::PopulateLocalUserInfo()
{
    if (CurrentAuthState != EVelvetAuthStates::SignedIn)
    {
		UKismetSystemLibrary::PrintString(this, TEXT("VelvetAuth: PopulateLocalUserInfo called but auth state is not SignedIn"));

        return;
    }

    if (!Identity.IsValid() || !LocalUserId.IsValid())
    {
		UKismetSystemLibrary::PrintString(this, TEXT("VelvetAuth: PopulateLocalUserInfo called but Identity interface or LocalUserId is invalid"));

        return;
    }

    // Récupérer les infos de l'utilisateur local
    TSharedPtr<FUserOnlineAccount> UserAccount = Identity->GetUserAccount(*LocalUserId);
    if (UserAccount.IsValid())
    {
        if (UVelvetMVVMSubsystem* MVVM = GetGameInstance()->GetSubsystem<UVelvetMVVMSubsystem>())
        {
            UVelvetSocialViewModel* VMSocial = MVVM->GetViewModelTyped<UVelvetSocialViewModel>(FName("Social"));
            VMSocial->SetLocalUserId(*LocalUserId);
            VMSocial->SetLocalDisplayName(UserAccount->GetDisplayName());
        }
    }
}

void UVelvetAuthSubsystem::SetPresence(EVelvetPresenceStatus Status)
{
	SetPresenceWithProperties(Status, {});
}

void UVelvetAuthSubsystem::SetPresenceWithProperties(EVelvetPresenceStatus Status, TMap<FString, FString> Properties)
{
    if (!Identity.IsValid() || !LocalUserId.IsValid()) return;

    IOnlinePresencePtr Presence = OSS->GetPresenceInterface();
    if (!Presence.IsValid()) return;

    FOnlineUserPresenceStatus PresenceStatus;
    PresenceStatus.State = EOnlinePresenceState::Online;
    PresenceStatus.StatusStr = (Status == EVelvetPresenceStatus::InMenus)
        ? TEXT("InMenus")
        : TEXT("InGame");

    for (const TPair<FString, FString>& Pair : Properties)
    {
        PresenceStatus.Properties.Add(Pair.Key, FVariantData(Pair.Value));
    }

    Presence->SetPresence(*LocalUserId, PresenceStatus);
}

void UVelvetAuthSubsystem::ClearPresence()
{
    if (!Identity.IsValid() || !LocalUserId.IsValid()) return;

    IOnlinePresencePtr Presence = OSS->GetPresenceInterface();
    if (!Presence.IsValid()) return;

    FOnlineUserPresenceStatus PresenceStatus;
    PresenceStatus.State = EOnlinePresenceState::Offline;

    Presence->SetPresence(*LocalUserId, PresenceStatus);
}