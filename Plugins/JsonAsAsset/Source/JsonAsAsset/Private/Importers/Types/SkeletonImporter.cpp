// Copyright JAA Contributors 2024-2025

#include "Importers/Types/SkeletonImporter.h"

#include "Animation/BlendProfile.h"
#include "Dom/JsonObject.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Utilities/MathUtilities.h"

bool CSkeletonAssetDerived::AddVirtualBone(const FName SourceBoneName, const FName TargetBoneName, const FName VirtualBoneRootName) {
	for (const FVirtualBone& SSBone : VirtualBones) {
		if (SSBone.SourceBoneName == SourceBoneName && SSBone.TargetBoneName == TargetBoneName) {
			return false;
		}
	}

	Modify();

	FVirtualBone VirtualBone = FVirtualBone(SourceBoneName, TargetBoneName);
	VirtualBone.VirtualBoneName = VirtualBoneRootName;

	VirtualBones.Add(VirtualBone);

	VirtualBoneGuid = FGuid::NewGuid();
	check(VirtualBoneGuid.IsValid());

	return true;
}

bool ISkeletonImporter::Import() {
	TSharedPtr<FJsonObject> Properties = JsonObject->GetObjectField(TEXT("Properties"));
	USkeleton* Skeleton = GetSelectedAsset<USkeleton>();

	// Must have a skeleton selected
	if (!Skeleton) return false;

	CSkeletonAssetDerived* SkeletonAsset = Cast<CSkeletonAssetDerived>(Skeleton);

	const TArray<TSharedPtr<FJsonValue>>& BoneTree = Properties->GetArrayField(TEXT("BoneTree"));

	for (int i = 0; i < BoneTree.Num(); i++) {
		if (!BoneTree[i].IsValid()) {
			continue;
		}

		const TSharedPtr<FJsonObject> BoneNode = BoneTree[i]->AsObject();
		if (!BoneNode.IsValid()) {
			continue;
		}

		FString TranslationRetargetingMode;
		if (!BoneNode->TryGetStringField(TEXT("TranslationRetargetingMode"), TranslationRetargetingMode)) {
			continue;
		}

		int64 EnumValue = StaticEnum<EBoneTranslationRetargetingMode::Type>()->GetValueByNameString(TranslationRetargetingMode);
		
		if (EnumValue == INDEX_NONE) {
			UE_LOG(LogTemp, Warning, TEXT("Invalid TranslationRetargetingMode: %s"), *TranslationRetargetingMode);
			continue;
		}

		Skeleton->SetBoneTranslationRetargetingMode(i, static_cast<EBoneTranslationRetargetingMode::Type>(EnumValue), false);
	}

	for (const TSharedPtr<FJsonValue> SlotGroupValue : Properties->GetArrayField(TEXT("SlotGroups"))) {
		const TSharedPtr<FJsonObject> SlotGroupObject = SlotGroupValue->AsObject();

		FString GroupName = SlotGroupObject->GetStringField(TEXT("GroupName"));
		TArray<TSharedPtr<FJsonValue>> SlotNamesArray = SlotGroupObject->GetArrayField(TEXT("SlotNames"));

		for (const TSharedPtr<FJsonValue> SlotName : SlotNamesArray) {
			Skeleton->Modify();
			Skeleton->SetSlotGroupName(FName(*SlotName->AsString()), FName(*GroupName));
		}
	}

	for (const TSharedPtr<FJsonValue> VirtualBoneValue : Properties->GetArrayField(TEXT("VirtualBones"))) {
		const TSharedPtr<FJsonObject> VirtualBoneObject = VirtualBoneValue->AsObject();

		const FString SourceBone = VirtualBoneObject->GetStringField(TEXT("SourceBoneName"));
		const FString TargetBone = VirtualBoneObject->GetStringField(TEXT("TargetBoneName"));
		const FString VirtualBone = VirtualBoneObject->GetStringField(TEXT("VirtualBoneName"));

		SkeletonAsset->AddVirtualBone(FName(*SourceBone), FName(*TargetBone), FName(*VirtualBone));
	}

	for (const TSharedPtr<FJsonValue> SecondaryPurposeValueObject : AllJsonObjects) {
		const TSharedPtr<FJsonObject> SecondaryPurposeObject = SecondaryPurposeValueObject->AsObject();

		FString SecondaryPurposeType = SecondaryPurposeObject->GetStringField(TEXT("Type"));
		FString SecondaryPurposeName = SecondaryPurposeObject->GetStringField(TEXT("Name"));

		if (SecondaryPurposeType == "BlendProfile") {
			const TSharedPtr<FJsonObject> SecondaryPurposeProperties = SecondaryPurposeObject->GetObjectField(TEXT("Properties"));
			bool bIsAlreadyCreated = false;

			for (const UBlendProfile* BlendProfileC : Skeleton->BlendProfiles) {
				if (!bIsAlreadyCreated) {
					bIsAlreadyCreated = BlendProfileC->GetFName() == FName(*SecondaryPurposeName);
				}
			}

			if (!bIsAlreadyCreated) {
				Skeleton->Modify();

				UBlendProfile* BlendProfile = NewObject<UBlendProfile>(Skeleton, *SecondaryPurposeName, RF_Public | RF_Transactional);
				Skeleton->BlendProfiles.Add(BlendProfile);

				for (const TSharedPtr<FJsonValue> ProfileEntryValue : SecondaryPurposeProperties->GetArrayField(TEXT("ProfileEntries"))) {
					const TSharedPtr<FJsonObject> ProfileEntry = ProfileEntryValue->AsObject();

					Skeleton->Modify();

#if ENGINE_MAJOR_VERSION < 5
					BlendProfile->SetBoneBlendScale(FName(*ProfileEntry->GetObjectField(TEXT("BoneReference"))->GetStringField(TEXT("BoneName"))), ProfileEntry->GetNumberField(TEXT("BlendScale")), false, true);
#endif
				}
			}
		}

		if (SecondaryPurposeType == "SkeletalMeshSocket") {
			TSharedPtr<FJsonObject> SecondaryPurposeProperties = SecondaryPurposeObject->GetObjectField(TEXT("Properties"));

			FString SocketName = SecondaryPurposeProperties->GetStringField(TEXT("SocketName"));
			FString BoneName = SecondaryPurposeProperties->GetStringField(TEXT("BoneName"));

			USkeletalMeshSocket* Socket = NewObject<USkeletalMeshSocket>(Skeleton);
			Socket->SocketName = FName(*SocketName);
			Socket->BoneName = FName(*BoneName);

			bool bIsAlreadyCreated = false;

			for (USkeletalMeshSocket* SocketO : Skeleton->Sockets) {
				if (!bIsAlreadyCreated) {
					bIsAlreadyCreated = SocketO->SocketName == FName(*SecondaryPurposeName);
				}
			}

			if (!bIsAlreadyCreated) {
				const TSharedPtr<FJsonObject>* RelativeLocationObjectVector;
				const TSharedPtr<FJsonObject>* RelativeScaleObjectVector;
				const TSharedPtr<FJsonObject>* RelativeRotationObjectRotator;
				if (SecondaryPurposeProperties->TryGetObjectField(TEXT("RelativeRotation"), RelativeRotationObjectRotator) == true)
					Socket->RelativeRotation = FMathUtilities::ObjectToRotator(RelativeRotationObjectRotator->Get());
				if (SecondaryPurposeProperties->TryGetObjectField(TEXT("RelativeLocation"), RelativeLocationObjectVector) == true)
					Socket->RelativeLocation = FMathUtilities::ObjectToVector(RelativeLocationObjectVector->Get());
				if (SecondaryPurposeProperties->TryGetObjectField(TEXT("RelativeScale"), RelativeScaleObjectVector) == true)
					Socket->RelativeScale = FMathUtilities::ObjectToVector(RelativeScaleObjectVector->Get());

				bool bForceAlwaysAnimated;
				if (SecondaryPurposeProperties->TryGetBoolField(TEXT("RelativeScale"), bForceAlwaysAnimated) == true)
					Socket->bForceAlwaysAnimated = bForceAlwaysAnimated;

				Skeleton->Modify();
				Skeleton->Sockets.Add(Socket);
			}
		}
	}

	return true;
}
