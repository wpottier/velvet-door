#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "VelvetAuthSubsystem.h"
#include "VelvetFriendViewModel.h"
#include "RedpointEOSFriends/FriendSystem.h"
#include "RedpointEOSPresence/PresenceUserState.h"
#include "RedpointEOSPlatform/Types/RelationshipType.h"
#include "RedpointEOSIdentity/IdentityUserId.h"
#include "RedpointEOSPresence/PresenceSystem.h"
#include "VelvetFriendsSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnFriendCodeSearchComplete,
    bool, bWasSuccessful,
    const TArray<FString>&, FoundUserIds
);

UCLASS()
class VELVETDOOR_API UVelvetFriendsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Velvet|Friends")
    void SendFriendInvitation(const FString& TargetNativeId);

    UFUNCTION(BlueprintCallable, Category = "Velvet|Friends")
    void AcceptFriendInvitation(const FString& SenderNativeId);

    UFUNCTION(BlueprintCallable, Category = "Velvet|Friends")
    void RejectFriendInvitation(const FString& SenderNativeId);

    UFUNCTION(BlueprintCallable, Category = "Velvet|Friends")
    void FindPlayersByFriendCode(const FString& FriendCode);

    UFUNCTION(BlueprintCallable, Category = "Velvet|Friends")
    void ResetFindPlayersByFriendCode();

    UFUNCTION(BlueprintCallable, Category = "Velvet|Friends")
    void DeleteFriend(const FString& FriendNativeId);

    UFUNCTION(BlueprintCallable, Category = "Velvet|Friends")
    void BlockPlayer(const FString& TargetNativeId);

    // Delegates exposés Blueprint
    UPROPERTY(BlueprintAssignable, Category = "Velvet|Friends")
    FOnFriendCodeSearchComplete OnFriendCodeSearchComplete;

private:
    UFUNCTION()
    void OnAuthStateChanged(EVelvetAuthStates NewState);

    void InitializeSystems();
    void LoadFriends();

    void OnFriendsChanged(
        const Redpoint::EOS::Identity::FIdentityUserId& InLocalUserId,
        Redpoint::EOS::Platform::Types::ERelationshipType ChangedRelationships);

    void OnFriendPresenceChanged(
        const Redpoint::EOS::Identity::FIdentityUserId& InLocalUserId,
        const Redpoint::EOS::Identity::FIdentityUserId& FriendUserId,
        const Redpoint::EOS::Presence::FPresenceUserState& NewPresenceState);

    void OnFriendRequestReceived(
        const Redpoint::EOS::Identity::FIdentityUserId& InLocalUserId,
        const Redpoint::EOS::UserCache::FUserInfoRef& SendingRemoteUser);

    EVelvetFriendPresence ConvertPresenceState(
        const Redpoint::EOS::Presence::FPresenceUserState& InPresence);

    UVelvetFriendViewModel* FindFriendViewModel(
        const Redpoint::EOS::Identity::FIdentityUserId& UserId);

    FString GetFriendDisplayName(
        const Redpoint::EOS::UserCache::FUserInfoRef& FriendInfo);

    void LoadInboundInvitations();

    // Systems
    TSharedPtr<Redpoint::EOS::API::FPlatformInstance> PlatformHandle;
    TSharedPtr<Redpoint::EOS::Friends::IFriendSystem> FriendSystem;
    TSharedPtr<Redpoint::EOS::Presence::IPresenceSystem> PresenceSystem;

    // Delegate handles
    FDelegateHandle FriendsChangedHandle;
    FDelegateHandle FriendPresenceChangedHandle;

    Redpoint::EOS::Identity::FIdentityUserId LocalUserId;
};