# Custom Sphere with Flame Slot — Setup Guide

## Overview

The MotionSphere uses a custom mesh (designed in Blender) with a specific emission socket where a Niagara flame particle system attaches. The flame always emits upward in world space regardless of sphere rotation.

---

## Blender Workflow

### Step 1: Model the Sphere

1. Open Blender, start with default UV Sphere
2. **Mesh settings**: 32 segments, 16 rings (good detail without excess polys)
3. Smooth shade the mesh
4. Scale to desired size (e.g., 1m diameter = 50 UE units radius)

### Step 2: Create the Flame Socket

The flame socket is a specific area on the sphere where the Niagara system attaches:

1. Select the sphere in Edit Mode
2. Select the vertex at the **35° angle** from the top (upper-right quadrant):
   - Position: `(cos(35°) × radius, 0, sin(35°) × radius)`
   - For 50-unit radius: approximately `(41, 0, 28.7)`
3. **Add an Empty** (Plain Axes) at this vertex position
4. Parent the Empty to the sphere mesh (Ctrl+P → Object)
5. Name the Empty: `FlameSocket`

### Step 3: UV Unwrap

1. Select sphere → Tab into Edit Mode → Select All (A)
2. UV Unwrap: `U → Smart UV Project` (or Sphere Projection)
3. This UV map is used for the emissive material in UE5

### Step 4: Export

1. Select the sphere mesh + FlameSocket empty
2. File → Export → FBX
3. Settings:
   - Scale: 1.0
   - Forward: -Y Forward (matches UE5)
   - Up: Z Up
   - Apply Transform: checked
   - Mesh: Smoothing → Face
   - Include: Selected Objects only

---

## Unreal Engine Import

### Step 1: Import FBX

1. Drag the .fbx into Content Browser
2. Import settings:
   - Import Mesh: Yes
   - Import as Skeletal: No (static mesh)
   - Generate Lightmap UVs: Yes
   - Auto Generate Collision: No (visual only)

### Step 2: Assign to MotionSphere

1. Open `BP_MotionSphere`
2. Select `SphereMesh` component
3. Set Static Mesh to your imported sphere
4. Verify `FlameSocket` component position matches your Blender socket:
   - Default: `(cos(35°)×50, 0, sin(35°)×50)` = `(41, 0, 28.7)`
   - Adjust in Details panel if needed

---

## Niagara Flame System

### Creating NS_SphereFlame

1. Content Browser → Right-click → Niagara System → New system from template
2. Choose **Fountain** template as base
3. Name: `NS_SphereFlame`

### Emitter Configuration

**Spawn Rate:**
- Module: Spawn Rate
- Rate: 100 particles/sec (controlled by `SpawnRate` parameter)

**Initialize Particle:**
- Lifetime: 0.3 - 0.8 seconds
- Size: 5-15 (randomized)
- Color: Bound to `FlameColor` user parameter
- Sprite: Use a soft circle or flame sprite texture

**Velocity:**
- Initial Velocity: (0, 0, 200) — upward in local space
- Add Noise: moderate curl noise for organic flickering

**Forces:**
- Drag: 2.0 (slows particles for flame look)
- No gravity (or very slight upward force)

**Size Over Life:**
- Curve: Start at 1.0, peak at 1.3 around 30%, fade to 0.2

**Color Over Life:**
- From: FlameColor at full brightness
- To: FlameColor at 0 opacity (fade out)

### User Parameters (exposed to C++)

| Parameter | Type | Default | C++ Control |
|-----------|------|---------|-------------|
| FlameColor | LinearColor | (1, 0.45, 0.05) | `SetFlameColor()` |
| SpawnRate | Float | 100 | `SetFlameIntensity()` |
| Intensity | Float | 1.0 | `SetFlameIntensity()` |
| MaxParticles | Int | 100 | `SetFlameParticleCount()` |

### Assign to MotionSphere

1. Open `BP_MotionSphere`
2. Select `FlameEffect` (NiagaraComponent)
3. Set Niagara System Asset: `NS_SphereFlame`
4. Set Auto Activate: true

---

## Niagara Smoke System (Optional)

### Creating NS_SphereSmokeTrail

Similar to flame but with:
- Longer lifetime (1-3 seconds)
- Larger particles, lower opacity
- More drag, slight downward drift
- `SmokeColor` parameter (tinted by flame color at 30%)
- Spawn Rate: 30 particles/sec

---

## Flame Direction (Always Upward)

The `MotionSphere::UpdateFlameDirection()` function ensures the flame socket always points up in world space:

```cpp
void AMotionSphere::UpdateFlameDirection()
{
    const FRotator ActorRot = GetActorRotation();
    const FRotator InverseRot = ActorRot.GetInverse();
    const FVector WorldUp = InverseRot.RotateVector(FVector::UpVector);
    FlameSocket->SetRelativeRotation(WorldUp.Rotation());
}
```

This runs every tick, counter-rotating the socket so particles always emit upward regardless of the sphere's roll/pitch/yaw.

---

## State-Aware Flame Behavior

The flame visually changes based on motion state:

| State | Flame Behavior | Visual Effect |
|-------|---------------|---------------|
| Idle | Steady, gentle | Calm flickering candle |
| Sway | Slight trail as sphere moves | Windswept flame leaning opposite to motion |
| Roll | Compressed, intense | Fast-moving flame with spark trails |

To implement state-aware changes in your Niagara system:
1. Add a `MotionState` int parameter to NS_SphereFlame
2. In the Niagara graph, branch on this parameter:
   - 0 (Idle): Normal spawn rate, low velocity noise
   - 1 (Sway): +20% spawn rate, directional bias opposite to velocity
   - 2 (Roll): +50% spawn rate, reduced lifetime, added sparks

---

## Flame Color Presets

Built into the `MotionSphere` C++ class:

| Preset | Color (RGB) | Intensity | Particles |
|--------|------------|-----------|-----------|
| Fire | (1.0, 0.45, 0.05) | 1.0 | 100 |
| Ice | (0.2, 0.6, 1.0) | 0.8 | 80 |
| Toxic | (0.1, 1.0, 0.2) | 0.9 | 90 |
| Magic | (0.7, 0.2, 1.0) | 1.0 | 120 |
| Pure | (1.0, 1.0, 1.0) | 1.0 | 150 |

Apply via:
```cpp
Sphere->ApplyFlamePreset(TEXT("Fire"));
```

Or from UI:
```cpp
Sphere->SetFlameColor(FLinearColor(0.2f, 0.6f, 1.0f));
Sphere->SetFlameIntensity(0.8f);
Sphere->SetFlameParticleCount(80);
```
