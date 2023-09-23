// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups.h"
#include "Components/StaticMeshComponent.h"
#include "PlayerStatsComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Texture2D.h"

// Sets default values
APickups::APickups()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");
	RootComponent = MeshComponent;
	IncreaseHungerAmount = 30.0f;
	bReplicates = true;
	bReplicateMovement = true;

	objectPicked = false;
	Icon = CreateDefaultSubobject<UTexture2D>("IconTexture");
}

// Called when the game starts or when spawned
void APickups::BeginPlay()
{
	Super::BeginPlay();
}

//This is the function which is used replicate variable across all the clients
void APickups::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// Replicate to everyone
	DOREPLIFETIME(APickups, objectPicked);
}

void APickups::UseItem(ASurvivalCharacter* Player)// This is the player who is actually using the item
{
	if(Role == ROLE_Authority) // Check if we are the server
	{
		if(PickupType == EPickupType::EFood)
		{
			UE_LOG(LogTemp, Warning, TEXT("ADDING HUNGER"));// Runs on server
			Player->PlayerStatComp->AddHunder(IncreaseHungerAmount);
		}
		else if (PickupType == EPickupType::EWater)
		{
			UE_LOG(LogTemp, Warning, TEXT("ADDING Thirst"));// Runs on server
			Player->PlayerStatComp->AddThirst(IncreaseHungerAmount);
		}
		else if (PickupType == EPickupType::EHeath)
		{
			UE_LOG(LogTemp, Warning, TEXT("ADDING Health"));// Runs on server
			Player->PlayerStatComp->AddHealth(IncreaseHungerAmount);
		}

		UE_LOG(LogTemp, Warning, TEXT("DESTROYING ACTOR"));// Runs on server
		Destroy();
	}
}

void APickups::OnRep_PickedUp()// When objectPicked is changed then OnRep_PickedUp is called automatically for all connected clients
{
	// If client 1 is joined the server and picked up the item, but then client 2 joins (late joining) he will still see the item, hence we hide the mesh component 
	this->MeshComponent->SetHiddenInGame(objectPicked);
	this->SetActorEnableCollision(!objectPicked);
}

void APickups::InInventory(bool In)
{
	if (Role == ROLE_Authority) // Check if we are the server
	{
		objectPicked = In;
		OnRep_PickedUp(); // TO run this on the server we have manually call it
	}
}


