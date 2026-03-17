// Fill out your copyright notice in the Description page of Project Settings.


#include "FriendSearchResultViewModel.h"

void UFriendSearchResultViewModel::SetUserId(const FString& InValue)
{
    UE_MVVM_SET_PROPERTY_VALUE(UserId, InValue);
}

void UFriendSearchResultViewModel::SetDisplayName(const FText& InValue)
{
    UE_MVVM_SET_PROPERTY_VALUE(DisplayName, InValue);
}

void UFriendSearchResultViewModel::SetDisplayName(const FString& InValue)
{
    UE_MVVM_SET_PROPERTY_VALUE(DisplayName, FText::FromString(InValue));
}

void UFriendSearchResultViewModel::SetIsAlreadyFriend(bool InValue)
{
    UE_MVVM_SET_PROPERTY_VALUE(bIsAlreadyFriend, InValue);
}

void UFriendSearchResultViewModel::SetIsRequestPending(bool InValue)
{
    UE_MVVM_SET_PROPERTY_VALUE(bIsRequestPending, InValue);
}