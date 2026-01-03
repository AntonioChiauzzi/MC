#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MyEnvironmentState.generated.h"

UCLASS()
class MC_API UMyEnvironmentState : public UObject
{
	GENERATED_BODY()

	public:
		UMyEnvironmentState();
		virtual ~UMyEnvironmentState();

		UPROPERTY()
		float SoilTemperature;

		UPROPERTY()
		float SoilHumidity;

		UPROPERTY()
		float SoilSolarIrradiance;

		UPROPERTY()
		float SoilpH;

		UPROPERTY()
		float WindSpeed;

		UPROPERTY()
		float AirTemperature;

		UPROPERTY()
		float AirHumidity;

		UPROPERTY()
		TMap<FString, float> SoilChemicalComposition;
};