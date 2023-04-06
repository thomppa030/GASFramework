// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/AbilityInputBindingComponent.h"
#include "AbilitySystemGlobals.h"

#include "AbilitySystemComponent.h"

namespace AbilityInputSystemComponent_Impl 
{
	constexpr int32 InvalidInputID{0};
	int32 IncrementingInputID{InvalidInputID};

	static int32 GetNextInputID()
	{
		return ++IncrementingInputID;
	}
}

void UAbilityInputBindingComponent::SetupInputBinding(UInputAction* InputAction,
	FGameplayAbilitySpecHandle AbilityHandle)
{
	using namespace AbilityInputSystemComponent_Impl;

	FGameplayAbilitySpec* BindingAbility = FindAbilitySpec(AbilityHandle);

	FAbilityInputBinding* AbilityInputBinding = MappedAbilities.Find(InputAction);

	if (AbilityInputBinding)
	{
		FGameplayAbilitySpec* OldBoundAbility = FindAbilitySpec(AbilityInputBinding->BoundAbilitiesStack.Top());
		if (OldBoundAbility && OldBoundAbility->InputID == AbilityInputBinding->InputID)
		{
			OldBoundAbility->InputID = InvalidInputID;
		}
	}
	else
	{
		AbilityInputBinding = &MappedAbilities.Add(InputAction);
		AbilityInputBinding->InputID = GetNextInputID();
	}

	if (BindingAbility)
	{
		BindingAbility->InputID = AbilityInputBinding->InputID;
	}

	AbilityInputBinding->BoundAbilitiesStack.Push(AbilityHandle);
	TryBindAbilityInput(InputAction, *AbilityInputBinding);
}

void UAbilityInputBindingComponent::ClearInputBinding(FGameplayAbilitySpecHandle AbilityHandle)
{
	using namespace AbilityInputSystemComponent_Impl;

	FGameplayAbilitySpec* FoundAbility = FindAbilitySpec(AbilityHandle);
	if (FoundAbility)
	{
		auto MappedIterator = MappedAbilities.CreateIterator();
		while (MappedIterator)
		{
			if (MappedIterator.Value().InputID == FoundAbility->InputID)
			{
				break;
			}
			
			++MappedIterator;
		}

		if (MappedIterator)
		{
			FAbilityInputBinding& AbilityInputBinding = MappedIterator.Value();
			
			if (AbilityInputBinding.BoundAbilitiesStack.Remove(AbilityHandle) > 0)
			{
				FGameplayAbilitySpec* StackedAbility = FindAbilitySpec(AbilityInputBinding.BoundAbilitiesStack.Top());
				if (StackedAbility && StackedAbility->InputID == 0)
				{
					StackedAbility->InputID = AbilityInputBinding.InputID;
				}
			}
			else
			{
				RemoveEntry(MappedIterator.Key());
			}

			FoundAbility->InputID = InvalidInputID;
		}
	}
}

void UAbilityInputBindingComponent::ClearAbilityBindings(UInputAction* InputAction)
{
	RemoveEntry(InputAction);
}

void UAbilityInputBindingComponent::SetupPlayerControls_Implementation(UEnhancedInputComponent* PlayerInputComponent)
{
	ResetBindings();

	for (auto& Ability : MappedAbilities)
	{
		UInputAction* InputAction = Ability.Key;

		InputComponent->BindAction(InputAction, ETriggerEvent::Started, this, &UAbilityInputBindingComponent::OnAbilityInputPressed, InputAction);
		InputComponent->BindAction(InputAction, ETriggerEvent::Completed, this, &UAbilityInputBindingComponent::OnAbilityInputReleased, InputAction);
	}

	RunAbilitySystemSetup();
}

void UAbilityInputBindingComponent::ReleaseInputComponent(AController* OldController)
{
	ResetBindings();
	
	Super::ReleaseInputComponent(OldController);
}

void UAbilityInputBindingComponent::ResetBindings()
{
	for (auto& InputBinding : MappedAbilities)
	{
		if (InputComponent)
		{
			InputComponent->RemoveBindingByHandle(InputBinding.Value.OnPressedHandle);
			InputComponent->RemoveBindingByHandle(InputBinding.Value.OnReleasedHandle);
		}

		if (AbilityComponent)
		{
			const int32 ExpectedInputID = InputBinding.Value.InputID;

			for (auto AbilityHandle : InputBinding.Value.BoundAbilitiesStack)
			{
				FGameplayAbilitySpec* FoundAbility = AbilityComponent->FindAbilitySpecFromHandle(AbilityHandle);
				if (FoundAbility && FoundAbility->InputID == ExpectedInputID)
				{
					FoundAbility->InputID = AbilityInputSystemComponent_Impl::InvalidInputID;
				}
			}
		}
	}

	AbilityComponent = nullptr;
}

void UAbilityInputBindingComponent::RunAbilitySystemSetup()
{
	AActor *Owner = GetOwner();
	check(Owner);

	AbilityComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner);
	if (AbilityComponent)
	{
		for (auto& InputBinding : MappedAbilities)
		{
			const int32 NewInputID = AbilityInputSystemComponent_Impl::GetNextInputID();
			InputBinding.Value.InputID = NewInputID;

			for (auto AbilityHandle : InputBinding.Value.BoundAbilitiesStack)
			{
				FGameplayAbilitySpec* FoundAbility = AbilityComponent->FindAbilitySpecFromHandle(AbilityHandle);
				if (FoundAbility)
				{
					FoundAbility->InputID = NewInputID;
				}
			}
		}
	}
}

void UAbilityInputBindingComponent::OnAbilityInputPressed(UInputAction* InputAction)
{
	if (!AbilityComponent)
	{
		return;
	}

	if (AbilityComponent)
	{
		using namespace AbilityInputSystemComponent_Impl;

		FAbilityInputBinding* FoundBinding = MappedAbilities.Find(InputAction);
		if (FoundBinding && ensure(FoundBinding->InputID != InvalidInputID))
		{
			AbilityComponent->AbilityLocalInputPressed(FoundBinding->InputID);
		}
	}
}

void UAbilityInputBindingComponent::OnAbilityInputReleased(UInputAction* InputAction)
{
	if (AbilityComponent)
	{
		using namespace AbilityInputSystemComponent_Impl;

		FAbilityInputBinding* FoundBinding = MappedAbilities.Find(InputAction);
		if (FoundBinding && ensure(FoundBinding->InputID != InvalidInputID))
		{
			AbilityComponent->AbilityLocalInputReleased(FoundBinding->InputID);
		}
	}
}

void UAbilityInputBindingComponent::RemoveEntry(UInputAction* InputAction)
{
	if (FAbilityInputBinding* Bindings = MappedAbilities.Find(InputAction))
	{
		if (InputComponent)
		{
			InputComponent->RemoveBindingByHandle(Bindings->OnPressedHandle);
			InputComponent->RemoveBindingByHandle(Bindings->OnReleasedHandle);
		}

		for (auto AbilityHandle : Bindings->BoundAbilitiesStack)
		{
			using namespace AbilityInputSystemComponent_Impl;

			FGameplayAbilitySpec* AbilitySpec = FindAbilitySpec(AbilityHandle);
			if (AbilitySpec && AbilitySpec->InputID == Bindings->InputID)
			{
				AbilitySpec->InputID = InvalidInputID;
			}
		}

		MappedAbilities.Remove(InputAction);
	}
}

FGameplayAbilitySpec* UAbilityInputBindingComponent::FindAbilitySpec(FGameplayAbilitySpecHandle Handle)
{
	FGameplayAbilitySpec* FoundAbility = nullptr;
	if (AbilityComponent)
	{
		FoundAbility = AbilityComponent->FindAbilitySpecFromHandle(Handle);
	}
	return FoundAbility;
}

void UAbilityInputBindingComponent::TryBindAbilityInput(UInputAction* InputAction,
                                                        FAbilityInputBinding& AbilityInputBinding)
{
	if (InputComponent)
	{
		if (AbilityInputBinding.OnPressedHandle == 0)
		{
			AbilityInputBinding.OnPressedHandle = InputComponent->BindAction(InputAction, ETriggerEvent::Started, this, &UAbilityInputBindingComponent::OnAbilityInputPressed, InputAction).GetHandle();
		}

		if (AbilityInputBinding.OnReleasedHandle == 0)
		{
			AbilityInputBinding.OnReleasedHandle = InputComponent->BindAction(InputAction, ETriggerEvent::Completed, this, &UAbilityInputBindingComponent::OnAbilityInputReleased, InputAction).GetHandle();
		}
	}
}
