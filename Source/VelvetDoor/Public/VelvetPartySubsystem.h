#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlinePartyInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "VelvetPartyViewModel.h"
#include "VelvetPartyTypes.h"
#include "VelvetPartySubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVelvetPartyStateChanged, EVelvetPartyState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVelvetPartyBootstrapped);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVelvetGameLaunched);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVelvetGameJoined);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVelvetPartyError, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVelvetTravelStarting);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVelvetMapLoaded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVelvetGameReady);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVelvetPartyInviteSent, const FString&, TargetPlayerIdStr, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVelvetPartyInvitationReceived, const FString&, InviterDisplayName);

namespace VelvetPartyDataKeys
{
    static const FString State = TEXT("VD_State");
    static const FString SessionToken = TEXT("VD_Token");
    static const FString ConnectStr = TEXT("VD_ConnectStr");
    static const FString MapName = TEXT("VD_MapName");
}

namespace VelvetSessionKeys
{
    static const FName SessionToken = TEXT("VD_Token");
}

UCLASS()
class VELVETDOOR_API UVelvetPartySubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION()
    void OnAuthStateChanged(EVelvetAuthStates NewState);

    UFUNCTION(BlueprintCallable, Category = "Velvet|Party")
    void LaunchGame(TSoftObjectPtr<UWorld> TargetMap);

    UFUNCTION(BlueprintCallable, Category = "Velvet|Party")
    void LeaveParty();

    UFUNCTION(BlueprintCallable, Category = "Velvet|Party")
    void NotifyGameModeReady();

    UFUNCTION(BlueprintCallable, Category = "Velvet|Party")
    bool AuthorizeJoiningPlayer(APlayerController* JoiningController);

    UFUNCTION(BlueprintPure, Category = "Velvet|Party")
    bool IsPartyMember(const FUniqueNetIdRepl& PlayerId) const;

    UFUNCTION(BlueprintPure, Category = "Velvet|Party")
    EVelvetPartyState GetPartyState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Velvet|Party")
    bool IsLocalPlayerPartyHost() const;

    UFUNCTION(BlueprintPure, Category = "Velvet|Party")
    bool IsInParty() const { return CurrentPartyId.IsValid(); }

    UFUNCTION(BlueprintCallable, Category = "Velvet|Party")
    void InviteToParty(const FString& TargetPlayerIdStr);

    UFUNCTION(BlueprintCallable, Category = "Velvet|Party")
    void AcceptPartyInvitation(const FString& InviterUserId);

    UFUNCTION(BlueprintCallable, Category = "Velvet|Party")
    void DeclinePartyInvitation(const FString& InviterUserId);

    UFUNCTION(BlueprintCallable, Category = "Velvet|Party")
    void KickPartyMember(const FString& MemberIdStr);

    UPROPERTY(BlueprintAssignable, Category = "Velvet|Party")
    FOnVelvetPartyStateChanged OnPartyStateChanged;

    /**
     * Fired when the solo party (initial or Redpoint-recreated) becomes ready.
     * Fires from OnOSSPartyJoined when the local player is the party leader.
     */
    UPROPERTY(BlueprintAssignable, Category = "Velvet|Party")
    FOnVelvetPartyBootstrapped OnPartyBootstrapped;

    UPROPERTY(BlueprintAssignable, Category = "Velvet|Party")
    FOnVelvetGameLaunched OnGameLaunched;

    UPROPERTY(BlueprintAssignable, Category = "Velvet|Party")
    FOnVelvetGameJoined OnGameJoined;

    UPROPERTY(BlueprintAssignable, Category = "Velvet|Party")
    FOnVelvetGameReady OnGameReady;

    UPROPERTY(BlueprintAssignable, Category = "Velvet|Party")
    FOnVelvetPartyError OnPartyError;

    UPROPERTY(BlueprintAssignable, Category = "Velvet|Party")
    FOnVelvetPartyInviteSent OnPartyInviteSent;

    UPROPERTY(BlueprintAssignable, Category = "Velvet|Party")
    FOnVelvetPartyInvitationReceived OnPartyInvitationReceived;

    UPROPERTY(BlueprintAssignable, Category = "Velvet|Loading")
    FOnVelvetTravelStarting OnTravelStarting;

    UPROPERTY(BlueprintAssignable, Category = "Velvet|Loading")
    FOnVelvetMapLoaded OnMapLoaded;

private:

    EVelvetPartyState CurrentState = EVelvetPartyState::NotInParty;
    TSharedPtr<const FOnlinePartyId> CurrentPartyId;

    FString PendingMapName;
    FString ActiveSessionToken;
    FString ActiveEOSSessionId;

    static const FName GameSessionName;

    static constexpr int32 MaxFindRetries = 5;
    static constexpr float FindRetryDelaySec = 1.0f;

    int32   FindRetryCount = 0;
    FString PendingJoinToken;
    TSharedPtr<FOnlineSessionSearch> ActiveSessionSearch;
    FTimerHandle FindRetryTimer;

    struct FPendingInvitation
    {
        TSharedRef<const IOnlinePartyJoinInfo> JoinInfo;
        FString DisplayName;
    };
    TMap<FString, FPendingInvitation> PendingInvitationsMap;

    void PublishPartyData_InMenu();
    void PublishPartyData_InGame(const FString& Token,
        const FString& ConnectStringFallback,
        const FString& Map);
    void ConsumePartyData(const FOnlinePartyData& Data);

    void CreateGameSession();
    void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

    void StartJoinFlow(const FString& Token);
    void AttemptFindSession();
    void OnFindSessionsComplete(bool bWasSuccessful);
    void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
    void TravelToJoinedSession();
    void CancelPendingJoin();

    void ExitToMainMenu();

    /**
     * Unified handler for all party joins.
     * Case A (local player IS leader):  solo/recreated party from Redpoint.
     *   → PublishPartyData_InMenu, fire OnPartyBootstrapped.
     * Case B (local player NOT leader): joined host party via invitation.
     *   → ConsumePartyData (may trigger StartJoinFlow if host is in-game).
     */
    void OnOSSPartyJoined(const FUniqueNetId& LocalUserId,
        const FOnlinePartyId& PartyId);

    /**
     * Resets CurrentPartyId and exits game if needed.
     * Does NOT call BootstrapParty — Redpoint fires OnOSSPartyJoined automatically.
     */
    void OnOSSPartyExited(const FUniqueNetId& LocalUserId,
        const FOnlinePartyId& PartyId);

    void OnOSSPartyDataReceived(const FUniqueNetId& LocalUserId,
        const FOnlinePartyId& PartyId,
        const FName& Namespace,
        const FOnlinePartyData& PartyData);

    void OnOSSPartyMemberJoined(const FUniqueNetId& LocalUserId,
        const FOnlinePartyId& PartyId,
        const FUniqueNetId& MemberId);

    void OnOSSPartyMemberExited(const FUniqueNetId& LocalUserId,
        const FOnlinePartyId& PartyId,
        const FUniqueNetId& MemberId,
        EMemberExitedReason Reason);

    void OnOSSPartyInviteReceived(const FUniqueNetId& LocalUserId,
        const IOnlinePartyJoinInfo& JoinInfo);

    void OnOSSPartyInviteRemoved(const FUniqueNetId& LocalUserId,
        const IOnlinePartyJoinInfo& JoinInfo,
        EPartyInvitationRemovedReason Reason);

    void OnAcceptInvitationComplete(const FUniqueNetId& LocalUserId,
        const FOnlinePartyId& PartyId,
        EJoinPartyCompletionResult Result,
        int32 NotApprovedReason);

    void OnSendInvitationComplete(const FUniqueNetId& LocalUserId,
        const FOnlinePartyId& PartyId,
        const FUniqueNetId& RecipientId,
        ESendPartyInvitationCompletionResult Result);

    void OnOSSPartyMemberPromoted(const FUniqueNetId& LocalUserId,
        const FOnlinePartyId& PartyId,
        const FUniqueNetId& NewLeaderId);

    void OnPostLoadMapWithWorld(UWorld* LoadedWorld);

    IOnlinePartyPtr   GetPartyInterface()   const;
    IOnlineSessionPtr GetSessionInterface() const;
    TSharedPtr<const FUniqueNetId> GetLocalNetId() const;

    bool IsCurrentParty(const FOnlinePartyId& PartyId) const;
    TSharedPtr<const FOnlinePartyId> FindCurrentPartyIdShared() const;
    FString MakeSessionToken() const;

    void SetState(EVelvetPartyState NewState);
    void BroadcastError(const FString& Msg);

    UVelvetPartyViewModel* GetPartyViewModel() const;
    void PushAllToViewModel();
    void RefreshMemberViewModels();
    void RefreshPendingInvitationViewModels();

    // ── Slot management (host only) ───────────────────────────────────────────────

    /**
     * Assigns the lowest available slot (2–4) to a new member.
     * Writes VD_Slot_N = MemberIdStr to party data.
     * Called by the host only from OnOSSPartyMemberJoined.
     */
    void AssignSlotToMember(const FUniqueNetId& MemberId);

    /**
     * Clears the slot held by an exiting member.
     * Called by the host only from OnOSSPartyMemberExited.
     */
    void RevokeSlotForMember(const FUniqueNetId& MemberId);

    FDelegateHandle H_PartyJoined;
    FDelegateHandle H_PartyExited;
    FDelegateHandle H_PartyDataReceived;
    FDelegateHandle H_PartyMemberJoined;
    FDelegateHandle H_PartyMemberExited;
    FDelegateHandle H_PartyInviteReceived;
    FDelegateHandle H_PartyInviteRemoved;
    FDelegateHandle H_PartyMemberPromoted;
    FDelegateHandle H_CreateSessionComplete;
    FDelegateHandle H_FindSessionsComplete;
    FDelegateHandle H_JoinSessionComplete;
    FDelegateHandle H_PostLoadMap;
};