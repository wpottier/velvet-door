#pragma once

#include "CoreMinimal.h"
#include "VelvetPartyTypes.generated.h"

/**
 * Party / game state machine shared between UVelvetPartySubsystem
 * and UVelvetPartyViewModel.
 *
 * Isolated here to avoid a circular include dependency between the two files.
 */
UENUM(BlueprintType)
enum class EVelvetPartyState : uint8
{
    NotInParty      UMETA(DisplayName = "Not In Party"),
    Bootstrapping   UMETA(DisplayName = "Bootstrapping"),
    InMenu          UMETA(DisplayName = "In Menu"),
    LaunchingGame   UMETA(DisplayName = "Launching Game"),
    InGame          UMETA(DisplayName = "In Game"),
};