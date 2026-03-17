// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "VelvetAuthSubsystem.h"
#include "VelvetFriendViewModel.h"
#include "VelvetInboundInvitationViewModel.h"
#include "VelvetSocialViewModel.generated.h"

/**
 * 
 */
UCLASS()
class VELVETDOOR_API UVelvetSocialViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()
	
public:
    UPROPERTY(FieldNotify, BlueprintReadOnly, Category = "Velvet|Social")
    EVelvetAuthStates LocalAuthState = EVelvetAuthStates::Initial;

    UPROPERTY(FieldNotify, BlueprintReadOnly, Category = "Velvet|Social")
    FUniqueNetIdRepl LocalUserId;

    UPROPERTY(FieldNotify, BlueprintReadOnly, Category = "Velvet|Social")
    FText LocalDisplayName;

    UPROPERTY(FieldNotify, BlueprintReadOnly, Category = "Velvet|Social")
    FText FriendCode;

    void SetLocalAuthState(EVelvetAuthStates InValue);
    void SetLocalUserId(const FUniqueNetId& InValue);
    void SetLocalDisplayName(const FText& InValue);
    void SetLocalDisplayName(const FString& InValue);
    void SetFriendCode(const FText& InValue);
    void SetFriendCode(const FString& InValue);

    const TArray<TObjectPtr<UVelvetFriendViewModel>>& GetFriendList() const { return FriendList; }
    const TArray<TObjectPtr<UVelvetInboundInvitationViewModel>>& GetInboundFriendInvitations() const { return InboundFriendInvitations; }


    void AddFriend(UVelvetFriendViewModel* InFriend);
    void RemoveFriend(const FUniqueNetId& UserId);
    void ClearFriends();
    void RemoveFriendByNativeId(const FString& NativeId);

    void AddInboundInvitation(UVelvetInboundInvitationViewModel* InInvitation);
    void RemoveInboundInvitation(const FString& SenderId);
    void ClearInboundInvitations();

private:
    UPROPERTY(FieldNotify, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    TArray<TObjectPtr<UVelvetFriendViewModel>> FriendList;

    UPROPERTY(FieldNotify, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    TArray<TObjectPtr<UVelvetInboundInvitationViewModel>> InboundFriendInvitations;


    void OnFriendPresenceChanged(UObject* Sender, UE::FieldNotification::FFieldId FieldId);
};
