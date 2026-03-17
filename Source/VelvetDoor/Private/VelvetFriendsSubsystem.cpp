#include "VelvetFriendsSubsystem.h"
#include "VelvetMVVMSubsystem.h"
#include "VelvetSocialViewModel.h"
#include "VelvetAuthSubsystem.h"
#include "RedpointEOSCore/Utils/WorldResolution.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSUserCache/UserInfo.h"
#include "RedpointEOSUserCache/ExternalUserInfo.h"
#include "RedpointEOSPresence/PresenceSystem.h"
#include "Kismet/KismetSystemLibrary.h"

using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Friends;
using namespace ::Redpoint::EOS::Presence;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Platform::Types;
using namespace ::Redpoint::EOS::UserCache;

void UVelvetFriendsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UVelvetAuthSubsystem* Auth = GetGameInstance()->GetSubsystem<UVelvetAuthSubsystem>();
    if (!Auth) return;

    Auth->OnAuthStateChanged.AddDynamic(this, &UVelvetFriendsSubsystem::OnAuthStateChanged);

    if (Auth->GetAuthState() == EVelvetAuthStates::SignedIn)
    {
        InitializeSystems();
    }
}

void UVelvetFriendsSubsystem::Deinitialize()
{
    UVelvetAuthSubsystem* Auth = GetGameInstance()->GetSubsystem<UVelvetAuthSubsystem>();
    if (Auth)
    {
        Auth->OnAuthStateChanged.RemoveDynamic(this, &UVelvetFriendsSubsystem::OnAuthStateChanged);
    }

    if (FriendSystem)
    {
        FriendSystem->OnFriendsChanged().Remove(FriendsChangedHandle);
        FriendSystem->OnFriendPresenceChanged().Remove(FriendPresenceChangedHandle);
    }

    FriendSystem = nullptr;
    PlatformHandle = nullptr;

    Super::Deinitialize();
}

void UVelvetFriendsSubsystem::OnAuthStateChanged(EVelvetAuthStates NewState)
{
    if (NewState == EVelvetAuthStates::SignedIn)
    {
        InitializeSystems();
    }
}

void UVelvetFriendsSubsystem::InitializeSystems()
{
    PlatformHandle = FWorldResolution::TryGetPlatformHandle(GetWorld());
    if (!PlatformHandle.IsValid()) return;

    FriendSystem = PlatformHandle->GetSystem<IFriendSystem>();
    if (!FriendSystem.IsValid()) return;

    PresenceSystem = PlatformHandle->GetSystem<IPresenceSystem>();
    if (!PresenceSystem.IsValid()) return;

    UVelvetAuthSubsystem* Auth = GetGameInstance()->GetSubsystem<UVelvetAuthSubsystem>();
    if (!Auth || !Auth->GetLocalUserId().IsValid()) return;

    LocalUserId = GetAccountId(*Auth->GetLocalUserId());

    FriendsChangedHandle = FriendSystem->OnFriendsChanged().AddUObject(
        this,
        &UVelvetFriendsSubsystem::OnFriendsChanged);

    FriendPresenceChangedHandle = FriendSystem->OnFriendPresenceChanged().AddUObject(
        this,
        &UVelvetFriendsSubsystem::OnFriendPresenceChanged);

    FriendSystem->OnFriendRequestReceived().AddUObject(
        this,
        &UVelvetFriendsSubsystem::OnFriendRequestReceived);

    // Get Friend Code
    UVelvetMVVMSubsystem* MVVM = GetGameInstance()->GetSubsystem<UVelvetMVVMSubsystem>();
    if (!MVVM) return;

	UVelvetSocialViewModel* VMSocial = MVVM->GetSocialViewModel();
    if (!VMSocial) return;

    TOptional<FString> FriendCode = PresenceSystem->GetFriendCode(LocalUserId);
    if (FriendCode.IsSet())
    {
        UKismetSystemLibrary::PrintString(this,
            FString::Printf(TEXT("VelvetFriends: Friend code is %s"), *FriendCode.GetValue()),
			true, true, FLinearColor::Green, 5.f);
		VMSocial->SetFriendCode(FriendCode.GetValue());
    }
    else {
        UKismetSystemLibrary::PrintString(this,
			TEXT("VelvetFriends: Failed to get friend code"),
            true, true, FLinearColor::Yellow, 5.f);
    }


    LoadFriends();
	LoadInboundInvitations();
}

void UVelvetFriendsSubsystem::LoadFriends()
{
    if (!FriendSystem) return;

    UVelvetMVVMSubsystem* MVVM = GetGameInstance()->GetSubsystem<UVelvetMVVMSubsystem>();
    if (!MVVM) return;

    UVelvetSocialViewModel* VM = MVVM->GetSocialViewModel();
    if (!VM) return;

    TAccountIdMap<FUserInfoRef> UnifiedFriends;
    TArray<FExternalUserInfoRef> NonUnifiedFriends;

    auto Error = FriendSystem->GetFriends(LocalUserId, UnifiedFriends, NonUnifiedFriends);
    if (!Error.WasSuccessful())
    {
        UKismetSystemLibrary::PrintString(this,
            TEXT("VelvetFriends: GetFriends failed"),
            true, true, FLinearColor::Red, 5.f);
        return;
    }

    // 1. Retirer les amis qui ne sont plus dans la liste
    TArray<FString> NewFriendIds;
    for (const auto& KV : UnifiedFriends)
    {
        NewFriendIds.Add(GetUserIdString(KV.Key));
    }

    TArray<FString> ToRemove;
    for (const TObjectPtr<UVelvetFriendViewModel>& Friend : VM->GetFriendList())
    {
        if (Friend && !NewFriendIds.Contains(Friend->UserId))
        {
            ToRemove.Add(Friend->UserId);
        }
    }
    for (const FString& Id : ToRemove)
    {
        VM->RemoveFriendByNativeId(Id);
    }

    // 2. Ajouter ou mettre à jour
    for (const auto& KV : UnifiedFriends)
    {
        FString FriendIdString = GetUserIdString(KV.Key);
        const FUserInfoRef& FriendInfo = KV.Value;

        UVelvetFriendViewModel* ExistingVM = FindFriendViewModel(KV.Key);

        if (ExistingVM)
        {
            // Mettre à jour les données statiques
            ExistingVM->SetDisplayName(GetFriendDisplayName(FriendInfo));
            ExistingVM->SetIsRemovable(FriendSystem->IsFriendDeletable(LocalUserId, KV.Key));
        }
        else
        {
            // Créer une nouvelle entrée
            UVelvetFriendViewModel* FriendVM = NewObject<UVelvetFriendViewModel>(VM);
            FriendVM->SetDisplayName(GetFriendDisplayName(FriendInfo));
            FriendVM->SetUserId(FriendIdString);
            FriendVM->SetIsRemovable(FriendSystem->IsFriendDeletable(LocalUserId, KV.Key));

            TOptional<FPresenceUserState> InitialPresence =
                FriendSystem->GetUnifiedFriendPresence(LocalUserId, KV.Key);
            if (InitialPresence.IsSet())
            {
                FriendVM->SetPresenceStatus(ConvertPresenceState(InitialPresence.GetValue()));
                FriendVM->SetIsJoinable(
                    InitialPresence.GetValue().AdvertisedSession.IsSet() ||
                    InitialPresence.GetValue().AdvertisedParty.IsSet()
                );
            }

            VM->AddFriend(FriendVM);
        }
    }

    UKismetSystemLibrary::PrintString(this,
        FString::Printf(TEXT("VelvetFriends: %d unified friends"), UnifiedFriends.Num()),
        true, true, FLinearColor::Green, 5.f);
}

void UVelvetFriendsSubsystem::OnFriendsChanged(
    const FIdentityUserId& InLocalUserId,
    ERelationshipType ChangedRelationships)
{
    if ((ChangedRelationships & ERelationshipType::MutualFriends) != ERelationshipType::None)
    {
        // Reload complet de la liste d'amis
        LoadFriends();
    }

    if ((ChangedRelationships & ERelationshipType::IncomingFriendRequest) != ERelationshipType::None)
    {
        // Mettre à jour les invitations reçues dans la VM
        LoadInboundInvitations();
    }

    if ((ChangedRelationships & ERelationshipType::OutgoingFriendRequest) != ERelationshipType::None)
    {
        // Mettre à jour les invitations envoyées
        // Pas nécessaire pour le VS
    }

    if ((ChangedRelationships & ERelationshipType::Blocked) != ERelationshipType::None)
    {
        // Un ami a été bloqué/débloqué
        // Reload la liste pour retirer le bloqué
        LoadFriends();
    }

    if ((ChangedRelationships & ERelationshipType::RecentPlayer) != ERelationshipType::None)
    {
        // Pas nécessaire pour le VS
    }
}

void UVelvetFriendsSubsystem::OnFriendPresenceChanged(
    const FIdentityUserId& InLocalUserId,
    const FIdentityUserId& FriendUserId,
    const FPresenceUserState& NewPresenceState)
{
    UVelvetFriendViewModel* FriendVM = FindFriendViewModel(FriendUserId);
    if (!FriendVM) return;

    FriendVM->SetPresenceStatus(ConvertPresenceState(NewPresenceState));
    FriendVM->SetIsJoinable(
        NewPresenceState.AdvertisedSession.IsSet() ||
        NewPresenceState.AdvertisedParty.IsSet()
    );
}

UVelvetFriendViewModel* UVelvetFriendsSubsystem::FindFriendViewModel(
    const FIdentityUserId& UserId)
{
    UVelvetMVVMSubsystem* MVVM = GetGameInstance()->GetSubsystem<UVelvetMVVMSubsystem>();
    if (!MVVM) return nullptr;

    UVelvetSocialViewModel* VM = MVVM->GetSocialViewModel();
    if (!VM) return nullptr;

    FString UserIdString = GetUserIdString(UserId);
    for (const TObjectPtr<UVelvetFriendViewModel>& Friend : VM->GetFriendList())
    {
        if (Friend && Friend->UserId == UserIdString)
            return Friend.Get();
    }
    return nullptr;
}

EVelvetFriendPresence UVelvetFriendsSubsystem::ConvertPresenceState(
    const FPresenceUserState& InPresence)
{
    if (!InPresence.bPresenceAdvertised)  return EVelvetFriendPresence::Offline;
    if (!InPresence.bIsPlayingThisGame)   return EVelvetFriendPresence::Online;
    return EVelvetFriendPresence::InGame;
}

FString UVelvetFriendsSubsystem::GetFriendDisplayName(
    const FUserInfoRef& FriendInfo)
{
    const TMap<FString, FString>& Attributes = FriendInfo->GetAttributes();

    if (const FString* PrefName = Attributes.Find(TEXT("prefDisplayName")))
        if (!PrefName->IsEmpty()) return *PrefName;

    if (const FString* Name = Attributes.Find(TEXT("displayName")))
        if (!Name->IsEmpty()) return *Name;

    return TEXT("Unknown");
}

void UVelvetFriendsSubsystem::LoadInboundInvitations()
{
    if (!FriendSystem) return;

    UVelvetMVVMSubsystem* MVVM = GetGameInstance()->GetSubsystem<UVelvetMVVMSubsystem>();
    if (!MVVM) return;

    UVelvetSocialViewModel* VM = MVVM->GetSocialViewModel();
    if (!VM) return;

    VM->ClearInboundInvitations();

    TAccountIdMap<FUserInfoRef> InboundRequests;
    auto Error = FriendSystem->GetInboundFriendRequests(LocalUserId, InboundRequests);
    if (!Error.WasSuccessful()) return;

    for (const auto& KV : InboundRequests)
    {
        UVelvetInboundInvitationViewModel* InvVM =
            NewObject<UVelvetInboundInvitationViewModel>(VM);
        InvVM->SetSenderId(GetUserIdString(KV.Key));
        InvVM->SetDisplayName(GetFriendDisplayName(KV.Value));
        VM->AddInboundInvitation(InvVM);
    }

    UKismetSystemLibrary::PrintString(this,
        FString::Printf(TEXT("VelvetFriends: %d inbound invitations"), InboundRequests.Num()),
		true, true, FLinearColor::Green, 5.f);
}

void UVelvetFriendsSubsystem::SendFriendInvitation(const FString& TargetNativeId)
{
    if (!FriendSystem)
    {
        UKismetSystemLibrary::PrintString(this,
            TEXT("VelvetFriends: FriendSystem not initialized"),
			true, true, FLinearColor::Red, 5.f);
        return;
    }

    // Convertir FString → FIdentityUserId
    TOptional<UE::Online::FAccountId> AccountId =
        Redpoint::EOS::Core::Id::TryParseAccountId(TargetNativeId);
    if (!AccountId.IsSet()) 
    {
        UKismetSystemLibrary::PrintString(this,
            FString::Printf(TEXT("VelvetFriends: Invalid TargetNativeId %s"), *TargetNativeId),
			true, true, FLinearColor::Red, 5.f);
        return;
    }

    FIdentityUserId TargetId = AccountId.GetValue();
    UKismetSystemLibrary::PrintString(this,
        FString::Printf(TEXT("VelvetFriends: Sending friend request to %s"), *TargetNativeId),
		true, true, FLinearColor::Green, 5.f);

    AsCallback(
        FriendSystem->SendFriendRequest(LocalUserId, TargetId),
        [this](FError Error)
        {
            if (!Error.WasSuccessful())
            {
                UKismetSystemLibrary::PrintString(this,
                    TEXT("VelvetFriends: SendFriendRequest failed"),
                    true, true, FLinearColor::Red, 5.f);
                return;
            }

            UKismetSystemLibrary::PrintString(this,
                TEXT("VelvetFriends: Friend request sent"),
                true, true, FLinearColor::Green, 5.f);
        });
}

void UVelvetFriendsSubsystem::AcceptFriendInvitation(const FString& SenderNativeId)
{
    if (!FriendSystem) return;

    TOptional<UE::Online::FAccountId> AccountId =
        Redpoint::EOS::Core::Id::TryParseAccountId(SenderNativeId);
    if (!AccountId.IsSet()) return;

    FIdentityUserId SenderId = AccountId.GetValue();

    AsCallback(
        FriendSystem->AcceptFriendRequest(LocalUserId, SenderId),
        [this, SenderNativeId](FError Error)
        {
            if (Error.WasSuccessful())
            {
                // Retirer l'invitation de la VM
                UVelvetMVVMSubsystem* MVVM =
                    GetGameInstance()->GetSubsystem<UVelvetMVVMSubsystem>();
                if (MVVM)
                {
                    MVVM->GetSocialViewModel()->RemoveInboundInvitation(SenderNativeId);
                }
                // OnFriendsChanged va déclencher LoadFriends automatiquement
            }
        });
}

void UVelvetFriendsSubsystem::RejectFriendInvitation(const FString& SenderNativeId)
{
    if (!FriendSystem) return;

    TOptional<UE::Online::FAccountId> AccountId =
        Redpoint::EOS::Core::Id::TryParseAccountId(SenderNativeId);
    if (!AccountId.IsSet()) return;

    FIdentityUserId SenderId = AccountId.GetValue();

    AsCallback(
        FriendSystem->RejectFriendRequest(LocalUserId, SenderId),
        [this, SenderNativeId](FError Error)
        {
            if (Error.WasSuccessful())
            {
                UVelvetMVVMSubsystem* MVVM =
                    GetGameInstance()->GetSubsystem<UVelvetMVVMSubsystem>();
                if (MVVM)
                {
                    MVVM->GetSocialViewModel()->RemoveInboundInvitation(SenderNativeId);
                }
            }
        });
}

void UVelvetFriendsSubsystem::FindPlayersByFriendCode(const FString& FriendCode)
{
    UVelvetMVVMSubsystem* MVVM = GetGameInstance()->GetSubsystem<UVelvetMVVMSubsystem>();
    if (!MVVM) return;

    UVelvetFriendSearchViewModel* SearchVM = MVVM->GetFriendSearchViewModel();
    if (!SearchVM) return;

    SearchVM->ClearResults();
    SearchVM->SetSearchState(EVelvetSearchState::Searching);

    PresenceSystem->FindByFriendCode(
        LocalUserId,
        FriendCode,
        IPresenceSystem::FOnFindByFriendCodeComplete::CreateWeakLambda(
            this,
            [this, SearchVM](const FError& Error, const TArray<FUserInfoRef>& DiscoveredUsers)
            {
                if (!Error.WasSuccessful() || DiscoveredUsers.IsEmpty())
                {
                    SearchVM->SetSearchState(
                        Error.WasSuccessful()
                        ? EVelvetSearchState::NoResults
                        : EVelvetSearchState::Error);
                    OnFriendCodeSearchComplete.Broadcast(false, TArray<FString>());
                    return;
                }

                TArray<FString> UserIds;

                for (const FUserInfoRef& User : DiscoveredUsers)
                {
                    FString UserId = GetUserIdString(User->GetUserId());
                    UserIds.Add(UserId);

                    UFriendSearchResultViewModel* ResultVM =
                        NewObject<UFriendSearchResultViewModel>(SearchVM);

                    ResultVM->SetUserId(UserId);
                    ResultVM->SetDisplayName(GetFriendDisplayName(User));

                    // Vérifie si déjà ami
                    UVelvetSocialViewModel* SocVM = GetGameInstance()
                        ->GetSubsystem<UVelvetMVVMSubsystem>()
                        ->GetSocialViewModel();

                    bool bAlreadyFriend = false;
                    if (SocVM)
                    {
                        for (const TObjectPtr<UVelvetFriendViewModel>& Friend : SocVM->GetFriendList())
                        {
                            if (Friend && *Friend == UserId)
                            {
                                bAlreadyFriend = true;
                                break;
                            }
                        }
                    }
                    ResultVM->SetIsAlreadyFriend(bAlreadyFriend);
                    ResultVM->SetIsRequestPending(false); // à enrichir plus tard

                    SearchVM->AddResult(ResultVM);
                }

                SearchVM->SetSearchState(EVelvetSearchState::Results);
                OnFriendCodeSearchComplete.Broadcast(true, UserIds);
            }));
}

void UVelvetFriendsSubsystem::ResetFindPlayersByFriendCode()
{
    UVelvetMVVMSubsystem* MVVM = GetGameInstance()->GetSubsystem<UVelvetMVVMSubsystem>();
    if (!MVVM) return;

    UVelvetFriendSearchViewModel* SearchVM = MVVM->GetFriendSearchViewModel();
    if (!SearchVM) return;

    SearchVM->ClearResults();
	SearchVM->SetSearchState(EVelvetSearchState::Idle);
}

void UVelvetFriendsSubsystem::DeleteFriend(const FString& FriendNativeId)
{
    if (!FriendSystem) return;

    TOptional<UE::Online::FAccountId> AccountId =
        Redpoint::EOS::Core::Id::TryParseAccountId(FriendNativeId);
    if (!AccountId.IsSet()) return;

    AsCallback(
        FriendSystem->DeleteFriend(LocalUserId, AccountId.GetValue()),
        [this](FError Error)
        {
            if (!Error.WasSuccessful())
            {
                UKismetSystemLibrary::PrintString(this,
                    TEXT("VelvetFriends: DeleteFriend failed"),
                    true, true, FLinearColor::Red, 5.f);
            }
            // OnFriendsChanged déclenche LoadFriends automatiquement
        });
}

void UVelvetFriendsSubsystem::BlockPlayer(const FString& TargetNativeId)
{
    if (!FriendSystem) return;

    TOptional<UE::Online::FAccountId> AccountId =
        Redpoint::EOS::Core::Id::TryParseAccountId(TargetNativeId);
    if (!AccountId.IsSet()) return;

    AsCallback(
        FriendSystem->BlockPlayer(LocalUserId, AccountId.GetValue()),
        [this](FError Error)
        {
            if (!Error.WasSuccessful())
            {
                UKismetSystemLibrary::PrintString(this,
                    TEXT("VelvetFriends: BlockPlayer failed"),
                    true, true, FLinearColor::Red, 5.f);
            }
        });
}

void UVelvetFriendsSubsystem::OnFriendRequestReceived(
    const FIdentityUserId& InLocalUserId,
    const FUserInfoRef& SendingRemoteUser)
{
    UKismetSystemLibrary::PrintString(this,
        FString::Printf(TEXT("VelvetFriends: Received friend request from %s"), *GetFriendDisplayName(SendingRemoteUser)),
		true, true, FLinearColor::Green, 5.f);

    LoadInboundInvitations();
}