#include "DynamicPawn.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Dom/JsonObject.h"

ADynamicPawn::ADynamicPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	Velocity = FVector::ZeroVector;
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
	if (BatteryLevel > 0.0f && !Velocity.IsNearlyZero())
	{
		if (TargetLocation.IsSet())
		{
			MoveToTarget(DeltaTime, TargetLocation.GetValue());
		}
		else
		{
			Move(DeltaTime);
		}
		BatteryLevel = FMath::Max(0.0f, BatteryLevel - (BatteryConsumptionRate * DeltaTime));
	}
	else if (BatteryLevel <= 0.0f)
	{
		Velocity = FVector::ZeroVector;
	}
}

void ADynamicPawn::ConfigureFromJson(const TSharedPtr<FJsonObject>& Json)
{
	const TSharedPtr<FJsonObject>* Params;
	if (Json->TryGetObjectField(TEXT("params"), Params))
	{
		const TSharedPtr<FJsonObject>* SpeedObj;
		if ((*Params)->TryGetObjectField(TEXT("speed"), SpeedObj))
		{
			Velocity.X = (*SpeedObj)->GetNumberField(TEXT("x"));
			Velocity.Y = (*SpeedObj)->GetNumberField(TEXT("y"));
			Velocity.Z = (*SpeedObj)->GetNumberField(TEXT("z"));
		}
		if (!(*Params)->TryGetNumberField(TEXT("battery"), BatteryLevel))
		{
			BatteryLevel = 100.0f;
		}
	}
	else
	{
		BatteryLevel = 100.0f;
		Velocity = FVector::ZeroVector;
	}
}

void ADynamicPawn::SaveToJson(const TSharedPtr<FJsonObject>& Json)
{
	TSharedPtr<FJsonObject> SpeedObj = MakeShared<FJsonObject>();
	SpeedObj->SetNumberField(TEXT("x"), Velocity.X);
	SpeedObj->SetNumberField(TEXT("y"), Velocity.Y);
	SpeedObj->SetNumberField(TEXT("z"), Velocity.Z);
	Json->SetObjectField(TEXT("speed"), SpeedObj);
	Json->SetNumberField(TEXT("battery"), BatteryLevel);
	if (TargetLocation.IsSet())
	{
		TSharedPtr<FJsonObject> TargetObj = MakeShared<FJsonObject>();
		TargetObj->SetNumberField(TEXT("x"), TargetLocation.GetValue().X);
		TargetObj->SetNumberField(TEXT("y"), TargetLocation.GetValue().Y);
		TargetObj->SetNumberField(TEXT("z"), TargetLocation.GetValue().Z);
		Json->SetObjectField(TEXT("targetLocation"), TargetObj);
	}
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

void ADynamicPawn::Move(float DeltaTime)
{
	FRotator TargetRotation = Velocity.Rotation();
	SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 5.0f));
	FVector NewLoc = GetActorLocation() + (Velocity * DeltaTime);
	if (NewLoc.X < MapMin.X || NewLoc.X > MapMax.X ||
		NewLoc.Y < MapMin.Y || NewLoc.Y > MapMax.Y ||
		NewLoc.Z < 0.0f || NewLoc.Z > 2000.0f)
	{
		Velocity = FVector::ZeroVector;
		return;
	}
	SetActorLocation(NewLoc);
}

void ADynamicPawn::SetTargetLocation(TOptional<FVector> NewTarget)
{
	TargetLocation = NewTarget;
}

void ADynamicPawn::MoveToTarget(float DeltaTime, FVector Target)
{
	FVector CurrentLoc = GetActorLocation();
	float Speed = Velocity.Size();
	if (Speed <= 0.0f) Speed = 400.0f;
	float Distance = FVector::Dist(CurrentLoc, Target);
	if (Distance < FMath::Max(15.0f, Speed * DeltaTime))
	{
		SetActorLocation(Target);
		Velocity = FVector::ZeroVector;
		TargetLocation.Reset();
		return;
	}
	FVector Direction = (Target - CurrentLoc).GetSafeNormal();
	SetActorRotation(FMath::RInterpTo(GetActorRotation(), Direction.Rotation(), DeltaTime, 5.0f));
	FVector NewLoc = CurrentLoc + (Direction * Speed * DeltaTime);
	if (NewLoc.X < MapMin.X || NewLoc.X > MapMax.X ||
		NewLoc.Y < MapMin.Y || NewLoc.Y > MapMax.Y ||
		NewLoc.Z < 0.0f || NewLoc.Z > 2000.0f)
	{
		Velocity = FVector::ZeroVector;
		TargetLocation.Reset();
		return;
	}
	SetActorLocation(NewLoc);
}