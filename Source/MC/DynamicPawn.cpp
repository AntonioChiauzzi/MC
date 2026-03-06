#include "DynamicPawn.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Dom/JsonObject.h"

ADynamicPawn::ADynamicPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMeshAsset.Succeeded())
	{
		MeshComponent->SetStaticMesh(CubeMeshAsset.Object);
	}
}

void ADynamicPawn::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("ADynamicPawn::BeginPlay - Mesh=%s Scale=%s"),
		*GetNameSafe(MeshComponent ? MeshComponent->GetStaticMesh() : nullptr),
		*GetActorScale3D().ToString());
}

void ADynamicPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADynamicPawn::ConfigureFromJson(const TSharedPtr<FJsonObject>& Json)
{
	const TSharedPtr<FJsonObject>* Params;
	if (Json->TryGetObjectField(TEXT("params"), Params))
	{
		if (!(*Params)->TryGetNumberField(TEXT("battery"), BatteryLevel))
		{
			BatteryLevel = 100.0f;
		}
	}
	else
	{
		BatteryLevel = 100.0f;
	}
}

void ADynamicPawn::SaveToJson(const TSharedPtr<FJsonObject>& Json)
{
	Json->SetNumberField(TEXT("battery"), BatteryLevel);
}

FString ADynamicPawn::GetEntityType() const
{
	return TEXT("Dynamic");
}

void ADynamicPawn::ApplyAutoScalingToMesh()
{
	USceneComponent* RootToScale = GetRootComponent();
	FVector Origin, BoxExtent;
	GetActorBounds(true, Origin, BoxExtent);
	FVector TotalSize = BoxExtent * 2.0f;
	if (TotalSize.X > 1.0f)
	{
		float ScaleX = MaxDimensions.X / TotalSize.X;
		float ScaleY = MaxDimensions.Y / TotalSize.Y;
		float ScaleZ = MaxDimensions.Z / TotalSize.Z;
		float UniformScale = FMath::Min3(ScaleX, ScaleY, ScaleZ);
		if (RootToScale)
		{
			RootToScale->SetRelativeScale3D(FVector(UniformScale));
			UE_LOG(LogTemp, Warning, TEXT("Mesh dinamica scalata a %f"), UniformScale);
		}
	}
}