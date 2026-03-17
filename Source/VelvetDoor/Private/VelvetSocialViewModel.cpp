// Fill out your copyright notice in the Description page of Project Settings.


#include "VelvetSocialViewModel.h"

void UVelvetSocialViewModel::SetLocalAuthState(EVelvetAuthStates InValue)
{
    UE_MVVM_SET_PROPERTY_VALUE(LocalAuthState, InValue);
}

void UVelvetSocialViewModel::SetLocalUserId(const FUniqueNetId& InValue)
{
    UE_MVVM_SET_PROPERTY_VALUE(LocalUserId, FUniqueNetIdRepl(InValue.AsShared()));
}

void UVelvetSocialViewModel::SetLocalDisplayName(const FText& InValue)
{
    UE_MVVM_SET_PROPERTY_VALUE(LocalDisplayName, InValue);
}

void UVelvetSocialViewModel::SetLocalDisplayName(const FString& InValue)
{
    UE_MVVM_SET_PROPERTY_VALUE(LocalDisplayName, FText::FromString(InValue));
}

void UVelvetSocialViewModel::SetFriendCode(const FText& InValue)
{
    UE_MVVM_SET_PROPERTY_VALUE(FriendCode, InValue);
}

void UVelvetSocialViewModel::SetFriendCode(const FString& InValue)
{
    UE_MVVM_SET_PROPERTY_VALUE(FriendCode, FText::FromString(InValue));
}

void UVelvetSocialViewModel::AddFriend(UVelvetFriendViewModel* InFriend)
{
    FriendList.Add(InFriend);
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(FriendList);

    InFriend->AddFieldValueChangedDelegate(
        UVelvetFriendViewModel::FFieldNotificationClassDescriptor::PresenceStatus,
        FFieldValueChangedDelegate::CreateUObject(this, &UVelvetSocialViewModel::OnFriendPresenceChanged)
    );
}

void UVelvetSocialViewModel::RemoveFriend(const FUniqueNetId& UserId)
{
    int32 Index = FriendList.IndexOfByPredicate([&](const TObjectPtr<UVelvetFriendViewModel>& F)
    {
        return F && F->UserId == UserId.ToString();
    });

    if (Index != INDEX_NONE)
    {
		FriendList[Index]->RemoveAllFieldValueChangedDelegates(this);
        FriendList.RemoveAt(Index);

        UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(FriendList);
    }
}

void UVelvetSocialViewModel::ClearFriends()
{
    for (UVelvetFriendViewModel* Friend : FriendList)
    {
        if (Friend)
        {
            Friend->RemoveAllFieldValueChangedDelegates(this);
        }
    }
    FriendList.Empty();
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(FriendList);
}

void UVelvetSocialViewModel::OnFriendPresenceChanged(UObject* Sender, UE::FieldNotification::FFieldId FieldId)
{
    if (FieldId == UVelvetFriendViewModel::FFieldNotificationClassDescriptor::PresenceStatus)
    {
        UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(FriendList);
    }
}

void UVelvetSocialViewModel::RemoveFriendByNativeId(const FString& NativeId)
{
    FriendList.RemoveAll([&](const TObjectPtr<UVelvetFriendViewModel>& F)
        {
            if (F && F->UserId == NativeId)
            {
                F->RemoveAllFieldValueChangedDelegates(this);
                return true;
            }
            return false;
        });
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(FriendList);
}

void UVelvetSocialViewModel::AddInboundInvitation(UVelvetInboundInvitationViewModel* InInvitation)
{
    InboundFriendInvitations.Add(InInvitation);
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(InboundFriendInvitations);
}

void UVelvetSocialViewModel::RemoveInboundInvitation(const FString& SenderId)
{
    InboundFriendInvitations.RemoveAll([&](const TObjectPtr<UVelvetInboundInvitationViewModel>& I)
        {
            return I && I->SenderId == SenderId;
        });
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(InboundFriendInvitations);
}

void UVelvetSocialViewModel::ClearInboundInvitations()
{
    InboundFriendInvitations.Empty();
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(InboundFriendInvitations);
}