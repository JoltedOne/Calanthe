#include "TexturePatternManager.h"
#include "Calanthe.h"

UTexturePatternManager::UTexturePatternManager()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Initialize 10 empty texture slots
	TextureSlots.SetNum(MaxTextureSlots);
	for (int32 i = 0; i < MaxTextureSlots; i++)
	{
		TextureSlots[i].SlotName = *FString::Printf(TEXT("Slot %d"), i + 1);
	}

	// Initialize 5 empty presets
	Presets.SetNum(MaxPresets);
	for (int32 i = 0; i < MaxPresets; i++)
	{
		Presets[i].PresetName = *FString::Printf(TEXT("Preset %d"), i + 1);
	}
}

void UTexturePatternManager::BeginPlay()
{
	Super::BeginPlay();

	// Activate first valid slot
	for (int32 i = 0; i < TextureSlots.Num(); i++)
	{
		if (TextureSlots[i].IsValid())
		{
			ActivateSlot(i);
			break;
		}
	}
}

void UTexturePatternManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bAutoAnimate)
	{
		UpdateAutoAnimation(DeltaTime);
	}
}

// =============================================================
// TEXTURE SLOTS
// =============================================================

void UTexturePatternManager::SetTextureSlot(int32 SlotIndex, const FTextureSlot& Slot)
{
	if (SlotIndex < 0 || SlotIndex >= MaxTextureSlots) return;

	if (SlotIndex >= TextureSlots.Num())
	{
		TextureSlots.SetNum(SlotIndex + 1);
	}

	TextureSlots[SlotIndex] = Slot;

	if (SlotIndex == ActiveSlotIndex)
	{
		BroadcastActiveSlot();
	}

	UE_LOG(LogCalanthe, Log, TEXT("Texture slot %d set: %s"), SlotIndex, *Slot.SlotName.ToString());
}

FTextureSlot UTexturePatternManager::GetTextureSlot(int32 SlotIndex) const
{
	if (TextureSlots.IsValidIndex(SlotIndex))
	{
		return TextureSlots[SlotIndex];
	}
	return FTextureSlot();
}

void UTexturePatternManager::ActivateSlot(int32 SlotIndex)
{
	if (!TextureSlots.IsValidIndex(SlotIndex)) return;

	ActiveSlotIndex = SlotIndex;
	BroadcastActiveSlot();

	UE_LOG(LogCalanthe, Log, TEXT("Activated texture slot %d: %s"), SlotIndex, *TextureSlots[SlotIndex].SlotName.ToString());
}

void UTexturePatternManager::CycleSlotForward()
{
	int32 Next = ActiveSlotIndex;
	for (int32 i = 0; i < TextureSlots.Num(); i++)
	{
		Next = (Next + 1) % TextureSlots.Num();
		if (TextureSlots[Next].IsValid())
		{
			ActivateSlot(Next);
			return;
		}
	}
}

void UTexturePatternManager::CycleSlotBackward()
{
	int32 Prev = ActiveSlotIndex;
	for (int32 i = 0; i < TextureSlots.Num(); i++)
	{
		Prev = (Prev - 1 + TextureSlots.Num()) % TextureSlots.Num();
		if (TextureSlots[Prev].IsValid())
		{
			ActivateSlot(Prev);
			return;
		}
	}
}

void UTexturePatternManager::BroadcastActiveSlot()
{
	if (TextureSlots.IsValidIndex(ActiveSlotIndex))
	{
		OnTextureSlotChanged.Broadcast(ActiveSlotIndex, TextureSlots[ActiveSlotIndex]);
	}
}

// =============================================================
// EMISSION CONTROL
// =============================================================

void UTexturePatternManager::SetEmissionBlend(float Blend)
{
	CurrentEmissionBlend = FMath::Clamp(Blend, 0.0f, 1.0f);
	OnEmissionBlendChanged.Broadcast(CurrentEmissionBlend);
}

void UTexturePatternManager::SetAutoAnimate(bool bEnabled)
{
	bAutoAnimate = bEnabled;
	if (!bEnabled)
	{
		AnimationPhase = 0.0f;
	}
}

void UTexturePatternManager::SetAutoAnimateSpeed(float Speed)
{
	AutoAnimateSpeed = FMath::Clamp(Speed, 0.1f, 10.0f);
}

void UTexturePatternManager::UpdateAutoAnimation(float DeltaTime)
{
	AnimationPhase += AutoAnimateSpeed * DeltaTime;

	// Sine wave oscillation: 0 → 1 → 0 → 1...
	const float BlendValue = (FMath::Sin(AnimationPhase * PI * 2.0f) + 1.0f) * 0.5f;
	CurrentEmissionBlend = BlendValue;
	OnEmissionBlendChanged.Broadcast(CurrentEmissionBlend);
}

// =============================================================
// COLOR CONTROL
// =============================================================

void UTexturePatternManager::SetSlotColorA(int32 SlotIndex, FLinearColor Color)
{
	if (!TextureSlots.IsValidIndex(SlotIndex)) return;
	TextureSlots[SlotIndex].ColorA = Color;
	if (SlotIndex == ActiveSlotIndex) BroadcastActiveSlot();
}

void UTexturePatternManager::SetSlotColorB(int32 SlotIndex, FLinearColor Color)
{
	if (!TextureSlots.IsValidIndex(SlotIndex)) return;
	TextureSlots[SlotIndex].ColorB = Color;
	if (SlotIndex == ActiveSlotIndex) BroadcastActiveSlot();
}

void UTexturePatternManager::SetSlotEmissiveIntensityA(int32 SlotIndex, float Intensity)
{
	if (!TextureSlots.IsValidIndex(SlotIndex)) return;
	TextureSlots[SlotIndex].EmissiveIntensityA = FMath::Clamp(Intensity, 0.0f, 100.0f);
	if (SlotIndex == ActiveSlotIndex) BroadcastActiveSlot();
}

void UTexturePatternManager::SetSlotEmissiveIntensityB(int32 SlotIndex, float Intensity)
{
	if (!TextureSlots.IsValidIndex(SlotIndex)) return;
	TextureSlots[SlotIndex].EmissiveIntensityB = FMath::Clamp(Intensity, 0.0f, 100.0f);
	if (SlotIndex == ActiveSlotIndex) BroadcastActiveSlot();
}

// =============================================================
// PRESET SYSTEM
// =============================================================

FVJPreset UTexturePatternManager::CaptureCurrentState() const
{
	FVJPreset State;
	State.ActiveTextureSlot = ActiveSlotIndex;
	State.EmissionBlend = CurrentEmissionBlend;
	State.bAutoAnimate = bAutoAnimate;
	State.AutoAnimateSpeed = AutoAnimateSpeed;
	return State;
}

void UTexturePatternManager::SavePreset(int32 PresetIndex, FName Name)
{
	if (PresetIndex < 0 || PresetIndex >= MaxPresets) return;

	if (PresetIndex >= Presets.Num())
	{
		Presets.SetNum(PresetIndex + 1);
	}

	Presets[PresetIndex] = CaptureCurrentState();
	Presets[PresetIndex].PresetName = Name;

	UE_LOG(LogCalanthe, Log, TEXT("Saved VJ preset %d: %s"), PresetIndex, *Name.ToString());
}

void UTexturePatternManager::LoadPreset(int32 PresetIndex)
{
	if (!Presets.IsValidIndex(PresetIndex)) return;

	const FVJPreset& Preset = Presets[PresetIndex];

	ActiveSlotIndex = FMath::Clamp(Preset.ActiveTextureSlot, 0, TextureSlots.Num() - 1);
	CurrentEmissionBlend = Preset.EmissionBlend;
	bAutoAnimate = Preset.bAutoAnimate;
	AutoAnimateSpeed = Preset.AutoAnimateSpeed;

	BroadcastActiveSlot();
	OnEmissionBlendChanged.Broadcast(CurrentEmissionBlend);
	OnPresetLoaded.Broadcast(Preset.PresetName);

	UE_LOG(LogCalanthe, Log, TEXT("Loaded VJ preset %d: %s"), PresetIndex, *Preset.PresetName.ToString());
}

FVJPreset UTexturePatternManager::GetPreset(int32 Index) const
{
	if (Presets.IsValidIndex(Index))
	{
		return Presets[Index];
	}
	return FVJPreset();
}

void UTexturePatternManager::ResetToDefaults()
{
	ActiveSlotIndex = 0;
	CurrentEmissionBlend = 0.0f;
	bAutoAnimate = false;
	AutoAnimateSpeed = 1.0f;
	AnimationPhase = 0.0f;

	BroadcastActiveSlot();
	OnEmissionBlendChanged.Broadcast(0.0f);
}
