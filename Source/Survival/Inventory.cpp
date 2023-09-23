// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory.h"
#include "Net/UnrealNetwork.h"
#include "Pickups.h"
#include "Components/StaticMeshComponent.h"
#include "SurvivalCharacter.h"
#include "Public/DrawDebugHelpers.h"

// Sets default values for this component's properties
UInventory::UInventory()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bReplicates = true;
}

// Called when the game starts
void UInventory::BeginPlay()
{
	Super::BeginPlay();
}

void UInventory::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// Replicate to everyone
	DOREPLIFETIME_CONDITION(UInventory, Items,COND_OwnerOnly);
}

bool UInventory::AddItem(APickups* Item)
{
	Items.Add(Item); //Replicated array of items

	Item->InInventory(true);

	/*for (APickups* pickup : Items)
	{
		UE_LOG(LogTemp, Warning, TEXT("Items: %s"), *pickup->GetName());
	}

	UE_LOG(LogTemp, Warning, TEXT("End of Items!"));*/
	return false;
}

void UInventory::DropAllInventory()
{
	if(GetOwnerRole() == ROLE_Authority)
	{
		for (APickups* pickup : Items)
		{
			DropItem(pickup);
		}
		Items.Empty();
	}
	
}

bool UInventory::CheckIfClientHasItem(APickups* Item)
{
	for (APickups* pc:Items)
	{
		if(pc == Item)
		{
			return true;
		}
	}

	return false; // Item which we are looking for is not in inventory
}

bool UInventory::RemoveItemFromInventory(APickups* Item)
{
	if (GetOwnerRole() == ROLE_Authority)//If we are server then find the item in our inventory and remove it 
	{
		int32 Counter = 0;
		for (APickups* pc : Items)
		{
			if (pc == Item)
			{
				Items.RemoveAt(Counter);
				return true;
			}
			++Counter;
		}
		return false;
	}
	return  false;
}

void UInventory::DropItem(APickups* Item) // This is called from BP which is then called on server
{
	Server_DropItem(Item);
}

bool UInventory::Server_DropItem_Validate(APickups* Item)
{
	return CheckIfClientHasItem(Item);// Check if the item is valid or not - if item is not in our inventory then we return false which it disconnects client 
}

void UInventory::Server_DropItem_Implementation(APickups* Item)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		//Get owner actor location and find random location near the actor
		FVector Location = GetOwner()->GetActorLocation();
		Location.X += FMath::RandRange(-50.0f, 100.0f);
		Location.Y += FMath::RandRange(-50.0f, 100.0f);
		FVector EndRay = Location;
		EndRay.Z -= 500.0f;

		FCollisionObjectQueryParams ObjectQuery;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(GetOwner());

		//Do the line trace and find nearest valid location 
		FHitResult hitresult;
		GetWorld()->LineTraceSingleByObjectType(OUT hitresult, Location, EndRay, ObjectQuery, QueryParams);

		//DrawDebugLine(GetWorld(), Location, EndRay, FColor::Red, true, 10.0f, 0, 5.0f);

		if (hitresult.ImpactPoint != FVector::ZeroVector)
			Location = hitresult.ImpactPoint;

		Item->SetActorLocation(Location);
		Item->InInventory(false);

		RemoveItemFromInventory(Item);
	}
}

void UInventory::UseItem(APickups* Item)
{
	Server_UseItem(Item);
}

bool UInventory::Server_UseItem_Validate(APickups* Item)
{
	return CheckIfClientHasItem(Item);
}

void UInventory::Server_UseItem_Implementation(APickups* Item)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		if(ASurvivalCharacter* Player = Cast<ASurvivalCharacter>(GetOwner()))
		{
			Item->UseItem(Player);
			RemoveItemFromInventory(Item);
		}
	}
}

TArray<class APickups*> UInventory::GetInventoryItems()
{
	return Items;
}

int32 UInventory::GetCurrentInventoryCount()
{
	return Items.Num()-1;
}



