// Fill out your copyright notice in the Description page of Project Settings.

#include "VelvetGameInstanceSubsystem.h"

UWorld* UVelvetGameInstanceSubsystem::GetWorld() const
{
    if (UGameInstance* GI = GetGameInstance())
    {
        return GI->GetWorld();
    }
    return nullptr;
}

void UVelvetGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    BP_Initialize();
}

void UVelvetGameInstanceSubsystem::Deinitialize()
{
    BP_Deinitialize();
    Super::Deinitialize();
}