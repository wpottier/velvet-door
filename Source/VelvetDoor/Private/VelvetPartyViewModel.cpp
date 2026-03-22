#include "VelvetPartyViewModel.h"
#include "VelvetPartyTypes.h"


// ─────────────────────────────────────────────────────────────────────────────
// UVelvetPartyViewModel
// ─────────────────────────────────────────────────────────────────────────────

void UVelvetPartyViewModel::SetPartyState(EVelvetPartyState InValue)
{
    UE_MVVM_SET_PROPERTY_VALUE(PartyState, InValue);
}

void UVelvetPartyViewModel::SetCanLaunch(bool InValue)
{
    UE_MVVM_SET_PROPERTY_VALUE(bCanLaunch, InValue);
}

void UVelvetPartyViewModel::SetIsTravelling(bool InValue)
{
    UE_MVVM_SET_PROPERTY_VALUE(bIsTravelling, InValue);
}

void UVelvetPartyViewModel::SetLastError(const FText& InValue)
{
    UE_MVVM_SET_PROPERTY_VALUE(LastError, InValue);
}

void UVelvetPartyViewModel::AddMember(const TObjectPtr<UVelvetPartyMemberViewModel>& MemberVM)
{
    if (!MemberVM) return;

    Members.Add(MemberVM);
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Members);
}

void UVelvetPartyViewModel::RemoveMember(const FString& UserId)
{
    int32 Index = Members.IndexOfByPredicate([&](const TObjectPtr<UVelvetPartyMemberViewModel>& M)
    {
        return M && M->UserId == UserId;
    });

    if (Index != INDEX_NONE)
    {
        Members.RemoveAt(Index);

        UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Members);
    }
}

void UVelvetPartyViewModel::ClearMembers()
{
    Members.Empty();
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Members);
}

UVelvetPartyMemberViewModel* UVelvetPartyViewModel::FindMemberVM(const FString& UserId) const
{
    const TObjectPtr<UVelvetPartyMemberViewModel>* FoundPtr = Members.FindByPredicate([&](const TObjectPtr<UVelvetPartyMemberViewModel>& M)
    {
        return M && M->UserId == UserId;
    });

    return FoundPtr ? FoundPtr->Get() : nullptr;
}

void UVelvetPartyViewModel::SetMembers(const TArray<TObjectPtr<UVelvetPartyMemberViewModel>>& InMembers)
{
    Members = InMembers;
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Members);
}

void UVelvetPartyViewModel::AddPendingInvitation(const TObjectPtr<UVelvetPendingPartyInvitationViewModel>& InvitationVM)
{
    if (!InvitationVM) return;

    PendingInvitations.Add(InvitationVM);
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(PendingInvitations);
}

void UVelvetPartyViewModel::RemovePendingInvitation(const FString& UserId)
{
    int32 Index = PendingInvitations.IndexOfByPredicate([&](const TObjectPtr<UVelvetPendingPartyInvitationViewModel>& I)
    {
        return I && I->InviterUserId == UserId;
    });

    if (Index != INDEX_NONE)
    {
        PendingInvitations.RemoveAt(Index);
        UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(PendingInvitations);
    }
}

void UVelvetPartyViewModel::ClearPendingInvitations()
{
    PendingInvitations.Empty();
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(PendingInvitations);
}

UVelvetPendingPartyInvitationViewModel* UVelvetPartyViewModel::FindPendingInvitationVM(const FString& UserId) const
{
    const TObjectPtr<UVelvetPendingPartyInvitationViewModel>* FoundPtr = PendingInvitations.FindByPredicate([&](const TObjectPtr<UVelvetPendingPartyInvitationViewModel>& I)
    {
        return I && I->InviterUserId == UserId;
    });

    return FoundPtr ? FoundPtr->Get() : nullptr;
}

void UVelvetPartyViewModel::SetPendingInvitations(const TArray<TObjectPtr<UVelvetPendingPartyInvitationViewModel>>& InInvitations)
{
    PendingInvitations = InInvitations;
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(PendingInvitations);
}

// ─────────────────────────────────────────────────────────────────────────────
// UVelvetPartyMemberViewModel
// ─────────────────────────────────────────────────────────────────────────────

void UVelvetPartyMemberViewModel::SetUserId(const FString& InValue)
{
    UE_MVVM_SET_PROPERTY_VALUE(UserId, InValue);
}

void UVelvetPartyMemberViewModel::SetDisplayName(const FText& InValue)
{
    UE_MVVM_SET_PROPERTY_VALUE(DisplayName, InValue);
}

void UVelvetPartyMemberViewModel::SetIsLocalPlayer(bool InValue)
{
    UE_MVVM_SET_PROPERTY_VALUE(bIsLocalPlayer, InValue);
}

void UVelvetPartyMemberViewModel::SetIsLeader(bool InValue)
{
    UE_MVVM_SET_PROPERTY_VALUE(bIsLeader, InValue);
}

void UVelvetPartyMemberViewModel::SetIsRemovable(bool InValue)
{
    UE_MVVM_SET_PROPERTY_VALUE(bIsRemovable, InValue);
}

void UVelvetPartyMemberViewModel::SetSlotIndex(int32 InValue)
{
    UE_MVVM_SET_PROPERTY_VALUE(SlotIndex, InValue);
}

// ─────────────────────────────────────────────────────────────────────────────
// UVelvetPartyMemberViewModel
// ─────────────────────────────────────────────────────────────────────────────

void UVelvetPendingPartyInvitationViewModel::SetInviterUserId(const FString& InValue)
{
    UE_MVVM_SET_PROPERTY_VALUE(InviterUserId, InValue);
}

void UVelvetPendingPartyInvitationViewModel::SetInviterDisplayName(const FText& InValue)
{
    UE_MVVM_SET_PROPERTY_VALUE(InviterDisplayName, InValue);
}