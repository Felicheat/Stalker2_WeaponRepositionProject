﻿// Copyright JAA Contributors 2024-2025

#include "Importers/Types/Tables/DataTableImporter.h"
#include "Dom/JsonObject.h"

// Shout-out to UEAssetToolkit
bool IDataTableImporter::Import() {
	TSharedPtr<FJsonObject> AssetData = JsonObject->GetObjectField(TEXT("Properties"));
	UDataTable* DataTable = NewObject<UDataTable>(Package, UDataTable::StaticClass(), *FileName, RF_Public | RF_Standalone);
	
	// ScriptClass for the Data Table
	FString TableStruct; {
		// --- Properties --> RowStruct --> ObjectName
		// --- Class'StructClass' --> StructClass
		AssetData->GetObjectField(TEXT("RowStruct"))
			->GetStringField(TEXT("ObjectName")).
			Split("'", nullptr, &TableStruct);
		TableStruct.Split("'", &TableStruct, nullptr);
	}

	// Find Table Row Struct
	UScriptStruct* TableRowStruct = FindObject<UScriptStruct>(ANY_PACKAGE, *TableStruct); {
		if (TableRowStruct == NULL) {
			AppendNotification(FText::FromString("DataTable Missing: " + TableStruct), FText::FromString(FileName), 2.0f, SNotificationItem::CS_Fail, true, 350.0f);

			return false;
		} else {
			DataTable->RowStruct = TableRowStruct;
		}
	}

	// Access Property Serializer
	UPropertySerializer* ObjectPropertySerializer = GetObjectSerializer()->GetPropertySerializer();
	TSharedPtr<FJsonObject> RowData = JsonObject->GetObjectField(TEXT("Rows"));

	// Loop throughout row data, and deserialize
	for (TPair<FString, TSharedPtr<FJsonValue>>& Pair : RowData->Values) {
		TSharedPtr<FStructOnScope> ScopedStruct = MakeShareable(new FStructOnScope(TableRowStruct));
		TSharedPtr<FJsonObject> StructData = Pair.Value->AsObject();

		// Deserialize, add row
		ObjectPropertySerializer->DeserializeStruct(TableRowStruct, StructData.ToSharedRef(), ScopedStruct->GetStructMemory());
		DataTable->AddRow(*Pair.Key, *(const FTableRowBase*)ScopedStruct->GetStructMemory());
	}

	// Handle edit changes, and add it to the content browser
	return OnAssetCreation(DataTable);
}
