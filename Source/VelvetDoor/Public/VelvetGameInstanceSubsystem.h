// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "VelvetGameInstanceSubsystem.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, Abstract)
class VELVETDOOR_API UVelvetGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual UWorld* GetWorld() const override;

protected:
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Initialize")
	void BP_Initialize();

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Deinitialize")
	void BP_Deinitialize();
};
