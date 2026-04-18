#include "Widgets/SLampStyledSlider.h"

#include "Input/Events.h"
#include "Math/UnrealMathUtility.h"
#include "Rendering/DrawElements.h"
#include "Styling/AppStyle.h"

void SLampStyledSlider::Construct(const FArguments& InArgs)
{
    Value = FMath::Clamp(InArgs._Value, 0.0f, 1.0f);
    StepSize = FMath::Clamp(InArgs._StepSize, KINDA_SMALL_NUMBER, 1.0f);
    TrackHeight = FMath::Max(1.0f, InArgs._TrackHeight);
    TrackInset = FMath::Max(0.0f, InArgs._TrackInset);
    ThumbTravelInset = FMath::Max(0.0f, InArgs._ThumbTravelInset);
    ThumbSize = FVector2D(FMath::Max(InArgs._ThumbSize.X, 1.0f), FMath::Max(InArgs._ThumbSize.Y, 1.0f));
    TrackBackgroundBrush = InArgs._TrackBackgroundBrush;
    TrackFillBrush = InArgs._TrackFillBrush;
    ThumbBrush = InArgs._ThumbBrush;
    bShowTrackBackground = InArgs._bShowTrackBackground;
    bShowTrackFill = InArgs._bShowTrackFill;
    bFillToValue = InArgs._bFillToValue;
    OnValueChanged = InArgs._OnValueChanged;
    OnCaptureStarted = InArgs._OnCaptureStarted;
    OnCaptureEnded = InArgs._OnCaptureEnded;
}

void SLampStyledSlider::SetValue(float InValue)
{
    Value = FMath::Clamp(InValue, 0.0f, 1.0f);
    Invalidate(EInvalidateWidgetReason::Paint);
}

void SLampStyledSlider::SetStepSize(float InStepSize)
{
    StepSize = FMath::Clamp(InStepSize, KINDA_SMALL_NUMBER, 1.0f);
}

void SLampStyledSlider::SetTrackHeight(float InTrackHeight)
{
    TrackHeight = FMath::Max(1.0f, InTrackHeight);
    Invalidate(EInvalidateWidgetReason::LayoutAndVolatility);
}

void SLampStyledSlider::SetTrackInset(float InTrackInset)
{
    TrackInset = FMath::Max(0.0f, InTrackInset);
    Invalidate(EInvalidateWidgetReason::LayoutAndVolatility);
}

void SLampStyledSlider::SetThumbTravelInset(float InThumbTravelInset)
{
    ThumbTravelInset = FMath::Max(0.0f, InThumbTravelInset);
    Invalidate(EInvalidateWidgetReason::LayoutAndVolatility);
}

void SLampStyledSlider::SetThumbSize(FVector2D InThumbSize)
{
    ThumbSize = FVector2D(FMath::Max(InThumbSize.X, 1.0f), FMath::Max(InThumbSize.Y, 1.0f));
    Invalidate(EInvalidateWidgetReason::LayoutAndVolatility);
}

void SLampStyledSlider::SetTrackBackgroundBrush(const FSlateBrush* InBrush)
{
    TrackBackgroundBrush = InBrush;
    Invalidate(EInvalidateWidgetReason::Paint);
}

void SLampStyledSlider::SetTrackFillBrush(const FSlateBrush* InBrush)
{
    TrackFillBrush = InBrush;
    Invalidate(EInvalidateWidgetReason::Paint);
}

void SLampStyledSlider::SetThumbBrush(const FSlateBrush* InBrush)
{
    ThumbBrush = InBrush;
    Invalidate(EInvalidateWidgetReason::Paint);
}

void SLampStyledSlider::SetShowTrackBackground(bool bInShowTrackBackground)
{
    bShowTrackBackground = bInShowTrackBackground;
    Invalidate(EInvalidateWidgetReason::Paint);
}

void SLampStyledSlider::SetShowTrackFill(bool bInShowTrackFill)
{
    bShowTrackFill = bInShowTrackFill;
    Invalidate(EInvalidateWidgetReason::Paint);
}

void SLampStyledSlider::SetFillToValue(bool bInFillToValue)
{
    bFillToValue = bInFillToValue;
    Invalidate(EInvalidateWidgetReason::Paint);
}

int32 SLampStyledSlider::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
    FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle,
    bool bParentEnabled) const
{
    const bool bEnabled = ShouldBeEnabled(bParentEnabled);
    const ESlateDrawEffect DrawEffects = bEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;
    const FVector2D LocalSize = AllottedGeometry.GetLocalSize();
    const FSlateRect TrackRect = GetTrackRect(LocalSize);
    const FVector2D TrackPos(TrackRect.Left, TrackRect.Top);
    const FVector2D TrackSize(TrackRect.Right - TrackRect.Left, TrackRect.Bottom - TrackRect.Top);

    const FSlateBrush* FallbackBrush = FAppStyle::Get().GetBrush("WhiteBrush");
    const FSlateBrush* BgBrush = TrackBackgroundBrush ? TrackBackgroundBrush : FallbackBrush;
    const FSlateBrush* FillBrush = TrackFillBrush ? TrackFillBrush : FallbackBrush;
    const FSlateBrush* HandleBrush = ThumbBrush ? ThumbBrush : FallbackBrush;

    const FLinearColor BgTint = TrackBackgroundBrush ? TrackBackgroundBrush->GetTint(InWidgetStyle) : FLinearColor(0.18f, 0.20f, 0.24f, 1.0f);
    const FLinearColor FillTint = TrackFillBrush ? TrackFillBrush->GetTint(InWidgetStyle) : FLinearColor(1.0f, 0.68f, 0.25f, 1.0f);
    const FLinearColor ThumbTint = ThumbBrush ? ThumbBrush->GetTint(InWidgetStyle) : FLinearColor::White;

    int32 CurrentLayer = LayerId;
    if (bShowTrackBackground)
    {
        FSlateDrawElement::MakeBox(
            OutDrawElements,
            CurrentLayer,
            AllottedGeometry.ToPaintGeometry(
                FVector2f(TrackSize),
                FSlateLayoutTransform(FVector2f(TrackPos))
            ),
            BgBrush,
            DrawEffects,
            BgTint
        );
    }

    const FSlateRect ThumbTravelRect = GetThumbTravelRect(LocalSize);
    const float TravelWidth = FMath::Max(1.0f, ThumbTravelRect.Right - ThumbTravelRect.Left);
    const float ValueWidth = TravelWidth * Value;
    const float FillWidth = bFillToValue ? ValueWidth : TrackSize.X;
    if (bShowTrackFill && FillWidth > 0.0f)
    {
        FSlateDrawElement::MakeBox(
            OutDrawElements,
            CurrentLayer + 1,
            AllottedGeometry.ToPaintGeometry(
                FVector2f(FillWidth, TrackSize.Y),
                FSlateLayoutTransform(FVector2f(TrackPos))
            ),
            FillBrush,
            DrawEffects,
            FillTint
        );
    }

    const float ThumbX = ThumbTravelRect.Left + ValueWidth - (ThumbSize.X * 0.5f);
    const float ThumbY = (LocalSize.Y - ThumbSize.Y) * 0.5f;
    FSlateDrawElement::MakeBox(
        OutDrawElements,
        CurrentLayer + 2,
        AllottedGeometry.ToPaintGeometry(
            FVector2f(ThumbSize),
            FSlateLayoutTransform(FVector2f(ThumbX, ThumbY))
        ),
        HandleBrush,
        DrawEffects,
        ThumbTint
    );

    return CurrentLayer + 2;
}

FVector2D SLampStyledSlider::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
    return FVector2D(220.0f, FMath::Max(ThumbSize.Y, TrackHeight + 8.0f));
}

FReply SLampStyledSlider::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
    {
        return FReply::Unhandled();
    }

    bDragging = true;
    OnCaptureStarted.ExecuteIfBound();
    const float LocalX = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()).X;
    CommitValueFromLocalX(MyGeometry, LocalX);
    return FReply::Handled().CaptureMouse(SharedThis(this));
}

FReply SLampStyledSlider::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    if (HasMouseCapture() && MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        bDragging = false;
        OnCaptureEnded.ExecuteIfBound();
        return FReply::Handled().ReleaseMouseCapture();
    }

    return FReply::Unhandled();
}

FReply SLampStyledSlider::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    if (bDragging && HasMouseCapture())
    {
        const float LocalX = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()).X;
        CommitValueFromLocalX(MyGeometry, LocalX);
        return FReply::Handled();
    }

    return FReply::Unhandled();
}

void SLampStyledSlider::OnMouseCaptureLost(const FCaptureLostEvent& CaptureLostEvent)
{
    if (bDragging)
    {
        bDragging = false;
        OnCaptureEnded.ExecuteIfBound();
    }

    SLeafWidget::OnMouseCaptureLost(CaptureLostEvent);
}

FCursorReply SLampStyledSlider::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const
{
    return FCursorReply::Cursor(EMouseCursor::ResizeLeftRight);
}

float SLampStyledSlider::GetSnappedValue(float InValue) const
{
    if (StepSize <= KINDA_SMALL_NUMBER)
    {
        return FMath::Clamp(InValue, 0.0f, 1.0f);
    }

    const float Snapped = FMath::RoundToFloat(InValue / StepSize) * StepSize;
    return FMath::Clamp(Snapped, 0.0f, 1.0f);
}

float SLampStyledSlider::PositionToNormalizedValue(const FGeometry& MyGeometry, float LocalX) const
{
    const FSlateRect ThumbRect = GetThumbTravelRect(MyGeometry.GetLocalSize());
    const float TrackWidth = FMath::Max(1.0f, ThumbRect.Right - ThumbRect.Left);
    return FMath::Clamp((LocalX - ThumbRect.Left) / TrackWidth, 0.0f, 1.0f);
}

void SLampStyledSlider::CommitValueFromLocalX(const FGeometry& MyGeometry, float LocalX)
{
    const float NewValue = GetSnappedValue(PositionToNormalizedValue(MyGeometry, LocalX));
    if (!FMath::IsNearlyEqual(NewValue, Value))
    {
        Value = NewValue;
        OnValueChanged.ExecuteIfBound(Value);
        Invalidate(EInvalidateWidgetReason::Paint);
    }
}

FSlateRect SLampStyledSlider::GetTrackRect(const FVector2D& LocalSize) const
{
    const float Left = TrackInset;
    const float Right = FMath::Max(Left + 1.0f, LocalSize.X - TrackInset);
    const float Top = (LocalSize.Y - TrackHeight) * 0.5f;
    return FSlateRect(Left, Top, Right, Top + TrackHeight);
}

FSlateRect SLampStyledSlider::GetThumbTravelRect(const FVector2D& LocalSize) const
{
    const float Left = ThumbTravelInset;
    const float Right = FMath::Max(Left + 1.0f, LocalSize.X - ThumbTravelInset);
    const float Top = (LocalSize.Y - TrackHeight) * 0.5f;
    return FSlateRect(Left, Top, Right, Top + TrackHeight);
}
