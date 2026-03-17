#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "MVVMGameSubsystem.h"
#include "VelvetSocialViewModel.h"
#include "VelvetFriendSearchViewModel.h"
#include "VelvetMVVMSubsystem.generated.h"

UCLASS()
class VELVETDOOR_API UVelvetMVVMSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Velvet|MVVM")
    UVelvetSocialViewModel* GetSocialViewModel();

    UFUNCTION(BlueprintCallable, Category = "Velvet|MVVM")
    UVelvetFriendSearchViewModel* GetFriendSearchViewModel();

    UFUNCTION(BlueprintCallable, Category = "Velvet|MVVM",
        meta = (DeterminesOutputType = "ViewModelClass", DynamicOutputParam = "ReturnValue"))
    UMVVMViewModelBase* GetViewModel(
        TSubclassOf<UMVVMViewModelBase> ViewModelClass,
        FName ViewModelName
    );

    template<typename T> 
    T* GetViewModelTyped(FName ViewModelName)
    {
        return Cast<T>(GetViewModel(T::StaticClass(), ViewModelName));
    }

private:
    UPROPERTY()
    TMap<FName, TObjectPtr<UMVVMViewModelBase>> ViewModelCache;
    
    UPROPERTY()
    UMVVMGameSubsystem* MVVMSubsystem;

    UMVVMViewModelBase* CreateAndRegisterViewModel(TSubclassOf<UMVVMViewModelBase> ViewModelClass, FName ViewModelName);
};