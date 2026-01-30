#include "DynamicPawn.h"

ADynamicPawn::ADynamicPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ADynamicPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADynamicPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADynamicPawn::ConfigureFromJson(const TSharedPtr<FJsonObject>& Json)
{
}

void ADynamicPawn::SaveToJson(const TSharedPtr<FJsonObject>& Json)
{
}

FString ADynamicPawn::GetEntityType() const
{
	return TEXT("Dynamic");
}

