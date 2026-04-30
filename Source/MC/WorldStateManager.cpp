#include "WorldStateManager.h"

#include "MCGameInstance.h"
#include "EntityRegistryDataAsset.h"
#include "TractorPawn.h"
#include "DronePawn.h"
#include "HarvesterPawn.h"
#include "GroundSensorActor.h"
#include "WeatherStationActor.h"
#include "EntityConfigurable.h"
#include "MovableVehicle.h"
#include "MyBaseActor.h"

#include "Json.h"
#include "JsonUtilities.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "Engine/SkyLight.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"

#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"

void UWorldStateManager::Initialize(UWorld* InWorld)
{
    World = InWorld;
}

void UWorldStateManager::LoadAssetOverridesFromProject()
{
    AssetOverridePathsById.Empty();
    AssetOverrideDimensionsById.Empty();
    const FString OverrideFilePath = FPaths::ConvertRelativePathToFull(
        FPaths::ProjectDir() / TEXT("Launcher/asset_overrides.json"));
    UE_LOG(LogTemp, Warning, TEXT("Asset overrides path risolto: %s"), *OverrideFilePath);
    if (!FPaths::FileExists(OverrideFilePath))
    {
        UE_LOG(LogTemp, Warning, TEXT("Asset overrides non trovato: %s"), *OverrideFilePath);
        return;
    }
    FString JsonText;
    if (!FFileHelper::LoadFileToString(JsonText, *OverrideFilePath))
    {
        UE_LOG(LogTemp, Warning, TEXT("Impossibile leggere asset overrides: %s"), *OverrideFilePath);
        return;
    }
    TSharedPtr<FJsonObject> Root;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
    if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("JSON asset overrides non valido: %s"), *OverrideFilePath);
        return;
    }
    const TArray<TSharedPtr<FJsonValue>>* AssetsArray = nullptr;
    if (!Root->TryGetArrayField(TEXT("assets"), AssetsArray) || !AssetsArray)
    {
        UE_LOG(LogTemp, Warning, TEXT("Campo 'assets' mancante in %s"), *OverrideFilePath);
        return;
    }
    for (const TSharedPtr<FJsonValue>& Value : *AssetsArray)
    {
        const TSharedPtr<FJsonObject> Obj = Value->AsObject();
        if (!Obj.IsValid())
        {
            continue;
        }
        FString Id;
        FString Path;
        Obj->TryGetStringField(TEXT("id"), Id);
        Obj->TryGetStringField(TEXT("path"), Path);
        if (Id.IsEmpty() || Path.IsEmpty())
        {
            continue;
        }
        AssetOverridePathsById.Add(Id, Path);
        UE_LOG(LogTemp, Log,
            TEXT("Asset override registrato | Id=%s | Path=%s"),
            *Id,
            *Path);
        const TSharedPtr<FJsonObject>* MaxDimensionsObj = nullptr;
        if (Obj->TryGetObjectField(TEXT("maxDimensions"), MaxDimensionsObj) &&
            MaxDimensionsObj && MaxDimensionsObj->IsValid())        {
            double X = 0.0;
            double Y = 0.0;
            double Z = 0.0;
            const bool bHasX = (*MaxDimensionsObj)->TryGetNumberField(TEXT("x"), X);
            const bool bHasY = (*MaxDimensionsObj)->TryGetNumberField(TEXT("y"), Y);
            const bool bHasZ = (*MaxDimensionsObj)->TryGetNumberField(TEXT("z"), Z);
            if (bHasX && bHasY && bHasZ && X > 0.0 && Y > 0.0 && Z > 0.0)
            {
                const FVector OverrideDimensions((float)X, (float)Y, (float)Z);
                AssetOverrideDimensionsById.Add(Id, OverrideDimensions);
                UE_LOG(LogTemp, Warning,
                    TEXT("MaxDimensions override registrato | Id=%s | Dimensions=%s"),
                    *Id,
                    *OverrideDimensions.ToString());
            }
        }
    }
    UE_LOG(LogTemp, Warning,
        TEXT("Asset overrides caricati: %d | Dimension overrides caricati: %d"),
        AssetOverridePathsById.Num(),
        AssetOverrideDimensionsById.Num());
}

void UWorldStateManager::LoadEntities()
{
    const FString FilePath = FPaths::ConvertRelativePathToFull(BaseDataPath);
    UE_LOG(LogTemp, Warning, TEXT("=== LoadEntities DEBUG ==="));
    UE_LOG(LogTemp, Warning, TEXT("Path: %s"), *FilePath);
    UE_LOG(LogTemp, Warning, TEXT("Exists: %d"), FPaths::FileExists(FilePath));
    if (FilePath.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("LoadEntities: BaseDataPath vuoto"));
        return;
    }
    FString Json;
    if (!FFileHelper::LoadFileToString(Json, *FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("LoadEntities: impossibile leggere %s"), *FilePath);
        return;
    }
    TSharedPtr<FJsonObject> Root;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) || !Root.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("LoadEntities: JSON non valido in %s"), *FilePath);
        return;
    }
    const TArray<TSharedPtr<FJsonValue>>* Arr = nullptr;
    if (!Root->TryGetArrayField(TEXT("entities"), Arr))
    {
        UE_LOG(LogTemp, Error, TEXT("LoadEntities: campo 'entities' mancante in %s"), *FilePath);
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("LoadEntities: trovate %d entita"), Arr->Num());
    for (const auto& V : *Arr)
    {
        const TSharedPtr<FJsonObject> Obj = V->AsObject();
        if (!Obj.IsValid())
        {
            continue;
        }
        const FString Type = Obj->GetStringField(TEXT("type"));
        if (!EntityRegistryAsset || !EntityRegistryAsset->EntityMappings.Contains(Type))
        {
            UE_LOG(LogTemp, Warning, TEXT("LoadEntities: type non registrato: %s"), *Type);
            continue;
        }
        if (AActor* NewActor = SpawnEntityFromJson(Obj))
        {
            SpawnedEntities.Add(NewActor);
        }
    }
}

void UWorldStateManager::ResizeLandscape(const FVector TargetSize)
{
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("ResizeLandscape: World nullo"));
        return;
    }
    TArray<ALandscapeProxy*> LandscapeProxies;
    FBox TotalBounds(ForceInit);

    for (TActorIterator<ALandscapeProxy> It(World); It; ++It)
    {
        ALandscapeProxy* LP = *It;
        if (LP)
        {
            LandscapeProxies.Add(LP);
            TotalBounds += LP->GetComponentsBoundingBox(true);

            UE_LOG(LogTemp, Warning, TEXT("ResizeLandscape: Found LandscapeProxy %s  Scale=%s"),
                *LP->GetName(),
                *LP->GetActorScale3D().ToString());
        }
    }
    if (LandscapeProxies.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("ResizeLandscape: nessun LandscapeProxy trovato"));
        return;
    }
    if (!TotalBounds.IsValid)
    {
        UE_LOG(LogTemp, Error, TEXT("ResizeLandscape: TotalBounds non validi"));
        return;
    }
    const FVector SizeBefore = TotalBounds.GetSize();
    if (SizeBefore.X <= KINDA_SMALL_NUMBER || SizeBefore.Y <= KINDA_SMALL_NUMBER)
    {
        UE_LOG(LogTemp, Error, TEXT("ResizeLandscape: bounds invalidi (%s)"), *SizeBefore.ToString());
        return;
    }
    const FVector OldScale = LandscapeProxies[0]->GetActorScale3D();
    FVector NewScale = OldScale;
    if (TargetSize.X > 0.f)
    {
        NewScale.X = OldScale.X * (TargetSize.X / SizeBefore.X);
    }
    if (TargetSize.Y > 0.f)
    {
        NewScale.Y = OldScale.Y * (TargetSize.Y / SizeBefore.Y);
    }
    NewScale.Z = OldScale.Z;
    UE_LOG(LogTemp, Warning,
        TEXT("ResizeLandscape BEFORE: Size=%s  OldScale=%s  Target=%s  NewScale=%s"),
        *SizeBefore.ToString(),
        *OldScale.ToString(),
        *TargetSize.ToString(),
        *NewScale.ToString()
    );
    for (ALandscapeProxy* LP : LandscapeProxies)
    {
        if (!LP)
        {
            continue;
        }
        LP->SetActorScale3D(NewScale);
        LP->MarkComponentsRenderStateDirty();
        LP->ReregisterAllComponents();
        UE_LOG(LogTemp, Warning, TEXT("ResizeLandscape: Applied scale to %s  NewScale=%s"),
            *LP->GetName(),
            *LP->GetActorScale3D().ToString());
    }
    UpdateLandscapeBounds();
    const FVector CenterAfter = (MinBound + MaxBound) * 0.5f;
    WorldOffset = FVector(-CenterAfter.X, -CenterAfter.Y, 0.f);
    const FVector SizeAfter = MaxBound - MinBound;
    UE_LOG(LogTemp, Warning,
        TEXT("ResizeLandscape AFTER: Size=%s  Center=%s  WorldOffset=%s"),
        *SizeAfter.ToString(),
        *CenterAfter.ToString(),
        *WorldOffset.ToString()
    );
}

void UWorldStateManager::UpdateLandscapeBounds()
{
    FBox TotalBounds(ForceInit);

    for (TActorIterator<ALandscapeProxy> It(World); It; ++It)
    {
        if (ALandscapeProxy* LP = *It)
        {
            TotalBounds += LP->GetComponentsBoundingBox(true);
        }
    }
    if (TotalBounds.IsValid)
    {
        MinBound = TotalBounds.Min;
        MaxBound = TotalBounds.Max;
    }
    else
    {
        for (TActorIterator<ALandscape> It(World); It; ++It)
        {
            FVector Origin, Extent;
            It->GetActorBounds(false, Origin, Extent);
            MinBound = Origin - Extent;
            MaxBound = Origin + Extent;
            break;
        }
    }
    const FVector MinLogical = MinBound + WorldOffset;
    const FVector MaxLogical = MaxBound + WorldOffset;
    const FVector LogicalSize = MaxLogical - MinLogical;
    const FVector WorldSize = MaxBound - MinBound;
    UE_LOG(LogTemp, Log,
        TEXT("Confini Landscape Aggiornati: Min %s - Max %s | Logical Min %s - Max %s | WorldSize %s | LogicalSize %s | WorldOffset %s"),
        *MinBound.ToString(),
        *MaxBound.ToString(),
        *MinLogical.ToString(),
        *MaxLogical.ToString(),
        *WorldSize.ToString(),
        *LogicalSize.ToString(),
        *WorldOffset.ToString());
}

void UWorldStateManager::ApplyAssetOverrideIfAny(AMyBaseActor* Actor)
{
    if (!Actor || Actor->ID.IsEmpty())
    {
        return;
    }
    const FString* FoundPath = AssetOverridePathsById.Find(Actor->ID);
    if (!FoundPath)
    {
        return;
    }
    Actor->bHasAssetOverride = true;
    Actor->RuntimeOverrideAssetPath = *FoundPath;
    const FString& Path = *FoundPath;
    UE_LOG(LogTemp, Warning,
        TEXT("Override trovato per %s | Path=%s"),
        *Actor->ID,
        *Path);
    const FVector* FoundDimensions = AssetOverrideDimensionsById.Find(Actor->ID);
    if (FoundDimensions)
    {
        Actor->MaxDimensions = *FoundDimensions;
        UE_LOG(LogTemp, Warning,
            TEXT("MaxDimensions override applicato a %s | %s"),
            *Actor->ID,
            *FoundDimensions->ToString());
    }
}

AActor* UWorldStateManager::SpawnEntityFromJson(const TSharedPtr<FJsonObject>& Obj)
{
    const FString Type = Obj->GetStringField(TEXT("type"));
    if (!EntityRegistryAsset || !EntityRegistryAsset->EntityMappings.Contains(Type))
    {
        return nullptr;
    }
    TSubclassOf<AActor> ClassToSpawn = EntityRegistryAsset->EntityMappings[Type].ActorClass;
    if (!World || !*ClassToSpawn)
    {
        return nullptr;
    }
    const FVector MinLogical = MinBound + WorldOffset;
    const FVector MaxLogical = MaxBound + WorldOffset;
    const TSharedPtr<FJsonObject> LocObj = Obj->GetObjectField(TEXT("location"));
    const FVector LocLogical(
        LocObj->GetNumberField(TEXT("x")),
        LocObj->GetNumberField(TEXT("y")),
        FMath::Max(0.0f, LocObj->GetNumberField(TEXT("z")))
    );
    if (LocLogical.X < MinLogical.X || LocLogical.X > MaxLogical.X ||
        LocLogical.Y < MinLogical.Y || LocLogical.Y > MaxLogical.Y)
    {
        UE_LOG(LogTemp, Warning, TEXT("Entita' %s fuori Landscape (Logical)! Loc=%s  LogicalMin=%s  LogicalMax=%s"),
            *Type, *LocLogical.ToString(), *MinLogical.ToString(), *MaxLogical.ToString());
        return nullptr;
    }
    const FVector LocWorld = ApplyWorldOffset_LogicalToWorld(LocLogical);
    const FTransform SpawnTransform(FRotator::ZeroRotator, LocWorld);
    AActor* A = World->SpawnActorDeferred<AActor>(
        ClassToSpawn,
        SpawnTransform,
        nullptr,
        nullptr,
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn
    );
    if (!A)
    {
        return nullptr;
    }
    if (AMyBaseActor* MyBaseActor = Cast<AMyBaseActor>(A))
    {
        MyBaseActor->ID = Obj->GetStringField(TEXT("id"));
        MyBaseActor->Name = Obj->HasField(TEXT("name")) ? Obj->GetStringField(TEXT("name")) : MyBaseActor->ID;
        MyBaseActor->MapMin = MinLogical;
        MyBaseActor->MapMax = MaxLogical;
        MyBaseActor->WorldOffset = WorldOffset;

        ApplyAssetOverrideIfAny(MyBaseActor);

#if WITH_EDITOR
        A->SetActorLabel(MyBaseActor->Name);
#endif
    }

    const TSharedPtr<FJsonObject>* TargetObjPtr;
    if (Obj->TryGetObjectField(TEXT("targetLocation"), TargetObjPtr) && TargetObjPtr && TargetObjPtr->IsValid())
    {
        const TSharedPtr<FJsonObject> TargetObj = *TargetObjPtr;
        const FVector TargetLogical(
            TargetObj->GetNumberField(TEXT("x")),
            TargetObj->GetNumberField(TEXT("y")),
            TargetObj->GetNumberField(TEXT("z"))
        );
        const FVector TargetWorld = ApplyWorldOffset_LogicalToWorld(TargetLogical);
        if (IMovableVehicle* Movable = Cast<IMovableVehicle>(A))
        {
            Movable->SetTargetLocation(TOptional<FVector>(TargetWorld));
        }
    }
    if (IEntityConfigurable* Configurable = Cast<IEntityConfigurable>(A))
    {
        Configurable->ConfigureFromJson(Obj);
    }
    UGameplayStatics::FinishSpawningActor(A, SpawnTransform);
    return A;
}

void UWorldStateManager::RemoveOldInstances()
{
    for (AActor* Actor : SpawnedEntities)
    {
        if (Actor && Actor->IsValidLowLevel())
        {
            Actor->Destroy();
        }
    }
    SpawnedEntities.Empty();
    UE_LOG(LogTemp, Warning, TEXT("Entita' distrutte"));
}

void UWorldStateManager::SaveEntities()
{
    if (!World)
    {
        return;
    }
    FString PathToUse = BaseDataPath;
    if (UMCGameInstance* GI = Cast<UMCGameInstance>(World->GetGameInstance()))
    {
        if (!GI->SavedBaseDataPath.IsEmpty())
        {
            PathToUse = GI->SavedBaseDataPath;
        }
    }
    if (PathToUse.IsEmpty())
    {
        return;
    }
    TSharedPtr<FJsonObject> RootObject = MakeShared<FJsonObject>();
    TArray<TSharedPtr<FJsonValue>> EntitiesArray;
    for (TActorIterator<AMyBaseActor> It(World); It; ++It)
    {
        if (AMyBaseActor* MyBaseActor = *It)
        {
            EntitiesArray.Add(MakeShared<FJsonValueObject>(ConvertEntityToJson(MyBaseActor)));
        }
    }
    RootObject->SetArrayField(TEXT("entities"), EntitiesArray);
    FString Output;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
    if (FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer))
    {
        const FString FilePath = FPaths::Combine(PathToUse, TEXT("entities.json"));
        FFileHelper::SaveStringToFile(Output, *FilePath);
        UE_LOG(LogTemp, Log, TEXT("Entità salvate in: %s"), *FilePath);
    }
}

TSharedPtr<FJsonObject> UWorldStateManager::ConvertEntityToJson(AMyBaseActor* Actor)
{
    TSharedPtr<FJsonObject> EntityObj = MakeShared<FJsonObject>();
    EntityObj->SetStringField(TEXT("id"), Actor->ID);
    EntityObj->SetStringField(TEXT("type"), Actor->GetEntityType());
    const FVector WorldLoc = Actor->GetActorLocation();
    const FVector LogicalLoc = WorldLoc + Actor->WorldOffset;
    TSharedPtr<FJsonObject> Loc = MakeShared<FJsonObject>();
    Loc->SetNumberField(TEXT("x"), LogicalLoc.X);
    Loc->SetNumberField(TEXT("y"), LogicalLoc.Y);
    Loc->SetNumberField(TEXT("z"), FMath::Max(0.0f, LogicalLoc.Z));
    EntityObj->SetObjectField(TEXT("location"), Loc);
    TSharedPtr<FJsonObject> Params = MakeShared<FJsonObject>();
    if (IEntityConfigurable* Configurable = Cast<IEntityConfigurable>(Actor))
    {
        Configurable->SaveToJson(Params);
    }
    if (Params->Values.Num() > 0)
    {
        EntityObj->SetObjectField(TEXT("params"), Params);
    }
    TOptional<FVector> CurrentTargetWorld;
    if (ATractorPawn* Tractor = Cast<ATractorPawn>(Actor))
    {
        CurrentTargetWorld = Tractor->TargetLocation;
    }
    else if (ADronePawn* Drone = Cast<ADronePawn>(Actor))
    {
        CurrentTargetWorld = Drone->TargetLocation;
    }
    else if (AHarvesterPawn* Harvester = Cast<AHarvesterPawn>(Actor))
    {
        CurrentTargetWorld = Harvester->TargetLocation;
    }
    if (CurrentTargetWorld.IsSet())
    {
        const FVector TargetLogical = CurrentTargetWorld.GetValue() + Actor->WorldOffset;
        TSharedPtr<FJsonObject> TObj = MakeShared<FJsonObject>();
        TObj->SetNumberField(TEXT("x"), TargetLogical.X);
        TObj->SetNumberField(TEXT("y"), TargetLogical.Y);
        TObj->SetNumberField(TEXT("z"), TargetLogical.Z);
        EntityObj->SetObjectField(TEXT("targetLocation"), TObj);
    }
    return EntityObj;
}

FVector UWorldStateManager::ComputeTargetSizeFromEntitiesJson(float Margin) const
{
    const FString FilePath = BaseDataPath;
    FString Json;
    if (!FFileHelper::LoadFileToString(Json, *FilePath))
    {
        UE_LOG(LogTemp, Warning, TEXT("ComputeTargetSizeFromEntitiesJson: impossibile leggere %s"), *FilePath);
        return FVector::ZeroVector;
    }
    TSharedPtr<FJsonObject> Root;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) || !Root.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("ComputeTargetSizeFromEntitiesJson: JSON non valido"));
        return FVector::ZeroVector;
    }
    const TArray<TSharedPtr<FJsonValue>>* Arr = nullptr;
    if (!Root->TryGetArrayField(TEXT("entities"), Arr) || !Arr)
    {
        UE_LOG(LogTemp, Warning, TEXT("ComputeTargetSizeFromEntitiesJson: campo 'entities' mancante"));
        return FVector::ZeroVector;
    }
    double MaxAbsX = 0.0;
    double MaxAbsY = 0.0;
    for (const auto& V : *Arr)
    {
        const TSharedPtr<FJsonObject> Obj = V->AsObject();
        if (!Obj.IsValid() || !Obj->HasField(TEXT("location")))
        {
            continue;
        }
        const TSharedPtr<FJsonObject> LocObj = Obj->GetObjectField(TEXT("location"));
        const double X = LocObj->GetNumberField(TEXT("x"));
        const double Y = LocObj->GetNumberField(TEXT("y"));
        MaxAbsX = FMath::Max(MaxAbsX, FMath::Abs(X));
        MaxAbsY = FMath::Max(MaxAbsY, FMath::Abs(Y));
    }
    const float TargetX = float(MaxAbsX * 2.0 + Margin * 2.0);
    const float TargetY = float(MaxAbsY * 2.0 + Margin * 2.0);
    UE_LOG(LogTemp, Warning, TEXT("FitToData: MaxAbsX=%.2f MaxAbsY=%.2f => TargetSize X=%.2f Y=%.2f (Margin=%.2f)"),
        float(MaxAbsX), float(MaxAbsY), TargetX, TargetY, Margin);
    return FVector(TargetX, TargetY, 0.f);
}