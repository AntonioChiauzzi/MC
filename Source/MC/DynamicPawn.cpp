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
	ApplyAutoScalingToMesh();
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
	TArray<UStaticMeshComponent*> MeshComps;
	GetComponents<UStaticMeshComponent>(MeshComps);
	if (MeshComps.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Nessuna StaticMesh trovata nel Blueprint!"));
		return;
	}
	for (UStaticMeshComponent* CurrentMesh : MeshComps)
	{
		if (CurrentMesh && CurrentMesh->GetStaticMesh())
		{
			FBox SphereBox = CurrentMesh->GetStaticMesh()->GetBoundingBox();
			FVector RawSize = SphereBox.GetSize();
			if (RawSize.X > 1.0f)
			{
				float ScaleX = MaxDimensions.X / RawSize.X;
				float ScaleY = MaxDimensions.Y / RawSize.Y;
				float ScaleZ = MaxDimensions.Z / RawSize.Z;
				float UniformScale = FMath::Min3(ScaleX, ScaleY, ScaleZ);

				CurrentMesh->SetRelativeScale3D(FVector(UniformScale));
				CurrentMesh->UpdateBounds();
				UE_LOG(LogTemp, Warning, TEXT("Mesh '%s' scalata a %f"), *CurrentMesh->GetName(), UniformScale);
			}
		}
	}
}