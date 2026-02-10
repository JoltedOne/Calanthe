#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Camera/CameraComponent.h"
#include "CameraControllerComponent.generated.h"

UENUM(BlueprintType)
enum class ECameraPreset : uint8
{
	StaticCenter   UMETA(DisplayName = "Static Center"),
	OrbitSlow      UMETA(DisplayName = "Orbit Slow"),
	OrbitFast      UMETA(DisplayName = "Orbit Fast"),
	TopDown        UMETA(DisplayName = "Top Down"),
	LowAngle       UMETA(DisplayName = "Low Angle"),
	CloseUp        UMETA(DisplayName = "Close Up"),
	DutchAngle     UMETA(DisplayName = "Dutch Angle")
};

USTRUCT(BlueprintType)
struct FCameraPresetData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "10.0", ClampMax = "170.0"))
	float FOV = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoOrbit = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OrbitSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OrbitRadius = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCameraPresetChanged, ECameraPreset, NewPreset);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CALANTHE_API UCameraControllerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCameraControllerComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// --- Preset Control ---

	UFUNCTION(BlueprintCallable, Category = "VJ|Camera")
	void SetPreset(ECameraPreset Preset, bool bFlashCut = false);

	UFUNCTION(BlueprintCallable, Category = "VJ|Camera")
	void CyclePresets();

	UFUNCTION(BlueprintPure, Category = "VJ|Camera")
	ECameraPreset GetCurrentPreset() const { return ActivePreset; }

	UFUNCTION(BlueprintCallable, Category = "VJ|Camera")
	void SetTransitionDuration(float Seconds);

	// --- Custom Positioning ---

	UFUNCTION(BlueprintCallable, Category = "VJ|Camera")
	void SetCustomPosition(FVector Location, FRotator Rotation, float FOV = 90.0f);

	// --- Look-At Target ---

	UFUNCTION(BlueprintCallable, Category = "VJ|Camera")
	void SetLookAtTarget(AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "VJ|Camera")
	void ClearLookAtTarget();

	// --- Events ---

	UPROPERTY(BlueprintAssignable, Category = "VJ|Camera")
	FOnCameraPresetChanged OnCameraPresetChanged;

	// --- Configuration ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VJ|Camera")
	TMap<ECameraPreset, FCameraPresetData> PresetConfigs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VJ|Camera", meta = (ClampMin = "0.01", ClampMax = "5.0"))
	float TransitionDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VJ|Camera")
	TObjectPtr<UCameraComponent> TargetCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VJ|Camera")
	FVector StageCenter = FVector::ZeroVector;

protected:
	virtual void BeginPlay() override;

private:
	void InitializeDefaultPresets();
	void UpdateTransition(float DeltaTime);
	void UpdateAutoOrbit(float DeltaTime);
	void ApplyPresetImmediate(const FCameraPresetData& Data);

	ECameraPreset ActivePreset = ECameraPreset::StaticCenter;

	// Transition state
	bool bTransitioning = false;
	float TransitionAlpha = 0.0f;
	FVector TransitionStartLoc;
	FRotator TransitionStartRot;
	float TransitionStartFOV;
	FVector TransitionEndLoc;
	FRotator TransitionEndRot;
	float TransitionEndFOV;

	// Auto-orbit state
	float OrbitAngle = 0.0f;
	bool bAutoOrbitActive = false;
	float CurrentOrbitSpeed = 0.0f;
	float CurrentOrbitRadius = 0.0f;

	UPROPERTY()
	TObjectPtr<AActor> LookAtTarget;
};
