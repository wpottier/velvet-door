#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "VelvetPartyTypes.h"
#include "VelvetPartyViewModel.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// UVelvetPartyViewModel
//
// Passive data container — reflects the state of UVelvetPartySubsystem.
// Only UVelvetPartySubsystem writes to it (via setters).
// No knowledge of the subsystem, no delegate bindings.
//
// Registered in UVelvetMVVMSubsystem under the name "Party".
// Retrieve via: MVVMSubsystem->GetPartyViewModel()
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(BlueprintType)
class VELVETDOOR_API UVelvetPartyViewModel : public UMVVMViewModelBase
{
    GENERATED_BODY()

public:

    // ── Bindable fields ───────────────────────────────────────────────────────

    /** Current party / game state. */
    UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Velvet|Party")
    EVelvetPartyState PartyState = EVelvetPartyState::NotInParty;

    /**
     * True when the local player is the party leader AND state is InMenu.
     * Bind directly to the Start Game button IsEnabled.
     */
    UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Velvet|Party")
    bool bCanLaunch = false;

    /**
     * True between OnTravelStarting and OnMapLoaded.
     * Bind to your loading screen widget visibility.
     */
    UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Velvet|Party")
    bool bIsTravelling = false;

    /**
     * Last error message pushed by the subsystem.
     * Cleared by the subsystem when a clean state is reached.
     */
    UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Velvet|Party")
    FText LastError;

    // ── Setters (called by UVelvetPartySubsystem only) ────────────────────────

    void SetPartyState(EVelvetPartyState InValue);
    void SetCanLaunch(bool InValue);
    void SetIsTravelling(bool InValue);
    void SetLastError(const FText& InValue);

    const TArray<TObjectPtr<UVelvetPartyMemberViewModel>>& GetMembers() const { return Members; }
	const TArray<TObjectPtr<UVelvetPendingPartyInvitationViewModel>>& GetPendingInvitations() const { return PendingInvitations; }

    /**
     * Convenience: find an existing member VM by net ID string, or return
     * nullptr. The subsystem uses this to upsert without full rebuilds.
     */
    UVelvetPartyMemberViewModel* FindMemberVM(const FString& UserId) const;

    /**
     * Add a member VM to the Members array and broadcast the field change.
     * Called by UVelvetPartySubsystem when a new member joins, without a full refresh.
	 */
	void AddMember(const TObjectPtr<UVelvetPartyMemberViewModel>& MemberVM);

    /**
	 * Remove a member VM from the Members array by net ID string and broadcast the field change.
     */
	void RemoveMember(const FString& UserId);

	void ClearMembers();

	void SetMembers(const TArray<TObjectPtr<UVelvetPartyMemberViewModel>>& InMembers);

    UVelvetPendingPartyInvitationViewModel* FindPendingInvitationVM(const FString& UserId) const;

	void AddPendingInvitation(const TObjectPtr<UVelvetPendingPartyInvitationViewModel>& InvitationVM);

	void RemovePendingInvitation(const FString& UserId);

	void ClearPendingInvitations();

	void SetPendingInvitations(const TArray<TObjectPtr<UVelvetPendingPartyInvitationViewModel>>& InInvitations);

private:

    /**
     * Ordered list of party member ViewModels.
     * Index 0 = local player. Length == MemberCount.
     * Bind to a ListView / TileView — each entry is a UVelvetPartyMemberViewModel.
     */
    UPROPERTY(FieldNotify, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    TArray<TObjectPtr<UVelvetPartyMemberViewModel>> Members;

    UPROPERTY(FieldNotify, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    TArray<TObjectPtr<UVelvetPendingPartyInvitationViewModel>> PendingInvitations;
};

// ─────────────────────────────────────────────────────────────────────────────
// UVelvetPartyMemberViewModel
//
// Passive data container — one instance per party member.
// Only UVelvetPartySubsystem writes to it (via setters).
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(BlueprintType)
class VELVETDOOR_API UVelvetPartyMemberViewModel : public UMVVMViewModelBase
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Velvet|Party|Member")
    FString UserId;

    UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Velvet|Party|Member")
    FText DisplayName;

    /** Slot permanent dans la party. Hôte = 1, guests = 2/3/4. 0 = non assigné. */
    UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Velvet|Party|Member")
    int32 SlotIndex = 0;

    /** True if this entry represents the local player. */
    UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Velvet|Party|Member")
    bool bIsLocalPlayer = false;

    /** True if this member is the current party leader. */
    UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Velvet|Party|Member")
    bool bIsLeader = false;

    UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Velvet|Party|Member")
    bool bIsRemovable = true;

    // ── Setters ───────────────────────────────────────────────────────────────

    void SetUserId(const FString& InValue);
    void SetDisplayName(const FText& InValue);
    void SetIsLocalPlayer(bool InValue);
    void SetIsLeader(bool InValue);
	void SetIsRemovable(bool InValue);
    void SetSlotIndex(int32 InValue);

};

// ─────────────────────────────────────────────────────────────────────────────
// UVelvetPendingPartyInvitationViewModel
//
// One instance per pending incoming party invitation.
// Written by UVelvetPartySubsystem only.
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(BlueprintType)
class VELVETDOOR_API UVelvetPendingPartyInvitationViewModel : public UMVVMViewModelBase
{
    GENERATED_BODY()

public:

    /** Net ID string of the player who sent the invitation. */
    UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Velvet|Party|Invitation")
    FString InviterUserId;

    /** Display name of the inviter, resolved at receive time. */
    UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Velvet|Party|Invitation")
    FText InviterDisplayName;

    // ── Setters ───────────────────────────────────────────────────────────────

    void SetInviterUserId(const FString& InValue);
    void SetInviterDisplayName(const FText& InValue);
};