#include "VelvetNotificationSubsystem.h"
#include "Kismet/KismetSystemLibrary.h"

// ─────────────────────────────────────────────────────────────────────────────
//  UVelvetNotificationViewModel — setters
// ─────────────────────────────────────────────────────────────────────────────

void UVelvetNotificationViewModel::SetType(EVelvetNotificationType InType)
{
    UE_MVVM_SET_PROPERTY_VALUE(Type, InType);
}

void UVelvetNotificationViewModel::SetMessage(const FText& InMessage)
{
    UE_MVVM_SET_PROPERTY_VALUE(Message, InMessage);
}

void UVelvetNotificationViewModel::SetTimeRemaining(float InTimeRemaining)
{
    UE_MVVM_SET_PROPERTY_VALUE(TimeRemaining, InTimeRemaining);
}

void UVelvetNotificationViewModel::SetState(EVelvetNotificationState InState)
{
    UE_MVVM_SET_PROPERTY_VALUE(State, InState);
}

void UVelvetNotificationViewModel::SetHasActions(bool bInHasActions)
{
    UE_MVVM_SET_PROPERTY_VALUE(bHasActions, bInHasActions);
}

void UVelvetNotificationViewModel::SetContextPayload(const FString& InPayload)
{
    // Intermediate variable — exact type match required by UE_MVVM_SET_PROPERTY_VALUE.
    FString Payload = InPayload;
    UE_MVVM_SET_PROPERTY_VALUE(ContextPayload, Payload);
}

// ─────────────────────────────────────────────────────────────────────────────
//  UVelvetNotificationViewModel — actions
// ─────────────────────────────────────────────────────────────────────────────

void UVelvetNotificationViewModel::Accept()
{
    if (State != EVelvetNotificationState::Pending &&
        State != EVelvetNotificationState::Displayed) return;

    SetState(EVelvetNotificationState::Accepted);
    OnAccepted.Broadcast();
    OnDismissed.Broadcast(); // accepted → notification is resolved
}

void UVelvetNotificationViewModel::Decline()
{
    if (State != EVelvetNotificationState::Pending &&
        State != EVelvetNotificationState::Displayed) return;

    SetState(EVelvetNotificationState::Declined);
    OnDeclined.Broadcast();
    OnDismissed.Broadcast();
}

void UVelvetNotificationViewModel::Dismiss()
{
    // Guard against multiple calls (e.g. timeout fires after user already acted).
    if (State != EVelvetNotificationState::Pending &&
        State != EVelvetNotificationState::Displayed) return;

    SetState(EVelvetNotificationState::Dismissed);
    OnDismissed.Broadcast();
}

// ─────────────────────────────────────────────────────────────────────────────
//  UVelvetNotificationSubsystem
// ─────────────────────────────────────────────────────────────────────────────

UVelvetNotificationViewModel* UVelvetNotificationSubsystem::PushPassive(
    EVelvetNotificationType Type,
    const FText& Message,
    float Duration)
{
    return CreateAndRegister(Type, Message, Duration, /*bHasActions=*/false, /*ContextPayload=*/{});
}

UVelvetNotificationViewModel* UVelvetNotificationSubsystem::PushActionable(
    EVelvetNotificationType Type,
    const FText& Message,
    const FString& ContextPayload)
{
    return CreateAndRegister(Type, Message, /*Duration=*/0.f, /*bHasActions=*/true, ContextPayload);
}

UVelvetNotificationViewModel* UVelvetNotificationSubsystem::CreateAndRegister(
    EVelvetNotificationType Type,
    const FText& Message,
    float Duration,
    bool bHasActions,
    const FString& ContextPayload)
{
    UVelvetNotificationViewModel* VM = NewObject<UVelvetNotificationViewModel>(this);
    VM->SetType(Type);
    VM->SetMessage(Message);
    VM->SetTimeRemaining(Duration);
    VM->SetHasActions(bHasActions);
    VM->SetContextPayload(ContextPayload);
    VM->InitialDuration = Duration;

    // Remove from active array when resolved by any means.
    VM->OnDismissed.AddWeakLambda(this, [this, VM]()
        {
            RemoveViewModel(VM);
        });
    
    ActiveNotifications.Add(VM);
    OnNotificationPushed.Broadcast(VM);

    return VM;
}

void UVelvetNotificationSubsystem::RemoveViewModel(UVelvetNotificationViewModel* VM)
{
    ActiveNotifications.RemoveSingle(VM);
}

void UVelvetNotificationSubsystem::DismissAll()
{
    // Copy — Dismiss() triggers removal from ActiveNotifications during iteration.
    TArray<TObjectPtr<UVelvetNotificationViewModel>> Copy = ActiveNotifications;
    for (auto& VM : Copy)
    {
        if (VM) VM->Dismiss();
    }
}

void UVelvetNotificationSubsystem::Tick(float DeltaTime)
{
    TArray<TObjectPtr<UVelvetNotificationViewModel>> Copy = ActiveNotifications;
    for (auto& VM : Copy)
    {
        if (!VM || VM->IsPersistent()) continue;
        if (VM->GetState() != EVelvetNotificationState::Displayed) continue;

        const float NewTime = FMath::Max(VM->GetTimeRemaining() - DeltaTime, 0.f);
        VM->SetTimeRemaining(NewTime);

        if (VM->HasExpired())
        {
            VM->Dismiss();
        }
    }
}

void UVelvetNotificationSubsystem::NotifyDisplayed(UVelvetNotificationViewModel* VM)
{
    if (VM && VM->GetState() == EVelvetNotificationState::Pending)
        VM->SetState(EVelvetNotificationState::Displayed);
}

bool UVelvetNotificationSubsystem::IsTickable() const
{
    for (const auto& VM : ActiveNotifications)
    {
        if (VM && !VM->IsPersistent())
        {
            return true;
        }
    }
    return false;
}

TStatId UVelvetNotificationSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UVelvetNotificationSubsystem, STATGROUP_Tickables);
}