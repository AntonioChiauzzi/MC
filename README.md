**Agricultural Digital Twin (Mirroring System)**

Sistema sviluppato in Unreal Engine 5 (C++) per il mirroring e la simulazione di un ambiente agricolo tramite un approccio data-driven basato su JSON.

**Panoramica**

Il progetto implementa un Digital Twin agricolo in cui lo stato del mondo viene definito tramite dati esterni e ricostruito a runtime all’interno di Unreal Engine.

Le entità vengono generate dinamicamente a partire da file JSON, permettendo di separare completamente la logica di mirroring dai dati.

**Obiettivo**
-rappresentare un ambiente agricolo reale tramite dati strutturati
-supportare mappe esterne caricabili a runtime
-mantenere un’architettura modulare ed estendibile

**Caratteristiche principali**
-sistema data-driven basato su JSON
-spawning dinamico delle entità
-supporto di mappe esterne
-launcher dedicato per configurazione e avvio
-override runtime di asset e dimensioni
-gestione centralizzata dello stato del mondo

**Requisiti**
-Unreal Engine 5.7.x
-Visual Studio 2022
    workload: sviluppo di giochi con C++

**Setup del progetto**
-Clonare la repository
-(Opzionale) Creare una mappa esterna con il plugin ExternalMaps
-Aprire MCLauncher.exe nella cartella Launcher
-Inserire i parametri desiderati
-Premere Start per avviare la build e il runtime
-Utilizzo

**Il launcher consente di:**
-selezionare il file JSON delle entità
-scegliere una mappa esterna (opzionale)
-configurare dimensioni del mondo e refresh rate
-applicare override agli asset (mesh e dimensioni)

**Una volta avviato:**

-il mondo viene caricato
-i dati vengono letti
-le entità vengono spawnate automaticamente

**Struttura dei dati**
entities.json
{
  "entities": [
    {
    "id": "id01",
		"name":"SensoreTerreno",
    "type": "GroundSensor",
			"location": {
				"x": 3000,
				"y": 150,
				"z": 0
			},
			"params": {
				"soilTemperature": 20,
				"soilHumidity": 70,
				"soilSolarIrradiance": 400,
				"soilpH": 7,
				"soilChemicalComposition": {
					"mineral": 45,
					"organic": 5,
					"air": 25,
					"water": 25
				}
			}
		}
  ]
}

**Tipologie di entità**
Drone
Tractor
WeatherStation
GroundSensor
Harvester
Dynamic

Ogni entità eredita da una classe base comune e può estendere comportamento e dati.

**Architettura**

Il sistema è organizzato in componenti principali:

-GameInstance: gestione configurazione runtime e caricamento mappe
-GameMode: orchestrazione iniziale del mondo
-WorldStateManager: parsing JSON e spawning entità
-MyBaseActor: classe base per tutte le entità
-Launcher: interfaccia utente per configurazione e avvio

**Flusso di esecuzione**
-Il launcher prepara la configurazione runtime
-I dati vengono salvati e passati al gioco
-Il GameInstance gestisce eventuale caricamento di mappe esterne
-Il GameMode inizializza il mondo
-Il WorldStateManager legge il JSON
-Le entità vengono spawnate e configurate
-Override runtime
-Controllo tramite free camera e possibilità di premere sull'entità per ricevere a schermo il loro stato

**Il sistema supporta override configurabili dal launcher:**

-sostituzione mesh degli asset
-modifica dimensioni (MaxDimensions)
-applicazione selettiva per singola entità

Gli override vengono applicati a runtime.

**Mappe esterne**

Le mappe possono essere caricate dinamicamente tramite l'uso di plugin esterno:
ExternalMaps
Per utilizzarlo scaricarlo dalla repository: https://github.com/AntonioChiauzzi/ExternalMaps
Flusso per creare mappe esterne:
-Creare nella directory del progetto esterno dove si vuole creare la mappa esterna la cartella Plugins
-Copiare il plugin all'interno della cartella
-Aprire il progetto esterno e assicurarsi che il plugin sia visibile e attivo
-Creare una mappa
-(Opzionale) Aggiungere material e mesh alla mappa
-Migrare mappa, material e mesh utilizzati nella cartella Plugins/ExternalMaps/Content