// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <CoreMinimal.h>
#include <Components/ActorComponent.h>
#include "FJMHumanControlComponent.generated.h"

class AFJMBaseHumanCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WORLDWARTWO_API UFJMHumanControlComponent : public UActorComponent
{
	GENERATED_BODY()


public:
	// Sets default values for this component's properties
	UFJMHumanControlComponent();

	//set the human player character that we are controlling
	void SetHumanCharacter(AFJMBaseHumanCharacter* NewHuman);

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//Binds all the inputs from the input component to the appropriate methods to handle those inputs within this class.
	void BindInputFunctions(class UInputComponent* PlayerInputComponent);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	class AFJMBaseHumanCharacter* HumanChar;
	AController* Controller;

	/*The mouse and gamepad Axis can give different sensitivties to the player.
	* these multipliers can be edited by the player in the options menu so they can use their preference.*/
	float GamepadSensitivityMultiplier = 5.0;
	float MouseSensitivityMultiplier = 1.5;

private:
	float ForwardInput;
	float RightInput;

	//the Camera sensitivity delta.
	float CameraEaseAlpha = .25f;

	class UFJMGameInstance* CurrentGameInstance = nullptr;
///////////////////////////////////////////////////////////////////////////////////////////
//Use/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
public:
	UFUNCTION()
	void UseFirstAid();

	UFUNCTION()
	void BinoPressed();

	UFUNCTION()
	void InteractWithActor();

	UFUNCTION()
	void ChangeInputState();

///////////////////////////////////////////////////////////////////////////////////////////
//Fire & Aim//////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
public:
	UFUNCTION()
	void FirePressed();

	UFUNCTION()
	void FireReleased();

	UFUNCTION()
	void AimPressed();

	UFUNCTION()
	void AimReleased();

	UFUNCTION()
	void ToggleIronsight();

	UFUNCTION()
	void ToggleWeapon();

	UFUNCTION()
	void ReloadWeapon();

///////////////////////////////////////////////////////////////////////////////////////////
//Movement////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
public:
	UFUNCTION()
	void MoveForward(float Value);

	UFUNCTION()
	void MoveRight(float Value);

	/*Used to handle both the forward movement and sideways movement axis, enables the player character to move even while in cover
	*If the player is using a gamepad (Checked in parent class) this will also handle the rotation of the order wheel.
	*This function can be run either from within the blueprint child of this class or with input delgates on C++
	*ForwardAxisValue: the forward axis value provided by the human input
	*RightAxisValue: the sideways axis value provided by the human input*/
	UFUNCTION()
	void HandleMovementInput(float ForwardAxisValue, float RightAxisValue);

	/*Handles input along the look up/down axis, the function checks if weapon is mounted as this will restrict movement and applies changes to the viewport.
	*This function can be run either from within the blueprint child of this class or with input delgates on C++*/
	UFUNCTION()
	void HandleLookUp(float LookUpAxis);

	/*Handles input along the turn left/right axis, the function checks if weapon is mounted as this will restrict movement and applies changes to the viewport.
	*This function can be run either from within the blueprint child of this class or with input delgates on C++*/
	UFUNCTION()
	void HandleTurn(float TurnAxis);

///////////////////////////////////////////////////////////////////////////////////////////
//Grenade/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
public:
	UFUNCTION()
	void GrenadePressed();

	UFUNCTION()
	void GrenadeReleased();

///////////////////////////////////////////////////////////////////////////////////////////
//Stances/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
public:
	/*Tells the player character to take cover
	*This function can be run either from within the blueprint child of this class or with input delgates on C++*/
	UFUNCTION()
	void HandleToggleCover();

	/*Runs a series of line traces to determine the hegiht of any obstacle in front of the player character, it then tells the character to perform the appropriate jump/climb.
	**This function can be run either from within the blueprint child of this class or with input delgates on C++*/
	UFUNCTION()
	void HandleJump();

	/*Handles the sprint toggle input
	*This function can be run either from within the blueprint child of this class or with input delgates on C++*/
	UFUNCTION()
	void HandleSprint();

	/*Handles the walk/jog toggle input
	*This function can be run either from within the blueprint child of this class or with input delgates on C++*/
	UFUNCTION()
	void HandleWalkJog();

///////////////////////////////////////////////////////////////////////////////////////////
//Crouch/Prone////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
public:
	UFUNCTION()
	void CrouchPressed();

	UFUNCTION()
	void CrouchReleased();

	/*Determines whether the character should crouch, stand or prone based on the length that the input is held down and the current state of the character
	**This function can be run either from within the blueprint child of this class or with input delgates on C++
	*Pressed: whether the input has been pressed, true = has been pressed, false = has been released*/
	void HandleCrouch(bool Pressed);

	/*Tells the character to crouch and moves the the camera to an appropriate height.*/
	void Crouch();

	/*Tells the character to prone and moves the the camera to an appropriate height.*/
	void Prone();

private:
	float CrouchTimerLength = 1.f;

	/*Timer Handle needed to check if the crouch button is tapped or held.*/
	struct FTimerHandle CrouchTimer;

	/*Used to determine if the crouch button is tapped or held.*/
	bool bCrouchTimerCompleted = false;

};
