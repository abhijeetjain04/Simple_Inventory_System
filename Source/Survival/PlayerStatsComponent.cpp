// Fill out your copyright notice in the Description page of Project Settings.
#include "PlayerStatsComponent.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

// Sets default values for this component's properties
UPlayerStatsComponent::UPlayerStatsComponent()
{
	bReplicates = true;
	Hunger = 100.0f;
	HungerDecrementValue= 0.5f;
	Thirst = 100.0f;
	ThirstDecrementValue = 0.5f;
	Stamina = 20.0f;
	Health = 30.0f;
}

// Called when the game starts
void UPlayerStatsComponent::BeginPlay()
{
	Super::BeginPlay();

	//SetIsReplicated(true);//ajn23 - this is need to replicate

	if (GetOwnerRole() == ROLE_Authority)
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UPlayerStatsComponent::HandleHungerAndThirst, 3.0f, true);//Call function to lower hunger and thirst
		GetWorld()->GetTimerManager().SetTimer(StaminaHandle, this, &UPlayerStatsComponent::RegenerateStamina, 1.0f, true);//regenerate stamina
	}
}

void UPlayerStatsComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate to everyone
	DOREPLIFETIME(UPlayerStatsComponent, Hunger);
	DOREPLIFETIME(UPlayerStatsComponent, Thirst);
	DOREPLIFETIME(UPlayerStatsComponent, Stamina);
	DOREPLIFETIME(UPlayerStatsComponent, Health);
}

void UPlayerStatsComponent::HandleHungerAndThirst()
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		//call lower hunger and thirst this will do the authority check for us
		LowerHunger(HungerDecrementValue);
		LowerThirst(ThirstDecrementValue);
	}
}

//Once you consume the pickup item increase hunger 
void UPlayerStatsComponent::AddHunder(float value)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		if (Hunger + value > 100.0f)
			Hunger = 100;
		else
			Hunger += value;
	}
}

//Once you consume the pickup item increase thirst 
void UPlayerStatsComponent::AddThirst(float value)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		if (Thirst + value > 100.0f)
			Thirst = 100.0f;
		else
			Thirst += value;
	}
}

//Once you consume the pickup item increase health 
void UPlayerStatsComponent::AddHealth(float value)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		if (Health + value > 100.0f)
			Health = 100.0f;
		else
			Health += value;
	}
}

void UPlayerStatsComponent::LowerHunger(float value)
{
	if(GetOwnerRole() < ROLE_Authority)
	{
		//Call server function that will call inter this funtion and set authority
		ServerLowerHunger(value);
	}
	else if(GetOwnerRole() == ROLE_Authority)
	{
		//Here we have authority - we are server
		Hunger -= value;
	}
}

void UPlayerStatsComponent::LowerThirst(float value)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		//Call server function that will call inter this funtion and set authority
		ServerLowerThirst(value);
	}
	else if(GetOwnerRole() == ROLE_Authority)
	{
		//Here we have authority- we are server
		Thirst -= value;
	}
}

void UPlayerStatsComponent::LowerStamina(float value)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		//Call server function that will call inter this funtion and set authority - we are client
		ServerLowerStamina(value);
	}
	else if (GetOwnerRole() == ROLE_Authority)
	{
		//Here we have authority- we are server
		Stamina -= value;
	}
}

bool UPlayerStatsComponent::ServerLowerHunger_Validate(float value)
{
	return true;
}

void UPlayerStatsComponent::ServerLowerHunger_Implementation(float value)
{
	if(GetOwnerRole() == ROLE_Authority)
	{
		LowerHunger(value);
	}
}

bool UPlayerStatsComponent::ServerLowerThirst_Validate(float value)
{
	return true;
}

void UPlayerStatsComponent::ServerLowerThirst_Implementation(float value)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		LowerThirst(value);
	}
}

bool UPlayerStatsComponent::ServerLowerStamina_Validate(float value)
{
	return true;
}

void UPlayerStatsComponent::ServerLowerStamina_Implementation(float value)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		LowerStamina(value);
	}
}

bool UPlayerStatsComponent::ServerControlSprintingTimer_Validate(bool IsSprinting)
{
	return true;
}

void UPlayerStatsComponent::ServerControlSprintingTimer_Implementation(bool IsSprinting)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		ControlSprintingTimer(IsSprinting);
	}
}

void UPlayerStatsComponent::RegenerateStamina()
{
	if(GetOwnerRole() == ROLE_Authority)
	{
	if (Stamina >= 100)
		Stamina = 100.0f;
	else
		++Stamina;
	}
}

void UPlayerStatsComponent::ControlSprintingTimer(bool IsSprinting)//make it pause timer on the server
{
	if(GetOwnerRole() < ROLE_Authority)
	{
		ServerControlSprintingTimer(IsSprinting);
	}

	else if (GetOwnerRole() == ROLE_Authority) // Check for server and start or stop timer to reduce the stamina
	{
		if (IsSprinting)
		{
			GetWorld()->GetTimerManager().PauseTimer(StaminaHandle);
		}
		else
		{
			GetWorld()->GetTimerManager().UnPauseTimer(StaminaHandle);
		}
	}
}

