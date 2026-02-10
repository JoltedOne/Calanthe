# Calanthe — Visual Synthesizer VJ System

## 30,000ft Overview

Calanthe is a **professional real-time VJ (Visual Jockey) system** built in Unreal Engine 5.4. It provides a sealed cube stage environment with programmable walls, deformable floors, an animated sphere with flame effects, and a complete UI control panel — all designed for live music video production and performance.

## Architecture

```
Calanthe/
├── Source/Calanthe/
│   ├── Public/                    (Headers)
│   │   ├── MotionSphere.h        — Sphere actor with Idle/Sway/Roll state machine
│   │   ├── LitCubeStage.h        — Sealed cube environment with procedural floor
│   │   ├── CameraControllerComponent.h — 7 camera presets with smooth transitions
│   │   ├── TexturePatternManager.h     — 10-slot texture system with dual-color emission
│   │   └── FloorDeformationController.h — Directional wave control via joystick
│   ├── Private/                   (Implementations)
│   │   ├── MotionSphere.cpp
│   │   ├── LitCubeStage.cpp
│   │   ├── CameraControllerComponent.cpp
│   │   ├── TexturePatternManager.cpp
│   │   └── FloorDeformationController.cpp
│   ├── Calanthe.h / .cpp         (Module definition)
│   └── Calanthe.Build.cs         (Build configuration)
├── Docs/                          (This documentation)
└── Calanthe.uproject              (UE5.4 project file)
```

## Core Systems

### 1. LitCubeStage
The stage is a sealed cube — no sky, no world visible. Perfect for controlled music video environments.
- 4 independent walls with per-wall pattern/color/emission control
- Procedural floor mesh (configurable resolution up to 200x200) for real-time deformation
- Optional emissive ceiling
- Side wall LED-style light strips with programmable chase animation

### 2. MotionSphere
A custom sphere actor with three motion states:
- **Idle**: Gentle orbit around origin with vertical bobbing
- **Sway**: Pendulum-like swing with figure-8 variation
- **Roll**: Bouncing roll along stage floor with auto-reverse at boundaries

Includes a flame socket at 35° for Niagara particle attachment, with 5 built-in color presets (Fire, Ice, Toxic, Magic, Pure) and audio-reactive intensity.

### 3. CameraControllerComponent
Seven camera presets with smooth interpolated transitions:
1. Static Center (wide establishing shot)
2. Orbit Slow (auto-orbiting)
3. Orbit Fast (auto-orbiting)
4. Top Down
5. Low Angle (dramatic upward)
6. Close Up
7. Dutch Angle (tilted)

Supports flash cuts (instant) and smooth transitions, plus look-at target tracking.

### 4. TexturePatternManager
10 texture slots for B&W alpha mask patterns with dual-color emission:
- Load any B&W texture (checkerboard, stripes, noise, etc.)
- Assign ColorA (black areas) and ColorB (white areas)
- Blend slider mixes emission between A and B
- Auto-animate mode oscillates the blend for pulsing effects
- 5 saveable/loadable presets

### 5. FloorDeformationController
Real-time floor mesh deformation driven by a virtual joystick:
- **Directional**: Waves travel in joystick direction
- **Radial**: Waves emanate outward from center (ripple)
- **Circular**: Waves spin around center (vortex)
- 3 wave layers for complex multi-frequency patterns
- Optional sphere-following mode (waves from sphere position)

## Dependencies
- Unreal Engine 5.4
- ProceduralMeshComponent (floor deformation)
- Niagara (flame/smoke particle systems)
- UMG (UI widgets, built in Blueprint)
- Enhanced Input (keyboard shortcuts)

## Quick Start
1. Open `Calanthe.uproject` in UE5.4
2. Compile C++ (Build > Build Solution or Ctrl+B)
3. Follow `LitCubeStage_IntegrationGuide.md` to create the stage Blueprint
4. Follow `CustomSphere_FlameSlot_Guide.md` for the sphere
5. Follow `UI_ControlSystem_Guide.md` to build the control panel
