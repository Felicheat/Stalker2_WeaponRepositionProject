// Copyright JAA Contributors 2024-2025

#include "Importers/Types/Audio/SoundCueImporter.h"
#include "Sound/SoundCue.h"

bool ISoundCueImporter::Import() {
	try {
		// Create Sound Cue
		USoundCue* SoundCue = NewObject<USoundCue>(Package, *FileName, RF_Public | RF_Standalone);
		SoundCue->PreEditChange(nullptr);

		TSharedPtr<FJsonObject> Properties = JsonObject->GetObjectField(TEXT("Properties"));
		
		// Start -------------------------------------------
		if (SoundCue) {
			TMap<FString, USoundNode*> SoundCueNodes;
			
			ConstructNodes(SoundCue, AllJsonObjects, SoundCueNodes);
			SetupNodes(SoundCue, SoundCueNodes, AllJsonObjects);
		}
		// END ---------------------------------------------

		GetObjectSerializer()->DeserializeObjectProperties(RemovePropertiesShared(Properties, TArray<FString>
		{
			"FirstNode"
		}), SoundCue);
		
		SoundCue->PostEditChange();
		SoundCue->CompileSoundNodesFromGraphNodes();

		return OnAssetCreation(SoundCue);
	} catch (const char* Exception) {
		UE_LOG(LogJson, Error, TEXT("%s"), *FString(Exception));
	}

	return false;
}
