// Fill out your copyright notice in the Description page of Project Settings.


#include "SpeedPickup.h"
#include "TPSProject/Character/TPSCharacter.h"
#include "TPSProject/TPSComponent/BuffComponent.h"

void ASpeedPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ATPSCharacter* TPSCharacter = Cast<ATPSCharacter>(OtherActor);
	if (TPSCharacter)
	{
		UBuffComponent* Buff = TPSCharacter->GetBuff();
		if (Buff)
		{
			Buff->BuffSpeed(BaseSpeedBuff, BaseAimSpeedBuff,CrouchSpeedBuff, CrouchAimSpeedBuff, SpeedBuffTime);
		}
	}

	Destroy();
}