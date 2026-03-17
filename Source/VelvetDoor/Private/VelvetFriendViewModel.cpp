#include "VelvetFriendViewModel.h"

void UVelvetFriendViewModel::SetUserId(const FString& InUserId)
{
    UE_MVVM_SET_PROPERTY_VALUE(UserId, InUserId);
}

void UVelvetFriendViewModel::SetDisplayName(const FText& InValue)
{
    UE_MVVM_SET_PROPERTY_VALUE(DisplayName, InValue);
}

void UVelvetFriendViewModel::SetDisplayName(const FString& InValue)
{
    UE_MVVM_SET_PROPERTY_VALUE(DisplayName, FText::FromString(InValue));
}

void UVelvetFriendViewModel::SetPresenceStatus(EVelvetFriendPresence InValue)
{
    UE_MVVM_SET_PROPERTY_VALUE(PresenceStatus, InValue);
}

void UVelvetFriendViewModel::SetIsJoinable(bool InValue)
{
    UE_MVVM_SET_PROPERTY_VALUE(bIsJoinable, InValue);
}

void UVelvetFriendViewModel::SetIsRemovable(bool InValue)
{
    UE_MVVM_SET_PROPERTY_VALUE(bIsRemovable, InValue);
}