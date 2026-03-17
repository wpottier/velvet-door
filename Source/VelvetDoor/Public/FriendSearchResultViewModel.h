// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "FriendSearchResultViewModel.generated.h"

/**
 * 
 */
UCLASS()
class VELVETDOOR_API UFriendSearchResultViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()
	
public:
    UPROPERTY(FieldNotify, BlueprintReadOnly, Category = "Velvet|Social")
    FString UserId;

    UPROPERTY(FieldNotify, BlueprintReadOnly, Category = "Velvet|Social")
    FText DisplayName;

    UPROPERTY(FieldNotify, BlueprintReadOnly, Category = "Velvet|Social")
    bool bIsAlreadyFriend = false;

    UPROPERTY(FieldNotify, BlueprintReadOnly, Category = "Velvet|Social")
    bool bIsRequestPending = false;

    void SetUserId(const FString& InValue);
    void SetDisplayName(const FText& InValue);
    void SetDisplayName(const FString& InValue);
    void SetIsAlreadyFriend(bool InValue);
    void SetIsRequestPending(bool InValue);
};
