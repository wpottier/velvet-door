#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "URetryAsync.generated.h"

UENUM(BlueprintType)
enum class ERetryAsyncResult : uint8
{
	Succeeded UMETA(DisplayName = "Succeeded"),
	Exhausted UMETA(DisplayName = "Exhausted"),
	Cancelled UMETA(DisplayName = "Cancelled"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FRetryAttempt, URetryAsync*, Handle, int32, AttemptIndex, int32, RemainingRetries);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRetryCancelled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRetryExhausted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRetrySucceed);

UCLASS()
class URetryAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	// Events (output exec pins)
	UPROPERTY(BlueprintAssignable)
	FRetryAttempt OnAttempt;

	UPROPERTY(BlueprintAssignable)
	FRetryCancelled OnCancelled;

	UPROPERTY(BlueprintAssignable)
	FRetryExhausted OnExhausted;

	UPROPERTY(BlueprintAssignable)
	FRetrySucceed OnSucceed;

	// Factory
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Retry (Async)"), Category = "Illuminate Works|Async")
	static URetryAsync* Retry(UObject* WorldContextObject, int32 AllowedRetries, float DelaySeconds);

	// Control methods (call on the returned handle)
	UFUNCTION(BlueprintCallable, Category = "Async|Retry")
	void Succeed();

	UFUNCTION(BlueprintCallable, Category = "Async|Retry")
	void FailAndRetry();

	UFUNCTION(BlueprintCallable, Category = "Async|Retry")
	void Cancel();

	// UBlueprintAsyncActionBase
	virtual void Activate() override;

private:
	UPROPERTY()
	TObjectPtr<UObject> WorldContextObject;

	int32 MaxRetries = 0;
	float Delay = 0.f;

	int32 AttemptIndex = 0;
	bool bCancelled = false;
	bool bCompleted = false;
	bool bAwaitingAck = false;

	FTimerHandle RetryTimerHandle;

	UWorld* GetWorldFromContext() const;
	void FireAttempt();
	void Finish(ERetryAsyncResult Result);
	void ClearTimer();
};