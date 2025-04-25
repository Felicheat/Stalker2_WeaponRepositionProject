// Copyright JAA Contributors 2024-2025

#pragma once

#include "Math/Color.h"

class JSONASASSET_API FMathUtilities {
public:
	static FVector ObjectToVector(const FJsonObject* Object);

	/* FVector3 and FVector4 only exist on UE5 so just make them a FVector on UE4 */
#if ENGINE_MAJOR_VERSION == 5
	static FVector3f ObjectToVector3f(const FJsonObject* Object);
	static FVector4f ObjectToVector4f(const FJsonObject* Object);
#else
	static FVector ObjectToVector3f(const FJsonObject* Object);
	static FVector ObjectToVector4f(const FJsonObject* Object);
#endif
	
	static FRotator ObjectToRotator(const FJsonObject* Object);
	static FQuat ObjectToQuat(const FJsonObject* Object);
	static FLinearColor ObjectToLinearColor(const FJsonObject* Object);
	static FColor ObjectToColor(const FJsonObject* Object);
	static FLightingChannels ObjectToLightingChannels(const FJsonObject* Object);
	static FFloatInterval ObjectToFloatInterval(const FJsonObject* Object);
	static FRichCurveKey ObjectToRichCurveKey(const TSharedPtr<FJsonObject>& Object);
};