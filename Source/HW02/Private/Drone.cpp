#include "Drone.h"

ADrone::ADrone()
{
	// 오토 호버링 off
	bUseAutoHovering = false;
	State = EDroneState::Air;
	CurrentMaxSpeed = AirMaxSpeed;

	PrimaryActorTick.bCanEverTick = true;
}

void ADrone::BeginPlay()
{
	Super::BeginPlay();
	InitEnhancedInput();
}

void ADrone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CheckGroundedByRay(DeltaTime);
	PhysicsMove(DeltaTime);
}

void ADrone::PhysicsMove(const float& DeltaTime)
{
	if (State == EDroneState::Ground && CurrentVelocity.Z <= 0.f)
	{
		CurrentVelocity.Z = 0;
	}
	else
	{
		// 드론과 친구에게 물어보니 요즘 드론(dji기종)들은 다 오토 호버링이 된다고합니당
		// 하지만 과제에서 중력가속도를 적용하라고 하였으니! 토글로 껏다 킬 수 있게끔 했습니다
		if (bUseAutoHovering && FMath::IsNearlyZero(AltitudeInput))
		{
			CurrentVelocity.Z = 0.f;
		}
		else
		{
			CurrentVelocity.Z += GravityScale * DeltaTime;
		}
	}

	CurrentVelocity.X = FMath::FInterpTo(CurrentVelocity.X, 0.f, DeltaTime, InterpSpeed_XY);
	CurrentVelocity.Y = FMath::FInterpTo(CurrentVelocity.Y, 0.f, DeltaTime, InterpSpeed_XY);
	
	if (CurrentVelocity.Z > 0)
	{
		CurrentVelocity.Z = FMath::FInterpTo(CurrentVelocity.Z, 0.f, DeltaTime, InterpSpeed_Z);
	}
	
	CurrentVelocity = CurrentVelocity.GetClampedToMaxSize(CurrentMaxSpeed);

	AddActorWorldOffset(CurrentVelocity * DeltaTime, true);
	
	AltitudeInput = 0.f;
}

void ADrone::CheckGroundedByRay(const float& DeltaTime)
{
	FHitResult HitResult;
	FVector RayStart = GetActorLocation();

	float RayLength = (State == EDroneState::Ground) ? 100.f : 50.f;
	FVector RayEnd = RayStart + (FVector::DownVector * RayLength);

	bool bOnHit = GetWorld()->LineTraceSingleByChannel(HitResult, RayStart, RayEnd, ECC_Visibility);
	
	if (bOnHit && State == EDroneState::Air)
	{
		State = EDroneState::Ground;
		CurrentMaxSpeed = GroundMaxSpeed;
		CurrentAcceleration = GroundAcceleration;

		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Magenta,TEXT("On Ground, Speed = 100%"));
	}
	else if (!bOnHit && State == EDroneState::Ground)
	{
		State = EDroneState::Air;
		CurrentMaxSpeed = AirMaxSpeed;
		CurrentAcceleration = AirAcceleration;
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Magenta,TEXT("On Air, Speed = 50%"));
	}
}

void ADrone::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	BindInputAction(PlayerInputComponent);
}

void ADrone::InitEnhancedInput()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	
	if (PlayerController == nullptr) return;

	auto Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if (!IsValid(Subsystem)) return;

	Subsystem->AddMappingContext(DroneInputMappingContext, 0);
}

void ADrone::BindInputAction(UInputComponent* PlayerInputComponent)
{
	auto EnhancedComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	EnhancedComponent->BindAction(MoveInputAction, ETriggerEvent::Triggered, this, &ADrone::MoveControl);
	EnhancedComponent->BindAction(AltitudeInputAction, ETriggerEvent::Triggered, this, &ADrone::AltitudeControl);
	EnhancedComponent->BindAction(RollInputAction, ETriggerEvent::Triggered, this, &ADrone::RollControl);
	EnhancedComponent->BindAction(PitchYawInputAction, ETriggerEvent::Triggered, this, &ADrone::PitchYawControl);
	EnhancedComponent->BindAction(AutoHoveringInputAction, ETriggerEvent::Started, this, &ADrone::ToggleAutoHovering);
}

void ADrone::MoveControl(const FInputActionValue& Value)
{
	FVector2D Input = Value.Get<FVector2D>();

	FVector Forward = GetActorForwardVector();
	FVector Right = GetActorRightVector();

	FVector TargetDir = (Forward * Input.Y) + (Right * Input.X);

	float DeltaTime = GetWorld()->GetDeltaSeconds();
	CurrentVelocity += TargetDir * CurrentAcceleration * DeltaTime;
}

void ADrone::AltitudeControl(const FInputActionValue& Value)
{
	FVector2D Input = Value.Get<FVector2D>();
	AltitudeInput = Input.X;

	float DeltaTime = GetWorld()->GetDeltaSeconds();
	CurrentVelocity += FVector::UpVector * Input.X * CurrentAcceleration * DeltaTime;
}

void ADrone::RollControl(const FInputActionValue& Value)
{
	if (State == EDroneState::Ground) return;
	
	FVector2D Input = Value.Get<FVector2D>();

	FRotator CurrentRotation = GetActorRotation();

	float DeltaTime = GetWorld()->GetDeltaSeconds();
	float AddRoll = Input.X * RollRotationSpeed * DeltaTime;

	CurrentRotation.SetComponentForAxis(EAxis::Type::X, AddRoll);
	AddActorLocalRotation(FRotator(0.f, 0.f, AddRoll));
}

void ADrone::ToggleAutoHovering()
{
	bUseAutoHovering = !bUseAutoHovering;

	FString Msg = bUseAutoHovering ? TEXT("Auto Hovering: ON") : TEXT("Auto Hovering: OFF");
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, Msg);
}

void ADrone::PitchYawControl(const FInputActionValue& Value)
{
	if (State == EDroneState::Ground) return;

	FVector2D Input = Value.Get<FVector2D>();
	float DeltaTime = GetWorld()->GetDeltaSeconds();

	float AddPitch = Input.Y * PitchRotationSpeed * DeltaTime;
	AddActorLocalRotation(FRotator(AddPitch, 0.f, 0.f));

	float AddYaw = Input.X * YawRotationSpeed * DeltaTime;
	AddActorWorldRotation(FRotator(0.f, AddYaw, 0.f));
}
