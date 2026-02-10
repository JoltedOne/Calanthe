# TexturePatternManager — Guide

## Overview

The TexturePatternManager is a component that manages up to 10 texture slots, each containing a B&W alpha mask with dual-color emission control. It provides blend mixing, auto-animation, and a 5-slot preset system.

---

## How the Dual-Color Emission System Works

```
B&W Texture (Alpha Mask)
├── Black areas (value 0.0) → emit ColorA × IntensityA
├── White areas (value 1.0) → emit ColorB × IntensityB
└── Gray areas → proportional mix

EmissionBlend slider (0.0 to 1.0):
├── 0.0 → ColorA at full, ColorB silent
├── 0.5 → Both colors at 50%
└── 1.0 → ColorB at full, ColorA silent
```

### Example: Checkerboard Pattern
```
Texture: Checkerboard.png (alternating black/white squares)
ColorA: Red (#FF0000)
ColorB: Blue (#0000FF)

Blend = 0.0 → Red squares glow, blue squares dark
Blend = 0.5 → Both glow at half intensity
Blend = 1.0 → Blue squares glow, red squares dark

Auto-Animate ON, Speed 2.0:
→ Emission pulses between red and blue at 2Hz
```

---

## Setting Up Texture Slots

### In Blueprint Details Panel

1. Add `TexturePatternManager` component to your actor
2. Expand the **Texture Slots** array (10 slots)
3. For each slot you want to use:
   - **SlotName**: Descriptive name (e.g., "Checkerboard", "Stripes")
   - **AlphaMask**: Your B&W texture asset
   - **ThumbnailPreview**: Small preview image for UI (optional)
   - **ColorA / ColorB**: Initial colors
   - **EmissiveIntensityA / B**: Brightness (0-100)

### Via C++
```cpp
FTextureSlot Slot;
Slot.SlotName = TEXT("Checkerboard");
Slot.AlphaMask = CheckerboardTexture;
Slot.ColorA = FLinearColor::Red;
Slot.ColorB = FLinearColor::Blue;
Slot.EmissiveIntensityA = 80.0f;
Slot.EmissiveIntensityB = 80.0f;

TextureManager->SetTextureSlot(0, Slot);
TextureManager->ActivateSlot(0);
```

---

## Slot Switching

```cpp
// Direct activation
TextureManager->ActivateSlot(3);  // Switch to slot 3

// Cycle through valid (non-empty) slots
TextureManager->CycleSlotForward();   // Next valid slot
TextureManager->CycleSlotBackward();  // Previous valid slot

// Check state
int32 Active = TextureManager->GetActiveSlotIndex();
int32 Count = TextureManager->GetSlotCount();
```

The cycling functions skip empty slots (where `AlphaMask == nullptr`).

---

## Emission Blend Control

```cpp
// Manual blend control
TextureManager->SetEmissionBlend(0.0f);  // Full ColorA
TextureManager->SetEmissionBlend(0.5f);  // 50/50 mix
TextureManager->SetEmissionBlend(1.0f);  // Full ColorB

// Auto-animate: oscillates blend via sine wave
TextureManager->SetAutoAnimate(true);
TextureManager->SetAutoAnimateSpeed(2.0f);  // 2Hz oscillation

// Read current state
float Blend = TextureManager->GetEmissionBlend();
bool bAnimating = TextureManager->IsAutoAnimating();
```

### Auto-Animate Curve
The blend value follows a sine wave:
```
Blend = (sin(time × speed × 2π) + 1) / 2
```
This produces smooth 0→1→0→1 oscillation.

---

## Color Control at Runtime

```cpp
// Change colors for a specific slot
TextureManager->SetSlotColorA(0, FLinearColor(1, 0, 1));  // Magenta
TextureManager->SetSlotColorB(0, FLinearColor(0, 1, 1));  // Cyan

// Change emission intensity
TextureManager->SetSlotEmissiveIntensityA(0, 60.0f);
TextureManager->SetSlotEmissiveIntensityB(0, 100.0f);
```

If the modified slot is the active slot, changes broadcast immediately via `OnTextureSlotChanged`.

---

## Connecting to Stage Walls

Bind the `OnTextureSlotChanged` delegate to update the stage:

### Blueprint Event Binding
```
Event BeginPlay:
  TextureManager → Bind to OnTextureSlotChanged → Custom Event "UpdateWalls"

Custom Event "UpdateWalls" (SlotIndex, Slot):
  StageRef → SetWallTexture(Front, Slot.AlphaMask)
  StageRef → SetWallColors(Front, Slot.ColorA, Slot.ColorB)
  StageRef → SetWallTexture(Back, Slot.AlphaMask)
  StageRef → SetWallColors(Back, Slot.ColorA, Slot.ColorB)
  ... etc for all walls

Event OnEmissionBlendChanged (BlendValue):
  StageRef → SetAllWallsEmissionBlend(BlendValue)
```

---

## Preset System

### Saving a Preset
```cpp
TextureManager->SavePreset(0, TEXT("Synthwave"));
// Captures: active slot, blend value, auto-animate state, speed
```

### Loading a Preset
```cpp
TextureManager->LoadPreset(0);
// Restores all captured state + broadcasts events
```

### Reading Preset Data
```cpp
FVJPreset Preset = TextureManager->GetPreset(0);
// Preset.PresetName, Preset.ActiveTextureSlot, etc.
```

### Reset
```cpp
TextureManager->ResetToDefaults();
// Slot 0, blend 0, no auto-animate
```

---

## Events / Delegates

| Delegate | Signature | When Fired |
|----------|-----------|------------|
| OnTextureSlotChanged | (int32 SlotIndex, FTextureSlot Slot) | Slot activated or active slot modified |
| OnEmissionBlendChanged | (float BlendValue) | Manual blend change or auto-animate tick |
| OnPresetLoaded | (FName PresetName) | Preset successfully loaded |

---

## Recommended Texture Patterns

| Pattern | Resolution | Tiling | Best For |
|---------|-----------|--------|----------|
| Checkerboard | 2×2 px | (8,8) | Classic grid look |
| Horizontal Stripes | 1×8 px | (1,4) | Scanning lines |
| Vertical Stripes | 8×1 px | (4,1) | Column patterns |
| Diagonal | 8×8 px | (4,4) | Dynamic angles |
| Perlin Noise | 256×256 px | (2,2) | Organic patterns |
| Radial Gradient | 256×256 px | (1,1) | Spotlight/vignette |
| Audio Spectrum | 64×1 px | (1,1) | Equalizer bars |
| Triangles | 16×16 px | (4,4) | Geometric patterns |

All textures should be:
- **Grayscale** (black and white only, or with gray gradients)
- **Power-of-2 dimensions** for optimal GPU performance
- **Texture settings**: sRGB OFF for linear sampling, Nearest or Bilinear filtering
