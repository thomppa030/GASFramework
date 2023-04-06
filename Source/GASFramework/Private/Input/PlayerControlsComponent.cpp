// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/PlayerControlsComponent.h"

#include "EnhancedInputSubsystems.h"

void UPlayerControlsComponent::OnRegister()
{
	Super::OnRegister();

	UWorld* World = GetWorld();
	APawn* Owner = GetPawn<APawn>();

	if (ensure(Owner) && World->IsGameWorld())
	{
		Owner->ReceiveRestartedDelegate.AddDynamic(this, &UPlayerControlsComponent::OnPawnRestarted);
		Owner->ReceiveControllerChangedDelegate.AddDynamic(this, &UPlayerControlsComponent::OnControllerChanged);

		if (Owner->InputComponent)
		{
			OnPawnRestarted(Owner);
		}
	}
}

void UPlayerControlsComponent::OnUnregister()
{
	UWorld* World = GetWorld();

	if (World && World->IsGameWorld())
	{
		ReleaseInputComponent();

		APawn* Owner = GetPawn<APawn>();
		if (Owner)
		{
			Owner->ReceiveRestartedDelegate.RemoveAll(this);
			Owner->ReceiveControllerChangedDelegate.RemoveAll(this);
		}
	}
	
	Super::OnUnregister();
}

void UPlayerControlsComponent::SetupPlayerControls_Implementation(UEnhancedInputComponent* PlayerInputComponent)
{
}

void UPlayerControlsComponent::TeardownPlayerControls_Implementation(UEnhancedInputComponent* PlayerInputComponent)
{
}

void UPlayerControlsComponent::OnPawnRestarted(APawn* Pawn)
{
	if (ensure(Pawn && Pawn == GetOwner()) && Pawn->InputComponent)
	{
		ReleaseInputComponent();

		if (Pawn->InputComponent)
		{
			SetupInputComponent(Pawn);
		}
	}
}

void UPlayerControlsComponent::OnControllerChanged(APawn* Pawn, AController* OldController, AController* NewController)
{
	// Only handle releasing, restart is a better time to handle binding
	if (ensure(Pawn && Pawn == GetOwner() && OldController))
	{
		ReleaseInputComponent(OldController);
	}
}

void UPlayerControlsComponent::SetupInputComponent(APawn* Pawn)
{
	InputComponent = CastChecked<UEnhancedInputComponent>(Pawn->InputComponent);

	if (ensureMsgf(InputComponent, TEXT("Project must use EnhancedInputComponent to support PlayerControlsComponent")))
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSubsystem();
		check(Subsystem);

		if (InputMappingContext)
		{
			Subsystem->AddMappingContext(InputMappingContext, InputPriority);
		}
		
		SetupPlayerControls(InputComponent);
	}
}

void UPlayerControlsComponent::ReleaseInputComponent(AController* OldController)
{
	UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSubsystem(OldController);
	if (Subsystem && InputComponent)
	{
		TeardownPlayerControls(InputComponent);
		
		if (InputMappingContext)
		{
			Subsystem->RemoveMappingContext(InputMappingContext);
		}
	}
	InputComponent = nullptr;
}

UEnhancedInputLocalPlayerSubsystem* UPlayerControlsComponent::GetEnhancedInputSubsystem(
	AController* OldController) const
{
	const APlayerController* PlayerController = GetController<APlayerController>();

	if (!PlayerController)
	{
		PlayerController = Cast<APlayerController>(OldController);
		if (!PlayerController)
		{
			return nullptr;
		}
	}

	const ULocalPlayer* LP = PlayerController->GetLocalPlayer();
	if (!LP)
	{
		return nullptr;
	}

	return LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
}
