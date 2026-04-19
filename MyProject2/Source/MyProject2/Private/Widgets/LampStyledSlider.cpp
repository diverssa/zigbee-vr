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
    , TrackInset(8.0f)
    , ThumbTravelInset(8.0f)
    , ThumbSize(18.0f, 28.0f)
    , bShowTrackBackground(true)
    , bShowTrackFill(true)
    , bFillToValue(true)
    , bDriveFillMaterialByValue(false)
    , FillPercentParameterName(TEXT("Percent"))
{
    RuntimeTrackBackgroundBrush = TrackBackgroundBrush;
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

float ULampStyledSlider::GetValue() const
{
    return Value;
}

UMaterialInstanceDynamic* ULampStyledSlider::GetTrackBackgroundDynamicMaterial()
{
    RefreshDynamicBackgroundMaterial();
    return DynamicTrackBackgroundMaterial;
}

void ULampStyledSlider::SetTrackBackgroundScalarParameter(FName ParameterName, float InValue)
{
    RefreshDynamicBackgroundMaterial();
    if (DynamicTrackBackgroundMaterial)
    {
        DynamicTrackBackgroundMaterial->SetScalarParameterValue(ParameterName, InValue);
        if (MySlider.IsValid())
        {
            MySlider->SetTrackBackgroundBrush(&RuntimeTrackBackgroundBrush);
        }
    }
}

void ULampStyledSlider::SetTrackBackgroundVectorParameter(FName ParameterName, FLinearColor InValue)
{
    RefreshDynamicBackgroundMaterial();
    if (DynamicTrackBackgroundMaterial)
    {
        DynamicTrackBackgroundMaterial->SetVectorParameterValue(ParameterName, InValue);
        if (MySlider.IsValid())
        {
            MySlider->SetTrackBackgroundBrush(&RuntimeTrackBackgroundBrush);
        }
    }
}

TSharedRef<SWidget> ULampStyledSlider::RebuildWidget()
{
    RefreshDynamicBackgroundMaterial();
    RefreshDynamicFillMaterial();

    MySlider = SNew(SLampStyledSlider)
        .Value(Value)
        .StepSize(StepSize)
        .TrackHeight(TrackHeight)
        .TrackInset(TrackInset)
        .ThumbTravelInset(ThumbTravelInset)
        .ThumbSize(ThumbSize)
        .TrackBackgroundBrush(&RuntimeTrackBackgroundBrush)
        .TrackFillBrush(&RuntimeTrackFillBrush)
        .ThumbBrush(&ThumbBrush)
        .bShowTrackBackground(bShowTrackBackground)
        .bShowTrackFill(bShowTrackFill)
        .bFillToValue(bFillToValue)
        .OnValueChanged(FOnLampStyledSliderValueChangedNative::CreateUObject(this, &ULampStyledSlider::HandleSlateValueChanged))
        .OnCaptureStarted(FOnLampStyledSliderCaptureNative::CreateUObject(this, &ULampStyledSlider::HandleSlateCaptureStarted))
        .OnCaptureEnded(FOnLampStyledSliderCaptureNative::CreateUObject(this, &ULampStyledSlider::HandleSlateCaptureEnded));

    return MySlider.ToSharedRef();
}

void ULampStyledSlider::SynchronizeProperties()
{
    Super::SynchronizeProperties();

    RefreshDynamicBackgroundMaterial();
    RefreshDynamicFillMaterial();
    UpdateDynamicFillMaterialValue();

    if (!MySlider.IsValid())
    {
        return;
    }

    MySlider->SetValue(Value);
    MySlider->SetStepSize(StepSize);
    MySlider->SetTrackHeight(TrackHeight);
    MySlider->SetTrackInset(TrackInset);
    MySlider->SetThumbTravelInset(ThumbTravelInset);
    MySlider->SetThumbSize(ThumbSize);
    MySlider->SetTrackBackgroundBrush(&RuntimeTrackBackgroundBrush);
    MySlider->SetTrackFillBrush(&RuntimeTrackFillBrush);
    MySlider->SetThumbBrush(&ThumbBrush);
    MySlider->SetShowTrackBackground(bShowTrackBackground);
    MySlider->SetShowTrackFill(bShowTrackFill);
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

void ULampStyledSlider::RefreshDynamicBackgroundMaterial()
{
    RuntimeTrackBackgroundBrush = TrackBackgroundBrush;

    UObject* Resource = TrackBackgroundBrush.GetResourceObject();
    UMaterialInterface* Material = Cast<UMaterialInterface>(Resource);
    if (!Material)
    {
        DynamicTrackBackgroundMaterial = nullptr;
        DynamicTrackBackgroundMaterialSource = nullptr;
        return;
    }

    if (!DynamicTrackBackgroundMaterial || DynamicTrackBackgroundMaterialSource != Material)
    {
        DynamicTrackBackgroundMaterial = UMaterialInstanceDynamic::Create(Material, this);
        DynamicTrackBackgroundMaterialSource = Material;
    }

    RuntimeTrackBackgroundBrush.SetResourceObject(DynamicTrackBackgroundMaterial);
}

void ULampStyledSlider::RefreshDynamicFillMaterial()
{
    RuntimeTrackFillBrush = TrackFillBrush;

    if (!bDriveFillMaterialByValue)
    {
        DynamicTrackFillMaterial = nullptr;
        DynamicTrackFillMaterialSource = nullptr;
        return;
    }

    UObject* Resource = TrackFillBrush.GetResourceObject();
    UMaterialInterface* Material = Cast<UMaterialInterface>(Resource);
    if (!Material)
    {
        DynamicTrackFillMaterial = nullptr;
        DynamicTrackFillMaterialSource = nullptr;
        return;
    }

    if (!DynamicTrackFillMaterial || DynamicTrackFillMaterialSource != Material)
    {
        DynamicTrackFillMaterial = UMaterialInstanceDynamic::Create(Material, this);
        DynamicTrackFillMaterialSource = Material;
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
