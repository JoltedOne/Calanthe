#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Texture2D.h"
#include "TexturePatternManager.generated.h"

USTRUCT(BlueprintType)
struct FTextureSlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SlotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> AlphaMask;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> ThumbnailPreview;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor ColorA = FLinearColor::Red;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor ColorB = FLinearColor::Blue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float EmissiveIntensityA = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float EmissiveIntensityB = 80.0f;

	bool IsValid() const { return AlphaMask != nullptr; }
};

USTRUCT(BlueprintType)
struct FVJPreset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PresetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ActiveTextureSlot = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EmissionBlend = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoAnimate = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AutoAnimateSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeformationAmount = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WaveSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor FlameColor = FLinearColor(1.0f, 0.45f, 0.05f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FlameIntensity = 1.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTextureSlotChanged, int32, SlotIndex, const FTextureSlot&, Slot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEmissionBlendChanged, float, BlendValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPresetLoaded, FName, PresetName);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CALANTHE_API UTexturePatternManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UTexturePatternManager();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// --- Texture Slots ---

	UFUNCTION(BlueprintCallable, Category = "VJ|Textures")
	void SetTextureSlot(int32 SlotIndex, const FTextureSlot& Slot);

	UFUNCTION(BlueprintCallable, Category = "VJ|Textures")
	FTextureSlot GetTextureSlot(int32 SlotIndex) const;

	UFUNCTION(BlueprintCallable, Category = "VJ|Textures")
	void ActivateSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "VJ|Textures")
	void CycleSlotForward();

	UFUNCTION(BlueprintCallable, Category = "VJ|Textures")
	void CycleSlotBackward();

	UFUNCTION(BlueprintPure, Category = "VJ|Textures")
	int32 GetActiveSlotIndex() const { return ActiveSlotIndex; }

	UFUNCTION(BlueprintPure, Category = "VJ|Textures")
	int32 GetSlotCount() const { return TextureSlots.Num(); }

	// --- Emission Control ---

	UFUNCTION(BlueprintCallable, Category = "VJ|Emission")
	void SetEmissionBlend(float Blend);

	UFUNCTION(BlueprintPure, Category = "VJ|Emission")
	float GetEmissionBlend() const { return CurrentEmissionBlend; }

	UFUNCTION(BlueprintCallable, Category = "VJ|Emission")
	void SetAutoAnimate(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "VJ|Emission")
	void SetAutoAnimateSpeed(float Speed);

	UFUNCTION(BlueprintPure, Category = "VJ|Emission")
	bool IsAutoAnimating() const { return bAutoAnimate; }

	// --- Color Control ---

	UFUNCTION(BlueprintCallable, Category = "VJ|Emission")
	void SetSlotColorA(int32 SlotIndex, FLinearColor Color);

	UFUNCTION(BlueprintCallable, Category = "VJ|Emission")
	void SetSlotColorB(int32 SlotIndex, FLinearColor Color);

	UFUNCTION(BlueprintCallable, Category = "VJ|Emission")
	void SetSlotEmissiveIntensityA(int32 SlotIndex, float Intensity);

	UFUNCTION(BlueprintCallable, Category = "VJ|Emission")
	void SetSlotEmissiveIntensityB(int32 SlotIndex, float Intensity);

	// --- Preset System ---

	UFUNCTION(BlueprintCallable, Category = "VJ|Presets")
	void SavePreset(int32 PresetIndex, FName Name);

	UFUNCTION(BlueprintCallable, Category = "VJ|Presets")
	void LoadPreset(int32 PresetIndex);

	UFUNCTION(BlueprintCallable, Category = "VJ|Presets")
	void ResetToDefaults();

	UFUNCTION(BlueprintPure, Category = "VJ|Presets")
	FVJPreset GetPreset(int32 Index) const;

	// --- Events ---

	UPROPERTY(BlueprintAssignable, Category = "VJ|Textures")
	FOnTextureSlotChanged OnTextureSlotChanged;

	UPROPERTY(BlueprintAssignable, Category = "VJ|Emission")
	FOnEmissionBlendChanged OnEmissionBlendChanged;

	UPROPERTY(BlueprintAssignable, Category = "VJ|Presets")
	FOnPresetLoaded OnPresetLoaded;

	// --- Data ---

	static constexpr int32 MaxTextureSlots = 10;
	static constexpr int32 MaxPresets = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VJ|Textures")
	TArray<FTextureSlot> TextureSlots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VJ|Presets")
	TArray<FVJPreset> Presets;

protected:
	virtual void BeginPlay() override;

private:
	void UpdateAutoAnimation(float DeltaTime);
	void BroadcastActiveSlot();
	FVJPreset CaptureCurrentState() const;

	int32 ActiveSlotIndex = 0;
	float CurrentEmissionBlend = 0.0f;
	bool bAutoAnimate = false;
	float AutoAnimateSpeed = 1.0f;
	float AnimationPhase = 0.0f;
};
