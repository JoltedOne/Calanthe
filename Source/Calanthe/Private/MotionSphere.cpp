#include "MotionSphere.h"
#include "Calanthe.h"

AMotionSphere::AMotionSphere()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	SphereMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SphereMesh"));
	SphereMesh->SetupAttachment(Root);
	SphereMesh->SetRelativeScale3D(FVector(1.0f));
	SphereMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Flame socket at 35-degree angle (upper-right quadrant)
	FlameSocket = CreateDefaultSubobject<USceneComponent>(TEXT("FlameSocket"));
	FlameSocket->SetupAttachment(SphereMesh);
	const float SocketAngleRad = FMath::DegreesToRadians(35.0f);
	FlameSocket->SetRelativeLocation(FVector(
		FMath::Cos(SocketAngleRad) * 50.0f,
		0.0f,
		FMath::Sin(SocketAngleRad) * 50.0f
	));

	FlameEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FlameEffect"));
	FlameEffect->SetupAttachment(FlameSocket);
	FlameEffect->SetAutoActivate(true);

	SmokeEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SmokeEffect"));
	SmokeEffect->SetupAttachment(FlameSocket);
	SmokeEffect->SetAutoActivate(false);

	InitializeDefaultPresets();
}

void AMotionSphere::InitializeDefaultPresets()
{
	FlamePresets.Empty();

	auto MakePreset = [](FName Name, FLinearColor Color, float Inten, int32 Count) -> FFlamePreset
	{
		FFlamePreset P;
		P.PresetName = Name;
		P.FlameColor = Color;
		P.Intensity = Inten;
		P.ParticleCount = Count;
		return P;
	};

	FlamePresets.Add(MakePreset(TEXT("Fire"),  FLinearColor(1.0f, 0.45f, 0.05f), 1.0f, 100));
	FlamePresets.Add(MakePreset(TEXT("Ice"),   FLinearColor(0.2f, 0.6f, 1.0f),   0.8f, 80));
	FlamePresets.Add(MakePreset(TEXT("Toxic"), FLinearColor(0.1f, 1.0f, 0.2f),   0.9f, 90));
	FlamePresets.Add(MakePreset(TEXT("Magic"), FLinearColor(0.7f, 0.2f, 1.0f),   1.0f, 120));
	FlamePresets.Add(MakePreset(TEXT("Pure"),  FLinearColor(1.0f, 1.0f, 1.0f),   1.0f, 150));
}

void AMotionSphere::BeginPlay()
{
	Super::BeginPlay();
	OriginLocation = GetActorLocation();
	StateTime = 0.0f;
	OrbitAngle = 0.0f;
}

void AMotionSphere::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	StateTime += DeltaTime;

	switch (CurrentState)
	{
	case ESphereMotionState::Idle:
		UpdateIdleMotion(DeltaTime);
		break;
	case ESphereMotionState::Sway:
		UpdateSwayMotion(DeltaTime);
		break;
	case ESphereMotionState::Roll:
		UpdateRollMotion(DeltaTime);
		break;
	}

	UpdateFlameDirection();

	// Audio-reactive flame intensity modulation
	if (bFlameAudioReactive && FlameEffect)
	{
		const float ModulatedIntensity = CurrentFlameIntensity * (1.0f + ExternalBassLevel * AudioReactivity * 2.0f);
		FlameEffect->SetVariableFloat(FName("SpawnRate"), ModulatedIntensity * 100.0f);
	}
}

void AMotionSphere::SetMotionState(ESphereMotionState NewState)
{
	if (CurrentState == NewState) return;

	CurrentState = NewState;
	StateTime = 0.0f;
	OnMotionStateChanged.Broadcast(NewState);

	UE_LOG(LogCalanthe, Log, TEXT("MotionSphere state changed to: %d"), static_cast<int32>(NewState));
}

void AMotionSphere::CycleMotionState()
{
	const int32 Next = (static_cast<int32>(CurrentState) + 1) % 3;
	SetMotionState(static_cast<ESphereMotionState>(Next));
}

void AMotionSphere::UpdateIdleMotion(float DeltaTime)
{
	OrbitAngle += IdleOrbitSpeed * DeltaTime;

	const float X = OriginLocation.X + FMath::Cos(OrbitAngle) * IdleOrbitRadius;
	const float Y = OriginLocation.Y + FMath::Sin(OrbitAngle) * IdleOrbitRadius;
	const float Z = OriginLocation.Z + FMath::Sin(StateTime * IdleBobFrequency * UE_PI * 2.0f) * IdleBobAmplitude;

	SetActorLocation(FVector(X, Y, Z));

	// Gentle rotation to face orbit direction
	const float YawDeg = FMath::RadiansToDegrees(OrbitAngle) + 90.0f;
	SetActorRotation(FRotator(0.0f, YawDeg, 0.0f));
}

void AMotionSphere::UpdateSwayMotion(float DeltaTime)
{
	const float SwayPhase = StateTime * SwayFrequency * UE_PI * 2.0f;

	const float X = OriginLocation.X + FMath::Sin(SwayPhase) * SwayAmplitude;
	const float Y = OriginLocation.Y + FMath::Sin(SwayPhase * 0.7f) * SwayAmplitude * 0.5f;
	const float Z = OriginLocation.Z + FMath::Sin(SwayPhase * 1.5f) * SwayVerticalOscillation;

	SetActorLocation(FVector(X, Y, Z));

	// Rock rotation following sway
	const float RollAngle = FMath::Sin(SwayPhase) * 15.0f;
	const float PitchAngle = FMath::Cos(SwayPhase * 0.7f) * 10.0f;
	SetActorRotation(FRotator(PitchAngle, 0.0f, RollAngle));
}

void AMotionSphere::UpdateRollMotion(float DeltaTime)
{
	FVector CurrentLoc = GetActorLocation();

	// Roll along X axis, bounce on Z
	CurrentLoc.X += RollSpeed * RollDirection * DeltaTime;

	// Reverse direction at stage boundaries (configurable, default +-500)
	const float StageBound = 450.0f;
	if (FMath::Abs(CurrentLoc.X - OriginLocation.X) > StageBound)
	{
		RollDirection *= -1.0f;
	}

	// Bounce effect
	const float BouncePhase = StateTime * 3.0f;
	CurrentLoc.Z = OriginLocation.Z + FMath::Abs(FMath::Sin(BouncePhase * UE_PI)) * RollBounceHeight;

	SetActorLocation(CurrentLoc);

	// Spin rotation
	FRotator CurrentRot = GetActorRotation();
	CurrentRot.Pitch += RollRotationRate * RollDirection * DeltaTime;
	SetActorRotation(CurrentRot);
}

void AMotionSphere::UpdateFlameDirection()
{
	if (!FlameSocket) return;

	// Keep flame emitting upward in world space regardless of sphere rotation
	const FRotator ActorRot = GetActorRotation();
	const FRotator InverseRot = ActorRot.GetInverse();
	// World up in local space
	const FVector WorldUp = InverseRot.RotateVector(FVector::UpVector);
	FlameSocket->SetRelativeRotation(WorldUp.Rotation());
}

void AMotionSphere::SetFlameColor(FLinearColor Color)
{
	if (FlameEffect)
	{
		FlameEffect->SetVariableLinearColor(FName("FlameColor"), Color);
	}
	if (SmokeEffect)
	{
		// Smoke tinted slightly by flame color
		FLinearColor SmokeColor = FLinearColor::LerpUsingHSV(FLinearColor(0.1f, 0.1f, 0.1f), Color, 0.3f);
		SmokeEffect->SetVariableLinearColor(FName("SmokeColor"), SmokeColor);
	}
}

void AMotionSphere::SetFlameIntensity(float Intensity)
{
	CurrentFlameIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	if (FlameEffect)
	{
		FlameEffect->SetVariableFloat(FName("SpawnRate"), CurrentFlameIntensity * 100.0f);
		FlameEffect->SetVariableFloat(FName("Intensity"), CurrentFlameIntensity);
	}
}

void AMotionSphere::SetFlameParticleCount(int32 Count)
{
	Count = FMath::Clamp(Count, 10, 200);
	if (FlameEffect)
	{
		FlameEffect->SetVariableInt(FName("MaxParticles"), Count);
	}
}

void AMotionSphere::ApplyFlamePreset(FName PresetName)
{
	for (const FFlamePreset& Preset : FlamePresets)
	{
		if (Preset.PresetName == PresetName)
		{
			SetFlameColor(Preset.FlameColor);
			SetFlameIntensity(Preset.Intensity);
			SetFlameParticleCount(Preset.ParticleCount);
			UE_LOG(LogCalanthe, Log, TEXT("Applied flame preset: %s"), *PresetName.ToString());
			return;
		}
	}
	UE_LOG(LogCalanthe, Warning, TEXT("Flame preset not found: %s"), *PresetName.ToString());
}

void AMotionSphere::SetFlameAudioReactive(bool bEnabled)
{
	bFlameAudioReactive = bEnabled;
}

void AMotionSphere::SetSmokeEnabled(bool bEnabled)
{
	bSmokeEnabled = bEnabled;
	if (SmokeEffect)
	{
		if (bEnabled)
		{
			SmokeEffect->Activate();
		}
		else
		{
			SmokeEffect->Deactivate();
		}
	}
}
