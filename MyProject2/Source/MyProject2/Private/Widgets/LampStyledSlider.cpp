#include "Widgets/LampStyledSlider.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Widgets/SLampStyledSlider.h"

#define LOCTEXT_NAMESPACE "LampStyledSlider"

ULampStyledSlider::ULampStyledSlider(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , Value(0.0f)
    , StepSize(0.01f)
    , TrackHeight(20.0f)
    , ThumbSize(18.0f, 28.0f)
    , bShowTrackBackground(true)
    , bFillToValue(true)
    , bDriveFillMaterialByValue(false)
    , FillPercentParameterName(TEXT("Percent"))
{
    RuntimeTrackFillBrush = TrackFillBrush;
}

void ULampStyledSlider::SetValue(float InValue)
{
    Value = FMath::Clamp(InValue, 0.0f, 1.0f);
    UpdateDynamicFillMaterialValue();
    if (MySlider.IsValid())
    {
        MySlider->SetValue(Value);
    }
}

TSharedRef<SWidget> ULampStyledSlider::RebuildWidget()
{
    RefreshDynamicFillMaterial();

    MySlider = SNew(SLampStyledSlider)
        .Value(Value)
        .StepSize(StepSize)
        .TrackHeight(TrackHeight)
        .ThumbSize(ThumbSize)
        .TrackBackgroundBrush(&TrackBackgroundBrush)
        .TrackFillBrush(&RuntimeTrackFillBrush)
        .ThumbBrush(&ThumbBrush)
        .bShowTrackBackground(bShowTrackBackground)
        .bFillToValue(bFillToValue)
        .OnValueChanged(FOnLampStyledSliderValueChangedNative::CreateUObject(this, &ULampStyledSlider::HandleSlateValueChanged))
        .OnCaptureStarted(FOnLampStyledSliderCaptureNative::CreateUObject(this, &ULampStyledSlider::HandleSlateCaptureStarted))
        .OnCaptureEnded(FOnLampStyledSliderCaptureNative::CreateUObject(this, &ULampStyledSlider::HandleSlateCaptureEnded));

    return MySlider.ToSharedRef();
}

void ULampStyledSlider::SynchronizeProperties()
{
    Super::SynchronizeProperties();

    RefreshDynamicFillMaterial();
    UpdateDynamicFillMaterialValue();

    if (!MySlider.IsValid())
    {
        return;
    }

    MySlider->SetValue(Value);
    MySlider->SetStepSize(StepSize);
    MySlider->SetTrackHeight(TrackHeight);
    MySlider->SetThumbSize(ThumbSize);
    MySlider->SetTrackBackgroundBrush(&TrackBackgroundBrush);
    MySlider->SetTrackFillBrush(&RuntimeTrackFillBrush);
    MySlider->SetThumbBrush(&ThumbBrush);
    MySlider->SetShowTrackBackground(bShowTrackBackground);
    MySlider->SetFillToValue(bFillToValue);
}

void ULampStyledSlider::ReleaseSlateResources(bool bReleaseChildren)
{
    Super::ReleaseSlateResources(bReleaseChildren);
    MySlider.Reset();
}

#if WITH_EDITOR
const FText ULampStyledSlider::GetPaletteCategory()
{
    return LOCTEXT("LampControlsPaletteCategory", "Lamp Controls");
}
#endif

void ULampStyledSlider::HandleSlateValueChanged(float InValue)
{
    Value = InValue;
    UpdateDynamicFillMaterialValue();
    OnValueChanged.Broadcast(InValue);
}

void ULampStyledSlider::HandleSlateCaptureStarted()
{
    OnCaptureStarted.Broadcast();
}

void ULampStyledSlider::HandleSlateCaptureEnded()
{
    OnCaptureEnded.Broadcast();
}

void ULampStyledSlider::RefreshDynamicFillMaterial()
{
    RuntimeTrackFillBrush = TrackFillBrush;

    if (!bDriveFillMaterialByValue)
    {
        DynamicTrackFillMaterial = nullptr;
        return;
    }

    UObject* Resource = TrackFillBrush.GetResourceObject();
    UMaterialInterface* Material = Cast<UMaterialInterface>(Resource);
    if (!Material)
    {
        DynamicTrackFillMaterial = nullptr;
        return;
    }

    if (!DynamicTrackFillMaterial || RuntimeTrackFillBrush.GetResourceObject() != DynamicTrackFillMaterial)
    {
        DynamicTrackFillMaterial = UMaterialInstanceDynamic::Create(Material, this);
    }

    RuntimeTrackFillBrush.SetResourceObject(DynamicTrackFillMaterial);
}

void ULampStyledSlider::UpdateDynamicFillMaterialValue()
{
    if (bDriveFillMaterialByValue && DynamicTrackFillMaterial)
    {
        DynamicTrackFillMaterial->SetScalarParameterValue(FillPercentParameterName, Value);
    }
}

#undef LOCTEXT_NAMESPACE
