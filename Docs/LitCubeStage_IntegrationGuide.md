# LitCubeStage — Integration Guide

## What Is the LitCubeStage?

A sealed cube environment actor that creates an enclosed performance space with:
- 4 programmable walls (Front, Back, Left, Right)
- Procedural deformable floor mesh
- Optional emissive ceiling
- LED-style vertical light strips on side walls

No sky, no world geometry — everything the camera sees is part of the stage.

---

## Creating the Blueprint

### Step 1: Blueprint Creation
1. Content Browser → Right-click → Blueprint Class
2. Search for `LitCubeStage` as parent class
3. Name: `BP_LitCubeStage`

### Step 2: Assign Static Meshes
The walls and ceiling use `UStaticMeshComponent`. You need to assign a plane mesh:
1. Open `BP_LitCubeStage`
2. The wall components are created dynamically at runtime — set the mesh reference via:
   - In your Construction Script or BeginPlay, set a default plane mesh on each wall
   - Or: assign the engine's built-in plane: `/Engine/BasicShapes/Plane`

### Step 3: Material Setup
Create `M_CubePanel_Master` material first (see `LitCubeStage_MaterialGuide.md`), then:
- **WallBaseMaterial**: Assign `M_CubePanel_Master`
- **FloorBaseMaterial**: Assign `M_CubePanel_Master` (or a floor-specific variant)
- **LightStripBaseMaterial**: Create `M_LightStrip` with `StripColor` and `Intensity` parameters

### Step 4: Stage Dimensions
Configure in the Details panel:

| Property | Default | Recommended Range |
|----------|---------|-------------------|
| StageWidth | 1000cm | 500-2000cm |
| StageDepth | 1000cm | 500-2000cm |
| StageHeight | 500cm | 300-800cm |

---

## Floor Mesh

### Grid Resolution
The floor is a procedural mesh generated at construction time.

| Resolution | Vertices | Performance |
|-----------|----------|-------------|
| 25×25 | 676 | Low-end / mobile |
| 50×50 | 2,601 | Mid-range |
| 100×100 | 10,201 | High-end (default) |
| 200×200 | 40,401 | Ultra (4K rendering) |

### Deformation API
```cpp
// Deform a single vertex
Stage->DeformFloorVertex(VertexIndex, HeightOffset);

// Batch update entire floor (preferred)
TArray<float> HeightMap;
HeightMap.SetNum(NumVertices);
// ... fill heights ...
Stage->UpdateFloorDeformation(HeightMap);
```

The `UpdateFloorDeformation` function automatically recalculates normals via finite differences.

---

## Wall Configuration

Each wall is independently configurable:

```cpp
FWallConfig Config;
Config.PatternColorA = FLinearColor(1, 0, 0);  // Red
Config.PatternColorB = FLinearColor(0, 0, 1);  // Blue
Config.EmissiveIntensityA = 80.0f;
Config.EmissiveIntensityB = 80.0f;
Config.EmissionBlend = 0.5f;
Config.AlphaMaskTexture = MyCheckerboardTexture;
Config.TextureTiling = FVector2D(4, 4);

Stage->SetWallConfig(EWallID::Front, Config);
```

### Quick Access Functions
```cpp
Stage->SetWallEmissionBlend(EWallID::Front, 0.7f);
Stage->SetWallColors(EWallID::Left, Red, Blue);
Stage->SetWallTexture(EWallID::Back, MyTexture);
Stage->SetAllWallsEmissionBlend(0.5f);  // All walls at once
```

---

## Light Strips

Vertical LED-style strips on Left and Right walls with chase animation.

```cpp
FLightStripConfig StripConfig;
StripConfig.StripColor = FLinearColor(1, 0, 0.5f);  // Magenta
StripConfig.Intensity = 60.0f;
StripConfig.ChaseSpeed = 3.0f;
StripConfig.bEnabled = true;

Stage->SetLightStripConfig(StripConfig);
Stage->ToggleLightStrips();  // On/off toggle
```

| Property | Default | Description |
|----------|---------|-------------|
| LightStripCount | 16 | Strips per side (32 total) |
| ChaseSpeed | 2.0 | Animation speed (Hz-like) |
| Intensity | 50 | Emissive brightness |

---

## Ceiling

The ceiling is optional and fully programmable like the walls:

```cpp
Stage->SetCeilingEnabled(true);   // Show ceiling
Stage->SetCeilingEnabled(false);  // Open top

// Configure ceiling colors/pattern via EWallID::Ceiling
Stage->SetWallConfig(EWallID::Ceiling, CeilingConfig);
```

---

## Runtime Rebuild

If you change stage dimensions at runtime:
```cpp
Stage->StageWidth = 1500.0f;
Stage->StageHeight = 600.0f;
Stage->RebuildStage();  // Reconstructs all geometry
```

This destroys and recreates wall/ceiling/strip components, so call sparingly.

---

## Performance Tips

1. **Floor resolution**: Start at 50×50, increase only if needed
2. **Light strips**: Reduce count on lower-end hardware (8 instead of 16)
3. **UpdateFloorDeformation**: Batch updates are much faster than per-vertex calls
4. **Material complexity**: Keep alpha mask textures small (256×256 or 512×512)
5. **Ceiling**: Disable if not visible in your camera angles to save draw calls
