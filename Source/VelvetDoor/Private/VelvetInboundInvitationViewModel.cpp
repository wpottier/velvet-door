// Fill out your copyright notice in the Description page of Project Settings.


#include "VelvetInboundInvitationViewModel.h"

void UVelvetInboundInvitationViewModel::SetDisplayName(const FText& InValue)
{
    UE_MVVM_SET_PROPERTY_VALUE(DisplayName, InValue);
}

void UVelvetInboundInvitationViewModel::SetDisplayName(const FString& InValue)
{
    UE_MVVM_SET_PROPERTY_VALUE(DisplayName, FText::FromString(InValue));
}

void UVelvetInboundInvitationViewModel::SetSenderId(const FString& InValue)
{
    UE_MVVM_SET_PROPERTY_VALUE(SenderId, InValue);
}