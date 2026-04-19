#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WidgetPointerVisualizerComponent.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class UNiagaraComponent;
class UNiagaraSystem;
class USceneComponent;
class UStaticMesh;
class UStaticMeshComponent;
class UWidgetInteractionComponent;

UCLASS(ClassGroup = (VR), meta = (BlueprintSpawnableComponent, DisplayName = "Widget Pointer Visualizer"))
class MYPROJECT2_API UWidgetPointerVisualizerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWidgetPointerVisualizerComponent();

	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "VR|Widget Pointer Visualizer")
	void SetPointerReferences(USceneComponent* InPointerOrigin, UWidgetInteractionComponent* InWidgetInteraction);

	UFUNCTION(BlueprintCallable, Category = "VR|Widget Pointer Visualizer")
	void SetVisualizationEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "VR|Widget Pointer Visualizer")
	void RefreshVisualization();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR|Widget Pointer Visualizer", meta = (DisplayName = "Pointer Origin", ToolTip = "Optional origin for the pointer beam. If left empty, the Widget Interaction component itself is used."))
	TObjectPtr<USceneComponent> PointerOrigin;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR|Widget Pointer Visualizer", meta = (DisplayName = "Widget Interaction", ToolTip = "Widget Interaction component that provides hit testing and interaction distance."))
	TObjectPtr<UWidgetInteractionComponent> WidgetInteraction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR|Widget Pointer Visualizer|Beam", meta = (DisplayName = "Pointer Niagara System", ToolTip = "Niagara system that renders the pointer beam. It should accept a vector array user parameter named User.PointArray."))
	TObjectPtr<UNiagaraSystem> PointerSystem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR|Widget Pointer Visualizer|Cursor", meta = (DisplayName = "Cursor Material", ToolTip = "Optional cursor material applied to the generated sphere mesh."))
	TObjectPtr<UMaterialInterface> CursorMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR|Widget Pointer Visualizer|Beam", meta = (ClampMin = "0.0"))
	float PointerDistanceScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR|Widget Pointer Visualizer|Cursor", meta = (ClampMin = "0.0"))
	float CursorHitOffset = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR|Widget Pointer Visualizer")
	bool bVisualizationEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR|Widget Pointer Visualizer|Beam")
	FName BeamColorParameterName = TEXT("BeamColor");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR|Widget Pointer Visualizer|Cursor")
	FName CursorColorParameterName = TEXT("Color");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR|Widget Pointer Visualizer|Beam")
	FLinearColor BeamDefaultColor = FLinearColor(0.08f, 0.7f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR|Widget Pointer Visualizer|Beam")
	FLinearColor BeamHoverColor = FLinearColor(0.2f, 1.0f, 0.55f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR|Widget Pointer Visualizer|Cursor")
	FLinearColor CursorDefaultColor = FLinearColor(0.08f, 0.7f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR|Widget Pointer Visualizer|Cursor")
	FLinearColor CursorHoverColor = FLinearColor(0.2f, 1.0f, 0.55f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR|Widget Pointer Visualizer|Cursor")
	FVector CursorScale = FVector(0.015f);

private:
	void EnsureVisualComponentsCreated();
	void UpdateVisualization();
	void HideVisualization();
	void RefreshTickState();
	void TryResolveMissingReferences();
	USceneComponent* GetEffectivePointerOrigin() const;
	UNiagaraComponent* CreateEffectComponent();
	UStaticMeshComponent* CreateCursorComponent();
	void ReleaseEffectComponent();
	void ReleaseCursorComponent();
	void UpdateCursorMaterial();

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> PointerEffectComponent;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> CursorComponent;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> CursorMaterialInstance;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMesh> DefaultCursorMesh;
};
