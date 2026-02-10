# Visual Synthesizer — Complete Setup Guide (V1)

## Overview

This guide walks you through setting up the entire Visual Synthesizer VJ system from scratch in your compiled Calanthe project.

## Prerequisites

- Unreal Engine 5.4 installed
- Calanthe project compiled successfully (C++ build)
- Basic familiarity with UE5 editor (placing actors, creating Blueprints)

---

## Step 1: Create the Stage Blueprint

1. In Content Browser: **Right-click > Blueprint Class > Search "LitCubeStage"**
2. Name it `BP_LitCubeStage`
3. Open the Blueprint and configure:

### Stage Dimensions
| Property | Default | Description |
|----------|---------|-------------|
| StageWidth | 1000 | X-axis extent (cm) |
| StageDepth | 1000 | Y-axis extent (cm) |
| StageHeight | 500 | Z-axis height (cm) |
| FloorGridResolution | 100 | Vertices per axis (100 = 10,000 verts) |

### Materials
Assign your `M_CubePanel_Master` material (see `LitCubeStage_MaterialGuide.md`) to:
- **WallBaseMaterial** — used for all 4 walls and ceiling
- **FloorBaseMaterial** — used for the procedural floor
- **LightStripBaseMaterial** — used for LED strips

4. Place `BP_LitCubeStage` at world origin (0, 0, 0)

---

## Step 2: Create the Motion Sphere

1. **Right-click > Blueprint Class > Search "MotionSphere"**
2. Name it `BP_MotionSphere`
3. Open and configure:

### Mesh Assignment
- Select `SphereMesh` component
- Assign your sphere static mesh (default UE sphere or custom Blender import)
- Assign emissive material

### Flame Setup
- `FlameEffect` component: Assign your Niagara flame system asset
- `SmokeEffect` component: Assign your Niagara smoke system asset (optional)

### Motion Parameters
| State | Property | Default | Range |
|-------|----------|---------|-------|
| Idle | OrbitRadius | 200 | 50-500 |
| Idle | OrbitSpeed | 0.5 | 0.1-3.0 |
| Idle | BobAmplitude | 30 | 0-100 |
| Sway | SwayAmplitude | 150 | 50-400 |
| Sway | SwayFrequency | 1.2 | 0.1-5.0 |
| Roll | RollSpeed | 300 | 100-800 |
| Roll | BounceHeight | 80 | 0-200 |

4. Place `BP_MotionSphere` inside the cube stage (above floor center)

---

## Step 3: Camera Rig

1. Create a new **Blueprint Actor** called `BP_VJCameraRig`
2. Add components:
   - **CameraComponent** (name: `MainCamera`)
   - **CameraControllerComponent** (our custom component)
3. In CameraControllerComponent details:
   - `TargetCamera` → assign the MainCamera component
   - `StageCenter` → (0, 0, 150) or wherever your stage center is
4. Place in the level and set as the active camera (Auto Activate or via Level Blueprint)

### Camera Presets (defaults, customize in Details panel):
| Preset | Position | Rotation | FOV |
|--------|----------|----------|-----|
| 1. Static Center | (0, -800, 250) | (-15, 0, 0) | 90 |
| 2. Orbit Slow | Auto-orbit R=600 | Auto | 75 |
| 3. Orbit Fast | Auto-orbit R=500 | Auto | 80 |
| 4. Top Down | (0, 0, 900) | (-90, 0, 0) | 70 |
| 5. Low Angle | (200, -400, 30) | (15, -20, 0) | 100 |
| 6. Close Up | (0, -200, 150) | (-5, 0, 0) | 50 |
| 7. Dutch Angle | (-300, -500, 350) | (-15, 25, 20) | 85 |

---

## Step 4: Texture Pattern Manager

1. On your `BP_LitCubeStage` (or a dedicated manager actor), add a **TexturePatternManager** component
2. In the Details panel, expand **Texture Slots** array
3. For each slot, assign:
   - A B&W alpha mask texture (import your .png files)
   - ColorA and ColorB
   - Emissive intensity for each color
4. The manager broadcasts `OnTextureSlotChanged` when switching — bind to your stage walls

### Connecting to Stage Walls:
In your Blueprint Event Graph:
```
OnTextureSlotChanged →
  Get Owner → Cast to LitCubeStage →
  SetWallTexture(Front, Slot.AlphaMask)
  SetWallColors(Front, Slot.ColorA, Slot.ColorB)
  ... repeat for other walls
```

---

## Step 5: Floor Deformation

1. Add a **FloorDeformationController** component to your stage actor
2. Configure:
   - `StageRef` → self (the LitCubeStage)
   - `SphereRef` → your BP_MotionSphere (for follow mode)
   - `DeformationAmount` → 100 (start moderate)
   - `WaveLayers` → 3 layers preconfigured

### Wave Modes:
- **Directional**: Connect joystick UI widget to `SetJoystickDirection()`
- **Radial**: Waves pulse outward from center (no joystick needed)
- **Circular**: Vortex pattern spinning around center

---

## Step 6: Hotkey Setup

In your Player Controller or Game Mode Blueprint, bind these inputs:

```
1-7         → CameraController.SetPreset(Preset, false)
Shift+1-7   → CameraController.SetPreset(Preset, true)  // flash cut
C           → CameraController.CyclePresets()
T           → MotionSphere.CycleMotionState()
Q           → TextureManager.CycleSlotBackward()
E           → TextureManager.CycleSlotForward()
F1-F7       → TextureManager.ActivateSlot(0-6)
Tab         → Toggle UI visibility
L           → LitCubeStage.ToggleLightStrips()
```

---

## Step 7: Test Run

1. Press Play in editor
2. Verify:
   - [ ] Cube stage visible, no sky showing
   - [ ] Floor is a solid mesh inside the cube
   - [ ] Sphere is orbiting (Idle state)
   - [ ] Flame particles emitting upward from socket
   - [ ] Press 1-7 to switch cameras
   - [ ] Press T to cycle sphere states
   - [ ] Light strips animate with chase pattern
3. If walls are dark, check material parameter connections
4. If floor doesn't deform, verify FloorDeformationController has StageRef set
