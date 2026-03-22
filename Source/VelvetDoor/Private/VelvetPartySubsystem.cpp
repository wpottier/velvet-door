#include "VelvetPartySubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSUserCache/UserInfo.h"
#include "RedpointEOSPlatform/Types/RelationshipType.h"
#include "RedpointEOSIdentity/IdentityUserId.h"
#include "RedpointEOSUserCache/ExternalUserInfo.h"
#include "Interfaces/OnlinePartyInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Misc/CoreDelegates.h"
#include "Engine/World.h"
#include "Engine/NetDriver.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "VelvetPartyTypes.h"
#include "VelvetPartyViewModel.h"
#include "VelvetMVVMSubsystem.h"
#include "TimerManager.h"
#include "Kismet/KismetSystemLibrary.h"

using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Identity;

DEFINE_LOG_CATEGORY_STATIC(LogVelvetParty, Log, All);

const FName UVelvetPartySubsystem::GameSessionName = NAME_GameSession;

// ─────────────────────────────────────────────────────────────────────────────
// UGameInstanceSubsystem
// ─────────────────────────────────────────────────────────────────────────────

void UVelvetPartySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    H_PostLoadMap = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
        this, &UVelvetPartySubsystem::OnPostLoadMapWithWorld);

    IOnlinePartyPtr Party = GetPartyInterface();
    if (!Party.IsValid())
    {
        UE_LOG(LogVelvetParty, Warning,
            TEXT("Initialize: party interface unavailable at startup."));
        return;
    }

    H_PartyJoined = Party->OnPartyJoinedDelegates.AddUObject(this, &UVelvetPartySubsystem::OnOSSPartyJoined);
    H_PartyExited = Party->OnPartyExitedDelegates.AddUObject(this, &UVelvetPartySubsystem::OnOSSPartyExited);
    H_PartyDataReceived = Party->OnPartyDataReceivedDelegates.AddUObject(this, &UVelvetPartySubsystem::OnOSSPartyDataReceived);
    H_PartyMemberJoined = Party->OnPartyMemberJoinedDelegates.AddUObject(this, &UVelvetPartySubsystem::OnOSSPartyMemberJoined);
    H_PartyMemberExited = Party->OnPartyMemberExitedDelegates.AddUObject(this, &UVelvetPartySubsystem::OnOSSPartyMemberExited);
    H_PartyInviteReceived = Party->OnPartyInviteReceivedExDelegates.AddUObject(this, &UVelvetPartySubsystem::OnOSSPartyInviteReceived);
    H_PartyInviteRemoved = Party->OnPartyInviteRemovedExDelegates.AddUObject(this, &UVelvetPartySubsystem::OnOSSPartyInviteRemoved);
    H_PartyMemberPromoted = Party->OnPartyMemberPromotedDelegates.AddUObject(this, &UVelvetPartySubsystem::OnOSSPartyMemberPromoted);

    // Auth observer for sign-out cleanup only.
    // Sign-in party creation is handled by Redpoint "Ensure Always In Party".
    if (UVelvetAuthSubsystem* Auth = GetGameInstance()->GetSubsystem<UVelvetAuthSubsystem>())
    {
        Auth->OnAuthStateChanged.AddDynamic(this, &UVelvetPartySubsystem::OnAuthStateChanged);
    }
}

void UVelvetPartySubsystem::Deinitialize()
{
    FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(H_PostLoadMap);

    if (IOnlinePartyPtr Party = GetPartyInterface())
    {
        Party->OnPartyJoinedDelegates.Remove(H_PartyJoined);
        Party->OnPartyExitedDelegates.Remove(H_PartyExited);
        Party->OnPartyDataReceivedDelegates.Remove(H_PartyDataReceived);
        Party->OnPartyMemberJoinedDelegates.Remove(H_PartyMemberJoined);
        Party->OnPartyMemberExitedDelegates.Remove(H_PartyMemberExited);
        Party->OnPartyInviteReceivedExDelegates.Remove(H_PartyInviteReceived);
        Party->OnPartyInviteRemovedExDelegates.Remove(H_PartyInviteRemoved);
        Party->OnPartyMemberPromotedDelegates.Remove(H_PartyMemberPromoted);
    }

    if (IOnlineSessionPtr Session = GetSessionInterface())
    {
        Session->ClearOnCreateSessionCompleteDelegate_Handle(H_CreateSessionComplete);
        Session->ClearOnFindSessionsCompleteDelegate_Handle(H_FindSessionsComplete);
        Session->ClearOnJoinSessionCompleteDelegate_Handle(H_JoinSessionComplete);
    }

    CancelPendingJoin();

    if (UVelvetAuthSubsystem* Auth = GetGameInstance()->GetSubsystem<UVelvetAuthSubsystem>())
    {
        Auth->OnAuthStateChanged.RemoveDynamic(this, &UVelvetPartySubsystem::OnAuthStateChanged);
    }

    Super::Deinitialize();
}

void UVelvetPartySubsystem::OnAuthStateChanged(EVelvetAuthStates NewState)
{
    if (NewState != EVelvetAuthStates::SignedIn)
    {
        CancelPendingJoin();
        CurrentPartyId.Reset();
        SetState(EVelvetPartyState::NotInParty);
        PushAllToViewModel();
    }
}

UVelvetPartyViewModel* UVelvetPartySubsystem::GetPartyViewModel() const
{
    UGameInstance* GI = GetGameInstance();
    if (!GI) { return nullptr; }
    UVelvetMVVMSubsystem* MVVM = GI->GetSubsystem<UVelvetMVVMSubsystem>();
    if (!MVVM) { return nullptr; }
    return MVVM->GetPartyViewModel();
}

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────

void UVelvetPartySubsystem::LaunchGame(TSoftObjectPtr<UWorld> TargetMap)
{
    if (TargetMap.IsNull()) {
		BroadcastError(TEXT("LaunchGame: TargetMap is null."));
        return;
    }

    if (!IsLocalPlayerPartyHost())
    {
        BroadcastError(TEXT("LaunchGame: only the party host can launch the game."));
        return;
    }

    if (CurrentState == EVelvetPartyState::LaunchingGame ||
        CurrentState == EVelvetPartyState::InGame)
    {
        UE_LOG(LogVelvetParty, Warning, TEXT("LaunchGame: already launching or in game."));
        return;
    }

    FString MapPath = TargetMap.ToSoftObjectPath().GetAssetName();
    if (MapPath.IsEmpty())
    {
        BroadcastError(TEXT("LaunchGame: MapPath is empty."));
        return;
    }

	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Launching game with map: %s"), *MapPath));

    PendingMapName = MapPath;
    ActiveSessionToken = MakeSessionToken();

    SetState(EVelvetPartyState::LaunchingGame);
    CreateGameSession();
}

void UVelvetPartySubsystem::LeaveParty()
{
    if (CurrentState == EVelvetPartyState::InGame ||
        CurrentState == EVelvetPartyState::LaunchingGame)
    {
        ExitToMainMenu();
    }

    CancelPendingJoin();

    IOnlinePartyPtr Party = GetPartyInterface();
    TSharedPtr<const FUniqueNetId> UserId = GetLocalNetId();
    TSharedPtr<const FOnlinePartyId> Pid = FindCurrentPartyIdShared();

    if (!Party.IsValid() || !UserId.IsValid() || !Pid.IsValid()) { return; }

    Party->LeaveParty(*UserId, *Pid, /*bSynchronizeLeave=*/false, FOnLeavePartyComplete());
    // Redpoint fires OnOSSPartyExited then auto-creates a new solo party → OnOSSPartyJoined (Case A).
}

// ─────────────────────────────────────────────────────────────────────────────
// Accessors
// ─────────────────────────────────────────────────────────────────────────────

bool UVelvetPartySubsystem::IsLocalPlayerPartyHost() const
{
    IOnlinePartyPtr Party = GetPartyInterface();
    TSharedPtr<const FUniqueNetId> UserId = GetLocalNetId();
    if (!Party.IsValid() || !UserId.IsValid() || !CurrentPartyId.IsValid()) { return false; }

    FOnlinePartyConstPtr OnlineParty = Party->GetParty(*UserId, *CurrentPartyId);
    if (!OnlineParty.IsValid()) { return false; }

    return *OnlineParty->LeaderId == *UserId;
}

// ─────────────────────────────────────────────────────────────────────────────
// Party Data
// ─────────────────────────────────────────────────────────────────────────────

void UVelvetPartySubsystem::PublishPartyData_InMenu()
{
    IOnlinePartyPtr Party = GetPartyInterface();
    TSharedPtr<const FUniqueNetId> UserId = GetLocalNetId();
    if (!Party.IsValid() || !UserId.IsValid() || !CurrentPartyId.IsValid()) { return; }

    FOnlinePartyData Data;
    Data.SetAttribute(VelvetPartyDataKeys::State, FVariantData(FString(TEXT("InMenu"))));
    Party->UpdatePartyData(*UserId, *CurrentPartyId, NAME_None, Data);
}

void UVelvetPartySubsystem::PublishPartyData_InGame(
    const FString& Token,
    const FString& ConnectStringFallback,
    const FString& Map)
{
    IOnlinePartyPtr Party = GetPartyInterface();
    TSharedPtr<const FUniqueNetId> UserId = GetLocalNetId();
    if (!Party.IsValid() || !UserId.IsValid() || !CurrentPartyId.IsValid()) { return; }

    FOnlinePartyData Data;
    Data.SetAttribute(VelvetPartyDataKeys::State, FVariantData(FString(TEXT("InGame"))));
    Data.SetAttribute(VelvetPartyDataKeys::SessionToken, FVariantData(Token));
    Data.SetAttribute(VelvetPartyDataKeys::ConnectStr, FVariantData(ConnectStringFallback));
    Data.SetAttribute(VelvetPartyDataKeys::MapName, FVariantData(Map));
    Party->UpdatePartyData(*UserId, *CurrentPartyId, NAME_None, Data);
}

void UVelvetPartySubsystem::ConsumePartyData(const FOnlinePartyData& Data)
{
    if (IsLocalPlayerPartyHost()) { return; }
    if (CurrentState != EVelvetPartyState::InMenu) { return; }

    FVariantData StateVar;
    if (!Data.GetAttribute(VelvetPartyDataKeys::State, StateVar)) { return; }

    FString StateStr;
    StateVar.GetValue(StateStr);

    if (StateStr == TEXT("InGame"))
    {
        FVariantData TokenVar;
        if (!Data.GetAttribute(VelvetPartyDataKeys::SessionToken, TokenVar))
        {
            BroadcastError(TEXT("Party data is InGame but SessionToken is missing."));
            return;
        }
        FString Token;
        TokenVar.GetValue(Token);
        StartJoinFlow(Token);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Game Launch — host
// ─────────────────────────────────────────────────────────────────────────────

void UVelvetPartySubsystem::CreateGameSession()
{
    IOnlineSessionPtr Session = GetSessionInterface();
    if (!Session.IsValid())
    {
        BroadcastError(TEXT("LaunchGame: session interface unavailable."));
        SetState(EVelvetPartyState::InMenu);
        return;
    }

    H_CreateSessionComplete = Session->AddOnCreateSessionCompleteDelegate_Handle(
        FOnCreateSessionCompleteDelegate::CreateUObject(
            this, &UVelvetPartySubsystem::OnCreateSessionComplete));

    FOnlineSessionSettings Settings;
    Settings.bIsLANMatch = false;
    Settings.bUsesPresence = false;
    Settings.bShouldAdvertise = true;
    Settings.bUseLobbiesIfAvailable = false;
    Settings.bAllowJoinInProgress = true;
    Settings.bAllowInvites = false;
    Settings.NumPublicConnections = 4;
    Settings.Set(FName(TEXT("MAPNAME")), PendingMapName,
        EOnlineDataAdvertisementType::ViaOnlineService);
    Settings.Set(VelvetSessionKeys::SessionToken, ActiveSessionToken,
        EOnlineDataAdvertisementType::ViaOnlineService);

	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Creating session with token: %s"), *ActiveSessionToken));
    Session->CreateSession(0, GameSessionName, Settings);
}

void UVelvetPartySubsystem::OnCreateSessionComplete(FName /*SessionName*/, bool bWasSuccessful)
{
	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OnCreateSessionComplete: success=%s"), bWasSuccessful ? TEXT("true") : TEXT("false")));

    IOnlineSessionPtr Session = GetSessionInterface();
    Session->ClearOnCreateSessionCompleteDelegate_Handle(H_CreateSessionComplete);

    if (!bWasSuccessful)
    {
        BroadcastError(TEXT("Session creation failed."));
        SetState(EVelvetPartyState::InMenu);
        return;
    }

    FString ConnectString;
    if (!Session->GetResolvedConnectString(GameSessionName, ConnectString))
    {
        UE_LOG(LogVelvetParty, Warning, TEXT("GetResolvedConnectString failed — storing empty fallback."));
    }

    PublishPartyData_InGame(ActiveSessionToken, ConnectString, PendingMapName);

    SetState(EVelvetPartyState::InGame);
    OnGameLaunched.Broadcast();
    OnTravelStarting.Broadcast();

    UWorld* World = GetGameInstance()->GetWorld();
    if (!World) { BroadcastError(TEXT("OnCreateSessionComplete: world is null.")); return; }

    if (UVelvetPartyViewModel* VM = GetPartyViewModel()) { VM->SetIsTravelling(true); }

    const FString TravelURL = FString::Printf(TEXT("%s?listen"), *PendingMapName);
    UE_LOG(LogVelvetParty, Log, TEXT("Host ServerTravel → %s"), *TravelURL);
    World->ServerTravel(TravelURL);
}

// ─────────────────────────────────────────────────────────────────────────────
// Game Join — client
// ─────────────────────────────────────────────────────────────────────────────

void UVelvetPartySubsystem::StartJoinFlow(const FString& Token)
{
    UE_LOG(LogVelvetParty, Log, TEXT("StartJoinFlow — token: %s"), *Token);
    PendingJoinToken = Token;
    FindRetryCount = 0;
    SetState(EVelvetPartyState::LaunchingGame);
    AttemptFindSession();
}

void UVelvetPartySubsystem::AttemptFindSession()
{
    IOnlineSessionPtr Session = GetSessionInterface();
    TSharedPtr<const FUniqueNetId> UserId = GetLocalNetId();

    if (!Session.IsValid() || !UserId.IsValid())
    {
        BroadcastError(TEXT("AttemptFindSession: unavailable."));
        SetState(EVelvetPartyState::InMenu);
        return;
    }

    ActiveSessionSearch = MakeShared<FOnlineSessionSearch>();
    ActiveSessionSearch->bIsLanQuery = false;
    ActiveSessionSearch->MaxSearchResults = 1;
    ActiveSessionSearch->QuerySettings.Set(
        VelvetSessionKeys::SessionToken, PendingJoinToken, EOnlineComparisonOp::Equals);

    H_FindSessionsComplete = Session->AddOnFindSessionsCompleteDelegate_Handle(
        FOnFindSessionsCompleteDelegate::CreateUObject(
            this, &UVelvetPartySubsystem::OnFindSessionsComplete));

    UE_LOG(LogVelvetParty, Log, TEXT("FindSessions attempt %d/%d"),
        FindRetryCount + 1, MaxFindRetries);

    Session->FindSessions(0, ActiveSessionSearch.ToSharedRef());
}

void UVelvetPartySubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
    IOnlineSessionPtr Session = GetSessionInterface();
    Session->ClearOnFindSessionsCompleteDelegate_Handle(H_FindSessionsComplete);

    const bool bFound = bWasSuccessful
        && ActiveSessionSearch.IsValid()
        && ActiveSessionSearch->SearchResults.Num() > 0;

    if (!bFound)
    {
        ++FindRetryCount;
        if (FindRetryCount < MaxFindRetries)
        {
            UE_LOG(LogVelvetParty, Log,
                TEXT("Session not found yet — retry %d/%d in %.1fs."),
                FindRetryCount, MaxFindRetries, FindRetryDelaySec);

            if (UWorld* World = GetGameInstance()->GetWorld())
            {
                World->GetTimerManager().SetTimer(
                    FindRetryTimer,
                    this, &UVelvetPartySubsystem::AttemptFindSession,
                    FindRetryDelaySec, /*bLoop=*/false);
                return;
            }
        }

        BroadcastError(FString::Printf(
            TEXT("Could not find session (token '%s') after %d attempts."),
            *PendingJoinToken, MaxFindRetries));
        SetState(EVelvetPartyState::InMenu);
        ActiveSessionSearch.Reset();
        return;
    }

    TSharedPtr<const FUniqueNetId> UserId = GetLocalNetId();
    if (!Session.IsValid() || !UserId.IsValid())
    {
        BroadcastError(TEXT("OnFindSessionsComplete: interface lost."));
        SetState(EVelvetPartyState::InMenu);
        return;
    }

    H_JoinSessionComplete = Session->AddOnJoinSessionCompleteDelegate_Handle(
        FOnJoinSessionCompleteDelegate::CreateUObject(
            this, &UVelvetPartySubsystem::OnJoinSessionComplete));

    Session->JoinSession(*UserId, GameSessionName, ActiveSessionSearch->SearchResults[0]);
}

void UVelvetPartySubsystem::OnJoinSessionComplete(
    FName /*SessionName*/,
    EOnJoinSessionCompleteResult::Type Result)
{
    IOnlineSessionPtr Session = GetSessionInterface();
    Session->ClearOnJoinSessionCompleteDelegate_Handle(H_JoinSessionComplete);
    ActiveSessionSearch.Reset();

    if (Result != EOnJoinSessionCompleteResult::Success)
    {
        BroadcastError(FString::Printf(
            TEXT("JoinSession failed: result %d."), static_cast<int32>(Result)));
        SetState(EVelvetPartyState::InMenu);
        return;
    }

    TravelToJoinedSession();
}

void UVelvetPartySubsystem::TravelToJoinedSession()
{
    IOnlineSessionPtr Session = GetSessionInterface();
    if (!Session.IsValid())
    {
        BroadcastError(TEXT("TravelToJoinedSession: session interface lost."));
        SetState(EVelvetPartyState::InMenu);
        return;
    }

    FString ConnectString;
    if (!Session->GetResolvedConnectString(GameSessionName, ConnectString))
    {
        BroadcastError(TEXT("TravelToJoinedSession: GetResolvedConnectString failed."));
        SetState(EVelvetPartyState::InMenu);
        return;
    }

    UWorld* World = GetGameInstance()->GetWorld();
    APlayerController* PC = World
        ? GetGameInstance()->GetFirstLocalPlayerController(World) : nullptr;

    if (!PC)
    {
        BroadcastError(TEXT("TravelToJoinedSession: no local PlayerController."));
        SetState(EVelvetPartyState::InMenu);
        return;
    }

    SetState(EVelvetPartyState::InGame);
    OnGameJoined.Broadcast();
    OnTravelStarting.Broadcast();

    UE_LOG(LogVelvetParty, Log, TEXT("Client open → %s"), *ConnectString);
    PC->ConsoleCommand(FString::Printf(TEXT("open %s"), *ConnectString));

    if (UVelvetPartyViewModel* VM = GetPartyViewModel()) { VM->SetIsTravelling(true); }
}

void UVelvetPartySubsystem::CancelPendingJoin()
{
    if (UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr)
    {
        World->GetTimerManager().ClearTimer(FindRetryTimer);
    }
    if (IOnlineSessionPtr Session = GetSessionInterface())
    {
        Session->ClearOnFindSessionsCompleteDelegate_Handle(H_FindSessionsComplete);
        Session->ClearOnJoinSessionCompleteDelegate_Handle(H_JoinSessionComplete);
    }
    ActiveSessionSearch.Reset();
    FindRetryCount = 0;
    PendingJoinToken = FString();
}

// ─────────────────────────────────────────────────────────────────────────────
// Exit
// ─────────────────────────────────────────────────────────────────────────────

void UVelvetPartySubsystem::ExitToMainMenu()
{
    UE_LOG(LogVelvetParty, Log, TEXT("ExitToMainMenu."));
    CancelPendingJoin();

    if (IOnlineSessionPtr Session = GetSessionInterface())
    {
        Session->DestroySession(GameSessionName);
    }

    SetState(EVelvetPartyState::InMenu);

    UWorld* World = GetGameInstance()->GetWorld();
    APlayerController* PC = World
        ? GetGameInstance()->GetFirstLocalPlayerController(World) : nullptr;
    if (!PC) { return; }

    OnTravelStarting.Broadcast();
    PC->ClientTravel(TEXT("/Game/Maps/Map_Menu"), TRAVEL_Absolute);
}

// ─────────────────────────────────────────────────────────────────────────────
// NotifyGameModeReady
// ─────────────────────────────────────────────────────────────────────────────

void UVelvetPartySubsystem::NotifyGameModeReady()
{
    UWorld* World = GetGameInstance()->GetWorld();
    if (!World) { return; }

    const ENetMode NetMode = World->GetNetMode();
    const bool bIsServer = (NetMode == NM_DedicatedServer ||
        NetMode == NM_ListenServer ||
        NetMode == NM_Standalone);
    if (!bIsServer)
    {
        UE_LOG(LogVelvetParty, Verbose, TEXT("NotifyGameModeReady: not server, skipping."));
        return;
    }

    IOnlineSessionPtr Session = GetSessionInterface();
    if (!Session.IsValid())
    {
        UE_LOG(LogVelvetParty, Warning, TEXT("NotifyGameModeReady: session interface unavailable."));
        OnGameReady.Broadcast();
        return;
    }

    FNamedOnlineSession* NamedSession = Session->GetNamedSession(GameSessionName);
    if (NamedSession && NamedSession->SessionInfo.IsValid())
    {
        ActiveEOSSessionId = NamedSession->SessionInfo->GetSessionId().ToString();
        UE_LOG(LogVelvetParty, Log,
            TEXT("NotifyGameModeReady: EOS Session ID = %s"), *ActiveEOSSessionId);
    }

    FString ConnectString;
    Session->GetResolvedConnectString(GameSessionName, ConnectString);
    PublishPartyData_InGame(ActiveSessionToken, ConnectString, PendingMapName);

    if (IOnlinePartyPtr Party = GetPartyInterface())
    {
        TSharedPtr<const FUniqueNetId> UserId = GetLocalNetId();
        if (UserId.IsValid() && CurrentPartyId.IsValid())
        {
            FOnlinePartyData EOSIdData;
            EOSIdData.SetAttribute(TEXT("VD_EOSSessionId"), FVariantData(ActiveEOSSessionId));
            Party->UpdatePartyData(*UserId, *CurrentPartyId, NAME_None, EOSIdData);
        }
    }

    OnGameReady.Broadcast();
    UE_LOG(LogVelvetParty, Log, TEXT("NotifyGameModeReady complete."));
}

// ─────────────────────────────────────────────────────────────────────────────
// AuthorizeJoiningPlayer
// ─────────────────────────────────────────────────────────────────────────────

bool UVelvetPartySubsystem::AuthorizeJoiningPlayer(APlayerController* JoiningController)
{
    if (!JoiningController)
    {
        UE_LOG(LogVelvetParty, Warning, TEXT("AuthorizeJoiningPlayer: null controller — denying."));
        return false;
    }

    if (ULocalPlayer* LP = GetGameInstance()->GetFirstGamePlayer())
    {
        if (LP->PlayerController == JoiningController) { return true; }
    }

    const APlayerState* PS = JoiningController->GetPlayerState<APlayerState>();
    if (!PS)
    {
        UE_LOG(LogVelvetParty, Warning, TEXT("AuthorizeJoiningPlayer: no PlayerState — denying."));
        return false;
    }

    const FUniqueNetIdRepl JoiningId = PS->GetUniqueId();
    if (!JoiningId.IsValid())
    {
        UE_LOG(LogVelvetParty, Warning, TEXT("AuthorizeJoiningPlayer: invalid UniqueNetId — denying."));
        return false;
    }

    const bool bAuthorized = IsPartyMember(JoiningId);
    UE_LOG(LogVelvetParty, Log, TEXT("AuthorizeJoiningPlayer: %s → %s"),
        *JoiningId->ToString(),
        bAuthorized ? TEXT("AUTHORIZED") : TEXT("DENIED — not in party"));

    return bAuthorized;
}

// ─────────────────────────────────────────────────────────────────────────────
// IsPartyMember
// ─────────────────────────────────────────────────────────────────────────────

bool UVelvetPartySubsystem::IsPartyMember(const FUniqueNetIdRepl& PlayerId) const
{
    if (!PlayerId.IsValid() || !CurrentPartyId.IsValid()) { return false; }

    IOnlinePartyPtr Party = GetPartyInterface();
    TSharedPtr<const FUniqueNetId> LocalId = GetLocalNetId();
    if (!Party.IsValid() || !LocalId.IsValid()) { return false; }

    TArray<FOnlinePartyMemberConstRef> Members;
    Party->GetPartyMembers(*LocalId, *CurrentPartyId, Members);

    TSharedPtr<const FUniqueNetId> IncomingNetId = PlayerId.GetUniqueNetId();
    if (!IncomingNetId.IsValid()) { return false; }

    const FString IncomingIdStr = IncomingNetId->ToString();
    for (const FOnlinePartyMemberConstRef& Member : Members)
    {
        if (Member->GetUserId()->ToString() == IncomingIdStr) { return true; }
    }
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// OSS Party Callbacks
// ─────────────────────────────────────────────────────────────────────────────

void UVelvetPartySubsystem::OnOSSPartyMemberPromoted(
    const FUniqueNetId& /*LocalUserId*/,
    const FOnlinePartyId& PartyId,
    const FUniqueNetId& NewLeaderId)
{
    UKismetSystemLibrary::PrintString(GetWorld(),
        FString::Printf(TEXT("OnOSSPartyMemberPromoted: new leader %s"),
            *NewLeaderId.ToString()),
        true, true, FLinearColor::Yellow, 5.f);

    if (!IsCurrentParty(PartyId)) { return; }

    UE_LOG(LogVelvetParty, Log, TEXT("Party leader changed to: %s"),
        *NewLeaderId.ToString());

    // bIsLeader, bIsRemovable et bCanLaunch dépendent tous du leader —
    // un refresh complet est nécessaire.
    RefreshMemberViewModels();
    PushAllToViewModel();
}

void UVelvetPartySubsystem::OnOSSPartyJoined(
    const FUniqueNetId& LocalUserId,
    const FOnlinePartyId& PartyId)
{
    UKismetSystemLibrary::PrintString(GetWorld(),
        FString::Printf(TEXT("OnOSSPartyJoined: %s"), *PartyId.ToDebugString()),
        true, true, FLinearColor::Green, 5.f);

    // Store new party ID.
    if (IOnlinePartyPtr Party = GetPartyInterface())
    {
        TArray<TSharedRef<const FOnlinePartyId>> JoinedParties;
        Party->GetJoinedParties(LocalUserId, JoinedParties);
        for (const TSharedRef<const FOnlinePartyId>& Pid : JoinedParties)
        {
            if (*Pid == PartyId) { CurrentPartyId = Pid; break; }
        }
    }

    SetState(EVelvetPartyState::InMenu);
    RefreshMemberViewModels();
    PushAllToViewModel();

    if (IsLocalPlayerPartyHost())
    {
        // ── Case A: solo party (initial login or Redpoint auto-recreate) ──────────
        PublishPartyData_InMenu();
        OnPartyBootstrapped.Broadcast();
        UE_LOG(LogVelvetParty, Log, TEXT("Solo party ready: %s"), *PartyId.ToDebugString());
    }
    else
    {
        // ── Case B: joined host's party via invitation ────────────────────────────
        // Read existing party data — triggers StartJoinFlow if host is already in-game.
        if (IOnlinePartyPtr Party = GetPartyInterface())
        {
            FOnlinePartyDataConstPtr ExistingData =
                Party->GetPartyData(LocalUserId, PartyId, NAME_None);
            if (ExistingData.IsValid())
            {
                ConsumePartyData(*ExistingData);
            }
        }
        UE_LOG(LogVelvetParty, Log, TEXT("Joined host party: %s"), *PartyId.ToDebugString());
    }
}

void UVelvetPartySubsystem::OnOSSPartyExited(
    const FUniqueNetId& /*LocalUserId*/,
    const FOnlinePartyId& PartyId)
{
    UKismetSystemLibrary::PrintString(GetWorld(),
        FString::Printf(TEXT("OnOSSPartyExited: %s"), *PartyId.ToDebugString()),
        true, true, FLinearColor::Red, 5.f);

    if (!IsCurrentParty(PartyId)) { return; }

    const bool bWasInGame = (CurrentState == EVelvetPartyState::InGame ||
        CurrentState == EVelvetPartyState::LaunchingGame);

    CurrentPartyId.Reset();
    RefreshMemberViewModels(); // clear member list immediately
    PushAllToViewModel();

    if (bWasInGame) { ExitToMainMenu(); }

    // DO NOT call BootstrapParty() here.
    // Redpoint "Ensure Always In Party" automatically fires OnOSSPartyJoined (Case A).
}

void UVelvetPartySubsystem::OnOSSPartyDataReceived(
    const FUniqueNetId& /*LocalUserId*/,
    const FOnlinePartyId& PartyId,
    const FName& /*Namespace*/,
    const FOnlinePartyData& PartyData)
{
    if (!IsCurrentParty(PartyId)) { return; }
    ConsumePartyData(PartyData);

    // Party data contient potentiellement des mises à jour de slots —
    // toujours resynchroniser le ViewModel.
    RefreshMemberViewModels();
    PushAllToViewModel();
}

void UVelvetPartySubsystem::OnOSSPartyMemberJoined(
    const FUniqueNetId& /*LocalUserId*/,
    const FOnlinePartyId& PartyId,
    const FUniqueNetId& MemberId)
{
    UKismetSystemLibrary::PrintString(GetWorld(),
        FString::Printf(TEXT("OnOSSPartyMemberJoined: %s"), *MemberId.ToString()),
        true, true, FLinearColor::Green, 5.f);

    if (!IsCurrentParty(PartyId)) { return; }
    UE_LOG(LogVelvetParty, Log, TEXT("Member joined: %s"), *MemberId.ToString());

    if (IsLocalPlayerPartyHost())
    {
        AssignSlotToMember(MemberId);
        // RefreshMemberViewModels sera déclenché par OnOSSPartyDataReceived
        // quand la party data mise à jour sera reçue en retour.
    }
    else
    {
        RefreshMemberViewModels();
        PushAllToViewModel();
    }
}

void UVelvetPartySubsystem::OnOSSPartyMemberExited(
    const FUniqueNetId& /*LocalUserId*/,
    const FOnlinePartyId& PartyId,
    const FUniqueNetId& MemberId,
    const EMemberExitedReason Reason)
{
    UKismetSystemLibrary::PrintString(GetWorld(),
        FString::Printf(TEXT("OnOSSPartyMemberExited: %s (reason %d)"),
            *MemberId.ToString(), static_cast<int32>(Reason)),
        true, true, FLinearColor::Red, 5.f);

    if (!IsCurrentParty(PartyId)) { return; }
    UE_LOG(LogVelvetParty, Log, TEXT("Member exited: %s (reason %d)"),
        *MemberId.ToString(), static_cast<int32>(Reason));

    if (IsLocalPlayerPartyHost())
    {
        RevokeSlotForMember(MemberId);
        // Idem — OnOSSPartyDataReceived déclenchera le refresh.
    }
    else
    {
        RefreshMemberViewModels();
        PushAllToViewModel();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Map loading
// ─────────────────────────────────────────────────────────────────────────────

void UVelvetPartySubsystem::OnPostLoadMapWithWorld(UWorld* /*LoadedWorld*/)
{
    if (UVelvetPartyViewModel* VM = GetPartyViewModel()) { VM->SetIsTravelling(false); }
    OnMapLoaded.Broadcast();
}

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

IOnlinePartyPtr UVelvetPartySubsystem::GetPartyInterface() const
{
    IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
    return OSS ? OSS->GetPartyInterface() : nullptr;
}

IOnlineSessionPtr UVelvetPartySubsystem::GetSessionInterface() const
{
    IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
    return OSS ? OSS->GetSessionInterface() : nullptr;
}

TSharedPtr<const FUniqueNetId> UVelvetPartySubsystem::GetLocalNetId() const
{
    UGameInstance* GI = GetGameInstance();
    if (!GI) { return nullptr; }
    ULocalPlayer* LP = GI->GetFirstGamePlayer();
    if (!LP) { return nullptr; }
    return LP->GetPreferredUniqueNetId().GetUniqueNetId();
}

bool UVelvetPartySubsystem::IsCurrentParty(const FOnlinePartyId& PartyId) const
{
    return CurrentPartyId.IsValid() && (*CurrentPartyId == PartyId);
}

TSharedPtr<const FOnlinePartyId> UVelvetPartySubsystem::FindCurrentPartyIdShared() const
{
    if (!CurrentPartyId.IsValid()) { return nullptr; }

    IOnlinePartyPtr Party = GetPartyInterface();
    TSharedPtr<const FUniqueNetId> UserId = GetLocalNetId();
    if (!Party.IsValid() || !UserId.IsValid()) { return nullptr; }

    TArray<TSharedRef<const FOnlinePartyId>> JoinedParties;
    Party->GetJoinedParties(*UserId, JoinedParties);
    for (const TSharedRef<const FOnlinePartyId>& Pid : JoinedParties)
    {
        if (*Pid == *CurrentPartyId) { return Pid; }
    }
    return nullptr;
}

FString UVelvetPartySubsystem::MakeSessionToken() const
{
    if (!CurrentPartyId.IsValid())
    {
        return FString::Printf(TEXT("%08X"), static_cast<uint32>(FPlatformTime::Cycles()));
    }
    const FString PartyIdStr = CurrentPartyId->ToDebugString();
    return PartyIdStr.Len() >= 8 ? PartyIdStr.Left(8) : PartyIdStr;
}

void UVelvetPartySubsystem::SetState(EVelvetPartyState NewState)
{
    if (CurrentState == NewState) { return; }
    UE_LOG(LogVelvetParty, Log, TEXT("State %d → %d"),
        static_cast<int32>(CurrentState), static_cast<int32>(NewState));
    CurrentState = NewState;
    OnPartyStateChanged.Broadcast(NewState);

    if (UVelvetPartyViewModel* VM = GetPartyViewModel())
    {
        VM->SetPartyState(NewState);
        VM->SetCanLaunch(IsLocalPlayerPartyHost() && NewState == EVelvetPartyState::InMenu);
        if (NewState == EVelvetPartyState::InMenu || NewState == EVelvetPartyState::InGame)
        {
            VM->SetLastError(FText::GetEmpty());
        }
    }
}

void UVelvetPartySubsystem::BroadcastError(const FString& Msg)
{
    UE_LOG(LogVelvetParty, Error, TEXT("%s"), *Msg);
    UKismetSystemLibrary::PrintString(GetWorld(), Msg, true, true, FLinearColor::Red, 5.f);
    OnPartyError.Broadcast(Msg);
    if (UVelvetPartyViewModel* VM = GetPartyViewModel()) { VM->SetLastError(FText::FromString(Msg)); }
}

void UVelvetPartySubsystem::PushAllToViewModel()
{
    UVelvetPartyViewModel* VM = GetPartyViewModel();
    if (!VM) { return; }

    VM->SetPartyState(CurrentState);
    VM->SetCanLaunch(IsLocalPlayerPartyHost() && CurrentState == EVelvetPartyState::InMenu);
    RefreshMemberViewModels();
}

void UVelvetPartySubsystem::RefreshMemberViewModels()
{
    UVelvetPartyViewModel* VM = GetPartyViewModel();
    if (!VM) { return; }

    if (!CurrentPartyId.IsValid())
    {
        VM->SetMembers(TArray<TObjectPtr<UVelvetPartyMemberViewModel>>());
        return;
    }

    IOnlinePartyPtr PartyIface = GetPartyInterface();
    IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
    TSharedPtr<const FUniqueNetId> LocalId = GetLocalNetId();
    if (!PartyIface.IsValid() || !LocalId.IsValid()) { return; }

    FOnlinePartyConstPtr OnlineParty = PartyIface->GetParty(*LocalId, *CurrentPartyId);
    TSharedPtr<const FUniqueNetId> LeaderId =
        OnlineParty.IsValid() ? OnlineParty->LeaderId : nullptr;

    TArray<FOnlinePartyMemberConstRef> RawMembers;
    PartyIface->GetPartyMembers(*LocalId, *CurrentPartyId, RawMembers);

    IOnlineIdentityPtr IdentityIface = OSS ? OSS->GetIdentityInterface() : nullptr;

    // ── Lecture des slots depuis la party data ────────────────────────────────────
    TMap<FString, int32> SlotMap; // memberId → slot (2/3/4)

    FOnlinePartyDataConstPtr PartyData =
        PartyIface->GetPartyData(*LocalId, *CurrentPartyId, NAME_None);

    if (PartyData.IsValid())
    {
        for (int32 Slot = 2; Slot <= 4; ++Slot)
        {
            FVariantData SlotVar;
            const FString Key = FString::Printf(TEXT("VD_Slot_%d"), Slot);
            if (PartyData->GetAttribute(Key, SlotVar))
            {
                FString AssignedId;
                SlotVar.GetValue(AssignedId);
                if (!AssignedId.IsEmpty())
                {
                    SlotMap.Add(AssignedId, Slot);
                }
            }
        }
    }

    TArray<TObjectPtr<UVelvetPartyMemberViewModel>> NewMembers;
    NewMembers.Reserve(RawMembers.Num());

    for (const FOnlinePartyMemberConstRef& Member : RawMembers)
    {
        const FString MemberIdStr = Member->GetUserId()->ToString();
        const bool bIsLocal = LocalId.IsValid() && MemberIdStr == LocalId->ToString();
        const bool bIsLeader = LeaderId.IsValid() && MemberIdStr == LeaderId->ToString();

        FString ResolvedName = MemberIdStr;
        FString AttrValue;
        if (Member->GetUserAttribute(TEXT("prefDisplayName"), AttrValue) && !AttrValue.IsEmpty())
            ResolvedName = AttrValue;
        else if (Member->GetUserAttribute(TEXT("displayName"), AttrValue) && !AttrValue.IsEmpty())
            ResolvedName = AttrValue;
        else if (IdentityIface.IsValid())
        {
            const FString Nickname = IdentityIface->GetPlayerNickname(*Member->GetUserId());
            if (!Nickname.IsEmpty()) { ResolvedName = Nickname; }
        }

        UVelvetPartyMemberViewModel* MemberVM = VM->FindMemberVM(MemberIdStr);
        if (!MemberVM)
        {
            MemberVM = NewObject<UVelvetPartyMemberViewModel>(VM);
            MemberVM->SetUserId(MemberIdStr);
        }
        MemberVM->SetDisplayName(FText::FromString(ResolvedName));
        MemberVM->SetIsLocalPlayer(bIsLocal);
        MemberVM->SetIsLeader(bIsLeader);
        MemberVM->SetIsRemovable(IsLocalPlayerPartyHost() && !bIsLocal && !bIsLeader);
        MemberVM->SetSlotIndex(bIsLeader ? 1 : SlotMap.FindRef(MemberIdStr));

        NewMembers.Add(MemberVM);
    }

    NewMembers.Sort([](const UVelvetPartyMemberViewModel& A,
    const UVelvetPartyMemberViewModel& B)
    {
        const int32 SlotA = A.SlotIndex > 0 ? A.SlotIndex : 999;
        const int32 SlotB = B.SlotIndex > 0 ? B.SlotIndex : 999;
        return SlotA < SlotB;
    });

    VM->SetMembers(NewMembers);
}

void UVelvetPartySubsystem::KickPartyMember(const FString& MemberIdStr)
{
    if (!IsLocalPlayerPartyHost())
    {
        BroadcastError(TEXT("KickPartyMember: only the host can kick members."));
        return;
    }

    IOnlinePartyPtr Party = GetPartyInterface();
    TSharedPtr<const FUniqueNetId> LocalId = GetLocalNetId();
    if (!Party.IsValid() || !LocalId.IsValid() || !CurrentPartyId.IsValid()) { return; }

    TSharedPtr<const FUniqueNetId> TargetId =
        IOnlineSubsystem::Get()->GetIdentityInterface()->CreateUniquePlayerId(MemberIdStr);

    if (!TargetId.IsValid())
    {
        BroadcastError(FString::Printf(
            TEXT("KickPartyMember: could not resolve ID '%s'."), *MemberIdStr));
        return;
    }

    Party->KickMember(*LocalId, *CurrentPartyId, *TargetId, FOnKickPartyMemberComplete());
}

// ─────────────────────────────────────────────────────────────────────────────
// Invitations
// ─────────────────────────────────────────────────────────────────────────────

void UVelvetPartySubsystem::OnOSSPartyInviteReceived(
    const FUniqueNetId& /*LocalUserId*/,
    const IOnlinePartyJoinInfo& JoinInfo)
{
    TSharedPtr<const FUniqueNetId> SenderId = JoinInfo.GetSourceUserId();
    if (!SenderId.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("VelvetParty: InviteReceived — could not extract sender ID."));
        return;
    }

    const FString SenderIdStr = SenderId->ToString();
    UKismetSystemLibrary::PrintString(GetWorld(),
        FString::Printf(TEXT("Invite received from %s"), *SenderIdStr),
        true, true, FLinearColor::Green, 5.f);

    FString DisplayName = SenderIdStr;
    if (IOnlineFriendsPtr FriendsInterface = Online::GetFriendsInterface(GetWorld()))
    {
        TSharedPtr<FOnlineFriend> Friend = FriendsInterface->GetFriend(
            0, *SenderId, EFriendsLists::ToString(EFriendsLists::Default));
        if (Friend.IsValid()) { DisplayName = Friend->GetDisplayName(); }
    }

    PendingInvitationsMap.Emplace(SenderIdStr, FPendingInvitation{ JoinInfo.AsShared(), DisplayName });

    UE_LOG(LogTemp, Log, TEXT("VelvetParty: invitation received from %s (%s)"),
        *DisplayName, *SenderIdStr);

    RefreshPendingInvitationViewModels();
    OnPartyInvitationReceived.Broadcast(DisplayName);
}

void UVelvetPartySubsystem::OnOSSPartyInviteRemoved(
    const FUniqueNetId& /*LocalUserId*/,
    const IOnlinePartyJoinInfo& JoinInfo,
    EPartyInvitationRemovedReason Reason)
{
    TSharedPtr<const FUniqueNetId> SenderId = JoinInfo.GetSourceUserId();
    if (!SenderId.IsValid()) { return; }

    const FString SenderIdStr = SenderId->ToString();
    if (PendingInvitationsMap.Remove(SenderIdStr) > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("VelvetParty: invitation from %s removed (reason: %d)"),
            *SenderIdStr, static_cast<int32>(Reason));
        RefreshPendingInvitationViewModels();
    }
}

void UVelvetPartySubsystem::AcceptPartyInvitation(const FString& InviterUserId)
{
    FPendingInvitation* Entry = PendingInvitationsMap.Find(InviterUserId);
    if (!Entry)
    {
        BroadcastError(FString::Printf(
            TEXT("AcceptPartyInvitation: no pending invitation from %s"), *InviterUserId));
        return;
    }

    IOnlinePartyPtr PartyInterface = GetPartyInterface();
    TSharedPtr<const FUniqueNetId> LocalId = GetLocalNetId();
    if (!PartyInterface || !LocalId)
    {
        BroadcastError(TEXT("AcceptPartyInvitation: party interface unavailable."));
        return;
    }

    TSharedRef<const IOnlinePartyJoinInfo> JoinInfo = Entry->JoinInfo;

    // Redpoint "Ensure Always In Party" atomically leaves the current solo party
    // and joins the host's party. No manual flag needed.
    // Flow: OnOSSPartyExited (solo) → OnOSSPartyJoined (host, Case B).
    PartyInterface->JoinParty(
        *LocalId,
        *JoinInfo,
        FOnJoinPartyComplete::CreateUObject(
            this, &UVelvetPartySubsystem::OnAcceptInvitationComplete));
}

void UVelvetPartySubsystem::DeclinePartyInvitation(const FString& InviterUserId)
{
    if (PendingInvitationsMap.Remove(InviterUserId) == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("VelvetParty: DeclinePartyInvitation — no invitation from %s"),
            *InviterUserId);
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("VelvetParty: declined invitation from %s"), *InviterUserId);
    RefreshPendingInvitationViewModels();
}

void UVelvetPartySubsystem::OnAcceptInvitationComplete(
    const FUniqueNetId& /*LocalUserId*/,
    const FOnlinePartyId& /*PartyId*/,
    EJoinPartyCompletionResult Result,
    int32 NotApprovedReason)
{
    if (Result == EJoinPartyCompletionResult::Succeeded)
    {
        UE_LOG(LogTemp, Log, TEXT("VelvetParty: JoinParty (via invitation) succeeded."));
        PendingInvitationsMap.Empty();
        RefreshPendingInvitationViewModels();
        // State/ViewModel already updated by OnOSSPartyJoined (Case B).
    }
    else
    {
        BroadcastError(FString::Printf(
            TEXT("VelvetParty: JoinParty (via invitation) failed (result: %d, reason: %d)."),
            static_cast<int32>(Result), NotApprovedReason));
        // Redpoint will auto-recreate the solo party → OnOSSPartyJoined (Case A).
    }
}

void UVelvetPartySubsystem::RefreshPendingInvitationViewModels()
{
    UVelvetPartyViewModel* VM = GetPartyViewModel();
    if (!VM) { return; }

    TArray<TObjectPtr<UVelvetPendingPartyInvitationViewModel>> NewList;
    NewList.Reserve(PendingInvitationsMap.Num());

    for (const auto& Pair : PendingInvitationsMap)
    {
        UVelvetPendingPartyInvitationViewModel* InvVM = VM->FindPendingInvitationVM(Pair.Key);
        if (!InvVM)
        {
            InvVM = NewObject<UVelvetPendingPartyInvitationViewModel>(this);
            InvVM->SetInviterUserId(Pair.Key);
        }
        InvVM->SetInviterDisplayName(FText::FromString(Pair.Value.DisplayName));
        NewList.Add(InvVM);
    }

    VM->SetPendingInvitations(NewList);
}

void UVelvetPartySubsystem::InviteToParty(const FString& TargetPlayerIdStr)
{
    if (!CurrentPartyId.IsValid())
    {
        BroadcastError(TEXT("InviteToParty: not currently in a party."));
        OnPartyInviteSent.Broadcast(TargetPlayerIdStr, false);
        return;
    }
    if (TargetPlayerIdStr.IsEmpty())
    {
        BroadcastError(TEXT("InviteToParty: empty target ID."));
        OnPartyInviteSent.Broadcast(TargetPlayerIdStr, false);
        return;
    }

    IOnlinePartyPtr PartyInterface = GetPartyInterface();
    TSharedPtr<const FUniqueNetId> LocalId = GetLocalNetId();
    if (!PartyInterface || !LocalId)
    {
        BroadcastError(TEXT("InviteToParty: party interface or local ID unavailable."));
        OnPartyInviteSent.Broadcast(TargetPlayerIdStr, false);
        return;
    }

    IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
    if (!OSS)
    {
        BroadcastError(TEXT("InviteToParty: OSS unavailable."));
        OnPartyInviteSent.Broadcast(TargetPlayerIdStr, false);
        return;
    }

    TSharedPtr<const FUniqueNetId> TargetId =
        OSS->GetIdentityInterface()->CreateUniquePlayerId(TargetPlayerIdStr);
    if (!TargetId.IsValid())
    {
        BroadcastError(FString::Printf(
            TEXT("InviteToParty: could not resolve ID from '%s'."), *TargetPlayerIdStr));
        OnPartyInviteSent.Broadcast(TargetPlayerIdStr, false);
        return;
    }

    const FUniqueNetIdRepl TargetRepl(TargetId);
    if (IsPartyMember(TargetRepl))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("VelvetParty: InviteToParty — %s is already a party member."), *TargetPlayerIdStr);
        OnPartyInviteSent.Broadcast(TargetPlayerIdStr, false);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("VelvetParty: sending party invitation to %s"), *TargetPlayerIdStr);
    UKismetSystemLibrary::PrintString(GetWorld(),
        FString::Printf(TEXT("Inviting %s..."), *TargetPlayerIdStr),
        true, true, FLinearColor::Green, 5.f);

    PartyInterface->SendInvitation(
        *LocalId,
        *CurrentPartyId,
        *TargetId,
        FOnSendPartyInvitationComplete::CreateUObject(
            this, &UVelvetPartySubsystem::OnSendInvitationComplete));
}

void UVelvetPartySubsystem::OnSendInvitationComplete(
    const FUniqueNetId& /*LocalUserId*/,
    const FOnlinePartyId& PartyId,
    const FUniqueNetId& RecipientId,
    ESendPartyInvitationCompletionResult Result)
{
    if (!IsCurrentParty(PartyId)) { return; }

    const bool bSuccess = (Result == ESendPartyInvitationCompletionResult::Succeeded);
    const FString RecipientStr = RecipientId.ToString();

    if (!bSuccess)
    {
        BroadcastError(FString::Printf(
            TEXT("VelvetParty: invitation to %s failed (result: %d)."),
            *RecipientStr, static_cast<int32>(Result)));
    }

    OnPartyInviteSent.Broadcast(RecipientStr, bSuccess);
}

void UVelvetPartySubsystem::AssignSlotToMember(const FUniqueNetId& MemberId)
{
    IOnlinePartyPtr Party = GetPartyInterface();
    TSharedPtr<const FUniqueNetId> LocalId = GetLocalNetId();
    if (!Party.IsValid() || !LocalId.IsValid() || !CurrentPartyId.IsValid()) { return; }

    // Lire les slots déjà pris
    TSet<int32> TakenSlots;
    FOnlinePartyDataConstPtr ExistingData =
        Party->GetPartyData(*LocalId, *CurrentPartyId, NAME_None);

    if (ExistingData.IsValid())
    {
        for (int32 Slot = 2; Slot <= 4; ++Slot)
        {
            FVariantData SlotVar;
            const FString Key = FString::Printf(TEXT("VD_Slot_%d"), Slot);
            if (ExistingData->GetAttribute(Key, SlotVar))
            {
                FString ExistingId;
                SlotVar.GetValue(ExistingId);
                if (!ExistingId.IsEmpty()) { TakenSlots.Add(Slot); }
            }
        }
    }

    // Plus petit slot libre
    int32 AssignedSlot = 0;
    for (int32 Slot = 2; Slot <= 4; ++Slot)
    {
        if (!TakenSlots.Contains(Slot)) { AssignedSlot = Slot; break; }
    }

    if (AssignedSlot == 0)
    {
        UE_LOG(LogVelvetParty, Warning,
            TEXT("AssignSlotToMember: no available slot for %s"), *MemberId.ToString());
        return;
    }

    // UpdatePartyData merge les clés — on écrit uniquement la nouvelle
    FOnlinePartyData SlotData;
    const FString Key = FString::Printf(TEXT("VD_Slot_%d"), AssignedSlot);
    SlotData.SetAttribute(Key, FVariantData(MemberId.ToString()));
    Party->UpdatePartyData(*LocalId, *CurrentPartyId, NAME_None, SlotData);

    UE_LOG(LogVelvetParty, Log, TEXT("Assigned slot %d to %s"),
        AssignedSlot, *MemberId.ToString());
}

void UVelvetPartySubsystem::RevokeSlotForMember(const FUniqueNetId& MemberId)
{
    IOnlinePartyPtr Party = GetPartyInterface();
    TSharedPtr<const FUniqueNetId> LocalId = GetLocalNetId();
    if (!Party.IsValid() || !LocalId.IsValid() || !CurrentPartyId.IsValid()) { return; }

    FOnlinePartyDataConstPtr ExistingData =
        Party->GetPartyData(*LocalId, *CurrentPartyId, NAME_None);
    if (!ExistingData.IsValid()) { return; }

    const FString MemberIdStr = MemberId.ToString();

    for (int32 Slot = 2; Slot <= 4; ++Slot)
    {
        FVariantData SlotVar;
        const FString Key = FString::Printf(TEXT("VD_Slot_%d"), Slot);
        if (ExistingData->GetAttribute(Key, SlotVar))
        {
            FString ExistingId;
            SlotVar.GetValue(ExistingId);
            if (ExistingId == MemberIdStr)
            {
                FOnlinePartyData ClearData;
                ClearData.SetAttribute(Key, FVariantData(FString()));
                Party->UpdatePartyData(*LocalId, *CurrentPartyId, NAME_None, ClearData);

                UE_LOG(LogVelvetParty, Log, TEXT("Revoked slot %d from %s"),
                    Slot, *MemberIdStr);
                return;
            }
        }
    }
}