// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "VelvetInboundInvitationViewModel.generated.h"

/**
 * 
 */
UCLASS()
class VELVETDOOR_API UVelvetInboundInvitationViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
    UPROPERTY(FieldNotify, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    FText DisplayName;

    UPROPERTY(FieldNotify, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    FString SenderId;

    void SetDisplayName(const FText& InValue);
    void SetDisplayName(const FString& InValue);
    void SetSenderId(const FString& InValue);
};
