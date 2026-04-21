#include "VR/Components/WidgetPointerVisualizerComponent.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	const FName PointArrayParameterName(TEXT("User.PointArray"));
	const TCHAR* DefaultCursorMeshPath = TEXT("/Engine/BasicShapes/Sphere.Sphere");
}

UWidgetPointerVisualizerComponent::UWidgetPointerVisualizerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CursorMeshFinder(DefaultCursorMeshPath);
	if (CursorMeshFinder.Succeeded())
	{
		DefaultCursorMesh = CursorMeshFinder.Object;
	}
}

void UWidgetPointerVisualizerComponent::OnRegister()
{
	Super::OnRegister();
	TryResolveMissingReferences();
	EnsureVisualComponentsCreated();
	UpdateCursorMaterial();
	RefreshTickState();
	HideVisualization();
}

void UWidgetPointerVisualizerComponent::OnUnregister()
{
	ReleaseCursorComponent();
	ReleaseEffectComponent();
	Super::OnUnregister();
}

void UWidgetPointerVisualizerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateVisualization();
}

void UWidgetPointerVisualizerComponent::SetPointerReferences(USceneComponent* InPointerOrigin, UWidgetInteractionComponent* InWidgetInteraction)
{
	PointerOrigin = InPointerOrigin;
	WidgetInteraction = InWidgetInteraction;
	TryResolveMissingReferences();
	RefreshTickState();

	if (!bVisualizationEnabled)
	{
		HideVisualization();
	}
}

void UWidgetPointerVisualizerComponent::SetVisualizationEnabled(bool bEnabled)
{
	bVisualizationEnabled = bEnabled;
	RefreshTickState();

	if (!bVisualizationEnabled)
	{
		HideVisualization();
		return;
	}

	UpdateVisualization();
}

void UWidgetPointerVisualizerComponent::SetShowOnlyOnWidgetHover(bool bEnabled)
{
	bShowOnlyOnWidgetHover = bEnabled;

	if (!bVisualizationEnabled)
	{
		HideVisualization();
		return;
	}

	UpdateVisualization();
}

void UWidgetPointerVisualizerComponent::RefreshVisualization()
{
	TryResolveMissingReferences();
	RefreshTickState();
	UpdateCursorMaterial();

	if (bVisualizationEnabled)
	{
		UpdateVisualization();
	}
	else
	{
		HideVisualization();
	}
}

void UWidgetPointerVisualizerComponent::EnsureVisualComponentsCreated()
{
	if (PointerEffectComponent == nullptr)
	{
		PointerEffectComponent = CreateEffectComponent();
	}

	if (CursorComponent == nullptr)
	{
		CursorComponent = CreateCursorComponent();
	}
}

void UWidgetPointerVisualizerComponent::UpdateVisualization()
{
	TryResolveMissingReferences();
	EnsureVisualComponentsCreated();

	USceneComponent* EffectivePointerOrigin = GetEffectivePointerOrigin();
	if (!bVisualizationEnabled || EffectivePointerOrigin == nullptr || WidgetInteraction == nullptr)
	{
		HideVisualization();
		return;
	}

	const FVector TraceStart = EffectivePointerOrigin->GetComponentLocation();
	const FVector ForwardVector = EffectivePointerOrigin->GetForwardVector();
	FVector TraceEnd = TraceStart + (ForwardVector * WidgetInteraction->InteractionDistance * PointerDistanceScale);

	const FHitResult& LastHitResult = WidgetInteraction->GetLastHitResult();
	if (LastHitResult.bBlockingHit)
	{
		TraceEnd = LastHitResult.ImpactPoint;
	}

	const bool bIsHoveringWidget = WidgetInteraction->IsOverHitTestVisibleWidget() || WidgetInteraction->IsOverInteractableWidget();
	if (bShowOnlyOnWidgetHover && !bIsHoveringWidget)
	{
		HideVisualization();
		return;
	}

	const FLinearColor ActiveBeamColor = bIsHoveringWidget ? BeamHoverColor : BeamDefaultColor;
	const FLinearColor ActiveCursorColor = bIsHoveringWidget ? CursorHoverColor : CursorDefaultColor;

	if (PointerEffectComponent != nullptr)
	{
		if (PointerSystem != nullptr)
		{
			PointerEffectComponent->SetAsset(PointerSystem, false);
			PointerEffectComponent->SetWorldLocation(TraceStart);
			PointerEffectComponent->SetWorldRotation(FRotator::ZeroRotator);
			PointerEffectComponent->SetWorldScale3D(FVector::OneVector);

			if (!BeamColorParameterName.IsNone())
			{
				PointerEffectComponent->SetVariableLinearColor(BeamColorParameterName, ActiveBeamColor);
			}

			TArray<FVector> BeamPoints;
			BeamPoints.Reserve(2);
			BeamPoints.Add(TraceStart);
			BeamPoints.Add(TraceEnd);
			UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(PointerEffectComponent, PointArrayParameterName, BeamPoints);
			PointerEffectComponent->SetVisibility(true, true);

			if (!PointerEffectComponent->IsActive())
			{
				PointerEffectComponent->Activate(true);
			}
		}
		else
		{
			PointerEffectComponent->Deactivate();
			PointerEffectComponent->SetVisibility(false, true);
		}
	}

	if (CursorComponent != nullptr)
	{
		CursorComponent->SetWorldLocation(TraceEnd + (ForwardVector * CursorHitOffset));
		CursorComponent->SetWorldScale3D(CursorScale);
		CursorComponent->SetVisibility(true, true);
		CursorComponent->SetHiddenInGame(false, true);

		if (CursorMaterialInstance != nullptr && !CursorColorParameterName.IsNone())
		{
			CursorMaterialInstance->SetVectorParameterValue(CursorColorParameterName, ActiveCursorColor);
		}
	}
}

void UWidgetPointerVisualizerComponent::HideVisualization()
{
	if (PointerEffectComponent != nullptr)
	{
		PointerEffectComponent->Deactivate();
		PointerEffectComponent->SetVisibility(false, true);

		if (PointerEffectComponent->GetAsset() != nullptr)
		{
			UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(PointerEffectComponent, PointArrayParameterName, TArray<FVector>());
		}
	}

	if (CursorComponent != nullptr)
	{
		CursorComponent->SetVisibility(false, true);
		CursorComponent->SetHiddenInGame(true, true);
	}
}

void UWidgetPointerVisualizerComponent::RefreshTickState()
{
	SetComponentTickEnabled(bVisualizationEnabled && WidgetInteraction != nullptr && GetEffectivePointerOrigin() != nullptr);
}

void UWidgetPointerVisualizerComponent::TryResolveMissingReferences()
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return;
	}

	if (WidgetInteraction == nullptr)
	{
		WidgetInteraction = Owner->FindComponentByClass<UWidgetInteractionComponent>();
	}

	if (PointerOrigin == nullptr && WidgetInteraction != nullptr)
	{
		PointerOrigin = WidgetInteraction->GetAttachParent();
	}
}

USceneComponent* UWidgetPointerVisualizerComponent::GetEffectivePointerOrigin() const
{
	if (PointerOrigin != nullptr)
	{
		return PointerOrigin;
	}

	return WidgetInteraction;
}

UNiagaraComponent* UWidgetPointerVisualizerComponent::CreateEffectComponent()
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return nullptr;
	}

	const FName ComponentName = MakeUniqueObjectName(Owner, UNiagaraComponent::StaticClass(), TEXT("WidgetPointerBeam"));
	UNiagaraComponent* EffectComponent = NewObject<UNiagaraComponent>(Owner, ComponentName);
	if (EffectComponent == nullptr)
	{
		return nullptr;
	}

	EffectComponent->SetAutoActivate(false);
	EffectComponent->SetVisibility(false);
	EffectComponent->SetUsingAbsoluteLocation(true);
	EffectComponent->SetUsingAbsoluteRotation(true);
	EffectComponent->SetUsingAbsoluteScale(true);
	Owner->AddOwnedComponent(EffectComponent);
	EffectComponent->RegisterComponent();
	return EffectComponent;
}

UStaticMeshComponent* UWidgetPointerVisualizerComponent::CreateCursorComponent()
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return nullptr;
	}

	const FName ComponentName = MakeUniqueObjectName(Owner, UStaticMeshComponent::StaticClass(), TEXT("WidgetPointerCursor"));
	UStaticMeshComponent* NewCursorComponent = NewObject<UStaticMeshComponent>(Owner, ComponentName);
	if (NewCursorComponent == nullptr)
	{
		return nullptr;
	}

	NewCursorComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NewCursorComponent->SetGenerateOverlapEvents(false);
	NewCursorComponent->SetCastShadow(false);
	NewCursorComponent->SetVisibility(false);
	NewCursorComponent->SetHiddenInGame(true);
	NewCursorComponent->SetStaticMesh(DefaultCursorMesh);
	NewCursorComponent->SetUsingAbsoluteLocation(true);
	NewCursorComponent->SetUsingAbsoluteRotation(true);
	NewCursorComponent->SetUsingAbsoluteScale(true);
	Owner->AddOwnedComponent(NewCursorComponent);
	NewCursorComponent->RegisterComponent();
	return NewCursorComponent;
}

void UWidgetPointerVisualizerComponent::ReleaseEffectComponent()
{
	if (PointerEffectComponent == nullptr)
	{
		return;
	}

	PointerEffectComponent->DeactivateImmediate();
	PointerEffectComponent->DestroyComponent();
	PointerEffectComponent = nullptr;
}

void UWidgetPointerVisualizerComponent::ReleaseCursorComponent()
{
	CursorMaterialInstance = nullptr;

	if (CursorComponent == nullptr)
	{
		return;
	}

	CursorComponent->DestroyComponent();
	CursorComponent = nullptr;
}

void UWidgetPointerVisualizerComponent::UpdateCursorMaterial()
{
	if (CursorComponent == nullptr)
	{
		return;
	}

	if (CursorMaterial != nullptr)
	{
		CursorMaterialInstance = UMaterialInstanceDynamic::Create(CursorMaterial, this);
		CursorComponent->SetMaterial(0, CursorMaterialInstance);
		return;
	}

	CursorMaterialInstance = nullptr;
	CursorComponent->SetMaterial(0, nullptr);
}
