// Fill out your copyright notice in the Description page of Project Settings.


#include "UOnlineLibrary.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"

FUniqueNetIdRepl UOnlineLibrary::CreateUniqueNetIdFromString(const FString& StringId)
{
	if (StringId.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("VelvetDoor|CreateUniqueNetIdFromString: StringId is empty."));
		return FUniqueNetIdRepl();
	}

	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (!OnlineSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("VelvetDoor|CreateUniqueNetIdFromString: No OnlineSubsystem available."));
		return FUniqueNetIdRepl();
	}

	IOnlineIdentityPtr IdentityInterface = OnlineSubsystem->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("VelvetDoor|CreateUniqueNetIdFromString: IdentityInterface is not valid."));
		return FUniqueNetIdRepl();
	}

	TSharedPtr<const FUniqueNetId> UniqueId = IdentityInterface->CreateUniquePlayerId(StringId);
	if (!UniqueId.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("VelvetDoor|CreateUniqueNetIdFromString: Failed to create UniqueNetId from string '%s'."), *StringId);
		return FUniqueNetIdRepl();
	}

	return FUniqueNetIdRepl(UniqueId.ToSharedRef());
}