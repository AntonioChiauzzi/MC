#include "ResourceManager.h"
#include "MyBaseActor.h"

void UResourceManager::EvaluateBusinessRules()
{
    if (!EnvState) return;
}

void UResourceManager::DispatchDrone(AActor* Drone)
{
    if (AMyBaseActor* Base = Cast<AMyBaseActor>(Drone))
    {
        UE_LOG(LogTemp, Warning, TEXT("Resource Manager: Drone %s inviato!"), *Base->ID);
    }
}

void UResourceManager::DispatchTractor(AActor* Tractor)
{
    if (AMyBaseActor* Base = Cast<AMyBaseActor>(Tractor))
    {
        UE_LOG(LogTemp, Warning, TEXT("Resource Manager: Trattore %s attivato!"), *Base->ID);
    }
}