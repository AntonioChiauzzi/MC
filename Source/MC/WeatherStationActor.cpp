#include "WeatherStationActor.h"

AWeatherStationActor::AWeatherStationActor()
{
    PrimaryActorTick.bCanEverTick = false;
    MaxDimensions = FVector(200.f, 200.f, 200.f);
}

void AWeatherStationActor::BeginPlay()
{
    Super::BeginPlay();
    ApplyAutoScalingToMesh();
}

void AWeatherStationActor::ConfigureFromJson(const TSharedPtr<FJsonObject>& Json)
{
    const TSharedPtr<FJsonObject>* Params;
    if (!Json->TryGetObjectField(TEXT("params"), Params)) return;
    if (!(*Params)->TryGetNumberField(TEXT("airTemperature"), AirTemperature))
    {
        AirTemperature = 20.0f;
    }
    if (!(*Params)->TryGetNumberField(TEXT("airHumidity"), AirHumidity))
    {
        AirHumidity = 70.0f;
    }
    if (!(*Params)->TryGetNumberField(TEXT("windSpeed"), WindSpeed))
    {
        WindSpeed = 30.0f;
    }
}

void AWeatherStationActor::SaveToJson(const TSharedPtr<FJsonObject>& Json)
{
    TSharedPtr<FJsonObject> SpeedObj = MakeShared<FJsonObject>();
    Json->SetNumberField(TEXT("airTemperature"), AirTemperature);
    Json->SetNumberField(TEXT("airHumidity"), AirHumidity);
    Json->SetNumberField(TEXT("windSpeed"), WindSpeed);
}

FString AWeatherStationActor::GetEntityType() const
{
    return TEXT("WeatherStation");
}

void AWeatherStationActor::ApplyAutoScalingToMesh()
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