#include "URetryAsync.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

URetryAsync* URetryAsync::Retry(UObject* WorldContextObject, int32 AllowedRetries, float DelaySeconds)
{
	URetryAsync* Node = NewObject<URetryAsync>();
	Node->WorldContextObject = WorldContextObject;
	Node->MaxRetries = FMath::Max(0, AllowedRetries);
	Node->Delay = FMath::Max(0.f, DelaySeconds);
	return Node;
}

void URetryAsync::Activate()
{
	bCancelled = false;
	bCompleted = false;
	bAwaitingAck = false;
	AttemptIndex = 0;

	FireAttempt();
}

void URetryAsync::Succeed()
{
	if (bCompleted || bCancelled) return;
	if (!bAwaitingAck) return; // ignore stray calls

	Finish(ERetryAsyncResult::Succeeded);
}

void URetryAsync::FailAndRetry()
{
	if (bCompleted || bCancelled) return;
	if (!bAwaitingAck) return;

	bAwaitingAck = false;

	// If we've already used up the last allowed retry, we are exhausted.
	// Example: AllowedRetries=0 => only one attempt (AttemptIndex=0) then Exhausted on Fail.
	if (AttemptIndex >= MaxRetries)
	{
		Finish(ERetryAsyncResult::Exhausted);
		return;
	}

	AttemptIndex++;

	UWorld* World = GetWorldFromContext();
	if (!World)
	{
		Finish(ERetryAsyncResult::Exhausted);
		return;
	}

	ClearTimer();

	FTimerDelegate D;
	D.BindUObject(this, &URetryAsync::FireAttempt);

	if (Delay <= 0.f)
	{
		World->GetTimerManager().SetTimerForNextTick(D);
	}
	else
	{
		World->GetTimerManager().SetTimer(RetryTimerHandle, D, Delay, false);
	}
}

void URetryAsync::Cancel()
{
	if (bCompleted) return;
	bCancelled = true;

	ClearTimer();
	Finish(ERetryAsyncResult::Cancelled);
}

void URetryAsync::FireAttempt()
{
	if (bCompleted) return;
	if (bCancelled)
	{
		Finish(ERetryAsyncResult::Cancelled);
		return;
	}

	bAwaitingAck = true;

	const int32 Remaining = MaxRetries - AttemptIndex;
	OnAttempt.Broadcast(this, AttemptIndex, Remaining);
}

void URetryAsync::Finish(ERetryAsyncResult Result)
{
	if (bCompleted) return;
	bCompleted = true;
	
	ClearTimer();
	switch (Result)
	{
		case ERetryAsyncResult::Succeeded:
			OnSucceed.Broadcast();
			break;
		case ERetryAsyncResult::Exhausted:
			OnExhausted.Broadcast();
			break;
		case ERetryAsyncResult::Cancelled:
			OnCancelled.Broadcast();
			break;
	}

	SetReadyToDestroy();
}

void URetryAsync::ClearTimer()
{
	if (UWorld* World = GetWorldFromContext())
	{
		World->GetTimerManager().ClearTimer(RetryTimerHandle);
	}
}

UWorld* URetryAsync::GetWorldFromContext() const
{
	if (!WorldContextObject) return nullptr;
	if (!GEngine) return nullptr;

	return GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
}