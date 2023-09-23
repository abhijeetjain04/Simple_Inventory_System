// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
#include "SurvivalCharacter.h"

#include "PlayerStatsComponent.h"
#include "LineTrace.h"
#include "Pickups.h"
#include "Inventory.h"

//#include <Actor.h>
#include "GameFramework/Actor.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"

#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"

#include "DestructibleActor.h"
#include "DestructibleComponent.h"

//////////////////////////////////////////////////////////////////////////
// ASurvivalCharacter

ASurvivalCharacter::ASurvivalCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	bIsSprinting = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	//So that we can have access to everything we do in playerstate over here
	PlayerStatComp = CreateDefaultSubobject<UPlayerStatsComponent>("PlayerStatComponent");
	LineTraceComp = CreateDefaultSubobject<ULineTrace>("LineTraceComponent");
	Inventory = CreateDefaultSubobject<UInventory>("InventoryComponent");

	//Finding Inventory UI in content browser and create ref so that we can use it later
	static ConstructorHelpers::FClassFinder<UUserWidget>InventoryRef(TEXT("/Game/UI/InventoryUI/BP_InventoryUI"));

	if(InventoryRef.Class)
	{
		InventoryWidgetClass = InventoryRef.Class;
	}
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

}

//////////////////////////////////////////////////////////////////////////
// Input

void ASurvivalCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ASurvivalCharacter::StartSprinting);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ASurvivalCharacter::StopSprinting);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASurvivalCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASurvivalCharacter::MoveRight);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ASurvivalCharacter::Interact);// When "F" is pressed it calls interact function
	PlayerInputComponent->BindAction("DestroyDestructible", IE_Pressed, this, &ASurvivalCharacter::DestroyDestructible);// When "K" is pressed it calls DestroyDestructible function
	PlayerInputComponent->BindAction("OpenInventory", IE_Pressed, this, &ASurvivalCharacter::OpenCloseInventory);// When "I" is pressed it calls Inventory function

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ASurvivalCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ASurvivalCharacter::LookUpAtRate);
}

void ASurvivalCharacter::BeginPlay()
{
	Super::BeginPlay();

	//Timer function to handle stamina
	GetWorld()->GetTimerManager().SetTimer(SprintingHandle, this, &ASurvivalCharacter::HandleSprinting, 1.0f, true);
}

void ASurvivalCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ASurvivalCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ASurvivalCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		//When "Shift" is not pressed reduce the speed to half
		if (!bIsSprinting)
			Value *= 0.5;

		AddMovementInput(Direction, Value);
	}
}

void ASurvivalCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		//When "Shift" is not pressed reduce the speed to half
		if (!bIsSprinting)
			Value *= 0.5;
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void ASurvivalCharacter::StartSprinting()//Unpause sprinting timer
{
	if (PlayerStatComp->GetStamina() > 10.0f)
	{
		bIsSprinting = true;
		PlayerStatComp->ControlSprintingTimer(true);
	}
	else if (PlayerStatComp->GetStamina() <= 0.0f)
		PlayerStatComp->ControlSprintingTimer(false);

}

void ASurvivalCharacter::StopSprinting()//Pause sprinting timer
{
	bIsSprinting = false;
	PlayerStatComp->ControlSprintingTimer(false);
}

void ASurvivalCharacter::HandleSprinting()
{
	if (bIsSprinting && this->GetVelocity().Size())// This will prevent stamina reduction when we are holding shift while standing 
	{
		PlayerStatComp->LowerStamina(2.0f);
		if(PlayerStatComp->GetStamina() <=0.0f)
		{
			StopSprinting();
		}
	}
}

void ASurvivalCharacter::OpenCloseInventory()
{
	if(InventoryWidget && InventoryWidget->IsInViewport())
	{
		InventoryWidget->RemoveFromViewport();// Remove widget from view port
		if (APlayerController* PlayerController = GetController()->CastToPlayerController())
		{
			PlayerController->bShowMouseCursor = false;
			PlayerController->SetInputMode(FInputModeGameOnly());
		}
	}
	else
	{
		InventoryWidget = CreateWidget<UUserWidget>(GetWorld(), InventoryWidgetClass);

		if (InventoryWidget)
		{
			InventoryWidget->AddToViewport();//Add widget to view port

			if(APlayerController* PlayerController = GetController()->CastToPlayerController())
			{
				PlayerController->bShowMouseCursor = true;
				PlayerController->SetInputMode(FInputModeGameAndUI());
			}
			
		}
	}
	
}

void ASurvivalCharacter::Interact()// This is going to be called on client
{
	FVector StartLine = GetMesh()->GetBoneLocation(FName("head")); //Start location 
	FVector EndLine = StartLine + FollowCamera->GetForwardVector() * 170.0f;//End location
	FHitResult HitResult = LineTraceComp->LineTraceSingle(StartLine, EndLine, false); // This is running on client 
	if (AActor* Actor = HitResult.GetActor())
	{
		//UE_LOG(LogTemp, Warning, TEXT("HIT ACTOR: %s"), *Actor->GetName());

		if (APickups* Pickup = Cast<APickups>(Actor))
		{
			UE_LOG(LogTemp, Warning, TEXT("HIT ACTOR IS PICKUP"));// runs on client

			//Client is just checking if it hit something , actual calculation is done on server using serverInteract() function
			ServerInteract(); //Now run this on server as well
			// If we run multicastdestroy here then only that client will see it not server or other client 
		}
	}
}

bool ASurvivalCharacter::ServerInteract_Validate()
{
	return true;
}

void ASurvivalCharacter::ServerInteract_Implementation()// This is line trace on server 
{
	if (Role == ROLE_Authority)// Make sure this is server
	{
		FVector StartLine = GetMesh()->GetBoneLocation(FName("head"));
		FVector EndLine = StartLine + FollowCamera->GetForwardVector() * 170.0f;
		FHitResult HitResult = LineTraceComp->LineTraceSingle(StartLine, EndLine); // This is running on client 
		if (AActor* Actor = HitResult.GetActor())
		{
			if (APickups* Pickup = Cast<APickups>(Actor))
			{
				Inventory->AddItem(Pickup);
			}

		}
	}
}

void ASurvivalCharacter::DestroyDestructible()
{
	FVector StartLine = GetMesh()->GetBoneLocation(FName("head")); //Start location 
	FVector EndLine = StartLine + FollowCamera->GetForwardVector() * 1500.0f;//End location
	FHitResult HitResult = LineTraceComp->LineTraceSingle(StartLine, EndLine, false); // This is running on client 
	if (AActor* Actor = HitResult.GetActor())
	{
		if (ADestructibleActor* Destructible = Cast<ADestructibleActor>(Actor))
		{
			UE_LOG(LogTemp, Warning, TEXT("HIT DestructibleActor IS PICKUP"));// runs on client

			//Client is just checking if just hits something , actual calculation is done on server using serverInteract() function
			ServerDestroyDestructibleMesh(); //Now run this on server as well
			// If we run multicastdestroy here then only that client will see it not server or other client 
		}
	}
}

bool ASurvivalCharacter::ServerDestroyDestructibleMesh_Validate()
{
	return true;
}

void ASurvivalCharacter::ServerDestroyDestructibleMesh_Implementation()
{
	if (Role == ROLE_Authority)// Make sure this is server
	{
		FVector StartLine = GetMesh()->GetBoneLocation(FName("head")); //Start location 
		FVector EndLine = StartLine + FollowCamera->GetForwardVector() * 1500.0f;//End location
		FHitResult HitResult = LineTraceComp->LineTraceSingle(StartLine, EndLine); // This is running on client 
		if (AActor* Actor = HitResult.GetActor())
		{
			if (ADestructibleActor* Destructible = Cast<ADestructibleActor>(Actor))
			{
				UE_LOG(LogTemp, Warning, TEXT("HIT DestructibleActor IS PICKUP"));// runs on client

				//Client is just checking if just hits something , actual calculation is done on server using serverInteract() function
				UDestructibleComponent* DestructibleComponent = Destructible->GetDestructibleComponent();
			
				DestroyMesh(DestructibleComponent);
			}
		}
	}
}

void ASurvivalCharacter::DestroyMesh(UDestructibleComponent* DestructibleComponent)
{
	if (Role == ROLE_Authority)// Make sure this is server
	{
		MultiDestroyMesh(DestructibleComponent);
	}
}

bool ASurvivalCharacter::MultiDestroyMesh_Validate(UDestructibleComponent* DestructibleComponent)
{
	return true;
}

void ASurvivalCharacter::MultiDestroyMesh_Implementation(UDestructibleComponent* DestructibleComponent)
{
	//Apply damage to Destructible object 
	DestructibleComponent->ApplyDamage(10000.0f, DestructibleComponent->GetComponentLocation(),
			DestructibleComponent->GetComponentTransform().GetUnitAxis(EAxis::Z), 10000.0f);
}

FString ASurvivalCharacter::ReturnPlayerStats() 
{
	//Return all the stats to display on player stats UI
	FString Restring = "Hunger: " + FString::SanitizeFloat(PlayerStatComp->GetHunger()) +
		" Thirst: " + FString::SanitizeFloat(PlayerStatComp->GetThirst()) +
		" Stamina: " + FString::SanitizeFloat(PlayerStatComp->GetStamina()) +
		" Health: " + FString::SanitizeFloat(PlayerStatComp->GetHealth()
		);
	return Restring;
}

UInventory* ASurvivalCharacter::GetInventoryComp()
{
	return Inventory;
}