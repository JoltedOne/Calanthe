#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FloorDeformationController.generated.h"

class ALitCubeStage;
class AMotionSphere;

UENUM(BlueprintType)
enum class EWaveMode : uint8
{
	Directional  UMETA(DisplayName = "Directional (Joystick)"),
	Radial       UMETA(DisplayName = "Radial (Outward)"),
	Circular     UMETA(DisplayName = "Circular (Spinning)")
};

USTRUCT(BlueprintType)
struct FWaveLayer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
	float Amplitude = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.01"))
	float Frequency = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1"))
	float Speed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PhaseOffset = 0.0f;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CALANTHE_API UFloorDeformationController : public UActorComponent
{
	GENERATED_BODY()

public:
	UFloorDeformationController();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// --- Joystick Input ---

	UFUNCTION(BlueprintCallable, Category = "VJ|Floor")
	void SetJoystickDirection(FVector2D Direction);

	UFUNCTION(BlueprintCallable, Category = "VJ|Floor")
	void ResetJoystick();

	UFUNCTION(BlueprintPure, Category = "VJ|Floor")
	FVector2D GetJoystickDirection() const { return JoystickDirection; }

	// --- Wave Mode ---

	UFUNCTION(BlueprintCallable, Category = "VJ|Floor")
	void SetWaveMode(EWaveMode Mode);

	UFUNCTION(BlueprintPure, Category = "VJ|Floor")
	EWaveMode GetWaveMode() const { return CurrentWaveMode; }

	// --- Deformation Parameters ---

	UFUNCTION(BlueprintCallable, Category = "VJ|Floor")
	void SetDeformationAmount(float Amount);

	UFUNCTION(BlueprintCallable, Category = "VJ|Floor")
	void SetWaveSpeed(float Speed);

	UFUNCTION(BlueprintCallable, Category = "VJ|Floor")
	void SetWaveFrequency(float Frequency);

	// --- Follow Sphere ---

	UFUNCTION(BlueprintCallable, Category = "VJ|Floor")
	void SetFollowSphere(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "VJ|Floor")
	void SetSphereReference(AMotionSphere* Sphere);

	// --- Stage Reference ---

	UFUNCTION(BlueprintCallable, Category = "VJ|Floor")
	void SetStageReference(ALitCubeStage* Stage);

	// --- Configuration ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VJ|Floor")
	float DeformationAmount = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VJ|Floor")
	float GlobalWaveSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VJ|Floor")
	float GlobalWaveFrequency = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VJ|Floor")
	TArray<FWaveLayer> WaveLayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VJ|Floor")
	bool bFollowSphere = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VJ|Floor", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AudioReactivity = 0.0f;

protected:
	virtual void BeginPlay() override;

private:
	void ComputeDeformation(float DeltaTime);
	float ComputeDirectionalWave(const FVector2D& GridPos, const FWaveLayer& Layer, float Time) const;
	float ComputeRadialWave(const FVector2D& GridPos, const FVector2D& Center, const FWaveLayer& Layer, float Time) const;
	float ComputeCircularWave(const FVector2D& GridPos, const FVector2D& Center, const FWaveLayer& Layer, float Time) const;

	FVector2D JoystickDirection = FVector2D::ZeroVector;
	EWaveMode CurrentWaveMode = EWaveMode::Directional;
	float WaveTime = 0.0f;
	float ExternalBassLevel = 0.0f;

	UPROPERTY()
	TObjectPtr<ALitCubeStage> StageRef;

	UPROPERTY()
	TObjectPtr<AMotionSphere> SphereRef;

	TArray<float> HeightMapBuffer;
};
