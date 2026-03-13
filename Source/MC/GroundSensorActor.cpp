#include "GroundSensorActor.h"

AGroundSensorActor::AGroundSensorActor()
{
    PrimaryActorTick.bCanEverTick = false;
    MaxDimensions = FVector(100.f, 100.f, 100.f);
}

void AGroundSensorActor::BeginPlay()
{
    Super::BeginPlay();
    ApplyAutoScalingToMesh();
}

void AGroundSensorActor::ConfigureFromJson(const TSharedPtr<FJsonObject>& Json)
{
    const TSharedPtr<FJsonObject>* Params;
    if (!Json->TryGetObjectField(TEXT("params"), Params)) return;
    if (!(*Params)->TryGetNumberField(TEXT("soilpH"), SoilpH))
    {
        SoilpH = 7.0f;
    }
    if (!(*Params)->TryGetNumberField(TEXT("soilTemperature"), SoilTemperature))
    {
        SoilTemperature = 22.0f;
    }
    if (!(*Params)->TryGetNumberField(TEXT("soilSolarIrradiance"), SoilSolarIrradiance))
    {
        SoilSolarIrradiance = 400.0f;
    }
    if (!(*Params)->TryGetNumberField(TEXT("soilHumidity"), SoilHumidity))
    {
        SoilHumidity = 70.0f;
    }
    const TSharedPtr<FJsonObject>* ChemicalObj;
    if ((*Params)->TryGetObjectField(TEXT("soilChemicalComposition"), ChemicalObj))
    {
        SoilChemicalComposition.Empty();

        for (const auto& Pair : (*ChemicalObj)->Values)
        {
            float Value = Pair.Value->AsNumber();
            SoilChemicalComposition.Add(Pair.Key, Value);
        }
    }
}

void AGroundSensorActor::SaveToJson(const TSharedPtr<FJsonObject>& Json)
{
    TSharedPtr<FJsonObject> SpeedObj = MakeShared<FJsonObject>();
    Json->SetNumberField(TEXT("soilpH"), SoilpH);
    Json->SetNumberField(TEXT("soilTemperature"), SoilTemperature);
    Json->SetNumberField(TEXT("soilSolarIrradiance"), SoilSolarIrradiance);
    Json->SetNumberField(TEXT("soilHumidity"), SoilHumidity);
    TSharedPtr<FJsonObject> ChemicalObj = MakeShared<FJsonObject>();
    for (const auto& Pair : SoilChemicalComposition)
    {
        ChemicalObj->SetNumberField(Pair.Key, Pair.Value);
    }
    Json->SetObjectField(TEXT("soilChemicalComposition"), ChemicalObj);
}


FString AGroundSensorActor::GetEntityType() const
{
    return TEXT("GroundSensor");
}

void AGroundSensorActor::ApplyAutoScalingToMesh()
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
            UE_LOG(LogTemp, Warning, TEXT("Harvester intero scalato a: %f"), UniformScale);
        }
    }
}