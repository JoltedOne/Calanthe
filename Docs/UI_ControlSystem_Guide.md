# UI Control System Guide

## Overview

The VJ Control Panel is a semi-transparent UMG widget overlay (80% opacity) that provides real-time control over all stage parameters. It is draggable, togglable with Tab, and organized into three sections: Textures, Joystick, and Sphere/Flame.

---

## Widget Blueprint: WBP_MasterVJControl

### Creation
1. Content Browser → Right-click → User Interface → Widget Blueprint
2. Name: `WBP_MasterVJControl`
3. Open the widget editor

### Root Layout
```
[Canvas Panel]
└── [Border] "PanelBackground" — semi-transparent, draggable
    ├── [Vertical Box] "MainLayout"
    │   ├── [Header Bar] — title + close button
    │   ├── [Horizontal Box] "ControlSections"
    │   │   ├── [Vertical Box] "TextureSection"
    │   │   ├── [Vertical Box] "JoystickSection"
    │   │   └── [Vertical Box] "SphereFlameSection"
    │   └── [Horizontal Box] "PresetBar"
    └── [Size Box] — constrain overall dimensions
```

---

## Panel Styling

### Background
- **Brush Color**: (12, 15, 30, 200) — dark blue-gray at ~78% opacity
- **Backdrop Blur**: Use a `BackgroundBlur` widget if available, or translucent material
- **Border**: 1px, rgba(80, 100, 160, 30)
- **Corner Radius**: 10px

### Typography
- **Section Headers**: 10px, uppercase, letter-spacing 1.8, color rgba(140, 155, 190, 128)
- **Labels**: 11px, color rgba(180, 188, 210, 190)
- **Values**: 11px, color rgba(210, 215, 230, 230)

---

## Section 1: Textures

### Texture Slot Grid
A 5×2 grid of thumbnail buttons:

```
[Uniform Grid Panel] Columns=5, Rows=2
├── [Button + Image] Slot 0  ← highlight when active
├── [Button + Image] Slot 1
├── ...
├── [Button + Image] Slot 9
└── [Button] "+ Add"
```

**Each thumbnail button:**
- Size: 48×48
- Image: `FTextureSlot::ThumbnailPreview` or placeholder
- Border: 2px cyan when active, 1px gray otherwise
- OnClicked → `TexturePatternManager->ActivateSlot(Index)`

### Color A Controls
```
[Horizontal Box]
├── [Text] "Color A:"
├── [Color Picker Button] — opens color picker
│   → On change: TexturePatternManager->SetSlotColorA(ActiveSlot, NewColor)
├── [Text] "Emit:"
└── [Slider] 0-100
    → On change: TexturePatternManager->SetSlotEmissiveIntensityA(ActiveSlot, Value)
```

### Color B Controls
Same layout as Color A, calling `SetSlotColorB` / `SetSlotEmissiveIntensityB`.

### Emission Blend
```
[Horizontal Box]
├── [Text] "Mix:"
├── [Slider] 0.0 - 1.0
│   → TexturePatternManager->SetEmissionBlend(Value)
├── [Text] "A ← → B"
```

### Auto-Animate Toggle
```
[Horizontal Box]
├── [CheckBox] "Auto"
│   → TexturePatternManager->SetAutoAnimate(bChecked)
├── [Text] "Speed:"
└── [Slider] 0.1 - 10.0
    → TexturePatternManager->SetAutoAnimateSpeed(Value)
```

---

## Section 2: Joystick

### Virtual Joystick Widget
Create a custom `WBP_Joystick` widget:

```
[Canvas Panel] 120×120
├── [Image] "JoystickBase" — circle background
├── [Image] "JoystickThumb" — draggable inner circle
└── [Overlay] "DirectionLabels" — N/S/E/W text
```

**Implementation (Blueprint):**
1. On `OnMouseButtonDown` on JoystickBase → start tracking
2. On `OnMouseMove` while tracking:
   - Calculate offset from center
   - Clamp to circle radius
   - Move JoystickThumb
   - Normalize to -1..1 range
   - Call `FloorDeformationController->SetJoystickDirection(FVector2D(X, Y))`
3. On `OnMouseButtonUp` → optional snap-back to center

### Deformation Sliders
```
[Vertical Box]
├── [Slider] "Deform:" 0-500
│   → FloorDeformationController->SetDeformationAmount(Value)
├── [Slider] "Speed:" 0.1-5.0
│   → FloorDeformationController->SetWaveSpeed(Value)
└── [Slider] "Freq:" 0.1-5.0
    → FloorDeformationController->SetWaveFrequency(Value)
```

### Wave Mode Dropdown
```
[ComboBox] Options: "Joystick", "Radial", "Circular"
→ FloorDeformationController->SetWaveMode(SelectedMode)
```

### Follow Sphere Toggle
```
[CheckBox] "Follow Sphere"
→ FloorDeformationController->SetFollowSphere(bChecked)
```

---

## Section 3: Sphere / Flame

### Motion State Display
```
[Vertical Box]
├── [Text] "State: IDLE" — updates via OnMotionStateChanged delegate
├── [Horizontal Box]
│   ├── [RadioButton] "Idle"
│   ├── [RadioButton] "Sway"
│   └── [RadioButton] "Roll"
└── Each → MotionSphere->SetMotionState(State)
```

### Flame Color
```
[Horizontal Box]
├── [Text] "Flame Color:"
├── [Color Picker]
│   → MotionSphere->SetFlameColor(Color)
└── [ComboBox] Presets: "Fire", "Ice", "Toxic", "Magic", "Pure"
    → MotionSphere->ApplyFlamePreset(Name)
```

### Flame Sliders
```
├── [Slider] "Intensity:" 0.0-1.0
│   → MotionSphere->SetFlameIntensity(Value)
└── [Slider] "Particles:" 10-200
    → MotionSphere->SetFlameParticleCount(Value)
```

### Toggles
```
├── [CheckBox] "Audio React"
│   → MotionSphere->SetFlameAudioReactive(bChecked)
└── [CheckBox] "Enable Smoke"
    → MotionSphere->SetSmokeEnabled(bChecked)
```

---

## Preset Bar

Bottom section with 5 preset buttons + Save/Load/Reset:

```
[Horizontal Box]
├── [Button] "Preset 1" → LoadPreset(0)
├── [Button] "Preset 2" → LoadPreset(1)
├── [Button] "Preset 3" → LoadPreset(2)
├── [Button] "Preset 4" → LoadPreset(3)
├── [Button] "Preset 5" → LoadPreset(4)
├── [Button] "Save"     → SavePreset(SelectedIndex, Name)
├── [Button] "Load"     → LoadPreset(SelectedIndex)
└── [Button] "Reset"    → ResetToDefaults()
```

### Global Audio Reactivity Slider
```
[Slider] "Audio React:" 0.0-1.0
→ Updates MotionSphere.AudioReactivity
→ Updates FloorDeformationController.AudioReactivity
```

---

## Draggable Panel Implementation

In your WBP_MasterVJControl Blueprint:

1. Add variables:
   - `bDragging` (bool)
   - `DragOffset` (Vector2D)

2. Override `OnMouseButtonDown`:
   ```
   If hit header bar:
     bDragging = true
     DragOffset = MousePosition - PanelPosition
     Return Handled
   ```

3. Override `OnMouseMove`:
   ```
   If bDragging:
     PanelPosition = MousePosition - DragOffset
     SetRenderTranslation(PanelPosition)
   ```

4. Override `OnMouseButtonUp`:
   ```
   bDragging = false
   ```

---

## Displaying the Widget

In your Player Controller or HUD Blueprint:

```
BeginPlay:
  Create Widget (WBP_MasterVJControl) → Store as "VJPanel"
  Add to Viewport

Input Action "Tab":
  VJPanel.SetVisibility(Toggle)
```

---

## Connecting to Systems

In your WBP_MasterVJControl, add variables for references:
- `StageRef` (ALitCubeStage*)
- `SphereRef` (AMotionSphere*)
- `TextureManagerRef` (UTexturePatternManager*)
- `FloorControllerRef` (UFloorDeformationController*)
- `CameraControllerRef` (UCameraControllerComponent*)

Set these on creation from your Player Controller, then all slider/button callbacks route through them.
