// Copyright JAA Contributors 2024-2025

#include "Importers/Constructor/TemplatedImporter.h"

// Explicit instantiation of ITemplatedImporter for UObject
template class ITemplatedImporter<UObject>;

template <typename AssetType>
bool ITemplatedImporter<AssetType>::Import() {
	try {
		// Make Properties if it doesn't exist
		if (!JsonObject->HasField(TEXT("Properties"))) {
			JsonObject->SetObjectField(TEXT("Properties"), TSharedPtr<FJsonObject>());
		}
		
		TSharedPtr<FJsonObject> Properties = JsonObject->GetObjectField(TEXT("Properties"));
		GetObjectSerializer()->SetPackageForDeserialization(Package);

		AssetType* Asset = NewObject<AssetType>(Package, AssetClass ? AssetClass : AssetType::StaticClass(), FName(FileName), RF_Public | RF_Standalone);

		// Property MASH
		for (FString& PropertyName : PropertyMash) {
			if (JsonObject->HasField(PropertyName)) {
				TSharedPtr<FJsonValue> FieldValue = JsonObject->TryGetField(PropertyName);
				if (FieldValue.IsValid()) {
					Properties->SetField(PropertyName, FieldValue);
				}
			}
		}

		GetObjectSerializer()->DeserializeObjectProperties(Properties, Asset);

		return OnAssetCreation(Asset);
	} catch (const char* Exception) {
		UE_LOG(LogJson, Error, TEXT("%s"), *FString(Exception));
	}

	return false;
}
