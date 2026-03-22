#include "VelvetMVVMSubsystem.h"
#include "MVVMGameSubsystem.h"

void UVelvetMVVMSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    MVVMSubsystem = GetGameInstance()->GetSubsystem<UMVVMGameSubsystem>();
    if (!MVVMSubsystem)
    {
        UE_LOG(LogCore, Error, TEXT("MVVMSubsystem not found when trying to initialize VelvetMVVMSubsystem."));
    }

    // Créer et enregistrer les VMs principales
	CreateAndRegisterViewModel(UVelvetSocialViewModel::StaticClass(), TEXT("Social"));
	CreateAndRegisterViewModel(UVelvetFriendSearchViewModel::StaticClass(), TEXT("FriendSearch"));
    CreateAndRegisterViewModel(UVelvetPartyViewModel::StaticClass(), TEXT("Party"));
}

UVelvetSocialViewModel* UVelvetMVVMSubsystem::GetSocialViewModel()
{
	return GetViewModelTyped<UVelvetSocialViewModel>(TEXT("Social"));
}

UVelvetFriendSearchViewModel* UVelvetMVVMSubsystem::GetFriendSearchViewModel()
{
    return GetViewModelTyped<UVelvetFriendSearchViewModel>(TEXT("FriendSearch"));
}

UVelvetPartyViewModel* UVelvetMVVMSubsystem::GetPartyViewModel()
{
    return GetViewModelTyped<UVelvetPartyViewModel>(TEXT("Party"));
}

UMVVMViewModelBase* UVelvetMVVMSubsystem::GetViewModel(
    TSubclassOf<UMVVMViewModelBase> ViewModelClass,
    FName ViewModelName)
{
    // Le cache local évite de recréer
    if (TObjectPtr<UMVVMViewModelBase>* Cached = ViewModelCache.Find(ViewModelName))
    {
        return Cached->Get();
    }
    
    return CreateAndRegisterViewModel(ViewModelClass, ViewModelName);
}

UMVVMViewModelBase* UVelvetMVVMSubsystem::CreateAndRegisterViewModel(TSubclassOf<UMVVMViewModelBase> ViewModelClass, FName ViewModelName)
{
    UMVVMViewModelBase* NewVM = Cast<UMVVMViewModelBase>(
        NewObject<UObject>(GetTransientPackage(), ViewModelClass)
    );

    FMVVMViewModelContext Context;
    Context.ContextClass = ViewModelClass;
    Context.ContextName = ViewModelName;
    MVVMSubsystem->GetViewModelCollection()->AddViewModelInstance(Context, NewVM);

    ViewModelCache.Add(ViewModelName, NewVM);

	return NewVM;
}