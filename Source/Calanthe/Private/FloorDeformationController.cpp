#include "FloorDeformationController.h"
#include "LitCubeStage.h"
#include "MotionSphere.h"
#include "Calanthe.h"

UFloorDeformationController::UFloorDeformationController()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Default 3 wave layers for complex patterns
	WaveLayers.Add({ 50.0f, 1.0f, 1.0f, 0.0f });
	WaveLayers.Add({ 25.0f, 2.3f, 1.5f, 1.2f });
	WaveLayers.Add({ 12.0f, 3.7f, 0.8f, 2.5f });
}

void UFloorDeformationController::BeginPlay()
{
	Super::BeginPlay();
	WaveTime = 0.0f;
}

void UFloorDeformationController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	WaveTime += DeltaTime * GlobalWaveSpeed;
	ComputeDeformation(DeltaTime);
}

// =============================================================
// INPUT
// =============================================================

void UFloorDeformationController::SetJoystickDirection(FVector2D Direction)
{
	// Clamp to unit circle
	if (Direction.SizeSquared() > 1.0f)
	{
		Direction.Normalize();
	}
	JoystickDirection = Direction;
}

void UFloorDeformationController::ResetJoystick()
{
	JoystickDirection = FVector2D::ZeroVector;
}

void UFloorDeformationController::SetWaveMode(EWaveMode Mode)
{
	CurrentWaveMode = Mode;
	UE_LOG(LogCalanthe, Log, TEXT("Wave mode changed to: %d"), static_cast<int32>(Mode));
}

void UFloorDeformationController::SetDeformationAmount(float Amount)
{
	DeformationAmount = FMath::Max(0.0f, Amount);
}

void UFloorDeformationController::SetWaveSpeed(float Speed)
{
	GlobalWaveSpeed = FMath::Max(0.1f, Speed);
}

void UFloorDeformationController::SetWaveFrequency(float Frequency)
{
	GlobalWaveFrequency = FMath::Max(0.01f, Frequency);
}

void UFloorDeformationController::SetFollowSphere(bool bEnabled)
{
	bFollowSphere = bEnabled;
}

void UFloorDeformationController::SetSphereReference(AMotionSphere* Sphere)
{
	SphereRef = Sphere;
}

void UFloorDeformationController::SetStageReference(ALitCubeStage* Stage)
{
	StageRef = Stage;
}

// =============================================================
// DEFORMATION COMPUTATION
// =============================================================

void UFloorDeformationController::ComputeDeformation(float DeltaTime)
{
	if (!StageRef) return;

	const int32 Res = StageRef->FloorGridResolution;
	const int32 VertCount = (Res + 1) * (Res + 1);
	const float Width = StageRef->StageWidth;
	const float Depth = StageRef->StageDepth;
	const float HalfW = Width * 0.5f;
	const float HalfD = Depth * 0.5f;

	HeightMapBuffer.SetNumUninitialized(VertCount);

	// Determine wave center
	FVector2D WaveCenter = FVector2D::ZeroVector;
	if (bFollowSphere && SphereRef)
	{
		const FVector SphereLoc = SphereRef->GetActorLocation();
		const FVector StageLoc = StageRef->GetActorLocation();
		WaveCenter = FVector2D(
			(SphereLoc.X - StageLoc.X) / HalfW,
			(SphereLoc.Y - StageLoc.Y) / HalfD
		);
	}

	// Audio modulation
	const float AudioMod = 1.0f + ExternalBassLevel * AudioReactivity * 2.0f;

	for (int32 Y = 0; Y <= Res; Y++)
	{
		for (int32 X = 0; X <= Res; X++)
		{
			const int32 Idx = Y * (Res + 1) + X;

			// Normalized grid position (-1 to 1)
			const float U = (static_cast<float>(X) / Res) * 2.0f - 1.0f;
			const float V = (static_cast<float>(Y) / Res) * 2.0f - 1.0f;
			const FVector2D GridPos(U, V);

			float TotalHeight = 0.0f;

			for (const FWaveLayer& Layer : WaveLayers)
			{
				switch (CurrentWaveMode)
				{
				case EWaveMode::Directional:
					TotalHeight += ComputeDirectionalWave(GridPos, Layer, WaveTime);
					break;
				case EWaveMode::Radial:
					TotalHeight += ComputeRadialWave(GridPos, WaveCenter, Layer, WaveTime);
					break;
				case EWaveMode::Circular:
					TotalHeight += ComputeCircularWave(GridPos, WaveCenter, Layer, WaveTime);
					break;
				}
			}

			TotalHeight *= DeformationAmount * 0.01f * AudioMod;
			HeightMapBuffer[Idx] = TotalHeight;
		}
	}

	StageRef->UpdateFloorDeformation(HeightMapBuffer);
}

float UFloorDeformationController::ComputeDirectionalWave(const FVector2D& GridPos, const FWaveLayer& Layer, float Time) const
{
	// Wave travels in joystick direction
	const float DotProduct = FVector2D::DotProduct(GridPos, JoystickDirection);
	const float FreqScaled = Layer.Frequency * GlobalWaveFrequency;
	const float Phase = DotProduct * FreqScaled * UE_PI * 2.0f - Time * Layer.Speed + Layer.PhaseOffset;
	return FMath::Sin(Phase) * Layer.Amplitude;
}

float UFloorDeformationController::ComputeRadialWave(const FVector2D& GridPos, const FVector2D& Center, const FWaveLayer& Layer, float Time) const
{
	// Waves emanate outward from center
	const float Dist = FVector2D::Distance(GridPos, Center);
	const float FreqScaled = Layer.Frequency * GlobalWaveFrequency;
	const float Phase = Dist * FreqScaled * UE_PI * 2.0f - Time * Layer.Speed + Layer.PhaseOffset;
	return FMath::Sin(Phase) * Layer.Amplitude;
}

float UFloorDeformationController::ComputeCircularWave(const FVector2D& GridPos, const FVector2D& Center, const FWaveLayer& Layer, float Time) const
{
	// Waves spin around center (vortex)
	const FVector2D Offset = GridPos - Center;
	const float Angle = FMath::Atan2(Offset.Y, Offset.X);
	const float Dist = Offset.Size();
	const float FreqScaled = Layer.Frequency * GlobalWaveFrequency;
	const float Phase = (Angle + Dist * FreqScaled) * 2.0f - Time * Layer.Speed + Layer.PhaseOffset;
	return FMath::Sin(Phase) * Layer.Amplitude;
}
