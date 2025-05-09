// Copyright JAA Contributors 2024-2025

#include "Utilities/Serializers/PropertyUtilities.h"

#include "GameplayTagContainer.h"
#include "Importers/Constructor/Importer.h"
#include "Utilities/Serializers/ObjectUtilities.h"
#include "UObject/TextProperty.h"

DECLARE_LOG_CATEGORY_CLASS(LogPropertySerializer, Error, Log);
PRAGMA_DISABLE_OPTIMIZATION

void FDateTimeSerializer::Serialize(UScriptStruct* Struct, const TSharedPtr<FJsonObject> JsonValue, const void* StructData, TArray<int32>* OutReferencedSubobjects) {
	const FDateTime* DateTime = (const FDateTime*)StructData;
	JsonValue->SetStringField(TEXT("Ticks"), FString::Printf(TEXT("%llu"), DateTime->GetTicks()));
}

void FDateTimeSerializer::Deserialize(UScriptStruct* Struct, void* StructData, const TSharedPtr<FJsonObject> JsonValue) {
	FDateTime* DateTime = (FDateTime*)StructData;
	const int64 Ticks = FCString::Atoi64(*JsonValue->GetStringField(TEXT("Ticks")));
	*DateTime = FDateTime(Ticks);
}

bool FDateTimeSerializer::Compare(UScriptStruct* Struct, const TSharedPtr<FJsonObject> JsonValue, const void* StructData, const TSharedPtr<FObjectCompareContext> Context) {
	const FDateTime* DateTime = (const FDateTime*)StructData;
	const int64 Ticks = FCString::Atoi64(*JsonValue->GetStringField(TEXT("Ticks")));
	return DateTime->GetTicks() == Ticks;
}

void FTimespanSerializer::Serialize(UScriptStruct* Struct, const TSharedPtr<FJsonObject> JsonValue, const void* StructData, TArray<int32>* OutReferencedSubobjects) {
	const FTimespan* Timespan = (const FTimespan*)StructData;
	JsonValue->SetStringField(TEXT("Ticks"), FString::Printf(TEXT("%llu"), Timespan->GetTicks()));
}

void FTimespanSerializer::Deserialize(UScriptStruct* Struct, void* StructData, const TSharedPtr<FJsonObject> JsonValue) {
	FTimespan* Timespan = (FTimespan*)StructData;
	const int64 Ticks = FCString::Atoi64(*JsonValue->GetStringField(TEXT("Ticks")));
	*Timespan = FTimespan(Ticks);
}

bool FTimespanSerializer::Compare(UScriptStruct* Struct, const TSharedPtr<FJsonObject> JsonValue, const void* StructData, const TSharedPtr<FObjectCompareContext> Context) {
	const FTimespan* Timespan = (const FTimespan*)StructData;
	const int64 Ticks = FCString::Atoi64(*JsonValue->GetStringField(TEXT("Ticks")));
	return Timespan->GetTicks() == Ticks;
}

FFallbackStructSerializer::FFallbackStructSerializer(UPropertySerializer* Serializer) : PropertySerializer(Serializer) {
}

void FFallbackStructSerializer::Serialize(UScriptStruct* Struct, const TSharedPtr<FJsonObject> JsonValue, const void* StructData, TArray<int32>* OutReferencedSubobjects) {
	for (FProperty* Property = Struct->PropertyLink; Property; Property = Property->PropertyLinkNext) {

		if (PropertySerializer->ShouldSerializeProperty(Property)) {
			const void* PropertyValue = Property->ContainerPtrToValuePtr<void>(StructData);

			const TSharedRef<FJsonValue> PropertyValueJson = PropertySerializer->SerializePropertyValue(Property, PropertyValue, OutReferencedSubobjects);
			JsonValue->SetField(Property->GetName(), PropertyValueJson);
		}
	}
}

void FFallbackStructSerializer::Deserialize(UScriptStruct* Struct, void* StructData, const TSharedPtr<FJsonObject> JsonValue) {
	for (FProperty* Property = Struct->PropertyLink; Property; Property = Property->PropertyLinkNext) {
		const FString PropertyName = Property->GetName();

		if (PropertySerializer->ShouldSerializeProperty(Property)) {
			void* PropertyValue = Property->ContainerPtrToValuePtr<void>(StructData);

			bool HasHandledProperty = PassthroughPropertyHandler(Property, PropertyName, PropertyValue, JsonValue, PropertySerializer);

			if (!HasHandledProperty && JsonValue->HasField(PropertyName)) {
				const TSharedPtr<FJsonValue> ValueObject = JsonValue->Values.FindChecked(PropertyName);

				if (Property->ArrayDim == 1 || ValueObject->Type == EJson::Array)
				{
					PropertySerializer->DeserializePropertyValue(Property, ValueObject.ToSharedRef(), PropertyValue);
				}
			}
		}
	}
}

bool FFallbackStructSerializer::Compare(UScriptStruct* Struct, const TSharedPtr<FJsonObject> JsonValue, const void* StructData, const TSharedPtr<FObjectCompareContext> Context) {
	for (FProperty* Property = Struct->PropertyLink; Property; Property = Property->PropertyLinkNext) {
		const FString PropertyName = Property->GetName();

		if (PropertySerializer->ShouldSerializeProperty(Property) && JsonValue->HasField(PropertyName)) {
			const void* PropertyValue = Property->ContainerPtrToValuePtr<void>(StructData);
			const TSharedPtr<FJsonValue> ValueObject = JsonValue->Values.FindChecked(PropertyName);

			if (!PropertySerializer->ComparePropertyValues(Property, ValueObject.ToSharedRef(), PropertyValue, Context)) {
				return false;
			}
		}
	}

	return true;
}

UPropertySerializer::UPropertySerializer() {
	this->FallbackStructSerializer = MakeShared<FFallbackStructSerializer>(this);

	UScriptStruct* DateTimeStruct = FindObject<UScriptStruct>(NULL, TEXT("/Script/CoreUObject.DateTime"));
	UScriptStruct* TimespanStruct = FindObject<UScriptStruct>(NULL, TEXT("/Script/CoreUObject.Timespan"));
	check(DateTimeStruct);
	check(TimespanStruct);

	this->StructSerializers.Add(DateTimeStruct, MakeShared<FDateTimeSerializer>());
	this->StructSerializers.Add(TimespanStruct, MakeShared<FTimespanSerializer>());
}

void UPropertySerializer::DeserializePropertyValue(FProperty* Property, const TSharedRef<FJsonValue>& JsonValue, void* Value) {
	DeserializePropertyValueInner(Property, JsonValue, Value);
}

void UPropertySerializer::DeserializePropertyValueInner(FProperty* Property, const TSharedRef<FJsonValue>& JsonValue, void* Value) {
	const FMapProperty* MapProperty = CastField<const FMapProperty>(Property);
	const FSetProperty* SetProperty = CastField<const FSetProperty>(Property);
	const FArrayProperty* ArrayProperty = CastField<const FArrayProperty>(Property);

	TSharedRef<FJsonValue> NewJsonValue = JsonValue;

	if (NewJsonValue->IsNull()) return;

	if (MapProperty) {
		FProperty* KeyProperty = MapProperty->KeyProp;
		FProperty* ValueProperty = MapProperty->ValueProp;
		FScriptMapHelper MapHelper(MapProperty, Value);
		const TArray<TSharedPtr<FJsonValue>>& PairArray = NewJsonValue->AsArray();

		for (int32 i = 0; i < PairArray.Num(); i++) {
			const TSharedPtr<FJsonObject>& Pair = PairArray[i]->AsObject();
			const TSharedPtr<FJsonValue>& EntryKey = Pair->Values.FindChecked(TEXT("Key"));
			const TSharedPtr<FJsonValue>& EntryValue = Pair->Values.FindChecked(TEXT("Value"));
			const int32 Index = MapHelper.AddDefaultValue_Invalid_NeedsRehash();
			uint8* PairPtr = MapHelper.GetPairPtr(Index);

			// Copy over imported key and value from temporary storage
			DeserializePropertyValue(KeyProperty, EntryKey.ToSharedRef(), PairPtr);
			DeserializePropertyValue(ValueProperty, EntryValue.ToSharedRef(), PairPtr + MapHelper.MapLayout.ValueOffset);
		}
		MapHelper.Rehash();

	} else if (SetProperty) {
		FProperty* ElementProperty = SetProperty->ElementProp;
		FScriptSetHelper SetHelper(SetProperty, Value);
		const TArray<TSharedPtr<FJsonValue>>& SetArray = NewJsonValue->AsArray();
		SetHelper.EmptyElements();
		uint8* TempElementStorage = static_cast<uint8*>(FMemory::Malloc(ElementProperty->ElementSize));
		ElementProperty->InitializeValue(TempElementStorage);

		for (int32 i = 0; i < SetArray.Num(); i++) {
			const TSharedPtr<FJsonValue>& Element = SetArray[i];
			DeserializePropertyValue(ElementProperty, Element.ToSharedRef(), TempElementStorage);

			const int32 NewElementIndex = SetHelper.AddDefaultValue_Invalid_NeedsRehash();
			uint8* NewElementPtr = SetHelper.GetElementPtr(NewElementIndex);

			// Copy over imported key from temporary storage
			ElementProperty->CopyCompleteValue_InContainer(NewElementPtr, TempElementStorage);
		}
		SetHelper.Rehash();

		ElementProperty->DestroyValue(TempElementStorage);
		FMemory::Free(TempElementStorage);
	} else if (ArrayProperty) {
		FProperty* ElementProperty = ArrayProperty->Inner;
		FScriptArrayHelper ArrayHelper(ArrayProperty, Value);
		const TArray<TSharedPtr<FJsonValue>>& SetArray = NewJsonValue->AsArray();
		ArrayHelper.EmptyValues();

		for (int32 i = 0; i < SetArray.Num(); i++) {
			const TSharedPtr<FJsonValue>& Element = SetArray[i];
			const uint32 AddedIndex = ArrayHelper.AddValue();
			uint8* ValuePtr = ArrayHelper.GetRawPtr(AddedIndex);
			DeserializePropertyValue(ElementProperty, Element.ToSharedRef(), ValuePtr);
		}
	}
	else if (Property->IsA<FMulticastDelegateProperty>()) {
	} else if (Property->IsA<FDelegateProperty>()) {
	} else if (const FInterfaceProperty* InterfaceProperty = CastField<const FInterfaceProperty>(Property)) {
		// UObject is enough to re-create value, since we known property on deserialization
		FScriptInterface* Interface = static_cast<FScriptInterface*>(Value);
		UObject* Object = ObjectSerializer ? ObjectSerializer->DeserializeObject((int32)NewJsonValue->AsNumber()) : NULL;
		if (Object != nullptr) {
			void* InterfacePtr = Object->GetInterfaceAddress(InterfaceProperty->InterfaceClass);
			check(InterfacePtr != nullptr);
			Interface->SetObject(Object);
			Interface->SetInterface(InterfacePtr);
		}
	}
	else if (FSoftObjectProperty* SoftObjectProperty = CastField<FSoftObjectProperty>(Property)) {
		TSharedPtr<FJsonObject> SoftJsonObjectProperty;
		FString PathString = "";
		
		switch (NewJsonValue->Type)
		{
			// FModel, extract it from the object
			case EJson::Object:
				SoftJsonObjectProperty = NewJsonValue->AsObject();
				PathString = SoftJsonObjectProperty->GetStringField(TEXT("AssetPathName"));
			break;

			// Older versions of FModel ??
			// Never encountered a case of this except on older game builds
			default:
				PathString = NewJsonValue->AsString();
			break;
		}

		if (PathString != "")
		{
			FSoftObjectPtr* ObjectPtr = static_cast<FSoftObjectPtr*>(Value);
			*ObjectPtr = FSoftObjectPath(PathString);

			if (!ObjectPtr->LoadSynchronous())
			{
				// Try importing it using Local Fetch
				IImporter* Importer = new IImporter();
				FString PackagePath;
				FString AssetName;
				PathString.Split(".", &PackagePath, &AssetName);
				TObjectPtr<UObject> T;

				FString PropertyClassName = SoftObjectProperty->PropertyClass->GetName();
				
				Importer->DownloadWrapper(T, PropertyClassName, AssetName, PackagePath);
			}
		}
	}
	else if (const FObjectPropertyBase* ObjectProperty = CastField<const FObjectPropertyBase>(Property)) {
		// Need to serialize full UObject for object property
		TObjectPtr<UObject> Object = NULL;

		auto JsonValueAsObject = NewJsonValue->AsObject();
		bool bUseDefaultLoadObject = !JsonValueAsObject->GetStringField(TEXT("ObjectName")).Contains(":ParticleModule");

		if (bUseDefaultLoadObject)
		{
			// Use IImporter to import the object
			IImporter* Importer = new IImporter();

			Importer->ParentObject = ObjectSerializer->ParentAsset;
			Importer->LoadObject(&JsonValueAsObject, Object);

			if (Object == nullptr)
			{
				if (ObjectProperty && ObjectProperty->PropertyClass)
				{
					UStruct* Struct = ObjectProperty->PropertyClass;

					FFailedPropertyInfo PropertyInfo;
					PropertyInfo.ClassName = ObjectProperty->PropertyClass->GetName();
					PropertyInfo.SuperStructName = Struct->GetSuperStruct() ? Struct->GetSuperStruct()->GetName() : TEXT("None");
					PropertyInfo.ObjectPath = JsonValueAsObject->GetStringField(TEXT("ObjectPath"));
					
					if (!FailedProperties.Contains(PropertyInfo))
					{
						FailedProperties.Add(PropertyInfo);
					}
				}
			}

			if ((Object != nullptr) && !Cast<UActorComponent>(Object.Get()))
			{
				ObjectProperty->SetObjectPropertyValue(Value, Object);
			}

			if (Object != nullptr) {
				// Get the export
				if (TSharedPtr<FJsonObject> Export = GetExport(JsonValueAsObject.Get(), ObjectSerializer->AllObjectsReference))
				{
					if (Export->HasField(TEXT("Properties")))
					{
						TSharedPtr<FJsonObject> Properties = Export->GetObjectField(TEXT("Properties"));

						if (Export->HasField(TEXT("LODData")))
						{
							Properties->SetArrayField(TEXT("LODData"), Export->GetArrayField(TEXT("LODData")));
						}
						
						ObjectSerializer->DeserializeObjectProperties(Properties, Object);
					}
				}
			}
		}

		FString ObjectName = JsonValueAsObject->GetStringField(TEXT("ObjectName"));

		if (UObject** FoundObjectPtr = ReferencedObjects.Find(ObjectName)) {
			UObject* FoundObject = *FoundObjectPtr;

			if (FoundObject) {
				ObjectProperty->SetObjectPropertyValue(Value, FoundObject);
			}
		}

		if (ObjectName.StartsWith("Distribution")) {
			FString DistributionSecondaryName;
			ObjectName.Split(".", nullptr, &DistributionSecondaryName);
			DistributionSecondaryName = DistributionSecondaryName.Replace(TEXT("'"), TEXT(""));

			if (UObject** FoundObjectPtr = ReferencedObjects.Find(DistributionSecondaryName)) {
				UObject* FoundObject = *FoundObjectPtr;

				if (FoundObject) {
					ObjectProperty->SetObjectPropertyValue(Value, FoundObject);
				}
			}
		}
	}
	else if (const FStructProperty* StructProperty = CastField<const FStructProperty>(Property)) {
		// FGameplayTag
		if (StructProperty->Struct == FGameplayTag::StaticStruct())
		{
			FGameplayTag* GameplayTagStr = static_cast<FGameplayTag*>(Value);
			*GameplayTagStr = FGameplayTag::RequestGameplayTag(FName(*NewJsonValue->AsString()));
			
			return;
		}

		// FGameplayTagContainer (handled from FModel data)
		if (StructProperty->Struct == FGameplayTagContainer::StaticStruct())
		{
			FGameplayTagContainer* GameplayTagContainerStr = static_cast<FGameplayTagContainer*>(Value);

			auto GameplayTags = JsonValue->AsArray();

			for (TSharedPtr<FJsonValue> GameplayTagValue : GameplayTags)
			{
				FString GameplayTagString = GameplayTagValue->AsString();
				FGameplayTag GameplayTag = FGameplayTag::RequestGameplayTag(FName(*GameplayTagString));
				
				GameplayTagContainerStr->AddTag(GameplayTag);
			}
			
			return;
		}

		if (StructProperty->Struct->GetFName() == "SoftObjectPath")
		{
			TSharedPtr<FJsonObject> SoftJsonObjectProperty;
			FString PathString = "";
		
			SoftJsonObjectProperty = NewJsonValue->AsObject();
			PathString = SoftJsonObjectProperty->GetStringField(TEXT("AssetPathName"));
			
			if (PathString != "")
			{
				FSoftObjectPtr* ObjectPtr = static_cast<FSoftObjectPtr*>(Value);
				*ObjectPtr = FSoftObjectPath(PathString);

				if (!ObjectPtr->LoadSynchronous())
				{
					// Try importing it using Local Fetch
					IImporter* Importer = new IImporter();
					FString PackagePath;
					FString AssetName;
					PathString.Split(".", &PackagePath, &AssetName);
					TObjectPtr<UObject> T;

					FString PropertyClassName = "DataAsset";
				
					Importer->DownloadWrapper(T, PropertyClassName, AssetName, PackagePath);
				}
			}
		}
		
		// JSON for FGuids are FStrings
		FString OutString;
		
		if (JsonValue->TryGetString(OutString)) {
			FGuid GUID = FGuid(OutString); // Create GUID from String

			TSharedRef<FJsonObject> SharedObject = MakeShareable(new FJsonObject());
			SharedObject->SetNumberField(TEXT("A"), GUID.A); SharedObject->SetNumberField(TEXT("B"), GUID.B);
			SharedObject->SetNumberField(TEXT("C"), GUID.C); SharedObject->SetNumberField(TEXT("D"), GUID.D);

			const TSharedRef<FJsonValue> NewValue = MakeShareable(new FJsonValueObject(SharedObject));
			NewJsonValue = NewValue;
		}

		// To serialize struct, we need it's type and value pointer, because struct value doesn't contain type information
		DeserializeStruct(StructProperty->Struct, NewJsonValue->AsObject().ToSharedRef(), Value);
	}
	else if (const FByteProperty* ByteProperty = CastField<const FByteProperty>(Property)) {
		// If we have a string provided, make sure Enum is not null
		if (JsonValue->Type == EJson::String) {
			FString EnumAsString = JsonValue->AsString();

			check(ByteProperty->Enum);
			int64 EnumerationValue = ByteProperty->Enum->GetValueByNameString(EnumAsString);

			ByteProperty->SetIntPropertyValue(Value, EnumerationValue);
		}
		else {
			// Should be a number, set property value accordingly
			const int64 NumberValue = (int64)NewJsonValue->AsNumber();
			ByteProperty->SetIntPropertyValue(Value, NumberValue);
		}
		// Primitives below, they are serialized as plain json values
	}
	else if (const FNumericProperty* NumberProperty = CastField<const FNumericProperty>(Property)) {
		const double NumberValue = NewJsonValue->AsNumber();
		if (NumberProperty->IsFloatingPoint())
			NumberProperty->SetFloatingPointPropertyValue(Value, NumberValue);
		else NumberProperty->SetIntPropertyValue(Value, static_cast<int64>(NumberValue));
	}
	else if (const FBoolProperty* BoolProperty = CastField<const FBoolProperty>(Property)) {
		const bool bBooleanValue = NewJsonValue->AsBool();
		BoolProperty->SetPropertyValue(Value, bBooleanValue);
	}
	else if (Property->IsA<FStrProperty>()) {
		const FString StringValue = NewJsonValue->AsString();
		*static_cast<FString*>(Value) = StringValue;
	}
	else if (const FEnumProperty* EnumProperty = CastField<const FEnumProperty>(Property)) {
		const FString EnumAsString = NewJsonValue->AsString();

		// Prefer readable enum names in result json to raw numbers
		int64 EnumerationValue = EnumProperty->GetEnum()->GetValueByNameString(EnumAsString);
		
		if (ensure(EnumerationValue != INDEX_NONE)) {
			EnumProperty->GetUnderlyingProperty()->SetIntPropertyValue(Value, EnumerationValue);
		}
	}
	else if (Property->IsA<FNameProperty>()) {
		// Name is perfectly representable as string
		const FString NameString = NewJsonValue->AsString();
		*static_cast<FName*>(Value) = *NameString;
	}
	else if (const FTextProperty* TextProperty = CastField<const FTextProperty>(Property)) {
		// For FText, standard ExportTextItem is okay to use, because it's serialization is quite complex
		const FString SerializedValue = NewJsonValue->AsString();
		if (!SerializedValue.IsEmpty()) {
			FTextStringHelper::ReadFromBuffer(*SerializedValue, *static_cast<FText*>(Value));
		} else {
			// TODO: Somehow add other needed things like Namespace, Key, and LocalizedString 
			TSharedPtr<FJsonObject> Object = NewJsonValue->AsObject().ToSharedRef();

			// Retrieve properties
			FString TextNamespace = Object->GetStringField(TEXT("Namespace"));
			FString UniqueKey = Object->GetStringField(TEXT("Key"));
			FString SourceString = Object->GetStringField(TEXT("SourceString"));

			TextProperty->SetPropertyValue(Value, FInternationalization::ForUseOnlyByLocMacroAndGraphNodeTextLiterals_CreateText(*SourceString, *TextNamespace, *UniqueKey));
		}
	}
	else if (const FFieldPathProperty* FieldPathProperty = CastField<const FFieldPathProperty>(Property)) {
		FFieldPath FieldPath;
		FieldPath.Generate(*NewJsonValue->AsString());
		*static_cast<FFieldPath*>(Value) = FieldPath;
	}
	else {
		UE_LOG(LogPropertySerializer, Fatal, TEXT("Found unsupported property type when deserializing value: %s"), *Property->GetClass()->GetName());
	}
}

void UPropertySerializer::ClearCachedData()
{
	FailedProperties.Empty();
}

void UPropertySerializer::DisablePropertySerialization(UStruct* Struct, FName PropertyName) {
	FProperty* Property = Struct->FindPropertyByName(PropertyName);
	checkf(Property, TEXT("Cannot find Property %s in Struct %s"), *PropertyName.ToString(), *Struct->GetPathName());
	this->PinnedStructs.Add(Struct);
	this->BlacklistedProperties.Add(Property);
}

void UPropertySerializer::AddStructSerializer(UScriptStruct* Struct, const TSharedPtr<FStructSerializer>& Serializer) {
	this->PinnedStructs.Add(Struct);
	this->StructSerializers.Add(Struct, Serializer);
}

bool UPropertySerializer::ShouldSerializeProperty(FProperty* Property) const {
	// Skip transient properties
	if (Property->HasAnyPropertyFlags(CPF_Transient)) {
		return true;
	}
	// Skip editor only properties altogether
	if (Property->HasAnyPropertyFlags(CPF_EditorOnly)) {
		return true;
	}
	// Skip deprecated properties
	if (Property->HasAnyPropertyFlags(CPF_Deprecated)) {
		return false;
	}
	if (this == nullptr) {
		return true;
	}
	// Skip blacklisted properties
	if (BlacklistedProperties.Num() > 0 && BlacklistedProperties.Contains(Property)) {
		return false;
	}
	return true;
}

TSharedRef<FJsonValue> UPropertySerializer::SerializePropertyValue(FProperty* Property, const void* Value, TArray<int32>* OutReferencedSubobjects) {
	// Serialize statically sized array properties
	if (Property->ArrayDim != 1) {
		TArray<TSharedPtr<FJsonValue>> OutJsonValueArray;
		for (int32 ArrayIndex = 0; ArrayIndex < Property->ArrayDim; ArrayIndex++) {
			const uint8* ArrayPropertyValue = (const uint8*)Value + Property->ElementSize * ArrayIndex;
			const TSharedRef<FJsonValue> ElementValue = SerializePropertyValueInner(Property, ArrayPropertyValue, OutReferencedSubobjects);
			OutJsonValueArray.Add(ElementValue);
		}
		return MakeShareable(new FJsonValueArray(OutJsonValueArray));
	} else {
		return SerializePropertyValueInner(Property, Value, OutReferencedSubobjects);
	}
}

TSharedRef<FJsonValue> UPropertySerializer::SerializePropertyValueInner(FProperty* Property, const void* Value, TArray<int32>* OutReferencedSubobjects) {
	const FMapProperty* MapProperty = CastField<const FMapProperty>(Property);
	const FSetProperty* SetProperty = CastField<const FSetProperty>(Property);
	const FArrayProperty* ArrayProperty = CastField<const FArrayProperty>(Property);

	if (MapProperty) {
		FProperty* KeyProperty = MapProperty->KeyProp;
		FProperty* ValueProperty = MapProperty->ValueProp;
		FScriptMapHelper MapHelper(MapProperty, Value);
		TArray<TSharedPtr<FJsonValue>> ResultArray;
		for (int32 i = 0; i < MapHelper.Num(); i++) {
			TSharedPtr<FJsonValue> EntryKey = SerializePropertyValue(KeyProperty, MapHelper.GetKeyPtr(i), OutReferencedSubobjects);
			TSharedPtr<FJsonValue> EntryValue = SerializePropertyValue(ValueProperty, MapHelper.GetValuePtr(i), OutReferencedSubobjects);
			TSharedRef<FJsonObject> Pair = MakeShareable(new FJsonObject());
			Pair->SetField(TEXT("Key"), EntryKey);
			Pair->SetField(TEXT("Value"), EntryValue);
			ResultArray.Add(MakeShareable(new FJsonValueObject(Pair)));
		}

		return MakeShareable(new FJsonValueArray(ResultArray));
	}

	if (SetProperty) {
		FProperty* ElementProperty = SetProperty->ElementProp;
		FScriptSetHelper SetHelper(SetProperty, Value);
		TArray<TSharedPtr<FJsonValue>> ResultArray;
		for (int32 i = 0; i < SetHelper.Num(); i++) {
			TSharedPtr<FJsonValue> Element = SerializePropertyValue(ElementProperty, SetHelper.GetElementPtr(i), OutReferencedSubobjects);
			ResultArray.Add(Element);
		}
		return MakeShareable(new FJsonValueArray(ResultArray));
	}

	if (ArrayProperty) {
		FProperty* ElementProperty = ArrayProperty->Inner;
		FScriptArrayHelper ArrayHelper(ArrayProperty, Value);
		TArray<TSharedPtr<FJsonValue>> ResultArray;
		for (int32 i = 0; i < ArrayHelper.Num(); i++) {
			TSharedPtr<FJsonValue> Element = SerializePropertyValue(ElementProperty, ArrayHelper.GetRawPtr(i), OutReferencedSubobjects);
			ResultArray.Add(Element);
		}

		return MakeShareable(new FJsonValueArray(ResultArray));
	}

	if (Property->IsA<FMulticastDelegateProperty>()) {
		FMulticastScriptDelegate* MulticastScriptDelegate = (FMulticastScriptDelegate*)Value;

		return MakeShareable(new FJsonValueString(TEXT("##NOT SERIALIZED##")));
	}

	if (Property->IsA<FDelegateProperty>()) {
		return MakeShareable(new FJsonValueString(TEXT("##NOT SERIALIZED##")));
	}

	if (Property->IsA<FInterfaceProperty>()) {
		//UObject is enough to re-create value, since we known property on deserialization
		const FScriptInterface* Interface = reinterpret_cast<const FScriptInterface*>(Value);
		int32 ObjectIndex = ObjectSerializer ? ObjectSerializer->SerializeObject(Interface->GetObject()) : 0;

		if (OutReferencedSubobjects) {
			OutReferencedSubobjects->AddUnique(ObjectIndex);
		}

		return MakeShareable(new FJsonValueNumber(ObjectIndex));
	}

	if (const FObjectPropertyBase* ObjectProperty = CastField<const FObjectPropertyBase>(Property)) {
		//Need to serialize full UObject for object property
		UObject* ObjectPointer = ObjectProperty->GetObjectPropertyValue(Value);
		int32 ObjectIndex = ObjectSerializer ? ObjectSerializer->SerializeObject(ObjectPointer) : 0;

		if (OutReferencedSubobjects) {
			OutReferencedSubobjects->AddUnique(ObjectIndex);
		}

		return MakeShareable(new FJsonValueNumber(ObjectIndex));
	}

	if (const FStructProperty* StructProperty = CastField<const FStructProperty>(Property)) {
		//To serialize struct, we need it's type and value pointer, because struct value doesn't contain type information
		return MakeShareable(new FJsonValueObject(SerializeStruct(StructProperty->Struct, Value, OutReferencedSubobjects)));
	}

	if (Property->IsA<FSoftObjectProperty>()) {
		//For soft object reference, path is enough too for deserialization.
		const FSoftObjectPtr* ObjectPtr = reinterpret_cast<const FSoftObjectPtr*>(Value);
		return MakeShareable(new FJsonValueString(ObjectPtr->ToSoftObjectPath().ToString()));
	}

	if (const FByteProperty* ByteProperty = CastField<const FByteProperty>(Property)) {
		//If Enum is NULL, property will be handled as standard UNumericProperty
		if (ByteProperty->Enum) {
			const int64 UnderlyingValue = ByteProperty->GetSignedIntPropertyValue(Value);
			const FString EnumName = ByteProperty->Enum->GetNameByValue(UnderlyingValue).ToString();
			return MakeShareable(new FJsonValueString(EnumName));
		}
	}

	if (const FNumericProperty* NumberProperty = CastField<const FNumericProperty>(Property)) {
		double ResultValue;
		if (NumberProperty->IsFloatingPoint())
			ResultValue = NumberProperty->GetFloatingPointPropertyValue(Value);
		else ResultValue = NumberProperty->GetSignedIntPropertyValue(Value);
		return MakeShareable(new FJsonValueNumber(ResultValue));
	}

	if (const FBoolProperty* BoolProperty = CastField<const FBoolProperty>(Property)) {
		const bool bBooleanValue = BoolProperty->GetPropertyValue(Value);
		return MakeShareable(new FJsonValueBoolean(bBooleanValue));
	}

	if (Property->IsA<FStrProperty>()) {
		const FString& StringValue = *reinterpret_cast<const FString*>(Value);
		return MakeShareable(new FJsonValueString(StringValue));
	}

	if (const FEnumProperty* EnumProperty = CastField<const FEnumProperty>(Property)) {
		const int64 UnderlyingValue = EnumProperty->GetUnderlyingProperty()->GetSignedIntPropertyValue(Value);
		const FString EnumName = EnumProperty->GetEnum()->GetNameByValue(UnderlyingValue).ToString();
		return MakeShareable(new FJsonValueString(EnumName));
	}

	if (Property->IsA<FNameProperty>()) {
		//Name is perfectly representable as string
		FName* Temp = ((FName*)Value);
		return MakeShareable(new FJsonValueString(Temp->ToString()));
	}

	if (const FTextProperty* TextProperty = CastField<const FTextProperty>(Property)) {
		FString ResultValue;
		const FText& TextValue = TextProperty->GetPropertyValue(Value);
		FTextStringHelper::WriteToBuffer(ResultValue, TextValue);
		return MakeShareable(new FJsonValueString(ResultValue));
	}

	if (Property->IsA<FFieldPathProperty>()) {
		FFieldPath* Temp = ((FFieldPath*)Value);
		return MakeShareable(new FJsonValueString(Temp->ToString()));
	}

	UE_LOG(LogPropertySerializer, Fatal, TEXT("Found unsupported property type when serializing value: %s"), *Property->GetClass()->GetName());
	return MakeShareable(new FJsonValueString(TEXT("#ERROR#")));
}

TSharedRef<FJsonObject> UPropertySerializer::SerializeStruct(UScriptStruct* Struct, const void* Value, TArray<int32>* OutReferencedSubobjects) {
	FStructSerializer* StructSerializer = GetStructSerializer(Struct);

	TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	StructSerializer->Serialize(Struct, JsonObject, Value, OutReferencedSubobjects);
	return JsonObject;
}

void UPropertySerializer::DeserializeStruct(UScriptStruct* Struct, const TSharedRef<FJsonObject>& Properties, void* OutValue) {
	FStructSerializer* StructSerializer = GetStructSerializer(Struct);
	StructSerializer->Deserialize(Struct, OutValue, Properties);
}

bool UPropertySerializer::ComparePropertyValues(FProperty* Property, const TSharedRef<FJsonValue>& JsonValue, const void* CurrentValue, const TSharedPtr<FObjectCompareContext> Context) {

	if (Property->ArrayDim != 1) {
		const TArray<TSharedPtr<FJsonValue>>& ArrayElements = JsonValue->AsArray();
		check(ArrayElements.Num() == Property->ArrayDim);

		for (int32 ArrayIndex = 0; ArrayIndex < Property->ArrayDim; ArrayIndex++) {
			const uint8* ArrayPropertyValue = (const uint8*)CurrentValue + Property->ElementSize * ArrayIndex;
			const TSharedRef<FJsonValue> ArrayJsonValue = ArrayElements[ArrayIndex].ToSharedRef();

			if (!ComparePropertyValuesInner(Property, ArrayJsonValue, ArrayPropertyValue, Context)) {
				return false;
			}
		}
		return true;
	}
	return ComparePropertyValuesInner(Property, JsonValue, CurrentValue, Context);
}

bool UPropertySerializer::ComparePropertyValuesInner(FProperty* Property, const TSharedRef<FJsonValue>& JsonValue, const void* CurrentValue, const TSharedPtr<FObjectCompareContext> Context) {
	const FMapProperty* MapProperty = CastField<const FMapProperty>(Property);
	const FSetProperty* SetProperty = CastField<const FSetProperty>(Property);
	const FArrayProperty* ArrayProperty = CastField<const FArrayProperty>(Property);

	if (MapProperty) {
		FProperty* KeyProperty = MapProperty->KeyProp;
		FProperty* ValueProperty = MapProperty->ValueProp;
		FScriptMapHelper MapHelper(MapProperty, CurrentValue);

		// Try to fail early to not attempt expensive map comparison operations
		const TArray<TSharedPtr<FJsonValue>>& PairArray = JsonValue->AsArray();
		if (PairArray.Num() != MapHelper.Num()) {
			return false;
		}

		// Iterate all json pairs and try to find matching map pairs for them
		for (int32 i = 0; i < PairArray.Num(); i++) {
			const TSharedPtr<FJsonObject>& Pair = PairArray[i]->AsObject();
			const TSharedPtr<FJsonValue>& EntryKey = Pair->Values.FindChecked(TEXT("Key"));
			const TSharedPtr<FJsonValue>& EntryValue = Pair->Values.FindChecked(TEXT("Value"));

			// Try to find an existing pair matching this one
			bool bFoundMatchingPair = false;
			for (int32 j = 0; j < MapHelper.Num(); j++) {
				const void* CurrentPairKeyPtr = MapHelper.GetKeyPtr(j);
				const void* CurrentPairValuePtr = MapHelper.GetValuePtr(j);

				// If both checks succeeded, we found a matching pair, check the next pair in the main array
				if (ComparePropertyValues(KeyProperty, EntryKey.ToSharedRef(), CurrentPairKeyPtr, Context) &&
					ComparePropertyValues(ValueProperty, EntryValue.ToSharedRef(), CurrentPairValuePtr, Context)) {

					bFoundMatchingPair = true;
					break;
				}
			}
			// If we didn't find a matching pair, we return false now
			if (!bFoundMatchingPair) {
				return false;
			}
		}

		// We can only end up here if we found a matching pair for every element, so return truE
		return true;
	}

	if (SetProperty) {
		FProperty* ElementProperty = SetProperty->ElementProp;
		FScriptSetHelper SetHelper(SetProperty, CurrentValue);
		const TArray<TSharedPtr<FJsonValue>>& SetArray = JsonValue->AsArray();

		// Try to return early if amount of elements doesn't match and avoid expensive array iteration
		if (SetArray.Num() != SetHelper.Num()) {
			return false;
		}

		//Iterate every value in json array and try to find a matching pair in the existing set values
		for (int32 i = 0; i < SetArray.Num(); i++) {
			const TSharedPtr<FJsonValue>& Element = SetArray[i];

			//Try to find an existing pair matching this one
			bool bFoundMatchingPair = false;
			for (int32 j = 0; j < SetHelper.Num(); j++) {
				const void* CurrentElementValue = SetHelper.GetElementPtr(j);
				if (ComparePropertyValues(ElementProperty, Element.ToSharedRef(), CurrentElementValue, Context)) {
					bFoundMatchingPair = true;
					break;
				}
			}
			//If we didn't find a matching pair, we return false now
			if (!bFoundMatchingPair) {
				return false;
			}
		}
		return true;
	}

	if (ArrayProperty) {
		FProperty* ElementProperty = ArrayProperty->Inner;
		FScriptArrayHelper ArrayHelper(ArrayProperty, CurrentValue);
		const TArray<TSharedPtr<FJsonValue>>& JsonArray = JsonValue->AsArray();

		//Try to return early if amount of elements doesn't match and avoid expensive array iteration
		if (JsonArray.Num() != ArrayHelper.Num()) {
			return false;
		}

		//Order of elements in arrays matter so arrays mismatch if elements at the same indices differ
		for (int32 i = 0; i < JsonArray.Num(); i++) {
			const TSharedPtr<FJsonValue>& Element = JsonArray[i];
			const void* CurrentElementValue = ArrayHelper.GetRawPtr(i);

			if (!ComparePropertyValues(ElementProperty, Element.ToSharedRef(), CurrentElementValue, Context)) {
				return false;
			}
		}
		return true;
	}

	if (Property->IsA<FInterfaceProperty>()) {
		//Interface properties are equal if objects they refer to are equal
		const FScriptInterface* Interface = static_cast<const FScriptInterface*>(CurrentValue);
		UObject* InterfaceObject = Interface->GetObject();
		const int32 InterfaceObjectIndex = JsonValue->AsNumber();

		return ObjectSerializer->CompareObjectsWithContext(InterfaceObjectIndex, InterfaceObject, Context);
	}

	if (const FObjectPropertyBase* ObjectProperty = CastField<const FObjectPropertyBase>(Property)) {
		//Need to serialize full UObject for object property
		UObject* PropertyObject = ObjectProperty->GetObjectPropertyValue(CurrentValue);
		const int32 ObjectIndex = JsonValue->AsNumber();

		return ObjectSerializer->CompareObjectsWithContext(ObjectIndex, PropertyObject, Context);
	}

	//To serialize struct, we need it's type and value pointer, because struct value doesn't contain type information
	if (const FStructProperty* StructProperty = CastField<const FStructProperty>(Property)) {
		return CompareStructs(StructProperty->Struct, JsonValue->AsObject().ToSharedRef(), CurrentValue, Context);
	}

	//If property hasn't been handled above, we can just deserialize it normally and then do FProperty.Identical
	FDefaultConstructedPropertyElement DeserializedElement(Property);
	//We use DeserializePropertyValueInner here because we handle statically sized array properties externally, so we need to bypass their handling
	DeserializePropertyValueInner(Property, JsonValue, DeserializedElement.GetObjAddress());

	if (const FTextProperty* TextProperty = CastField<const FTextProperty>(Property)) {
		// FTextProperty::Identical compares the CultureInvariant flag, and sometimes empty deserialized texts don't have it while the exiting texts do
		if (TextProperty->GetPropertyValue(CurrentValue).IsEmpty() && TextProperty->GetPropertyValue(DeserializedElement.GetObjAddress()).IsEmpty())
			return true;
	}

	return Property->Identical(CurrentValue, DeserializedElement.GetObjAddress(), PPF_None);
}

bool UPropertySerializer::CompareStructs(UScriptStruct* Struct, const TSharedRef<FJsonObject>& JsonValue, const void* CurrentValue, const TSharedPtr<FObjectCompareContext> Context) {
	FStructSerializer* StructSerializer = GetStructSerializer(Struct);
	return StructSerializer->Compare(Struct, JsonValue, CurrentValue, Context);
}

FStructSerializer* UPropertySerializer::GetStructSerializer(UScriptStruct* Struct) const {
	check(Struct);
	TSharedPtr<FStructSerializer> const* StructSerializer = StructSerializers.Find(Struct);
	return StructSerializer && ensure(StructSerializer->IsValid()) ? StructSerializer->Get() : FallbackStructSerializer.Get();
}

PRAGMA_ENABLE_OPTIMIZATION