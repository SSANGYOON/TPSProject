// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WeaponPickupWidget.generated.h"

/**
 * 
 */
UCLASS()
class TPSPROJECT_API UWeaponPickupWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class AWeapon* OwningWeapon;
	
};
