#pragma once

#include "CoreMinimal.h"
#include "DroneState.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/PlayerController.h"
#include "Drone.generated.h"
UCLASS()
class HW02_API ADrone : public APawn
{
	GENERATED_BODY()

public:
	ADrone();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Inputs | Input Action")
	TObjectPtr<UInputMappingContext> DroneInputMappingContext;
	
	// 앞 뒤 좌 우
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Inputs | Input Action")
	TObjectPtr<UInputAction> MoveInputAction;
	
	// 상 하
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Inputs | Input Action")
	TObjectPtr<UInputAction> AltitudeInputAction;
	
	// 좌우 기울기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Inputs | Input Action")
	TObjectPtr<UInputAction> RollInputAction;
	
	// 마우스(pitch, yaw)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Inputs | Input Action")
	TObjectPtr<UInputAction> PitchYawInputAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Inputs | Input Action")
	TObjectPtr<UInputAction> AutoHoveringInputAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Rotation | Speed")
	float RollRotationSpeed = 500.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Rotation | Speed")
	float PitchRotationSpeed = 250.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Rotation | Speed")
	float YawRotationSpeed = 250.f;
	
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	EDroneState State;
	
	float InterpSpeed_XY = 2.f;
	float InterpSpeed_Z = 2.f;
	
	float AltitudeInput = 0.f;
	
	// MaxSpeed
	float CurrentMaxSpeed = 20000.0f;
	float GroundMaxSpeed = 20000.f;
	float AirMaxSpeed = 10000.f;
	
	// Acc
	float CurrentAcceleration = 10000.f;
	float GroundAcceleration = 10000.f;
	float AirAcceleration = 5000.f;
	
	// Physics
	float FrictionValue = 0.98f;
	float GravityScale = -1500.f; //-980.0f;
	
	FVector CurrentVelocity = FVector::ZeroVector;
	
	bool bUseAutoHovering;

private:
	void ToggleAutoHovering();
	void PhysicsMove(const float& DeltaTime);
	void CheckGroundedByRay(const float &DeltaTime);
	void InitEnhancedInput();
	void BindInputAction(UInputComponent * PlayerInputComponent);
	void MoveControl(const FInputActionValue & Value);
	void AltitudeControl(const FInputActionValue & Value);
	void RollControl(const FInputActionValue & Value);
	void PitchYawControl(const FInputActionValue & Value);
};
