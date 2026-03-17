#pragma once

#include "CoreMinimal.h"
#include "VelvetActivatableWidget.h"
#include "VelvetShellChildWidget.generated.h"

USTRUCT(BlueprintType)
struct FPanelShellConfig
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Shell")
    FText ApplyButtonLabel = FText::FromString("Apply");

    UPROPERTY(BlueprintReadWrite, Category = "Shell")
    bool bShowApply = false;

    UPROPERTY(BlueprintReadWrite, Category = "Shell")
    bool bShowBack = true;
};

UCLASS(Abstract, BlueprintType, Blueprintable)
class VELVETDOOR_API UVelvetShellChildWidget : public UVelvetActivatableWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Velvet|UI|Shell")
    FPanelShellConfig GetShellConfig();

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Velvet|UI|Shell")
    void OnApplyRequested();

protected:
    virtual FPanelShellConfig GetShellConfig_Implementation();
};