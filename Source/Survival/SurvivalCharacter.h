// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "SurvivalCharacter.generated.h"

UCLASS(config=Game)
class ASurvivalCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	
public:
	ASurvivalCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	class UPlayerStatsComponent* PlayerStatComp;
protected:

	class ULineTrace* LineTraceComp;
	class UInventory* Inventory;

	UFUNCTION(BlueprintPure)
		FString ReturnPlayerStats();

	bool bIsSprinting;

	FTimerHandle SprintingHandle;

protected:

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	void StartSprinting();

	void StopSprinting();

	void HandleSprinting();

	void OpenCloseInventory();
	
	void Interact();//Interact with the pickup object

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerInteract();

	//Server function
	bool ServerInteract_Validate();
	void ServerInteract_Implementation();

	void DestroyDestructible();//Destroy the destructible mesh 

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerDestroyDestructibleMesh();

	//Server function
	bool ServerDestroyDestructibleMesh_Validate();
	void ServerDestroyDestructibleMesh_Implementation();

	void DestroyMesh(class UDestructibleComponent* DestructibleComponent);

	UFUNCTION(NetMulticast,Reliable,WithValidation)
		void MultiDestroyMesh(class UDestructibleComponent* DestructibleComponent);

	bool MultiDestroyMesh_Validate(class UDestructibleComponent* DestructibleComponent);
	void MultiDestroyMesh_Implementation(class UDestructibleComponent* DestructibleComponent);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
		class UInventory* GetInventoryComp();

	class UUserWidget* InventoryWidget;
	TSubclassOf<class UUserWidget> InventoryWidgetClass;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

