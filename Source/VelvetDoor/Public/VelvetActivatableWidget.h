#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "VelvetActivatableWidget.generated.h"

UCLASS(Abstract, BlueprintType, Blueprintable)
class VELVETDOOR_API UVelvetActivatableWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    // Appelé quand le widget devient actif (top of stack)
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Velvet|UI")
    void OnWidgetActivated();

    // Appelé quand le widget est désactivé (recouvert ou retiré)
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Velvet|UI")
    void OnWidgetDeactivated();

protected:
    virtual void NativeOnActivated() override;
    virtual void NativeOnDeactivated() override;
};