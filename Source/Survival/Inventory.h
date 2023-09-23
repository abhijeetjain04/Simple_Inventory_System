// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SURVIVAL_API UInventory : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventory();

protected:
	UPROPERTY(Replicated)
		TArray<class APickups*> Items;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

		UFUNCTION(Server, Reliable, WithValidation)
		void Server_DropItem(class APickups* Item);

		//Server function
		bool Server_DropItem_Validate(class APickups* Item);
		void Server_DropItem_Implementation(class APickups* Item);

		UFUNCTION(Server, Reliable, WithValidation)
			void Server_UseItem(class APickups* Item);

		//Server function
		bool Server_UseItem_Validate(class APickups* Item);
		void Server_UseItem_Implementation(class APickups* Item);

		bool CheckIfClientHasItem(class APickups* Item);
		bool RemoveItemFromInventory(class APickups* Item);

public:
	bool AddItem(class APickups* Item);

	void DropAllInventory();

	UFUNCTION(BlueprintCallable)
		void DropItem(class APickups* Item);

	UFUNCTION(BlueprintCallable)
		void UseItem(class APickups* Item);

	UFUNCTION(BlueprintCallable)
		TArray<class APickups*> GetInventoryItems();

	UFUNCTION(BlueprintCallable)
		int32 GetCurrentInventoryCount();
};
