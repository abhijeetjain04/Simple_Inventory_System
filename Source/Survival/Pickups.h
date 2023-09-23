// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SurvivalCharacter.h"

#include "Pickups.generated.h"


UENUM(BlueprintType)
enum class EPickupType:uint8
{
	EWater UMETA(DisplayName = "Water"),
	EFood UMETA(DisplayName = "Food"),
	EHeath UMETA(DisplayName = "Heath")
};

UCLASS()
class SURVIVAL_API APickups : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickups();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
		float IncreaseHungerAmount;

	UPROPERTY(EditAnywhere)
		class UTexture2D* Icon;

protected:
	UPROPERTY(EditAnywhere, Category = "Pickups")
		class UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditAnywhere, Category = "Enums")
		EPickupType PickupType;

	UPROPERTY(ReplicatedUsing = OnRep_PickedUp)// When objectPicked is changed then OnRep_PickedUp is called automatically for all connected clients
		bool objectPicked;

	UFUNCTION()
		void OnRep_PickedUp();
public:	
	void UseItem(ASurvivalCharacter* Player);

	void InInventory(bool In);// To check if the item is present in inventory

	UFUNCTION(BlueprintCallable)
	class UTexture2D* GetItemIcon() { return Icon; }
};
