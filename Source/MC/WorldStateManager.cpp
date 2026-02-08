#include "WorldStateManager.h"

#include "MCGameInstance.h"
#include "MyEnvironmentState.h"
#include "EntityRegistryDataAsset.h"
#include "ResourceManager.h"
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

#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h" 



void UWorldStateManager::Initialize(UWorld* InWorld)
{
    World = InWorld;
    if (!ResourceManager)
    {
        ResourceManager = NewObject<UResourceManager>(this);
        if (Environment)
        {
            ResourceManager->EnvState = Environment;
        }
    }
}


void UWorldStateManager::LoadEnvironment()
{
    const FString FilePath = FPaths::Combine(BaseDataPath, TEXT("environment.json"));
    
    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("FALLITO: Impossibile trovare environment.json in: %s"), *FilePath);
        return;
    }

    TSharedPtr<FJsonObject> RootJson;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (!FJsonSerializer::Deserialize(Reader, RootJson) || !RootJson.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("FALLITO: Parsing fallito per environment.json"));
        return;
    }

    TSharedPtr<FJsonObject> EnvJson = RootJson->GetObjectField(TEXT("environment"));
    if (!EnvJson.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("FALLITO: Campo 'environment' non trovato nel JSON"));
        return;
    }

    if (!EnvJson.IsValid()) return;

    if (!Environment)
    {
        Environment = NewObject<UMyEnvironmentState>(this);
    }

    EnvJson->TryGetNumberField(TEXT("soilTemperature"), Environment->SoilTemperature);
    EnvJson->TryGetNumberField(TEXT("soilHumidity"), Environment->SoilHumidity);
    EnvJson->TryGetNumberField(TEXT("soilSolarIrradiance"), Environment->SoilSolarIrradiance);
    EnvJson->TryGetNumberField(TEXT("soilpH"), Environment->SoilpH);
    EnvJson->TryGetNumberField(TEXT("cropMaturity"), Environment->cropMaturity);
    EnvJson->TryGetNumberField(TEXT("windSpeed"), Environment->WindSpeed);
    EnvJson->TryGetNumberField(TEXT("airTemperature"), Environment->AirTemperature);
    EnvJson->TryGetNumberField(TEXT("airHumidity"), Environment->AirHumidity);
    Environment->SoilChemicalComposition.Empty();
    const TSharedPtr<FJsonObject>* ChemJson;
    if (EnvJson->TryGetObjectField(TEXT("soilChemicalComposition"), ChemJson))
    {
        for (const auto& Elem : (*ChemJson)->Values)
        {
            Environment->SoilChemicalComposition.Add(Elem.Key, Elem.Value->AsNumber());
        }
    }
    UE_LOG(LogTemp, Log, TEXT("Environment loaded correctly"));
}


void UWorldStateManager::SaveEnvironment() const
{
    if (!Environment) return;
    FString PathToUse;
    if (UMCGameInstance* GI = Cast<UMCGameInstance>(GetWorld()->GetGameInstance()))
    {
        PathToUse = GI->SavedBaseDataPath;
    }
    if (PathToUse.IsEmpty())
    {
        PathToUse = BaseDataPath;
    }
    if (PathToUse.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Salvataggio fallito: Nessun percorso base trovato!"));
        return;
    }
    TSharedPtr<FJsonObject> EnvJson = MakeShared<FJsonObject>();
    EnvJson->SetNumberField("airTemperature", Environment->AirTemperature);
    EnvJson->SetNumberField("airHumidity", Environment->AirHumidity);
    EnvJson->SetNumberField("windSpeed", Environment->WindSpeed);
    EnvJson->SetNumberField("soilTemperature", Environment->SoilTemperature);
    EnvJson->SetNumberField("soilHumidity", Environment->SoilHumidity);
    EnvJson->SetNumberField("soilSolarIrradiance", Environment->SoilSolarIrradiance);
    EnvJson->SetNumberField("soilpH", Environment->SoilpH);
    EnvJson->SetNumberField("cropMaturity", Environment->cropMaturity);
    TSharedPtr<FJsonObject> ChemJson = MakeShared<FJsonObject>();
    for (const auto& Elem : Environment->SoilChemicalComposition)
    {
        ChemJson->SetNumberField(Elem.Key, Elem.Value);
    }
    EnvJson->SetObjectField("soilChemicalComposition", ChemJson);
    TSharedPtr<FJsonObject> RootJson = MakeShared<FJsonObject>();
    RootJson->SetObjectField("environment", EnvJson);
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    if (FJsonSerializer::Serialize(RootJson.ToSharedRef(), Writer))
    {
        const FString FilePath = FPaths::Combine(PathToUse, TEXT("environment.json"));
        if (FFileHelper::SaveStringToFile(OutputString, *FilePath))
        {
            UE_LOG(LogTemp, Log, TEXT("Environment salvato correttamente in: %s"), *FilePath);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Errore fisico durante il salvataggio del file in: %s"), *FilePath);
        }
    }
}

void UWorldStateManager::LoadEntities()
{
    FBox TotalBounds(ForceInit);
    for (TActorIterator<ALandscapeProxy> It(World); It; ++It)
    {
        ALandscapeProxy* LP = *It;
        if (LP)
        {
            TotalBounds += LP->GetComponentsBoundingBox();
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
    UE_LOG(LogTemp, Log, TEXT("Confini Landscape Finali: Min %s - Max %s"), *MinBound.ToString(), *MaxBound.ToString());

    const FString FilePath = FPaths::Combine(BaseDataPath, TEXT("entities.json"));
    UE_LOG(LogTemp, Warning, TEXT("Cerco il file in: %s"), *FilePath);
    FString Json;
    if (!FFileHelper::LoadFileToString(Json, *FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("ERRORE: File entities.json NON TROVATO!"));
        return;
    }

    TSharedPtr<FJsonObject> Root;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) || !Root.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("ERRORE: JSON corrotto o malformato!"));
        return;
    }

    const TArray<TSharedPtr<FJsonValue>>* Arr;
    if (!Root->TryGetArrayField(TEXT("entities"), Arr)) return;

    for (const auto& V : *Arr)
    {
        auto Obj = V->AsObject();
        if (!Obj.IsValid()) continue;

        FString Type = Obj->GetStringField(TEXT("type"));

        if (!EntityRegistryAsset || !EntityRegistryAsset->EntityMappings.Contains(Type))
        {
            UE_LOG(LogTemp, Warning, TEXT("Tipo %s non trovato nel Data Asset"), *Type);
            continue;
        }

        TSubclassOf<AActor> ClassToSpawn = EntityRegistryAsset->EntityMappings[Type].ActorClass;

        auto LocObj = Obj->GetObjectField(TEXT("location"));
        FVector Loc(
            LocObj->GetNumberField(TEXT("x")),
            LocObj->GetNumberField(TEXT("y")),
            FMath::Max(0.0f, LocObj->GetNumberField(TEXT("z"))) 
        );

        if (Loc.X < MinBound.X || Loc.X > MaxBound.X || Loc.Y < MinBound.Y || Loc.Y > MaxBound.Y)
        {
            UE_LOG(LogTemp, Warning, TEXT("Entità %s fuori Landscape! Loc: %s | Limiti: Min(%f, %f) Max(%f, %f)"),*Type, *Loc.ToString(), MinBound.X, MinBound.Y, MaxBound.X, MaxBound.Y);
            continue;
        }

        AActor* A = World->SpawnActor<AActor>(ClassToSpawn, Loc, FRotator::ZeroRotator);
        if (!A) continue;

        if (AMyBaseActor* MyBaseActor = Cast<AMyBaseActor>(A))
        {
            MyBaseActor->ID = Obj->GetStringField(TEXT("id"));
            FString NameField;
            if (Obj->TryGetStringField(TEXT("name"), NameField)) {
                MyBaseActor->Name = NameField;
            }
            else {
                MyBaseActor->Name = MyBaseActor->ID;
            }
            MyBaseActor->MapMin = MinBound;
            MyBaseActor->MapMax = MaxBound;

#if WITH_EDITOR
            A->SetActorLabel(MyBaseActor->Name);
#endif
        }

        const TSharedPtr<FJsonObject>* TargetObjPtr;
        if (Obj->TryGetObjectField(TEXT("targetLocation"), TargetObjPtr))
        {
            auto TargetObj = *TargetObjPtr;
            FVector TLoc(
                TargetObj->GetNumberField(TEXT("x")),
                TargetObj->GetNumberField(TEXT("y")),
                TargetObj->GetNumberField(TEXT("z"))
            );
            if (TLoc.X >= MinBound.X && TLoc.X <= MaxBound.X && TLoc.Y >= MinBound.Y && TLoc.Y <= MaxBound.Y)
            {
                if (IMovableVehicle* Movable = Cast<IMovableVehicle>(A))
                {
                    Movable->SetTargetLocation(TLoc);
                }
            }
        }

        if (A->GetClass()->ImplementsInterface(UEntityConfigurable::StaticClass()))
            if (IEntityConfigurable* Configurable = Cast<IEntityConfigurable>(A))
            {
                Configurable->ConfigureFromJson(Obj);
            }

        if (A->Implements<UEnvironmentInjectable>())
        {
            IEnvironmentInjectable::Execute_SetEnvironment(A, Environment);
        }

        if (A->Implements<UEnvironmentReader>())
        {
            IEnvironmentReader::Execute_ReadEnvironment(A, Environment);
        }

        SpawnedEntities.Add(A);

        if (ResourceManager)
        {
            ResourceManager->ManagedEntities.Add(A);
        }
    }
}

void UWorldStateManager::RemoveOldInstances()
{
    for (AActor* Actor : SpawnedEntities) {
        if (Actor && Actor->IsValidLowLevel()) {
            Actor->Destroy();
        }
    }
    SpawnedEntities.Empty();
    if (ResourceManager)
    {
        for (AActor* Actor : ResourceManager->ManagedEntities) {
            if (Actor && Actor->IsValidLowLevel()) {
                Actor->Destroy();
            }
        }
        ResourceManager->ManagedEntities.Empty();
    }
    UE_LOG(LogTemp, Warning, TEXT("Entità distrutte"));
}

void UWorldStateManager::SaveEntities()
{
    if (!World) return;
    FString PathToUse;
    if (UMCGameInstance* GI = Cast<UMCGameInstance>(World->GetGameInstance()))
    {
        PathToUse = GI->SavedBaseDataPath;
    }
    if (PathToUse.IsEmpty())
    {
        PathToUse = BaseDataPath; 
    }
    if (PathToUse.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Salvataggio Entità fallito: Percorso non valido!"));
        return;
    }
    TSharedPtr<FJsonObject> RootObject = MakeShared<FJsonObject>();
    TArray<TSharedPtr<FJsonValue>> EntitiesArray;
    for (TActorIterator<AMyBaseActor> It(World); It; ++It)
    {
        AMyBaseActor* MyBaseActor = *It;
        if (!MyBaseActor) continue;
        TSharedPtr<FJsonObject> EntityObj = MakeShared<FJsonObject>();
        EntityObj->SetStringField(TEXT("id"), MyBaseActor->ID);
        EntityObj->SetStringField(TEXT("type"), MyBaseActor->GetEntityType());
        FVector L = MyBaseActor->GetActorLocation();
        TSharedPtr<FJsonObject> Loc = MakeShared<FJsonObject>();
        Loc->SetNumberField(TEXT("x"), L.X);
        Loc->SetNumberField(TEXT("y"), L.Y);
        Loc->SetNumberField(TEXT("z"), FMath::Max(0.0f, L.Z));
        EntityObj->SetObjectField(TEXT("location"), Loc);
        TSharedPtr<FJsonObject> Params = MakeShared<FJsonObject>();
        if (MyBaseActor->Implements<UEntityConfigurable>())
        {
            if (IEntityConfigurable* Configurable = Cast<IEntityConfigurable>(MyBaseActor))
            {
                Configurable->SaveToJson(Params);
            }
            if (Params->Values.Num() > 0)
            {
                EntityObj->SetObjectField(TEXT("params"), Params);
            }
        }
        TOptional<FVector> CurrentTarget;
        if (ATractorPawn* Tractor = Cast<ATractorPawn>(MyBaseActor)) {
            CurrentTarget = Tractor->TargetLocation;
        }
        else if (ADronePawn* Drone = Cast<ADronePawn>(MyBaseActor)) {
            CurrentTarget = Drone->TargetLocation;
        }
        else if (AHarvesterPawn* Harvester = Cast<AHarvesterPawn>(MyBaseActor)) {
            CurrentTarget = Harvester->TargetLocation;
        }
        if (CurrentTarget.IsSet())
        {
            TSharedPtr<FJsonObject> TObj = MakeShared<FJsonObject>();
            TObj->SetNumberField(TEXT("x"), CurrentTarget.GetValue().X);
            TObj->SetNumberField(TEXT("y"), CurrentTarget.GetValue().Y);
            TObj->SetNumberField(TEXT("z"), CurrentTarget.GetValue().Z);
            Params->SetObjectField(TEXT("targetLocation"), TObj);
        }
        EntitiesArray.Add(MakeShared<FJsonValueObject>(EntityObj));
    }
    RootObject->SetArrayField(TEXT("entities"), EntitiesArray);
    FString Output;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
    if (FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer))
    {
        const FString FilePath = FPaths::Combine(PathToUse, TEXT("entities.json"));
        if (FFileHelper::SaveStringToFile(Output, *FilePath))
        {
            UE_LOG(LogTemp, Log, TEXT("Entità salvate con successo in: %s"), *FilePath);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Errore durante la scrittura del file entities.json!"));
        }
    }
}

void UWorldStateManager::UploadMap()
{
    UMCGameInstance* GI = Cast<UMCGameInstance>(GetWorld()->GetGameInstance());
    if (!GI || GI->SavedMapPath.IsEmpty()) return;
    FString SelectedDirectory = GI->SavedMapPath;
    TArray<FString> FoundFiles;
    IFileManager::Get().FindFiles(FoundFiles, *SelectedDirectory, TEXT("*.umap"));
    if (FoundFiles.Num() > 0)
    {
        FString MapName = FPaths::GetBaseFilename(FoundFiles[0]);
        FPackageName::RegisterMountPoint(TEXT("/Game/"), SelectedDirectory);
        FString PackagePath = TEXT("/Game/") / MapName;
        UE_LOG(LogTemp, Warning, TEXT("Caricamento da path esterno scelto: %s"), *PackagePath);
        UGameplayStatics::OpenLevel(GetWorld(), FName(*PackagePath));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Nessun file .umap trovato in: %s"), *SelectedDirectory);
    }
}

void UWorldStateManager::OpenDirectoryDialogJson()
{
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
    FString SelectedDirectory;
    if (DesktopPlatform)
    {
        bool bOpened = DesktopPlatform->OpenDirectoryDialog(
            nullptr,
            TEXT("Seleziona la cartella dei JSON (Annulla per usare Content/Data)"),
            FPaths::ProjectContentDir(),
            SelectedDirectory
        );
        if (!bOpened || SelectedDirectory.IsEmpty())
        {
            SelectedDirectory = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Data"));
            UE_LOG(LogTemp, Warning, TEXT("Dialog JSON annullato. Uso fallback: %s"), *SelectedDirectory);
        }
    }
    if (UMCGameInstance* GI = Cast<UMCGameInstance>(GetWorld()->GetGameInstance()))
    {
        FPaths::NormalizeDirectoryName(SelectedDirectory);
        GI->SavedBaseDataPath = SelectedDirectory;
        BaseDataPath = SelectedDirectory;
        UE_LOG(LogTemp, Log, TEXT("SavedBaseDataPath impostato a: %s"), *GI->SavedBaseDataPath);
    }
}

void UWorldStateManager::OpenDirectoryDialogMap()
{
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
    FString SelectedDirectory;
    if (DesktopPlatform)
    {
        bool bOpened = DesktopPlatform->OpenDirectoryDialog(
            nullptr,
            TEXT("Seleziona la cartella contenente la Mappa (Annulla per caricare NewMap di default)"),
            FPaths::ProjectContentDir(),
            SelectedDirectory
        );
        if (!bOpened || SelectedDirectory.IsEmpty())
        {
            UE_LOG(LogTemp, Warning, TEXT("Dialog annullato. Utilizzo percorso di default: Content/"));
            SelectedDirectory = FPaths::ProjectContentDir();
        }
    }
    if (UMCGameInstance* GI = Cast<UMCGameInstance>(GetWorld()->GetGameInstance()))
    {
        FPaths::NormalizeDirectoryName(SelectedDirectory);
        GI->SavedMapPath = SelectedDirectory;
        UE_LOG(LogTemp, Log, TEXT("SavedMapPath impostato a: %s"), *GI->SavedMapPath);
    }
}