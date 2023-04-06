// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "EnhancedInputComponent.h"
#include "PlayerControlsComponent.generated.h"


class UEnhancedInputLocalPlayerSubsystem;
UCLASS(MinimalAPI, Blueprintable, BlueprintType, Category="Input", meta=(BlueprintSpawnableComponent))
class UPlayerControlsComponent : public UPawnComponent
{
	GENERATED_BODY()

public:

	virtual void OnRegister() override;
	virtual void OnUnregister() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player Controls")
	UInputMappingContext* InputMappingContext = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player Controls")
	int InputPriority = 0;

protected:

	UFUNCTION(BlueprintNativeEvent, Category="Player Controls")
	void SetupPlayerControls(UEnhancedInputComponent* PlayerInputComponent);

	UFUNCTION(BlueprintNativeEvent, Category="Player Controls")
	void TeardownPlayerControls(UEnhancedInputComponent* PlayerInputComponent);

	template<class UserClass, typename FuncType>
	bool BindInputAction(const UInputAction* Action, const ETriggerEvent EventType, UserClass* Object, FuncType Func)
	{
		if (ensure(InputComponent != nullptr) && ensure(Action != nullptr))
		{
			InputComponent->BindAction(Action, EventType, Object, Func);
			return true;
		}

		return false;
	}

	/**Called When pawn restarts, bound to dynamic Delegate*/
	UFUNCTION()
	virtual void OnPawnRestarted(APawn* Pawn);

	/**Called when pawn restarts, bound to dynamic delegate*/
	UFUNCTION()
	virtual void OnControllerChanged(APawn* Pawn, AController* OldController, AController* NewController);

	virtual void SetupInputComponent(APawn* Pawn);
	virtual void ReleaseInputComponent(AController* OldController = nullptr);
	UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem(AController* OldController = nullptr) const;
		
	UPROPERTY(Transient)
	UEnhancedInputComponent* InputComponent;
};
