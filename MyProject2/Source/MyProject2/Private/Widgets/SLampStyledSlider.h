#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "Layout/Geometry.h"
#include "Styling/SlateBrush.h"
#include "Widgets/SLeafWidget.h"

DECLARE_DELEGATE_OneParam(FOnLampStyledSliderValueChangedNative, float)
DECLARE_DELEGATE(FOnLampStyledSliderCaptureNative)

class SLampStyledSlider : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(SLampStyledSlider)
        : _Value(0.0f)
        , _StepSize(0.01f)
        , _TrackHeight(20.0f)
        , _ThumbSize(FVector2D(18.0f, 28.0f))
        , _TrackBackgroundBrush(nullptr)
        , _TrackFillBrush(nullptr)
        , _ThumbBrush(nullptr)
        , _bShowTrackBackground(true)
        , _bFillToValue(true)
    {
    }
        SLATE_ARGUMENT(float, Value)
        SLATE_ARGUMENT(float, StepSize)
        SLATE_ARGUMENT(float, TrackHeight)
        SLATE_ARGUMENT(FVector2D, ThumbSize)
        SLATE_ARGUMENT(const FSlateBrush*, TrackBackgroundBrush)
        SLATE_ARGUMENT(const FSlateBrush*, TrackFillBrush)
        SLATE_ARGUMENT(const FSlateBrush*, ThumbBrush)
        SLATE_ARGUMENT(bool, bShowTrackBackground)
        SLATE_ARGUMENT(bool, bFillToValue)
        SLATE_EVENT(FOnLampStyledSliderValueChangedNative, OnValueChanged)
        SLATE_EVENT(FOnLampStyledSliderCaptureNative, OnCaptureStarted)
        SLATE_EVENT(FOnLampStyledSliderCaptureNative, OnCaptureEnded)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    void SetValue(float InValue);
    void SetStepSize(float InStepSize);
    void SetTrackHeight(float InTrackHeight);
    void SetThumbSize(FVector2D InThumbSize);
    void SetTrackBackgroundBrush(const FSlateBrush* InBrush);
    void SetTrackFillBrush(const FSlateBrush* InBrush);
    void SetThumbBrush(const FSlateBrush* InBrush);
    void SetShowTrackBackground(bool bInShowTrackBackground);
    void SetFillToValue(bool bInFillToValue);

    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
        FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle,
        bool bParentEnabled) const override;

    virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
    virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual void OnMouseCaptureLost(const FCaptureLostEvent& CaptureLostEvent) override;
    virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override;

private:
    float GetSnappedValue(float InValue) const;
    float PositionToNormalizedValue(const FGeometry& MyGeometry, float LocalX) const;
    void CommitValueFromLocalX(const FGeometry& MyGeometry, float LocalX);
    FSlateRect GetTrackRect(const FVector2D& LocalSize) const;

private:
    float Value = 0.0f;
    float StepSize = 0.01f;
    float TrackHeight = 20.0f;
    FVector2D ThumbSize = FVector2D(18.0f, 28.0f);

    const FSlateBrush* TrackBackgroundBrush = nullptr;
    const FSlateBrush* TrackFillBrush = nullptr;
    const FSlateBrush* ThumbBrush = nullptr;
    bool bShowTrackBackground = true;
    bool bFillToValue = true;

    FOnLampStyledSliderValueChangedNative OnValueChanged;
    FOnLampStyledSliderCaptureNative OnCaptureStarted;
    FOnLampStyledSliderCaptureNative OnCaptureEnded;
    bool bDragging = false;
};
