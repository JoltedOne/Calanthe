# LitCubeStage — Material System Guide

## M_CubePanel_Master Material

This is the core material used for all stage surfaces. It implements a dual-color emission system driven by B&W alpha mask textures.

---

## Material Creation

1. Content Browser → Right-click → Material → Name: `M_CubePanel_Master`
2. Open the material editor
3. Set properties:
   - **Material Domain**: Surface
   - **Blend Mode**: Opaque
   - **Shading Model**: Unlit (pure emissive, no lighting needed)

---

## Parameters

Create these material parameters (right-click → Create Parameter):

### Texture Parameters
| Name | Type | Default | Description |
|------|------|---------|-------------|
| AlphaMaskTexture | Texture2D | White | B&W pattern texture |

### Color Parameters
| Name | Type | Default | Description |
|------|------|---------|-------------|
| PatternColorA | LinearColor | (1,0,0,1) Red | Emission color for black areas |
| PatternColorB | LinearColor | (0,0,1,1) Blue | Emission color for white areas |

### Scalar Parameters
| Name | Type | Default | Range | Description |
|------|------|---------|-------|-------------|
| EmissiveIntensityA | Scalar | 80 | 0-100 | Brightness of ColorA emission |
| EmissiveIntensityB | Scalar | 80 | 0-100 | Brightness of ColorB emission |
| EmissionBlend | Scalar | 0 | 0-1 | Mix between A and B (0=A full, 1=B full) |

### Vector Parameters
| Name | Type | Default | Description |
|------|------|---------|-------------|
| TextureTiling | Vector2D | (1,1) | UV repeat scale |

---

## Node Graph

### Step 1: Texture Sampling
```
TextureCoordinate → Multiply(TextureTiling) → AlphaMaskTexture Sampler → R channel → "AlphaMask"
```

### Step 2: Color A Emission
```
PatternColorA × EmissiveIntensityA × AlphaMask → "EmissionA"
```

### Step 3: Color B Emission
```
PatternColorB × EmissiveIntensityB × (1 - AlphaMask) → "EmissionB"
```

### Step 4: Blend
```
Lerp(EmissionA, EmissionB, EmissionBlend) → Emissive Color output
```

### Complete Graph (pseudo-code):
```hlsl
float AlphaMask = Texture2DSample(AlphaMaskTexture, UV * TextureTiling).r;

float3 EmissionA = PatternColorA.rgb * EmissiveIntensityA * AlphaMask;
float3 EmissionB = PatternColorB.rgb * EmissiveIntensityB * (1.0 - AlphaMask);

float3 FinalEmission = lerp(EmissionA, EmissionB, EmissionBlend);

// Output
EmissiveColor = FinalEmission;
```

---

## Material Instances

Create Material Instances for each surface:

1. Right-click `M_CubePanel_Master` → Create Material Instance
2. Name per use case:
   - `MI_Wall_Front`
   - `MI_Wall_Back`
   - `MI_Wall_Left`
   - `MI_Wall_Right`
   - `MI_Ceiling`
   - `MI_Floor`

Each instance can have different textures, colors, and blend values — all controlled at runtime via the C++ `SetWallConfig` system.

---

## Light Strip Material

### M_LightStrip
A simpler emissive material for the LED strips:

**Parameters:**
| Name | Type | Default |
|------|------|---------|
| StripColor | LinearColor | (1, 0, 0.5) Magenta |
| Intensity | Scalar | 50 |

**Graph:**
```hlsl
EmissiveColor = StripColor.rgb * Intensity;
```

---

## Example Patterns

### Checkerboard
- Texture: 2×2 pixel B&W checkerboard, set to Nearest filtering
- TextureTiling: (8, 8) for 8×8 grid
- ColorA: Red, ColorB: Blue
- Result: Alternating red/blue emissive squares

### Horizontal Stripes
- Texture: Vertical gradient (black top, white bottom)
- TextureTiling: (1, 4) for 4 horizontal bands
- ColorA: Magenta, ColorB: Cyan

### Noise Pattern
- Texture: Perlin noise exported as B&W
- TextureTiling: (2, 2)
- Enable auto-animate blend for organic pulsing

### Audio Spectrum
- Texture: Vertical bars (like equalizer)
- TextureTiling: (1, 1)
- Use EmissionBlend driven by audio bass level

---

## Runtime Material Updates

The `LitCubeStage` C++ class handles material parameter updates automatically. When you call:

```cpp
Stage->SetWallConfig(EWallID::Front, Config);
```

It internally calls:
```cpp
MID->SetVectorParameterValue("PatternColorA", Config.PatternColorA);
MID->SetVectorParameterValue("PatternColorB", Config.PatternColorB);
MID->SetScalarParameterValue("EmissiveIntensityA", Config.EmissiveIntensityA);
MID->SetScalarParameterValue("EmissiveIntensityB", Config.EmissiveIntensityB);
MID->SetScalarParameterValue("EmissionBlend", Config.EmissionBlend);
MID->SetTextureParameterValue("AlphaMaskTexture", Config.AlphaMaskTexture);
```

All updates are immediate — no recompilation needed.

---

## Post-Process Recommendations

For best visual results with emissive materials:

| Setting | Value | Reason |
|---------|-------|--------|
| Bloom Intensity | 1.5 | Glow from bright emissives |
| Bloom Threshold | 0.0 | Everything blooms proportionally |
| Auto Exposure | OFF | Manual control prevents flicker |
| Motion Blur | OFF | Emissive flicker artifacts |
| Tonemapper Sharpen | 1.0 | Crisp pattern edges |
