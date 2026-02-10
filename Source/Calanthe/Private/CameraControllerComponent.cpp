#include "CameraControllerComponent.h"
#include "Calanthe.h"
#include "Kismet/KismetMathLibrary.h"

UCameraControllerComponent::UCameraControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	InitializeDefaultPresets();
}

void UCameraControllerComponent::InitializeDefaultPresets()
{
	// Preset 1: Static Center — wide establishing shot
	FCameraPresetData StaticCenter;
	StaticCenter.Location = FVector(0, -800, 250);
	StaticCenter.Rotation = FRotator(-15, 0, 0);
	StaticCenter.FOV = 90.0f;
	PresetConfigs.Add(ECameraPreset::StaticCenter, StaticCenter);

	// Preset 2: Orbit Slow
	FCameraPresetData OrbitSlow;
	OrbitSlow.Location = FVector(0, -600, 300);
	OrbitSlow.Rotation = FRotator(-20, 0, 0);
	OrbitSlow.FOV = 75.0f;
	OrbitSlow.bAutoOrbit = true;
	OrbitSlow.OrbitSpeed = 15.0f;
	OrbitSlow.OrbitRadius = 600.0f;
	PresetConfigs.Add(ECameraPreset::OrbitSlow, OrbitSlow);

	// Preset 3: Orbit Fast
	FCameraPresetData OrbitFast;
	OrbitFast.Location = FVector(0, -500, 200);
	OrbitFast.Rotation = FRotator(-10, 0, 0);
	OrbitFast.FOV = 80.0f;
	OrbitFast.bAutoOrbit = true;
	OrbitFast.OrbitSpeed = 45.0f;
	OrbitFast.OrbitRadius = 500.0f;
	PresetConfigs.Add(ECameraPreset::OrbitFast, OrbitFast);

	// Preset 4: Top Down
	FCameraPresetData TopDown;
	TopDown.Location = FVector(0, 0, 900);
	TopDown.Rotation = FRotator(-90, 0, 0);
	TopDown.FOV = 70.0f;
	PresetConfigs.Add(ECameraPreset::TopDown, TopDown);

	// Preset 5: Low Angle — dramatic upward
	FCameraPresetData LowAngle;
	LowAngle.Location = FVector(200, -400, 30);
	LowAngle.Rotation = FRotator(15, -20, 0);
	LowAngle.FOV = 100.0f;
	PresetConfigs.Add(ECameraPreset::LowAngle, LowAngle);

	// Preset 6: Close Up
	FCameraPresetData CloseUp;
	CloseUp.Location = FVector(0, -200, 150);
	CloseUp.Rotation = FRotator(-5, 0, 0);
	CloseUp.FOV = 50.0f;
	PresetConfigs.Add(ECameraPreset::CloseUp, CloseUp);

	// Preset 7: Dutch Angle — tilted dramatic
	FCameraPresetData Dutch;
	Dutch.Location = FVector(-300, -500, 350);
	Dutch.Rotation = FRotator(-15, 25, 20);
	Dutch.FOV = 85.0f;
	PresetConfigs.Add(ECameraPreset::DutchAngle, Dutch);
}

void UCameraControllerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Find or assign camera component
	if (!TargetCamera)
	{
		TargetCamera = GetOwner()->FindComponentByClass<UCameraComponent>();
	}

	// Apply initial preset
	if (TargetCamera)
	{
		if (const FCameraPresetData* Data = PresetConfigs.Find(ActivePreset))
		{
			ApplyPresetImmediate(*Data);
		}
	}
}

void UCameraControllerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!TargetCamera) return;

	if (bTransitioning)
	{
		UpdateTransition(DeltaTime);
	}

	if (bAutoOrbitActive)
	{
		UpdateAutoOrbit(DeltaTime);
	}

	// Look-at override
	if (LookAtTarget)
	{
		const FVector CamLoc = TargetCamera->GetComponentLocation();
		const FVector TargetLoc = LookAtTarget->GetActorLocation();
		const FRotator LookRot = UKismetMathLibrary::FindLookAtRotation(CamLoc, TargetLoc);
		TargetCamera->SetWorldRotation(FMath::RInterpTo(
			TargetCamera->GetComponentRotation(),
			LookRot,
			DeltaTime,
			5.0f
		));
	}
}

void UCameraControllerComponent::SetPreset(ECameraPreset Preset, bool bFlashCut)
{
	const FCameraPresetData* Data = PresetConfigs.Find(Preset);
	if (!Data || !TargetCamera) return;

	ActivePreset = Preset;

	if (bFlashCut)
	{
		// Instant cut — no transition
		ApplyPresetImmediate(*Data);
		bTransitioning = false;
	}
	else
	{
		// Smooth transition
		TransitionStartLoc = TargetCamera->GetRelativeLocation();
		TransitionStartRot = TargetCamera->GetRelativeRotation();
		TransitionStartFOV = TargetCamera->FieldOfView;

		TransitionEndLoc = Data->Location;
		TransitionEndRot = Data->Rotation;
		TransitionEndFOV = Data->FOV;

		TransitionAlpha = 0.0f;
		bTransitioning = true;
	}

	bAutoOrbitActive = Data->bAutoOrbit;
	CurrentOrbitSpeed = Data->OrbitSpeed;
	CurrentOrbitRadius = Data->OrbitRadius;
	OrbitAngle = 0.0f;

	OnCameraPresetChanged.Broadcast(Preset);
	UE_LOG(LogCalanthe, Log, TEXT("Camera preset changed to: %d (FlashCut: %s)"),
		static_cast<int32>(Preset), bFlashCut ? TEXT("true") : TEXT("false"));
}

void UCameraControllerComponent::CyclePresets()
{
	const int32 Next = (static_cast<int32>(ActivePreset) + 1) % 7;
	SetPreset(static_cast<ECameraPreset>(Next));
}

void UCameraControllerComponent::SetTransitionDuration(float Seconds)
{
	TransitionDuration = FMath::Clamp(Seconds, 0.01f, 5.0f);
}

void UCameraControllerComponent::SetCustomPosition(FVector Location, FRotator Rotation, float FOV)
{
	if (!TargetCamera) return;

	TransitionStartLoc = TargetCamera->GetRelativeLocation();
	TransitionStartRot = TargetCamera->GetRelativeRotation();
	TransitionStartFOV = TargetCamera->FieldOfView;

	TransitionEndLoc = Location;
	TransitionEndRot = Rotation;
	TransitionEndFOV = FOV;

	TransitionAlpha = 0.0f;
	bTransitioning = true;
	bAutoOrbitActive = false;
}

void UCameraControllerComponent::SetLookAtTarget(AActor* Target)
{
	LookAtTarget = Target;
}

void UCameraControllerComponent::ClearLookAtTarget()
{
	LookAtTarget = nullptr;
}

void UCameraControllerComponent::UpdateTransition(float DeltaTime)
{
	TransitionAlpha += DeltaTime / TransitionDuration;

	if (TransitionAlpha >= 1.0f)
	{
		TransitionAlpha = 1.0f;
		bTransitioning = false;
	}

	// Smooth ease-in-out curve
	const float T = FMath::InterpEaseInOut(0.0f, 1.0f, TransitionAlpha, 2.0f);

	const FVector Loc = FMath::Lerp(TransitionStartLoc, TransitionEndLoc, T);
	const FRotator Rot = FMath::Lerp(TransitionStartRot, TransitionEndRot, T);
	const float FOV = FMath::Lerp(TransitionStartFOV, TransitionEndFOV, T);

	TargetCamera->SetRelativeLocation(Loc);
	TargetCamera->SetRelativeRotation(Rot);
	TargetCamera->SetFieldOfView(FOV);
}

void UCameraControllerComponent::UpdateAutoOrbit(float DeltaTime)
{
	if (!TargetCamera || bTransitioning) return;

	OrbitAngle += CurrentOrbitSpeed * DeltaTime;

	const float RadAngle = FMath::DegreesToRadians(OrbitAngle);
	const FVector OrbitPos(
		StageCenter.X + FMath::Cos(RadAngle) * CurrentOrbitRadius,
		StageCenter.Y + FMath::Sin(RadAngle) * CurrentOrbitRadius,
		TargetCamera->GetRelativeLocation().Z
	);

	TargetCamera->SetRelativeLocation(OrbitPos);

	// Face toward stage center
	if (!LookAtTarget)
	{
		const FRotator LookRot = UKismetMathLibrary::FindLookAtRotation(OrbitPos, StageCenter);
		TargetCamera->SetRelativeRotation(LookRot);
	}
}

void UCameraControllerComponent::ApplyPresetImmediate(const FCameraPresetData& Data)
{
	if (!TargetCamera) return;

	TargetCamera->SetRelativeLocation(Data.Location);
	TargetCamera->SetRelativeRotation(Data.Rotation);
	TargetCamera->SetFieldOfView(Data.FOV);
}
