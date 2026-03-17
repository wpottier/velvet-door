#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UClipboardLibrary.generated.h"

UCLASS()
class UClipboardLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Improbability Works|Utilities")
    static void CopyToClipboard(const FString& Text);
};
