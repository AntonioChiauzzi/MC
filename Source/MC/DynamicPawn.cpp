#include "DynamicPawn.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Dom/JsonObject.h"

ADynamicPawn::ADynamicPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(SceneRoot);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMeshAsset.Succeeded())
	{
		MeshComponent->SetStaticMesh(CubeMeshAsset.Object);
	}
}

void ADynamicPawn::BeginPlay()
{
	Super::BeginPlay();

	FVector Origin;
	FVector Extent;
	GetActorBounds(true, Origin, Extent);
	UE_LOG(LogTemp, Warning,
		TEXT("ADynamicPawn::BeginPlay - Mesh=%s ActorScale=%s ActorLocation=%s BoundsOrigin=%s BoundsExtent=%s"),
		*GetNameSafe(MeshComponent ? MeshComponent->GetStaticMesh() : nullptr),
		*GetActorScale3D().ToString(),
		*GetActorLocation().ToString(),
		*Origin.ToString(),
		*Extent.ToString());
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
	FVector Origin;
	FVector Extent;

	GetActorBounds(true, Origin, Extent);
	const FVector Size = Extent * 2.f;
	const float MaxCurrent = FMath::Max3(Size.X, Size.Y, Size.Z);
	const float MaxTarget = FMath::Max3(MaxDimensions.X, MaxDimensions.Y, MaxDimensions.Z);
	if (MaxCurrent <= KINDA_SMALL_NUMBER)
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyAutoScalingToMesh: invalid bounds %s"), *Size.ToString());
		return;
	}
	const float UniformScale = MaxTarget / MaxCurrent;
	SceneRoot->SetRelativeScale3D(FVector(UniformScale));
	UE_LOG(LogTemp, Warning,
		TEXT("AutoScaling | Size=%s MaxCurrent=%f MaxTarget=%f Scale=%f"),
		*Size.ToString(),
		MaxCurrent,
		MaxTarget,
		UniformScale);
}

void ADynamicPawn::AutoOrientModelFromBounds()
{
	FVector Origin;
	FVector Extent;
	GetActorBounds(true, Origin, Extent);
	const FVector Size = Extent * 2.f;
	FRotator NewRot = FRotator::ZeroRotator;
	if (Size.Y > Size.X)
	{
		NewRot = FRotator(0.f, -90.f, 0.f);
	}
	else
	{
		NewRot = FRotator::ZeroRotator;
	}
	SceneRoot->SetRelativeRotation(NewRot);
	UE_LOG(LogTemp, Warning,
		TEXT("AutoOrientModelFromBounds: Size=%s Rotation=%s"),
		*Size.ToString(),
		*NewRot.ToString());
}