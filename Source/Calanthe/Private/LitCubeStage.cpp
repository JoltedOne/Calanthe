#include "LitCubeStage.h"
#include "Calanthe.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ALitCubeStage::ALitCubeStage()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	FloorMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("FloorMesh"));
	FloorMesh->SetupAttachment(Root);
	FloorMesh->bUseAsyncCooking = true;

	// Load engine default plane mesh for walls and ceiling
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneFinder(TEXT("/Engine/BasicShapes/Plane"));
	if (PlaneFinder.Succeeded())
	{
		PlaneMesh = PlaneFinder.Object;
	}

	// Initialize default wall configs
	WallConfigs.Add(EWallID::Front, FWallConfig());
	WallConfigs.Add(EWallID::Back, FWallConfig());
	WallConfigs.Add(EWallID::Left, FWallConfig());
	WallConfigs.Add(EWallID::Right, FWallConfig());
	WallConfigs.Add(EWallID::Ceiling, FWallConfig());
	WallConfigs.Add(EWallID::Floor, FWallConfig());
}

void ALitCubeStage::BeginPlay()
{
	Super::BeginPlay();
	RebuildStage();
}

void ALitCubeStage::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RebuildStage();
}

void ALitCubeStage::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateLightStripChase(DeltaTime);
}

void ALitCubeStage::RebuildStage()
{
	BuildFloorMesh();
	BuildWalls();
	BuildCeiling();
	BuildLightStrips();
}

// =============================================================
// FLOOR MESH - Procedural grid for deformation
// =============================================================

void ALitCubeStage::BuildFloorMesh()
{
	if (!FloorMesh) return;

	const int32 Res = FloorGridResolution;
	const float HalfW = StageWidth * 0.5f;
	const float HalfD = StageDepth * 0.5f;
	const int32 VertCount = (Res + 1) * (Res + 1);
	const int32 TriCount = Res * Res * 6;

	FloorVertices.Empty(VertCount);
	FloorBaseVertices.Empty(VertCount);
	FloorTriangles.Empty(TriCount);
	FloorNormals.Empty(VertCount);
	FloorUVs.Empty(VertCount);
	FloorColors.Empty(VertCount);

	// Generate vertices
	for (int32 Y = 0; Y <= Res; Y++)
	{
		for (int32 X = 0; X <= Res; X++)
		{
			const float U = static_cast<float>(X) / Res;
			const float V = static_cast<float>(Y) / Res;

			FVector Pos(
				-HalfW + U * StageWidth,
				-HalfD + V * StageDepth,
				0.0f
			);

			FloorVertices.Add(Pos);
			FloorBaseVertices.Add(Pos);
			FloorNormals.Add(FVector::UpVector);
			FloorUVs.Add(FVector2D(U, V));
			FloorColors.Add(FLinearColor::White);
		}
	}

	// Generate triangles
	for (int32 Y = 0; Y < Res; Y++)
	{
		for (int32 X = 0; X < Res; X++)
		{
			const int32 BL = Y * (Res + 1) + X;
			const int32 BR = BL + 1;
			const int32 TL = BL + (Res + 1);
			const int32 TR = TL + 1;

			FloorTriangles.Add(BL);
			FloorTriangles.Add(TL);
			FloorTriangles.Add(BR);

			FloorTriangles.Add(BR);
			FloorTriangles.Add(TL);
			FloorTriangles.Add(TR);
		}
	}

	FloorMesh->CreateMeshSection_LinearColor(
		0,
		FloorVertices,
		FloorTriangles,
		FloorNormals,
		FloorUVs,
		FloorColors,
		TArray<FProcMeshTangent>(),
		true
	);

	if (FloorBaseMaterial)
	{
		UMaterialInstanceDynamic* FloorMID = UMaterialInstanceDynamic::Create(FloorBaseMaterial, this);
		FloorMesh->SetMaterial(0, FloorMID);

		const FWallConfig& FloorCfg = WallConfigs[EWallID::Floor];
		FloorMID->SetVectorParameterValue(TEXT("PatternColorA"), FloorCfg.PatternColorA);
		FloorMID->SetVectorParameterValue(TEXT("PatternColorB"), FloorCfg.PatternColorB);
		FloorMID->SetScalarParameterValue(TEXT("EmissiveIntensityA"), FloorCfg.EmissiveIntensityA);
		FloorMID->SetScalarParameterValue(TEXT("EmissiveIntensityB"), FloorCfg.EmissiveIntensityB);
		FloorMID->SetScalarParameterValue(TEXT("EmissionBlend"), FloorCfg.EmissionBlend);
	}
}

// =============================================================
// WALLS - 4 independent panels
// =============================================================

void ALitCubeStage::BuildWalls()
{
	// Clean up existing
	for (auto& WallMesh : WallMeshes)
	{
		if (WallMesh) WallMesh->DestroyComponent();
	}
	WallMeshes.Empty();
	WallMaterialInstances.Empty();

	const float HalfW = StageWidth * 0.5f;
	const float HalfD = StageDepth * 0.5f;
	const float HalfH = StageHeight * 0.5f;

	struct FWallDef
	{
		EWallID ID;
		FVector Location;
		FRotator Rotation;
		FVector Scale;
	};

	// Wall definitions: location, rotation, and scale relative to a unit plane
	TArray<FWallDef> Defs = {
		{ EWallID::Front,  FVector(0, -HalfD, HalfH),  FRotator(90, 0, 0),    FVector(StageWidth, 1, StageHeight) },
		{ EWallID::Back,   FVector(0, HalfD, HalfH),   FRotator(-90, 0, 0),   FVector(StageWidth, 1, StageHeight) },
		{ EWallID::Left,   FVector(-HalfW, 0, HalfH),  FRotator(0, 0, -90),   FVector(1, StageDepth, StageHeight) },
		{ EWallID::Right,  FVector(HalfW, 0, HalfH),   FRotator(0, 0, 90),    FVector(1, StageDepth, StageHeight) },
	};

	for (const FWallDef& Def : Defs)
	{
		UStaticMeshComponent* Wall = NewObject<UStaticMeshComponent>(this);
		Wall->SetupAttachment(RootComponent);
		Wall->RegisterComponent();
		Wall->SetRelativeLocation(Def.Location);
		Wall->SetRelativeRotation(Def.Rotation);
		Wall->SetRelativeScale3D(Def.Scale * 0.01f); // Scale for unit plane mesh
		Wall->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		if (PlaneMesh)
		{
			Wall->SetStaticMesh(PlaneMesh);
		}

		if (WallBaseMaterial)
		{
			UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(WallBaseMaterial, this);
			Wall->SetMaterial(0, MID);
			WallMaterialInstances.Add(MID);
			ApplyWallMaterialConfig(Def.ID);
		}
		else
		{
			WallMaterialInstances.Add(nullptr);
		}

		WallMeshes.Add(Wall);
	}
}

// =============================================================
// CEILING
// =============================================================

void ALitCubeStage::BuildCeiling()
{
	if (CeilingMesh)
	{
		CeilingMesh->DestroyComponent();
		CeilingMesh = nullptr;
	}

	if (!bCeilingEnabled) return;

	CeilingMesh = NewObject<UStaticMeshComponent>(this);
	CeilingMesh->SetupAttachment(RootComponent);
	CeilingMesh->RegisterComponent();
	CeilingMesh->SetRelativeLocation(FVector(0, 0, StageHeight));
	CeilingMesh->SetRelativeRotation(FRotator(180, 0, 0));
	CeilingMesh->SetRelativeScale3D(FVector(StageWidth, StageDepth, 1) * 0.01f);
	CeilingMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (PlaneMesh)
	{
		CeilingMesh->SetStaticMesh(PlaneMesh);
	}

	if (WallBaseMaterial)
	{
		UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(WallBaseMaterial, this);
		CeilingMesh->SetMaterial(0, MID);

		const FWallConfig& CeilCfg = WallConfigs[EWallID::Ceiling];
		MID->SetVectorParameterValue(TEXT("PatternColorA"), CeilCfg.PatternColorA);
		MID->SetVectorParameterValue(TEXT("PatternColorB"), CeilCfg.PatternColorB);
		MID->SetScalarParameterValue(TEXT("EmissiveIntensityA"), CeilCfg.EmissiveIntensityA);
		MID->SetScalarParameterValue(TEXT("EmissiveIntensityB"), CeilCfg.EmissiveIntensityB);
		MID->SetScalarParameterValue(TEXT("EmissionBlend"), CeilCfg.EmissionBlend);
	}
}

// =============================================================
// LIGHT STRIPS - vertical LED chase on side walls
// =============================================================

void ALitCubeStage::BuildLightStrips()
{
	for (auto& Strip : LightStripMeshes)
	{
		if (Strip) Strip->DestroyComponent();
	}
	LightStripMeshes.Empty();
	LightStripMaterialInstances.Empty();

	if (!LightStrips.bEnabled || LightStripCount <= 0) return;

	const float HalfW = StageWidth * 0.5f;
	const float HalfD = StageDepth * 0.5f;
	const float StripSpacing = StageDepth / (LightStripCount + 1);
	const float StripWidth = StripSpacing * 0.15f;
	const float StripHeight = StageHeight * 0.9f;

	// Place strips on both left and right walls
	for (int32 Side = 0; Side < 2; Side++)
	{
		const float XPos = (Side == 0) ? -HalfW + 1.0f : HalfW - 1.0f;
		const float YawRot = (Side == 0) ? 90.0f : -90.0f;

		for (int32 i = 0; i < LightStripCount; i++)
		{
			const float YPos = -HalfD + StripSpacing * (i + 1);

			UStaticMeshComponent* Strip = NewObject<UStaticMeshComponent>(this);
			Strip->SetupAttachment(RootComponent);
			Strip->RegisterComponent();
			Strip->SetRelativeLocation(FVector(XPos, YPos, StageHeight * 0.5f));
			Strip->SetRelativeRotation(FRotator(0, YawRot, 0));
			Strip->SetRelativeScale3D(FVector(StripWidth, 1, StripHeight) * 0.01f);
			Strip->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			if (PlaneMesh)
			{
				Strip->SetStaticMesh(PlaneMesh);
			}

			if (LightStripBaseMaterial)
			{
				UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(LightStripBaseMaterial, this);
				Strip->SetMaterial(0, MID);
				MID->SetVectorParameterValue(TEXT("StripColor"), LightStrips.StripColor);
				MID->SetScalarParameterValue(TEXT("Intensity"), LightStrips.Intensity);
				LightStripMaterialInstances.Add(MID);
			}

			LightStripMeshes.Add(Strip);
		}
	}
}

void ALitCubeStage::UpdateLightStripChase(float DeltaTime)
{
	if (!LightStrips.bEnabled || LightStripMaterialInstances.Num() == 0) return;

	LightChasePhase += LightStrips.ChaseSpeed * DeltaTime;
	if (LightChasePhase > 1.0f) LightChasePhase -= 1.0f;

	const int32 TotalStrips = LightStripMaterialInstances.Num();
	const int32 StripsPerSide = TotalStrips / 2;

	for (int32 i = 0; i < TotalStrips; i++)
	{
		if (!LightStripMaterialInstances[i]) continue;

		const int32 StripIndex = i % StripsPerSide;
		const float NormPos = static_cast<float>(StripIndex) / StripsPerSide;

		// Chase wave — bright spot travels along strip row
		const float ChaseValue = FMath::Exp(-FMath::Square(FMath::Frac(NormPos - LightChasePhase) - 0.5f) * 20.0f);
		const float FinalIntensity = LightStrips.Intensity * (0.2f + 0.8f * ChaseValue);

		LightStripMaterialInstances[i]->SetScalarParameterValue(TEXT("Intensity"), FinalIntensity);
	}
}

// =============================================================
// WALL CONFIGURATION
// =============================================================

void ALitCubeStage::SetWallConfig(EWallID Wall, const FWallConfig& Config)
{
	WallConfigs.FindOrAdd(Wall) = Config;
	ApplyWallMaterialConfig(Wall);
}

FWallConfig ALitCubeStage::GetWallConfig(EWallID Wall) const
{
	if (const FWallConfig* Found = WallConfigs.Find(Wall))
	{
		return *Found;
	}
	return FWallConfig();
}

void ALitCubeStage::SetWallEmissionBlend(EWallID Wall, float Blend)
{
	if (FWallConfig* Cfg = WallConfigs.Find(Wall))
	{
		Cfg->EmissionBlend = FMath::Clamp(Blend, 0.0f, 1.0f);
		ApplyWallMaterialConfig(Wall);
	}
}

void ALitCubeStage::SetWallColors(EWallID Wall, FLinearColor ColorA, FLinearColor ColorB)
{
	if (FWallConfig* Cfg = WallConfigs.Find(Wall))
	{
		Cfg->PatternColorA = ColorA;
		Cfg->PatternColorB = ColorB;
		ApplyWallMaterialConfig(Wall);
	}
}

void ALitCubeStage::SetWallTexture(EWallID Wall, UTexture2D* Texture)
{
	if (FWallConfig* Cfg = WallConfigs.Find(Wall))
	{
		Cfg->AlphaMaskTexture = Texture;
		ApplyWallMaterialConfig(Wall);
	}
}

void ALitCubeStage::SetAllWallsEmissionBlend(float Blend)
{
	for (auto& Pair : WallConfigs)
	{
		Pair.Value.EmissionBlend = FMath::Clamp(Blend, 0.0f, 1.0f);
		ApplyWallMaterialConfig(Pair.Key);
	}
}

void ALitCubeStage::ApplyWallMaterialConfig(EWallID Wall)
{
	// Map wall ID to material instance index
	int32 MatIndex = INDEX_NONE;
	switch (Wall)
	{
	case EWallID::Front:   MatIndex = 0; break;
	case EWallID::Back:    MatIndex = 1; break;
	case EWallID::Left:    MatIndex = 2; break;
	case EWallID::Right:   MatIndex = 3; break;
	default: return; // Ceiling/Floor handled separately
	}

	if (!WallMaterialInstances.IsValidIndex(MatIndex) || !WallMaterialInstances[MatIndex]) return;

	UMaterialInstanceDynamic* MID = WallMaterialInstances[MatIndex];
	const FWallConfig& Cfg = WallConfigs[Wall];

	MID->SetVectorParameterValue(TEXT("PatternColorA"), Cfg.PatternColorA);
	MID->SetVectorParameterValue(TEXT("PatternColorB"), Cfg.PatternColorB);
	MID->SetScalarParameterValue(TEXT("EmissiveIntensityA"), Cfg.EmissiveIntensityA);
	MID->SetScalarParameterValue(TEXT("EmissiveIntensityB"), Cfg.EmissiveIntensityB);
	MID->SetScalarParameterValue(TEXT("EmissionBlend"), Cfg.EmissionBlend);
	MID->SetVectorParameterValue(TEXT("TextureTiling"), FLinearColor(Cfg.TextureTiling.X, Cfg.TextureTiling.Y, 0, 0));

	if (Cfg.AlphaMaskTexture)
	{
		MID->SetTextureParameterValue(TEXT("AlphaMaskTexture"), Cfg.AlphaMaskTexture);
	}
}

void ALitCubeStage::SetCeilingEnabled(bool bEnabled)
{
	bCeilingEnabled = bEnabled;
	BuildCeiling();
}

void ALitCubeStage::SetLightStripConfig(const FLightStripConfig& Config)
{
	LightStrips = Config;
	BuildLightStrips();
}

void ALitCubeStage::ToggleLightStrips()
{
	LightStrips.bEnabled = !LightStrips.bEnabled;
	BuildLightStrips();
}

// =============================================================
// FLOOR DEFORMATION
// =============================================================

void ALitCubeStage::DeformFloorVertex(int32 Index, float HeightOffset)
{
	if (!FloorVertices.IsValidIndex(Index)) return;

	FloorVertices[Index].Z = FloorBaseVertices[Index].Z + HeightOffset;
}

void ALitCubeStage::UpdateFloorDeformation(const TArray<float>& HeightMap)
{
	if (!FloorMesh) return;

	const int32 Count = FMath::Min(HeightMap.Num(), FloorVertices.Num());
	for (int32 i = 0; i < Count; i++)
	{
		FloorVertices[i].Z = FloorBaseVertices[i].Z + HeightMap[i];
	}

	// Recompute normals for deformed mesh
	const int32 Res = FloorGridResolution;
	for (int32 Y = 0; Y <= Res; Y++)
	{
		for (int32 X = 0; X <= Res; X++)
		{
			const int32 Idx = Y * (Res + 1) + X;
			FVector Normal = FVector::UpVector;

			// Finite difference normal computation
			if (X > 0 && X < Res && Y > 0 && Y < Res)
			{
				const FVector& Left = FloorVertices[Idx - 1];
				const FVector& Right = FloorVertices[Idx + 1];
				const FVector& Down = FloorVertices[Idx - (Res + 1)];
				const FVector& Up = FloorVertices[Idx + (Res + 1)];

				const FVector DX = Right - Left;
				const FVector DY = Up - Down;
				Normal = FVector::CrossProduct(DX, DY).GetSafeNormal();
				if (Normal.Z < 0) Normal = -Normal;
			}
			FloorNormals[Idx] = Normal;
		}
	}

	FloorMesh->UpdateMeshSection_LinearColor(
		0,
		FloorVertices,
		FloorNormals,
		FloorUVs,
		FloorColors,
		TArray<FProcMeshTangent>()
	);
}
