﻿#pragma once

// Name: mace, Version: 1.9.1.12285


/*!!DEFINE!!*/

/*!!HELPER_DEF!!*/

/*!!HELPER_INC!!*/

#ifdef _MSC_VER
	#pragma pack(push, 0x01)
#endif

namespace CG
{
//---------------------------------------------------------------------------
// Classes
//---------------------------------------------------------------------------

// BlueprintGeneratedClass ObjectInfo_ResupplyPickup.ObjectInfo_ResupplyPickup_C
// 0x0000 (FullSize[0x00F8] - InheritedSize[0x00F8])
class UObjectInfo_ResupplyPickup_C : public UObjectInfo
{
public:


	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;
		if(!ptr){
			ptr = UObject::FindClass("BlueprintGeneratedClass ObjectInfo_ResupplyPickup.ObjectInfo_ResupplyPickup_C");
		}
		return ptr;
	}



	struct FText GetActionText(class AActor* Actor);
};

}

#ifdef _MSC_VER
	#pragma pack(pop)
#endif
