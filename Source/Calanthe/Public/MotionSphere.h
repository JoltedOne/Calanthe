#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "MotionSphere.generated.h"

UENUM(BlueprintType)
enum class ESphereMotionState : uint8
{
	Idle    UMETA(DisplayName = "Idle"),
	Sway    UMETA(DisplayName = "Sway"),
	Roll    UMETA(DisplayName = "Roll")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMotionStateChanged, ESphereMotionState, NewState);

USTRUCT(BlueprintType)
struct FFlamePreset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PresetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor FlameColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Intensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "10", ClampMax = "200"))
	int32 ParticleCount = 100;
};

UCLASS(BlueprintType, Blueprintable)
class CALANTHE_API AMotionSphere : public AActor
{
	GENERATED_BODY()

public:
	AMotionSphere();

	virtual void Tick(float DeltaTime) override;

	// --- State Machine ---

	UFUNCTION(BlueprintCallable, Category = "VJ|Sphere")
	void SetMotionState(ESphereMotionState NewState);

	UFUNCTION(BlueprintCallable, Category = "VJ|Sphere")
	void CycleMotionState();

	UFUNCTION(BlueprintPure, Category = "VJ|Sphere")
	ESphereMotionState GetMotionState() const { return CurrentState; }

	UPROPERTY(BlueprintAssignable, Category = "VJ|Sphere")
	FOnMotionStateChanged OnMotionStateChanged;

	// --- Flame Control ---

	UFUNCTION(BlueprintCallable, Category = "VJ|Flame")
	void SetFlameColor(FLinearColor Color);

	UFUNCTION(BlueprintCallable, Category = "VJ|Flame")
	void SetFlameIntensity(float Intensity);

	UFUNCTION(BlueprintCallable, Category = "VJ|Flame")
	void SetFlameParticleCount(int32 Count);

	UFUNCTION(BlueprintCallable, Category = "VJ|Flame")
	void ApplyFlamePreset(FName PresetName);

	UFUNCTION(BlueprintCallable, Category = "VJ|Flame")
	void SetFlameAudioReactive(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "VJ|Flame")
	void SetSmokeEnabled(bool bEnabled);

	// --- Orbit Parameters ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VJ|Sphere|Idle")
	float IdleOrbitRadius = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VJ|Sphere|Idle")
	float IdleOrbitSpeed = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VJ|Sphere|Idle")
	float IdleBobAmplitude = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VJ|Sphere|Idle")
	float IdleBobFrequency = 0.8f;

	// --- Sway Parameters ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VJ|Sphere|Sway")
	float SwayAmplitude = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VJ|Sphere|Sway")
	float SwayFrequency = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VJ|Sphere|Sway")
	float SwayVerticalOscillation = 40.0f;

	// --- Roll Parameters ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VJ|Sphere|Roll")
	float RollSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VJ|Sphere|Roll")
	float RollBounceHeight = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VJ|Sphere|Roll")
	float RollRotationRate = 360.0f;

	// --- Flame Presets ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VJ|Flame")
	TArray<FFlamePreset> FlamePresets;

	// --- Audio Reactivity ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VJ|Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AudioReactivity = 0.7f;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> SphereMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> FlameSocket;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> FlameEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> SmokeEffect;

private:
	void InitializeDefaultPresets();

	void UpdateIdleMotion(float DeltaTime);
	void UpdateSwayMotion(float DeltaTime);
	void UpdateRollMotion(float DeltaTime);
	void UpdateFlameDirection();

	UPROPERTY()
	ESphereMotionState CurrentState = ESphereMotionState::Idle;

	FVector OriginLocation;
	float StateTime = 0.0f;
	float OrbitAngle = 0.0f;
	float RollDirection = 1.0f;
	float CurrentFlameIntensity = 1.0f;
	bool bFlameAudioReactive = false;
	bool bSmokeEnabled = false;
	float ExternalBassLevel = 0.0f;
};
