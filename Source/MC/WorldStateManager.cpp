#include "WorldStateManager.h"

#include "Json.h"
#include "JsonUtilities.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"


void WorldStateManager::Initialize(UWorld* InWorld)
{
    World = InWorld;
    RegisterClasses();
}

void WorldStateManager::RegisterClasses()
{
    //ClassRegistry.Add("Pawn", APawn::StaticClass());
}

void WorldStateManager::LoadEnvironment()
{
    FString JsonString;
    FFileHelper::LoadFileToString(
        JsonString,
        *(FPaths::ProjectContentDir() + "Source/environment.json")
    );
}


void WorldStateManager::LoadEntities()
{
    FString JsonString;
    FFileHelper::LoadFileToString(
        JsonString,
        *(FPaths::ProjectContentDir() + "Source/entities.json")
    );

    TSharedPtr<FJsonObject> Root;
    TSharedRef<TJsonReader<>> Reader =
        TJsonReaderFactory<>::Create(JsonString);

    if (!FJsonSerializer::Deserialize(Reader, Root))
        return;

    const TArray<TSharedPtr<FJsonValue>> Entities =
        Root->GetArrayField("entities");

    for (const auto& Value : Entities)
    {
        TSharedPtr<FJsonObject> Obj = Value->AsObject();

        FString ClassName = Obj->GetStringField("class");
        if (!ClassRegistry.Contains(ClassName))
            continue;

        FVector Location = FVector::ZeroVector;
        if (Obj->HasField("location"))
        {
            auto Loc = Obj->GetObjectField("location");
            Location.X = Loc->GetNumberField("x");
            Location.Y = Loc->GetNumberField("y");
            Location.Z = Loc->GetNumberField("z");
        }

        AActor* Actor = World->SpawnActor<AActor>(
            ClassRegistry[ClassName],
            Location,
            FRotator::ZeroRotator
        );

        if (!Actor)
            continue;

        SpawnedEntities.Add(Actor);

        if (ABasePawn* Pawn = Cast<ABasePawn>(Actor))
        {
            Pawn->SetEntityId(
                Obj->GetStringField("id")
            );

            if (Obj->HasField("params"))
            {
                Pawn->InitializeFromData(
                    Obj->GetObjectField("params")
                );
            }
        }
    }
}


void WorldStateManager::SaveEntities()
{
    TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
    TArray<TSharedPtr<FJsonValue>> EntitiesArray;

    for (AActor* Actor : SpawnedEntities)
    {
        if (!Actor)
            continue;

        TSharedPtr<FJsonObject> Obj = MakeShared<FJsonObject>();

        
        if (ABasePawn* Pawn = Cast<ABasePawn>(Actor))
        {
            Obj->SetStringField(
                "id",
                Pawn->GetEntityId()
            );
        }

        
        Obj->SetStringField(
            "class",
            Actor->GetClass()->GetName()
        );

        
        FVector L = Actor->GetActorLocation();
        TSharedPtr<FJsonObject> Loc = MakeShared<FJsonObject>();
        Loc->SetNumberField("x", L.X);
        Loc->SetNumberField("y", L.Y);
        Loc->SetNumberField("z", L.Z);
        Obj->SetObjectField("location", Loc);

        EntitiesArray.Add(
            MakeShared<FJsonValueObject>(Obj)
        );
    }

    Root->SetArrayField("entities", EntitiesArray);

    FString Output;
    TSharedRef<TJsonWriter<>> Writer =
        TJsonWriterFactory<>::Create(&Output);

    FJsonSerializer::Serialize(
        Root.ToSharedRef(),
        Writer
    );

    FFileHelper::SaveStringToFile(
        Output,
        *(FPaths::ProjectContentDir() + "Source/entities.json")
    );
}


