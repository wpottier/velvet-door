// Fill out your copyright notice in the Description page of Project Settings.


#include "VelvetFriendSearchViewModel.h"

void UVelvetFriendSearchViewModel::SetSearchState(EVelvetSearchState InValue)
{
    UE_MVVM_SET_PROPERTY_VALUE(SearchState, InValue);
}

void UVelvetFriendSearchViewModel::SetSearchQuery(const FText& InValue)
{
    UE_MVVM_SET_PROPERTY_VALUE(SearchQuery, InValue);
}

void UVelvetFriendSearchViewModel::SetResults(
    const TArray<TObjectPtr<UFriendSearchResultViewModel>>& InResults)
{
    UE_MVVM_SET_PROPERTY_VALUE(Results, InResults);
}

void UVelvetFriendSearchViewModel::AddResult(UFriendSearchResultViewModel* InResult)
{
    Results.Add(InResult);
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Results);
}

void UVelvetFriendSearchViewModel::ClearResults()
{
    Results.Empty();
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Results);
}