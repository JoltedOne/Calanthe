#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "LitCubeStage.generated.h"

UENUM(BlueprintType)
enum class EWallID : uint8
{
	Front   UMETA(DisplayName = "Front Wall"),
	Back    UMETA(DisplayName = "Back Wall"),
	Left    UMETA(DisplayName = "Left Wall"),
	Right   UMETA(DisplayName = "Right Wall"),
	Ceiling UMETA(DisplayName = "Ceiling"),
	Floor   UMETA(DisplayName = "Floor")
};

USTRUCT(BlueprintType)
struct FWallConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PatternColorA = FLinearColor(1.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PatternColorB = FLinearColor(0.0f, 0.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float EmissiveIntensityA = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float EmissiveIntensityB = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EmissionBlend = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D TextureTiling = FVector2D(1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> AlphaMaskTexture;
};

USTRUCT(BlueprintType)
struct FLightStripConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor StripColor = FLinearColor(1.0f, 0.0f, 0.5f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float Intensity = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float ChaseSpeed = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled = true;
};

UCLASS(BlueprintType, Blueprintable)
class CALANTHE_API ALitCubeStage : public AActor
{
	GENERATED_BODY()

public:
	ALitCubeStage();

	virtual void Tick(float DeltaTime) override;

	// --- Stage Dimensions ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VJ|Stage", meta = (ClampMin = "100.0"))
	float StageWidth = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VJ|Stage", meta = (ClampMin = "100.0"))
	float StageDepth = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VJ|Stage", meta = (ClampMin = "100.0"))
	float StageHeight = 500.0f;

	// --- Floor Grid ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VJ|Stage|Floor", meta = (ClampMin = "4", ClampMax = "200"))
	int32 FloorGridResolution = 100;

	// --- Wall Control ---

	UFUNCTION(BlueprintCallable, Category = "VJ|Stage")
	void SetWallConfig(EWallID Wall, const FWallConfig& Config);

	UFUNCTION(BlueprintCallable, Category = "VJ|Stage")
	FWallConfig GetWallConfig(EWallID Wall) const;

	UFUNCTION(BlueprintCallable, Category = "VJ|Stage")
	void SetWallEmissionBlend(EWallID Wall, float Blend);

	UFUNCTION(BlueprintCallable, Category = "VJ|Stage")
	void SetWallColors(EWallID Wall, FLinearColor ColorA, FLinearColor ColorB);

	UFUNCTION(BlueprintCallable, Category = "VJ|Stage")
	void SetWallTexture(EWallID Wall, UTexture2D* Texture);

	UFUNCTION(BlueprintCallable, Category = "VJ|Stage")
	void SetAllWallsEmissionBlend(float Blend);

	// --- Ceiling ---

	UFUNCTION(BlueprintCallable, Category = "VJ|Stage")
	void SetCeilingEnabled(bool bEnabled);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VJ|Stage|Ceiling")
	bool bCeilingEnabled = true;

	// --- Light Strips ---

	UFUNCTION(BlueprintCallable, Category = "VJ|Stage")
	void SetLightStripConfig(const FLightStripConfig& Config);

	UFUNCTION(BlueprintCallable, Category = "VJ|Stage")
	void ToggleLightStrips();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VJ|Stage|LightStrips")
	FLightStripConfig LightStrips;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VJ|Stage|LightStrips")
	int32 LightStripCount = 16;

	// --- Floor Deformation Access ---

	UFUNCTION(BlueprintCallable, Category = "VJ|Stage")
	UProceduralMeshComponent* GetFloorMesh() const { return FloorMesh; }

	UFUNCTION(BlueprintCallable, Category = "VJ|Stage")
	void DeformFloorVertex(int32 Index, float HeightOffset);

	UFUNCTION(BlueprintCallable, Category = "VJ|Stage")
	void UpdateFloorDeformation(const TArray<float>& HeightMap);

	UFUNCTION(BlueprintCallable, Category = "VJ|Stage")
	void RebuildStage();

	// --- Wall Material Instances ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VJ|Stage|Materials")
	TObjectPtr<UMaterialInterface> WallBaseMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VJ|Stage|Materials")
	TObjectPtr<UMaterialInterface> FloorBaseMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VJ|Stage|Materials")
	TObjectPtr<UMaterialInterface> LightStripBaseMaterial;

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	void BuildFloorMesh();
	void BuildWalls();
	void BuildCeiling();
	void BuildLightStrips();
	void UpdateLightStripChase(float DeltaTime);
	void ApplyWallMaterialConfig(EWallID Wall);

	UPROPERTY()
	TObjectPtr<UProceduralMeshComponent> FloorMesh;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> WallMeshes;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> CeilingMesh;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> LightStripMeshes;

	UPROPERTY()
	TMap<EWallID, FWallConfig> WallConfigs;

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> WallMaterialInstances;

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> LightStripMaterialInstances;

	// Floor mesh data cache for deformation updates
	TArray<FVector> FloorVertices;
	TArray<FVector> FloorBaseVertices;
	TArray<int32> FloorTriangles;
	TArray<FVector> FloorNormals;
	TArray<FVector2D> FloorUVs;
	TArray<FLinearColor> FloorColors;

	float LightChasePhase = 0.0f;
};
