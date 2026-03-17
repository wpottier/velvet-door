// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "VelvetFriendViewModel.generated.h"

UENUM(BlueprintType)
enum class EVelvetFriendPresence : uint8
{
    Offline  UMETA(DisplayName = "Offline"),
    Online   UMETA(DisplayName = "Online"),
    InGame   UMETA(DisplayName = "In Game")
};

/**
 * 
 */
UCLASS()
class VELVETDOOR_API UVelvetFriendViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()
	
public:
    UPROPERTY(FieldNotify, BlueprintReadOnly, Category = "Velvet|Social")
    FString UserId;

    UPROPERTY(FieldNotify, BlueprintReadOnly, Category = "Velvet|Social")
    FText DisplayName;

    UPROPERTY(FieldNotify, BlueprintReadOnly, Category = "Velvet|Social")
    EVelvetFriendPresence PresenceStatus = EVelvetFriendPresence::Offline;

    UPROPERTY(FieldNotify, BlueprintReadOnly, Category = "Velvet|Social")
    bool bIsJoinable = false;

    UPROPERTY(FieldNotify, BlueprintReadOnly, Category = "Velvet|Social")
    bool bIsRemovable = false;

    void SetUserId(const FString& InUserId);
    void SetDisplayName(const FText& InValue);
    void SetDisplayName(const FString& InValue);
    void SetPresenceStatus(EVelvetFriendPresence InValue);
    void SetIsJoinable(bool InValue);
    void SetIsRemovable(bool InValue);

};

FORCEINLINE bool operator==(const UVelvetFriendViewModel& A, const UVelvetFriendViewModel& B)
{
    return A.UserId == B.UserId;
}

FORCEINLINE bool operator==(const UVelvetFriendViewModel& A, const FString& UserId)
{
    return A.UserId == UserId;
}
