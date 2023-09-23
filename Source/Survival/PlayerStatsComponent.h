// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerStatsComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SURVIVAL_API UPlayerStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayerStatsComponent();

protected:
	UPROPERTY(Replicated)
		float  Hunger;

	UPROPERTY(EditAnywhere,Category="S|PlayerStats")
		float HungerDecrementValue;

	UPROPERTY(Replicated)
		float  Thirst;

	UPROPERTY(EditAnywhere, Category = "S|PlayerStats")
	float ThirstDecrementValue;

	UPROPERTY(Replicated)
		float  Health;

	UPROPERTY(Replicated)
		float Stamina;

	FTimerHandle TimerHandle;
	FTimerHandle StaminaHandle;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void HandleHungerAndThirst();

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerLowerHunger(float value);

	//Server function
	bool ServerLowerHunger_Validate(float value);
	void ServerLowerHunger_Implementation(float value);

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerLowerThirst(float value);

	//Server function
	bool ServerLowerThirst_Validate(float value);
	void ServerLowerThirst_Implementation(float value);

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerLowerStamina(float value);

	//Server function
	bool ServerLowerStamina_Validate(float value);
	void ServerLowerStamina_Implementation(float value);

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerControlSprintingTimer(bool IsSprinting);

	//Server function
	bool ServerControlSprintingTimer_Validate(bool IsSprinting);
	void ServerControlSprintingTimer_Implementation(bool IsSprinting);

	void RegenerateStamina();

public:

	void AddHunder(float value);
	void AddThirst(float value);
	void AddHealth(float value);

	void LowerHunger(float value);
	void LowerThirst(float value);
	void LowerStamina(float value);


	float GetHunger() { return Hunger; }
	float GetThirst() { return Thirst; }
	float GetStamina() { return Stamina; }
	float GetHealth() { return Health; }

	void ControlSprintingTimer(bool IsSprinting);
};
