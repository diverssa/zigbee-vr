#pragma once

#include "Components/Widget.h"
#include "Styling/SlateBrush.h"
#include "LampStyledSlider.generated.h"

class SLampStyledSlider;
class UMaterialInstanceDynamic;
class UMaterialInterface;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLampStyledSliderValueChanged, float, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLampStyledSliderCaptureEvent);

UCLASS(meta = (DisplayName = "Lamp Styled Slider"))
class MYPROJECT2_API ULampStyledSlider : public UWidget
{
    GENERATED_BODY()

public:
    ULampStyledSlider(const FObjectInitializer& ObjectInitializer);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Value;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "0.001", ClampMax = "1.0"))
    float StepSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "1.0"))
    float TrackHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FVector2D ThumbSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FSlateBrush TrackBackgroundBrush;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FSlateBrush TrackFillBrush;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FSlateBrush ThumbBrush;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bShowTrackBackground;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bFillToValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance|Material")
    bool bDriveFillMaterialByValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance|Material")
    FName FillPercentParameterName;

    UPROPERTY(BlueprintAssignable, Category = "Widget Event")
    FOnLampStyledSliderValueChanged OnValueChanged;

    UPROPERTY(BlueprintAssignable, Category = "Widget Event")
    FOnLampStyledSliderCaptureEvent OnCaptureStarted;

    UPROPERTY(BlueprintAssignable, Category = "Widget Event")
    FOnLampStyledSliderCaptureEvent OnCaptureEnded;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetValue(float InValue);

    UFUNCTION(BlueprintPure, Category = "Behavior")
    float GetValue() const;

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void SynchronizeProperties() override;
    virtual void ReleaseSlateResources(bool bReleaseChildren) override;

#if WITH_EDITOR
    virtual const FText GetPaletteCategory() override;
#endif

private:
    void HandleSlateValueChanged(float InValue);
    void HandleSlateCaptureStarted();
    void HandleSlateCaptureEnded();
    void RefreshDynamicFillMaterial();
    void UpdateDynamicFillMaterialValue();

private:
    TSharedPtr<SLampStyledSlider> MySlider;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInstanceDynamic> DynamicTrackFillMaterial;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInterface> DynamicTrackFillMaterialSource;

    FSlateBrush RuntimeTrackFillBrush;
};
