// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "FriendSearchResultViewModel.h"
#include "VelvetFriendSearchViewModel.generated.h"

UENUM(BlueprintType)
enum class EVelvetSearchState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Searching   UMETA(DisplayName = "Searching"),
    Results     UMETA(DisplayName = "Results"),
    NoResults   UMETA(DisplayName = "No Results"),
    Error       UMETA(DisplayName = "Error")
};

/**
 * 
 */
UCLASS()
class VELVETDOOR_API UVelvetFriendSearchViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()
	
public:
    UPROPERTY(FieldNotify, BlueprintReadOnly, Category = "Velvet|Social")
    EVelvetSearchState SearchState = EVelvetSearchState::Idle;

    UPROPERTY(FieldNotify, BlueprintReadWrite, Category = "Velvet|Social")
    FText SearchQuery;

    void SetSearchState(EVelvetSearchState InValue);
    void SetSearchQuery(const FText& InValue);
    void SetResults(const TArray<TObjectPtr<UFriendSearchResultViewModel>>& InResults);
    void AddResult(UFriendSearchResultViewModel* InResult);
    void ClearResults();

    const TArray<TObjectPtr<UFriendSearchResultViewModel>>& GetResults() const { return Results; }

private:
    UPROPERTY(FieldNotify, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    TArray<TObjectPtr<UFriendSearchResultViewModel>> Results;
};
