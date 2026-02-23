#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "IWDoorClipboardLibrary.generated.h"

UCLASS()
class UIWDoorClipboardLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Improbability Works|Utilities")
    static void CopyToClipboard(const FString& Text);
};
