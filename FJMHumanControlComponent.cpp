// Copyright Fergus Marsden 2021


#include "WorldWarTwo/Components/Characters/Gameplay/FJMHumanControlComponent.h"
#include <GameFramework/Controller.h>
#include "WorldWarTwo/Characters/FJMBasePlayerCharacter.h"
#include "WorldWarTwo/CommonObjects/FJMBaseTurret.h"
#include "WorldWarTwo/Controllers/FJMPlayerController.h"
#include "WorldWarTwo/Components/Characters/FJMRootMotionMovementComponent.h"
#include "WorldWarTwo/Components/Characters/Vehicles/FJMVehicleUseComponent.h"
#include "WorldWarTwo/Components/Characters/Dialogue/FJMDialogueComponent.h"
#include "WorldWarTwo/Components/Characters/Gameplay/FJMPlayerPickupComponent.h"
#include "WorldWarTwo/Components/Characters/Combat/Aiming/FJMCharacterAimingComponent.h"
#include "WorldWarTwo/Components/Characters/Combat/Cover/FJMCoverUseComponent.h"
#include "WorldWarTwo/Components/Characters/Combat/FJMCharacterWeaponComponent.h"
#include "WorldWarTwo/Components/Characters/Combat/FJMCharacterHealthComponent.h"
#include "WorldWarTwo/Components/Characters/Building/FJMBuildingSearchComponent.h"
#include "WorldWarTwo/CommonObjects/FJMEquipmentSelector.h"
#include "WorldWarTwo/FJMGameInstance.h"
#include <Kismet/KismetMathLibrary.h>
#include <Kismet/GameplayStatics.h>
#include <TimerManager.h>
#include "WorldWarTwo/Vehicles/FJMBaseVehicle.h"
#include <Components/InputComponent.h>
#include "WorldWarTwo/Components/Characters/Gameplay/FJMOrdersComponent.h"
#include <GameFramework/Pawn.h>
#include "WorldWarTwo/Components/Building/FJMInteriorCoverComponent.h"

// Sets default values for this component's properties
UFJMHumanControlComponent::UFJMHumanControlComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UFJMHumanControlComponent::SetHumanCharacter(AFJMBaseHumanCharacter* NewHuman)
{
	HumanChar = NewHuman;
}

// Called when the game starts
void UFJMHumanControlComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentGameInstance = Cast<UFJMGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	Controller = Cast<AController>(GetOwner());
	if (Controller)
	{
		HumanChar = Cast<AFJMBaseHumanCharacter>(Controller->GetPawn());
		if (HumanChar)
		{
			//
		}
	}
	// ...

}

// Called every frame
void UFJMHumanControlComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UFJMHumanControlComponent::BindInputFunctions(UInputComponent* PlayerInputComponent)
{
	if (ensure(PlayerInputComponent))
	{
		//Axis bindings
		PlayerInputComponent->BindAxis("MoveForward", this, &UFJMHumanControlComponent::MoveForward);
		PlayerInputComponent->BindAxis("MoveRight", this, &UFJMHumanControlComponent::MoveRight);

		PlayerInputComponent->BindAxis("LookUp", this, &UFJMHumanControlComponent::HandleLookUp);
		PlayerInputComponent->BindAxis("Turn", this, &UFJMHumanControlComponent::HandleTurn);

		//Pressed/Released Mappings
		PlayerInputComponent->BindAction("ToggleStand/Crouch/Prone", IE_Pressed, this, &UFJMHumanControlComponent::CrouchPressed);
		PlayerInputComponent->BindAction("ToggleStand/Crouch/Prone", IE_Released, this, &UFJMHumanControlComponent::CrouchReleased);

		PlayerInputComponent->BindAction("ThrowGrenade", IE_Pressed, this, &UFJMHumanControlComponent::GrenadePressed);
		PlayerInputComponent->BindAction("ThrowGrenade", IE_Released, this, &UFJMHumanControlComponent::GrenadeReleased);

		PlayerInputComponent->BindAction("Fire Weapon", IE_Pressed, this, &UFJMHumanControlComponent::FirePressed);
		PlayerInputComponent->BindAction("Fire Weapon", IE_Released, this, &UFJMHumanControlComponent::FireReleased);

		PlayerInputComponent->BindAction("ToggleAim", IE_Pressed, this, &UFJMHumanControlComponent::AimPressed);
		PlayerInputComponent->BindAction("ToggleAim", IE_Released, this, &UFJMHumanControlComponent::AimReleased);

		PlayerInputComponent->BindAction("ToggleBino", IE_Pressed, this, &UFJMHumanControlComponent::BinoPressed);

		//Single press inputs mappings
		PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &UFJMHumanControlComponent::HandleSprint);
		PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &UFJMHumanControlComponent::HandleJump);
		PlayerInputComponent->BindAction("Toggle Walk/Run", IE_Pressed, this, &UFJMHumanControlComponent::HandleWalkJog);
		PlayerInputComponent->BindAction("ToggleCover", IE_Pressed, this, &UFJMHumanControlComponent::HandleToggleCover);
		PlayerInputComponent->BindAction("Toggle Ironsight/GiveOrder", IE_Pressed, this, &UFJMHumanControlComponent::ToggleIronsight);

		PlayerInputComponent->BindAction("HolsterWeapon", IE_Pressed, this, &UFJMHumanControlComponent::ToggleWeapon);

		PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &UFJMHumanControlComponent::ReloadWeapon);

		PlayerInputComponent->BindAction("MountWeapon/Vehicle", IE_Pressed, this, &UFJMHumanControlComponent::InteractWithActor);

		PlayerInputComponent->BindAction("FirstAid", IE_Pressed, this, &UFJMHumanControlComponent::UseFirstAid);
	}

}

void UFJMHumanControlComponent::ChangeInputState()
{
	if (ensure(HumanChar))
	{
		UFJMVehicleUseComponent* VehicleComp = Cast<UFJMVehicleUseComponent>(HumanChar->GetComponentByClass(UFJMVehicleUseComponent::StaticClass()));

		HumanChar->SetIsSprinting(false);

		if (HumanChar->GetIsTurretAvailable())
		{
			//tell character to take cover and stop sprinting
			HumanChar->UseTurret();

			AFJMPlayerController* PlayerController = Cast<AFJMPlayerController>(Controller);
			if (PlayerController)
			{
				PlayerController->EnableTurretInputs(true);
			}
		}
		else if (VehicleComp)
		{
			if (VehicleComp->GetIsVehicleAvailable())
			{
				VehicleComp->UseVehicle();

				AFJMPlayerController* PlayerController = Cast<AFJMPlayerController>(Controller);
				if (PlayerController)
				{
					PlayerController->SetControlRotation(FRotator(0.f, 0.f, 0.f));
					PlayerController->EnableVehicleInputs(true);
					PlayerController->SetViewTargetWithBlend(VehicleComp->GetUsedVehicleInfo().Vehicle, 0.2f);
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
//Use/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

void UFJMHumanControlComponent::UseFirstAid()
{
	if (ensure(HumanChar))
	{
		UFJMCharacterHealthComponent* HealthComp = Cast<UFJMCharacterHealthComponent>(HumanChar->GetComponentByClass(UFJMCharacterHealthComponent::StaticClass()));
		if (HealthComp)
		{
			HealthComp->UseFirstAidKit();
		}
	}
}

void UFJMHumanControlComponent::BinoPressed()
{
	if (ensure(HumanChar))
	{
		UFJMCharacterWeaponComponent* WeaponComp = Cast<UFJMCharacterWeaponComponent>(HumanChar->GetComponentByClass(UFJMCharacterWeaponComponent::StaticClass()));
		if (WeaponComp)
		{
			WeaponComp->ToggleBinoculars();
		}
	}
}

void UFJMHumanControlComponent::InteractWithActor()
{
	if (ensure(HumanChar))
	{
		AFJMBasePickup* PossiblePickup = nullptr;

		bool PickupAvailable = false;
		UFJMPlayerPickupComponent* PickupComp = Cast<UFJMPlayerPickupComponent>(HumanChar->GetComponentByClass(UFJMPlayerPickupComponent::StaticClass()));
		if (PickupComp)
		{
			PickupAvailable = PickupComp->GetPickupAvailable(PossiblePickup);
		}

		bool DialogueAvailable = false;
		UFJMDialogueComponent * DialogueComp = Cast<UFJMDialogueComponent>(HumanChar->GetComponentByClass(UFJMDialogueComponent::StaticClass()));
		if (DialogueComp)
		{
			DialogueAvailable = DialogueComp->GetDialogueAvailable();
		}

		bool VehicleAvailable = false;
		UFJMVehicleUseComponent* VehicleComp = Cast<UFJMVehicleUseComponent>(HumanChar->GetComponentByClass(UFJMVehicleUseComponent::StaticClass()));
		if (VehicleComp)
		{
			VehicleAvailable = VehicleComp->GetIsVehicleAvailable();
		}

		bool EquipmentAvailable = false;
		AFJMBasePlayerCharacter* PlayerChar = Cast<AFJMBasePlayerCharacter>(HumanChar);
		if (PlayerChar)
		{
			EquipmentAvailable = PlayerChar->GetIsEquipmentSelectorAvailable();
		}

		if (VehicleAvailable || HumanChar->GetIsTurretAvailable())
		{
			ChangeInputState();
		}
		else if (DialogueAvailable)
		{
			//begin dialogue
			DialogueComp->BeginDialogue(DialogueComp->GetDialogueCharacter());
		}
		else if (PickupAvailable)
		{
			PickupComp->UsePickup(PossiblePickup);
		}
		else if (HumanChar->GetCanBreachDoorway())
		{
			UFJMBuildingSearchComponent* SearchComp = Cast<UFJMBuildingSearchComponent>(HumanChar->GetComponentByClass(UFJMBuildingSearchComponent::StaticClass()));
			if (SearchComp)
			{
				SearchComp->BreachEntrance(HumanChar->GetBreachDoorway(), true);
			}
		}
		else if (EquipmentAvailable)
		{
			AFJMEquipmentSelector* NearEquipment = PlayerChar->GetNearbyEquipment();
			if (NearEquipment)
			{
				PlayerChar->OnToggleEquipmentInterface.Broadcast(true);

				AFJMPlayerController* PlayerController = Cast<AFJMPlayerController>(Controller);
				if (PlayerController)
				{
					NearEquipment->HideBillboard(true);
					PlayerController->SetViewTargetWithBlend(NearEquipment, 0.2f);

					FRotator PlayerRot = FRotator(0.f, (NearEquipment->GetActorRotation().Yaw + 180), 0.f);

					PlayerController->SetControlRotation(PlayerRot);
					PlayerChar->SetActorLocationAndRotation(NearEquipment->GetPlayerPosition()->GetComponentLocation(), PlayerRot);
				}
			}
		}
		else if (AFJMBasePlayerCharacter* Player = Cast<AFJMBasePlayerCharacter>(HumanChar))
		{
			if (Player->GetCanPlantDemo()) { Player->PlantDemoOnActor(); }
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
//Fire & Aim//////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

void UFJMHumanControlComponent::FirePressed()
{
	if (ensure(HumanChar))
	{
		UFJMCharacterWeaponComponent* WeaponComp = Cast<UFJMCharacterWeaponComponent>(HumanChar->GetComponentByClass(UFJMCharacterWeaponComponent::StaticClass()));
		if (WeaponComp)
		{
			WeaponComp->BeginFiring();
		}
	}
}

void UFJMHumanControlComponent::FireReleased()
{
	if (ensure(HumanChar))
	{
		UFJMCharacterWeaponComponent* WeaponComp = Cast<UFJMCharacterWeaponComponent>(HumanChar->GetComponentByClass(UFJMCharacterWeaponComponent::StaticClass()));
		if (WeaponComp)
		{
			WeaponComp->CeaseFire();
		}
	}
}

void UFJMHumanControlComponent::AimPressed()
{
	if (ensure(HumanChar))
	{
		bool Supressed = false;
		UFJMCharacterHealthComponent* HealthComp = Cast<UFJMCharacterHealthComponent>(HumanChar->GetComponentByClass(UFJMCharacterHealthComponent::StaticClass()));
		if (HealthComp)
		{
			Supressed = HealthComp->GetIsSupressed();
		}
		UFJMCharacterWeaponComponent* WeaponComp = Cast<UFJMCharacterWeaponComponent>(HumanChar->GetComponentByClass(UFJMCharacterWeaponComponent::StaticClass()));
		if (WeaponComp && !Supressed)
		{
			WeaponComp->BeginAim();
		}
	}
}

void UFJMHumanControlComponent::AimReleased()
{
	if (ensure(HumanChar))
	{
		UFJMCharacterWeaponComponent* WeaponComp = Cast<UFJMCharacterWeaponComponent>(HumanChar->GetComponentByClass(UFJMCharacterWeaponComponent::StaticClass()));
		if (WeaponComp)
		{
			WeaponComp->EndAim();
		}
	}
}

void UFJMHumanControlComponent::ToggleIronsight()
{
	if (ensure(HumanChar))
	{
		AFJMBasePlayerCharacter* PlayerChar = Cast<AFJMBasePlayerCharacter>(HumanChar);
		if (PlayerChar)
		{
			UFJMOrdersComponent* OrdersComponent = Cast<UFJMOrdersComponent>(GetOwner()->GetComponentByClass(UFJMOrdersComponent::StaticClass()));
			UFJMCharacterWeaponComponent* WeaponComp = Cast<UFJMCharacterWeaponComponent>(HumanChar->GetComponentByClass(UFJMCharacterWeaponComponent::StaticClass()));

			if (OrdersComponent)
			{
				if (OrdersComponent->bOrdersWheelIsInUse)
				{
					//TODO - set up give order
					//if order wheel in use, give order
					OrdersComponent->SelectOrder();
					OrdersComponent->CloseOrderWheels();
				}
				else if (OrdersComponent->bTeamSelectionWheelIsInUse)
				{
					//if team wheel in use, select team.
					OrdersComponent->SelectTeam();
					OrdersComponent->CloseOrderWheels();
				}
				else if (WeaponComp)
				{
					if (WeaponComp->GetIsAiming())
					{
						//toggle ironsight
						PlayerChar->ToggleIronsight();
					}
				}
			}
		}
	}
}

void UFJMHumanControlComponent::ToggleWeapon()
{
	if (ensure(HumanChar))
	{
		UFJMCharacterWeaponComponent* WeaponComp = Cast<UFJMCharacterWeaponComponent>(HumanChar->GetComponentByClass(UFJMCharacterWeaponComponent::StaticClass()));
		if (WeaponComp)
		{
			WeaponComp->SwitchBetweenWeapons();
		}
	}
}

void UFJMHumanControlComponent::ReloadWeapon()
{
	if (ensure(HumanChar))
	{
		UFJMCharacterWeaponComponent* WeaponComp = Cast<UFJMCharacterWeaponComponent>(HumanChar->GetComponentByClass(UFJMCharacterWeaponComponent::StaticClass()));
		if (WeaponComp && WeaponComp->GetEquippedWeapon())
		{
			IFJMRangedWeaponInterface* WeaponIF = Cast<IFJMRangedWeaponInterface>(WeaponComp->GetEquippedWeapon());
			if (WeaponIF)
			{
				if (WeaponComp->GetMagazinesRemainingForEquippedWeapon() > 0)
				{
					//Stop firing and reload.
					WeaponIF->Execute_WeaponCeaseFire(WeaponComp->GetEquippedWeapon(), HumanChar);
					WeaponIF->Execute_WeaponReload(WeaponComp->GetEquippedWeapon(), HumanChar);

					if (HumanChar && HumanChar->GetIsPlayer())
					{
						AFJMBasePlayerCharacter* Player = Cast<AFJMBasePlayerCharacter>(HumanChar);

						Player->SwitchToThirdPerson();
						Player->SetFieldOfView(false);
					}
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
//Movement////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

void UFJMHumanControlComponent::MoveForward(float Value)
{
	ForwardInput = Value;
	HandleMovementInput(ForwardInput, RightInput);
}

void UFJMHumanControlComponent::MoveRight(float Value)
{
	bool IsInCover = false;
	UFJMCoverUseComponent* CoverComp = Cast<UFJMCoverUseComponent>(HumanChar->GetComponentByClass(UFJMCoverUseComponent::StaticClass()));
	if (CoverComp)
	{
		IsInCover = CoverComp->GetIsTakingCover();
	}

	bool IsCoveringDoor = HumanChar->GetCanBreachDoorway() == true;

	if (IsCoveringDoor && IsInCover)
	{
		//dont move covering door.
	}
	else
	{
		RightInput = Value;
		HandleMovementInput(ForwardInput, RightInput);
	}
}

void UFJMHumanControlComponent::HandleMovementInput(float ForwardAxisValue, float RightAxisValue)
{
	if (HumanChar)
	{
		UFJMOrdersComponent* OrdersComponent = Cast<UFJMOrdersComponent>(GetOwner()->GetComponentByClass(UFJMOrdersComponent::StaticClass()));
		if (OrdersComponent)
		{
			/*Check if the order or team select wheel is open*/
			if (OrdersComponent->bTeamSelectionWheelIsInUse || OrdersComponent->bOrdersWheelIsInUse)
			{
				//Make sure that the character stops moving.
				HumanChar->SetMoveForwardAxis(0.f);
				HumanChar->SetMoveRightAxis(0.f);

				AFJMPlayerController* PlayerController = Cast<AFJMPlayerController>(Controller);
				if (PlayerController)
				{
					//Check for a gamepad as we only use movement input with the order wheel on a gamepad.
					if (PlayerController->bIsUsingGamepad)
					{
						//TODO Set Menu Axis
						float AtanAxis = UKismetMathLibrary::Atan2(ForwardAxisValue, RightAxisValue);// +360;
						float AtanDegrees = UKismetMathLibrary::RadiansToDegrees(AtanAxis) + 180;
						float FinalDegrees = UKismetMathLibrary::Round(AtanDegrees + 180) % 360;//(AtanDegrees + 180) % 360;
						float LocalAxis = FinalDegrees / 360;
						OrdersComponent->SetMenuAxis(LocalAxis);
					}
				}
			}
			else
			{
				//if order wheel isnt open 
				//check for our player character
				if (ensure(HumanChar))
				{
					UFJMRootMotionMovementComponent* RMMovementComp = Cast<UFJMRootMotionMovementComponent>(HumanChar->GetCharacterMovement());
					if (ensure(RMMovementComp))
					{
						RMMovementComp->MoveCharacter(ForwardAxisValue, RightAxisValue);
					}
				}
			}
		}
	}
}

void UFJMHumanControlComponent::HandleLookUp(float LookUpAxis)
{
	if (CurrentGameInstance)
	{
		float OTSMulti = CurrentGameInstance->GetOTSSensitivity();
		float AimMulti = CurrentGameInstance->GetAimSensitivity();

		AFJMPlayerController* PlayerController = Cast<AFJMPlayerController>(Controller);
		if (PlayerController)
		{
			//check whether the axis should be inverted
			float Axis = PlayerController->bInvertAim ? LookUpAxis : (LookUpAxis * -1);

			//ease axis for smooth camera movement.
			float EasedInput = UKismetMathLibrary::Ease(0.f, Axis, CameraEaseAlpha, EEasingFunc::Linear);

			if (ensure(HumanChar))
			{
				UFJMCharacterWeaponComponent* WeaponComp = Cast<UFJMCharacterWeaponComponent>(HumanChar->GetComponentByClass(UFJMCharacterWeaponComponent::StaticClass()));
				if (WeaponComp)
				{
					if (WeaponComp->GetIsAiming())
					{
						if (PlayerController->bIsUsingGamepad)
						{
							float GamepadInput = EasedInput * GamepadSensitivityMultiplier;
							PlayerController->AddPitchInput(GamepadInput * AimMulti);
						}
						else
						{
							float MouseInput = EasedInput * MouseSensitivityMultiplier;
							PlayerController->AddPitchInput(MouseInput * AimMulti);
						}
					}
					else
					{
						if (PlayerController->bIsUsingGamepad)
						{
							float GamepadInput = EasedInput * GamepadSensitivityMultiplier;
							PlayerController->AddPitchInput(GamepadInput * OTSMulti);
						}
						else
						{
							float MouseInput = EasedInput * MouseSensitivityMultiplier;
							PlayerController->AddPitchInput(MouseInput * OTSMulti);
						}
					}
				}
			}
		}
	}
}

void UFJMHumanControlComponent::HandleTurn(float TurnAxis)
{
	if (CurrentGameInstance)
	{
		float OTSMulti = CurrentGameInstance->GetOTSSensitivity();
		float AimMulti = CurrentGameInstance->GetAimSensitivity();

		//ease input for smooth camera movement
		float EasedInput = UKismetMathLibrary::Ease(0.f, TurnAxis, CameraEaseAlpha, EEasingFunc::Linear);

		if (ensure(HumanChar))
		{
			AFJMPlayerController* PlayerController = Cast<AFJMPlayerController>(Controller);
			if (PlayerController)
			{
				UFJMCharacterWeaponComponent* WeaponComp = Cast<UFJMCharacterWeaponComponent>(HumanChar->GetComponentByClass(UFJMCharacterWeaponComponent::StaticClass()));
				if (WeaponComp)
				{
					if (WeaponComp->GetIsAiming())
					{
						if (PlayerController->bIsUsingGamepad)
						{
							float GamepadInput = EasedInput * GamepadSensitivityMultiplier;
							PlayerController->AddYawInput(GamepadInput * AimMulti);
						}
						else
						{
							PlayerController->AddYawInput(EasedInput * AimMulti);
						}
					}
					else
					{
						if (PlayerController->bIsUsingGamepad)
						{
							float GamepadInput = EasedInput * GamepadSensitivityMultiplier;
							PlayerController->AddYawInput(GamepadInput * OTSMulti);
						}
						else
						{
							float MouseInput = EasedInput * MouseSensitivityMultiplier;
							PlayerController->AddYawInput(MouseInput * OTSMulti);
						}
					}
				}
				else
				{
					if (PlayerController->bIsUsingGamepad)
					{
						float GamepadInput = EasedInput * GamepadSensitivityMultiplier;
						PlayerController->AddYawInput(GamepadInput * OTSMulti);
					}
					else
					{
						float MouseInput = EasedInput * MouseSensitivityMultiplier;
						PlayerController->AddYawInput(MouseInput * OTSMulti);
					}
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
//Grenade/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

void UFJMHumanControlComponent::GrenadePressed()
{
	if (ensure(HumanChar))
	{
		UFJMCharacterWeaponComponent* WeaponComp = Cast<UFJMCharacterWeaponComponent>(HumanChar->GetComponentByClass(UFJMCharacterWeaponComponent::StaticClass()));
		if (WeaponComp)
		{
			WeaponComp->PrimeGrenade();
		}
	}
}

void UFJMHumanControlComponent::GrenadeReleased()
{
	if (ensure(HumanChar))
	{
		UFJMCharacterWeaponComponent* WeaponComp = Cast<UFJMCharacterWeaponComponent>(HumanChar->GetComponentByClass(UFJMCharacterWeaponComponent::StaticClass()));
		if (WeaponComp)
		{
			WeaponComp->ThrowGrenade();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
//Stances/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

void UFJMHumanControlComponent::HandleToggleCover()
{
	if (ensure(HumanChar))
	{
		HumanChar->SetIsSprinting(false);

		UFJMCoverUseComponent* CoverComp = Cast<UFJMCoverUseComponent>(HumanChar->GetComponentByClass(UFJMCoverUseComponent::StaticClass()));
		if (CoverComp)
		{
			if (!CoverComp->GetIsTakingCover())
			{
				if (CoverComp->GetIsCoverAvailable() && CoverComp->GetOverlappedCoverBox() != nullptr)
				{
					float Distance = 100.f;
					FCoverPositionData* SelectedCover = nullptr;
					TArray<FCoverPositionData*> CoverPointers;

					for (int32 CoverIndex = 0; CoverIndex < CoverComp->GetOverlappedCoverBox()->ThisCoverPositions.Num(); CoverIndex++)
					{
						FVector CoverLocation = CoverComp->GetOverlappedCoverBox()->ThisCoverPositions[CoverIndex].WorldPosition;
						bool InUse = CoverComp->GetOverlappedCoverBox()->ThisCoverPositions[CoverIndex].bPositionBeingUsed;
						float thisDistance = (CoverLocation - HumanChar->GetActorLocation()).Size();

						if (thisDistance < Distance && !InUse) { SelectedCover = &CoverComp->GetOverlappedCoverBox()->ThisCoverPositions[CoverIndex]; Distance = thisDistance; }
					}

					if (SelectedCover)
					{
						//tell character to take cover and stop sprinting
						CoverComp->TakeCover(SelectedCover);
					}
				}
			}
			else
			{
				CoverComp->ExitCover();
			}
		}

	}
}

void UFJMHumanControlComponent::HandleJump()
{
	if (ensure(HumanChar))
	{
		UFJMRootMotionMovementComponent* RMMovementComp = Cast<UFJMRootMotionMovementComponent>(HumanChar->GetCharacterMovement());
		if (ensure(RMMovementComp))
		{
			RMMovementComp->Jump();
		}
	}
}

void UFJMHumanControlComponent::HandleSprint()
{
	//check for player character
	if (ensure(HumanChar))
	{
		bool TakingCover = false;
		UFJMCoverUseComponent* CoverComp = Cast<UFJMCoverUseComponent>(HumanChar->GetComponentByClass(UFJMCoverUseComponent::StaticClass()));
		if (CoverComp)
		{
			TakingCover = CoverComp->GetIsTakingCover();
		}

		//if player is not in cover and is moving forward
		bool bCanSprint = !TakingCover && HumanChar->GetMoveForwardAxis() > 0.f;
		if (bCanSprint)
		{
			UFJMCharacterWeaponComponent* WeaponComp = Cast<UFJMCharacterWeaponComponent>(HumanChar->GetComponentByClass(UFJMCharacterWeaponComponent::StaticClass()));
			if (WeaponComp)
			{
				WeaponComp->SetCanFire(!HumanChar->GetIsSprinting());

				//check the sprint is working
				if (HumanChar->GetIsSprinting())
				{
					if (AFJMBasePlayerCharacter* PlayerChar = Cast<AFJMBasePlayerCharacter>(HumanChar))
					{
						//set field of view back to third person zoomed out.
						PlayerChar->SetFieldOfView(WeaponComp->GetIsAiming());
					}
				}
			}
			//set character to sprinting and cant fire
			HumanChar->SetIsSprinting(!HumanChar->GetIsSprinting());

		}
	}
}

void UFJMHumanControlComponent::HandleWalkJog()
{
	if (ensure(HumanChar))
	{
		if (HumanChar->GetIsWalking())
		{
			//tell character to jog.
			HumanChar->SetIsWalking(false);
		}
		else
		{
			//set character to walk
			HumanChar->SetIsWalking(true);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
//Crouch/Prone////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

void UFJMHumanControlComponent::CrouchPressed()
{
	HandleCrouch(true);
}

void UFJMHumanControlComponent::CrouchReleased()
{
	HandleCrouch(false);
}

void UFJMHumanControlComponent::HandleCrouch(bool Pressed)
{
	if (Pressed)
	{
		//when pressed begin 1 second timer, to begin the prone function
		GetWorld()->GetTimerManager().SetTimer(CrouchTimer, this, &UFJMHumanControlComponent::Prone, CrouchTimerLength, false);
	}
	else
	{
		//when released, clear the timer set above
		GetWorld()->GetTimerManager().ClearTimer(CrouchTimer);

		//if the release occured in less than 1 second, the timer will not have completed and this check will return false.
		if (bCrouchTimerCompleted)
		{
			//if the timer has completed and the character gone into prone. on release just reset the boolean.
			bCrouchTimerCompleted = false;
		}
		else
		{
			//if timer has not completed then tell the character to crouch.
			Crouch();
		}
	}
}

void UFJMHumanControlComponent::Crouch()
{
	if (ensure(HumanChar))
	{
		//if player is crouching
		if (HumanChar->GetIsCrouching())
		{
			//stop crouching
			HumanChar->SetIsCrouching(false);
		}
		else
		{
			//otherwise begin crouching
			HumanChar->SetIsCrouching(true);
		}

		HumanChar->SetIsProne(false);
		bCrouchTimerCompleted = false;
	}
}

void UFJMHumanControlComponent::Prone()
{
	if (ensure(HumanChar))
	{
		//tell the class the timer is completed and should not now crouch
		bCrouchTimerCompleted = true;

		if (HumanChar->GetIsProne())
		{
			//if is prone then stand
			HumanChar->SetIsProne(false);
		}
		else
		{
			UFJMCharacterWeaponComponent* WeaponComp = Cast<UFJMCharacterWeaponComponent>(HumanChar->GetComponentByClass(UFJMCharacterWeaponComponent::StaticClass()));
			if (WeaponComp)
			{
				if (WeaponComp->GetUsingBinoculars())
				{
					WeaponComp->ToggleBinoculars();
				}
			}

			//if not prone then prone.
			HumanChar->SetIsProne(true);
		}

		HumanChar->SetIsCrouching(false);
	}
}
