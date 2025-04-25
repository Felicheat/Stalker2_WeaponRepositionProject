// Copyright JAA Contributors 2024-2025

#include "Utilities/MathUtilities.h"
#include "Dom/JsonObject.h"

FVector FMathUtilities::ObjectToVector(const FJsonObject* Object) {
	return FVector(Object->GetNumberField(TEXT("X")), Object->GetNumberField(TEXT("Y")), Object->GetNumberField(TEXT("Z")));
}

#if ENGINE_MAJOR_VERSION >= 5
FVector3f FMathUtilities::ObjectToVector3f(const FJsonObject* Object) {
	return FVector3f(Object->GetNumberField(TEXT("X")), Object->GetNumberField(TEXT("Y")), Object->GetNumberField(TEXT("Z")));
}

FVector4f FMathUtilities::ObjectToVector4f(const FJsonObject* Object) {
	return FVector4f(Object->GetNumberField(TEXT("X")), Object->GetNumberField(TEXT("Y")), Object->GetNumberField(TEXT("Z")));
}

#else
FVector FMathUtilities::ObjectToVector3f(const FJsonObject* Object) {
	return FVector(Object->GetNumberField(TEXT("X")), Object->GetNumberField(TEXT("Y")), Object->GetNumberField(TEXT("Z")));
}

FVector FMathUtilities::ObjectToVector4f(const FJsonObject* Object) {
	return FVector(Object->GetNumberField(TEXT("X")), Object->GetNumberField(TEXT("Y")), Object->GetNumberField(TEXT("Z")));
}
#endif

FRotator FMathUtilities::ObjectToRotator(const FJsonObject* Object) {
	return FRotator(Object->GetNumberField(TEXT("Pitch")), Object->GetNumberField(TEXT("Yaw")), Object->GetNumberField(TEXT("Roll")));
}

FQuat FMathUtilities::ObjectToQuat(const FJsonObject* Object) {
	return FQuat(Object->GetNumberField(TEXT("X")), Object->GetNumberField(TEXT("Y")), Object->GetNumberField(TEXT("Z")), Object->GetNumberField(TEXT("W")));
}

FLinearColor FMathUtilities::ObjectToLinearColor(const FJsonObject* Object) {
	return FLinearColor(Object->GetNumberField(TEXT("R")), Object->GetNumberField(TEXT("G")), Object->GetNumberField(TEXT("B")), Object->GetNumberField(TEXT("A")));
}

FColor FMathUtilities::ObjectToColor(const FJsonObject* Object) {
	return ObjectToLinearColor(Object).ToFColor(true);
}

FLightingChannels FMathUtilities::ObjectToLightingChannels(const FJsonObject* Object) {
	FLightingChannels LightingChannels = FLightingChannels();

	LightingChannels.bChannel0 = Object->GetBoolField(TEXT("bChannel0"));
	LightingChannels.bChannel1 = Object->GetBoolField(TEXT("bChannel1"));
	LightingChannels.bChannel2 = Object->GetBoolField(TEXT("bChannel2"));

	return LightingChannels;
}

FFloatInterval FMathUtilities::ObjectToFloatInterval(const FJsonObject* Object) {
	return FFloatInterval(Object->GetNumberField(TEXT("Min")), Object->GetNumberField(TEXT("Max")));
}

FRichCurveKey FMathUtilities::ObjectToRichCurveKey(const TSharedPtr<FJsonObject>& Object) {
	FString InterpMode = Object->GetStringField(TEXT("InterpMode"));
	return FRichCurveKey(Object->GetNumberField(TEXT("Time")), Object->GetNumberField(TEXT("Value")), Object->GetNumberField(TEXT("ArriveTangent")), Object->GetNumberField(TEXT("LeaveTangent")), static_cast<ERichCurveInterpMode>(StaticEnum<ERichCurveInterpMode>()->GetValueByNameString(InterpMode)));
}
