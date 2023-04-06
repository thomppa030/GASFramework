// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Containers/Array.h"
#include "Containers/Map.h"
#include "PlayerControlsComponent.h"
#include "GameplayAbilitySpec.h"
#include "GameplayAbilitySpecHandle.h"
#include "AbilityInputBindingComponent.generated.h"


class AController;
class UAbilitySystemComponent;
class UEnhancedInputComponent;
class UInputAction;
class UObject;
struct FFrame;

USTRUCT()
struct FAbilityInputBinding
{
	GENERATED_BODY()

	int32 InputID{0};
	uint32 OnPressedHandle{0};
	uint32 OnReleasedHandle{0};
	TArray<FGameplayAbilitySpecHandle> BoundAbilitiesStack;
};

UCLASS(meta=(BlueprintSpawnableComponent))
class GASFRAMEWORK_API UAbilityInputBindingComponent : public UPlayerControlsComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Abilities")
	void SetupInputBinding(UInputAction* InputAction, FGameplayAbilitySpecHandle AbilityHandle);

	UFUNCTION(BlueprintCallable, Category="Abilities")
	void ClearInputBinding(FGameplayAbilitySpecHandle AbilityHandle);

	UFUNCTION(BlueprintCallable, Category="Abilities")
	void ClearAbilityBindings(UInputAction* InputAction);

	// UPlayerControlsComponent interface
	virtual void SetupPlayerControls_Implementation(UEnhancedInputComponent* PlayerInputComponent) override;
	virtual void ReleaseInputComponent(AController* OldController) override;
	// End of UPlayerControlsComponent interface

private:
	void ResetBindings();
	void RunAbilitySystemSetup();
	void OnAbilityInputPressed(UInputAction* InputAction);
	void OnAbilityInputReleased(UInputAction* InputAction);

	void RemoveEntry(UInputAction* InputAction);

	FGameplayAbilitySpec* FindAbilitySpec(FGameplayAbilitySpecHandle Handle);
	void TryBindAbilityInput(UInputAction* InputAction, FAbilityInputBinding& AbilityInputBinding);

private:
	UPROPERTY(Transient)
	UAbilitySystemComponent* AbilityComponent;

	UPROPERTY(Transient)
	TMap<UInputAction*, FAbilityInputBinding> MappedAbilities;
};
