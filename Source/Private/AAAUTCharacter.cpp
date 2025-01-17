#include "UTVehiclesPrivatePCH.h"
#include "AAAUTCharacter.h"
#include "Vehicle.h"

#define AUTCharacter AAAAUTCharacter

AUTCharacter::AUTCharacter(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	bCanUse = true;
	VehicleCheckRadius = 150.f;
}

void AUTCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (Role == ROLE_Authority)
	{
		// Bind OnDied event to this pawn to garbage collect this zombie actor
		OnDied.AddDynamic(this, &AUTCharacter::OnPawnDied);
	}
}

bool AUTCharacter::Died(AController* EventInstigator, const FDamageEvent& DamageEvent)
{
	if (Role == ROLE_Authority && !IsDead())
	{
		// notify the vehicle we are currently driving
		AVehicle* DrivenVehicleVec = Cast<AVehicle>(DrivenVehicle);
		if (DrivenVehicleVec != NULL)
		{
			if (GetCharacterMovement() != NULL)
			{
				GetCharacterMovement()->Velocity = DrivenVehicle->GetVelocity();
			}

			DrivenVehicleVec->DriverDied();
		}
	}

	return Super::Died(EventInstigator, DamageEvent);
}

void AUTCharacter::OnPawnDied(AController* Killer, const UDamageType* DamageType)
{
	if (Role < ROLE_Authority)
		return;
	
	// clear driven vehicle
	DrivenVehicle = NULL;
}

bool AUTCharacter::CanUse() const
{
	return CanUseInternal();
}

bool AUTCharacter::CanUseInternal_Implementation() const
{
	return bCanUse;
}

// TODO: Implement once methods are virtual
/*void AUTCharacter::StartDriving(APawn* Vehicle)
{
	Super::StartDriving(Vehicle);

	if (AVehicle* V = Cast<AVehicle>(Vehicle))
	{
		V->AttachDriver(this);
	}
}

void AUTCharacter::StopDriving(APawn* Vehicle)
{
	Super::StopDriving(Vehicle);

	if (AVehicle* V = Cast<AVehicle>(Vehicle))
	{
		V->StopFiring();
		V->DetachDriver(this);
	}
}*/

// Note: Workaround for StartDriving being non-virtual
void AUTCharacter::StartDriving_WORKAROUND(APawn* Vehicle)
{
	if (AVehicle* V = Cast<AVehicle>(Vehicle))
	{
		V->AttachDriver(this);
	}
}

// Note: Workaround for StopDriving being non-virtual
void AUTCharacter::StopDriving_WORKAROUND(APawn* Vehicle)
{
	if (AVehicle* V = Cast<AVehicle>(Vehicle))
	{
		V->StopFiring();
		V->DetachDriver(this);
	}
}

void AUTCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bDrivenVehicleStored && DrivenVehicle != NULL && OldDrivenVehicle == NULL)
	{
		OldDrivenVehicle = DrivenVehicle;
		bDrivenVehicleStored = true;
		StartDriving_WORKAROUND(DrivenVehicle);
	}
	else if (bDrivenVehicleStored && DrivenVehicle == NULL && OldDrivenVehicle != NULL)
	{
		StopDriving_WORKAROUND(OldDrivenVehicle);
		bDrivenVehicleStored = false;
		OldDrivenVehicle = NULL;
	}
}

#undef AUTCharacter