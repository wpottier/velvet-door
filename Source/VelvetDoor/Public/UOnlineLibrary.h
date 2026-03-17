// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameFramework/OnlineReplStructs.h"
#include "UOnlineLibrary.generated.h"

/**
 * 
 */
UCLASS()
class UOnlineLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Creates a Blueprint-compatible Unique Net Id (FUniqueNetIdRepl)
	 * from a raw string (e.g. a session id string retrieved via GetNamedSession).
	 *
	 * Use this to feed the "Searching User Id", "Friend Id" or "Session Id" pins
	 * on nodes like FindSessionById.
	 *
	 * Returns an invalid FUniqueNetIdRepl if the OnlineSubsystem is unavailable
	 * or if the string cannot be parsed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Improbability Works|Online")
	static FUniqueNetIdRepl CreateUniqueNetIdFromString(const FString& StringId);
	
};
