#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MVVMViewModelBase.h"
#include "Tickable.h"
#include "VelvetNotificationSubsystem.generated.h"

UENUM(BlueprintType)
enum class EVelvetNotificationType : uint8
{
    Info                  UMETA(DisplayName = "Info"),
    Success               UMETA(DisplayName = "Success"),
    Warning               UMETA(DisplayName = "Warning"),
    FriendRequestReceived UMETA(DisplayName = "Friend Request Received"),
    PartyInviteReceived   UMETA(DisplayName = "Party Invite Received"),
};

UENUM(BlueprintType)
enum class EVelvetNotificationState : uint8
{
    Pending   UMETA(DisplayName = "Pending"),
    Displayed UMETA(DisplayName = "Displayed"),
    Accepted  UMETA(DisplayName = "Accepted"),
    Declined  UMETA(DisplayName = "Declined"),
    Dismissed UMETA(DisplayName = "Dismissed"),
};

// ─────────────────────────────────────────────────────────────────────────────
//  ViewModel
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(BlueprintType)
class VELVETDOOR_API UVelvetNotificationViewModel : public UMVVMViewModelBase
{
    GENERATED_BODY()

public:
    // ── Readable properties (BlueprintReadOnly + FieldNotify) ──────────────
    UPROPERTY(BlueprintReadOnly, FieldNotify)
    EVelvetNotificationType Type = EVelvetNotificationType::Info;

    UPROPERTY(BlueprintReadOnly, FieldNotify)
    FText Message;

    // Timed notifications: counts down from InitialDuration to 0.
    // 0 at construction = persistent (no auto-dismiss).
    UPROPERTY(BlueprintReadOnly, FieldNotify)
    float TimeRemaining = 0.f;

    UPROPERTY(BlueprintReadOnly, FieldNotify)
    EVelvetNotificationState State = EVelvetNotificationState::Pending;

    // Drives Accept / Decline button visibility in the widget.
    UPROPERTY(BlueprintReadOnly, FieldNotify)
    bool bHasActions = false;

    // EOS invite handle, friend product user id, etc.
    UPROPERTY(BlueprintReadOnly, FieldNotify)
    FString ContextPayload;

    // ── C++-only setters (called by the subsystem) ─────────────────────────
    void SetType(EVelvetNotificationType InType);
    void SetMessage(const FText& InMessage);
    void SetTimeRemaining(float InTimeRemaining);
    void SetState(EVelvetNotificationState InState);
    void SetHasActions(bool bInHasActions);
    void SetContextPayload(const FString& InPayload);

    // ── C++-only getters (called by the subsystem) ─────────────────────────
    EVelvetNotificationState GetState()    const { return State; }
    float                    GetTimeRemaining() const { return TimeRemaining; }
    EVelvetNotificationType  GetType()     const { return Type; }
    FText                    GetMessage()  const { return Message; }
    FString                  GetContextPayload() const { return ContextPayload; }
    bool                     GetHasActions() const { return bHasActions; }

    // ── Actions (called from widget buttons) ──────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Notification")
    void Accept();

    UFUNCTION(BlueprintCallable, Category = "Notification")
    void Decline();

    UFUNCTION(BlueprintCallable, Category = "Notification")
    void Dismiss();

    // ── Internal delegates (bound by the subsystem at creation) ───────────
    // OnDismissed fires on Accept, Decline, and Dismiss — it means
    // "this notification no longer needs to be displayed or processed".
    DECLARE_MULTICAST_DELEGATE(FOnNotificationAction);
    FOnNotificationAction OnAccepted;
    FOnNotificationAction OnDeclined;
    FOnNotificationAction OnDismissed;

    // ── Helpers ────────────────────────────────────────────────────────────
    bool IsPersistent() const { return InitialDuration <= 0.f; }
    bool HasExpired()   const { return !IsPersistent() && TimeRemaining <= 0.f; }

private:
    float InitialDuration = 0.f;
    friend class UVelvetNotificationSubsystem;
};

// ─────────────────────────────────────────────────────────────────────────────
//  Subsystem
// ─────────────────────────────────────────────────────────────────────────────

UCLASS()
class VELVETDOOR_API UVelvetNotificationSubsystem : public UGameInstanceSubsystem,
    public FTickableGameObject
{
    GENERATED_BODY()

public:
    // Push a passive notification (Info / Success / Warning).
    // Duration <= 0 → persistent until explicitly dismissed.
    UFUNCTION(BlueprintCallable, Category = "Velvet|Notifications")
    UVelvetNotificationViewModel* PushPassive(
        EVelvetNotificationType Type,
        const FText& Message,
        float Duration = 3.f);

    // Push an actionable notification (FriendRequest / PartyInvite).
    // Always persistent — the user must Accept, Decline, or Dismiss.
    UFUNCTION(BlueprintCallable, Category = "Velvet|Notifications")
    UVelvetNotificationViewModel* PushActionable(
        EVelvetNotificationType Type,
        const FText& Message,
        const FString& ContextPayload);

    UFUNCTION(BlueprintCallable, Category = "Velvet|Notifications")
    void DismissAll();

    UFUNCTION(BlueprintCallable, Category = "Velvet|Notifications")
    void NotifyDisplayed(UVelvetNotificationViewModel* VM);

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNotificationPushed, UVelvetNotificationViewModel*, Notification);

    UPROPERTY(BlueprintAssignable, Category = "Velvet|Notifications")
    FOnNotificationPushed OnNotificationPushed;

    // ── FTickableGameObject ────────────────────────────────────────────────
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const override;
    virtual TStatId GetStatId() const override;

private:
    // UPROPERTY is mandatory — prevents GC from collecting active VMs.
    UPROPERTY()
    TArray<TObjectPtr<UVelvetNotificationViewModel>> ActiveNotifications;

    UVelvetNotificationViewModel* CreateAndRegister(
        EVelvetNotificationType Type,
        const FText& Message,
        float Duration,
        bool bHasActions,
        const FString& ContextPayload);

    void RemoveViewModel(UVelvetNotificationViewModel* VM);
};