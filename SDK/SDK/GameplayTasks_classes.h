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

// Class GameplayTasks.GameplayTaskResource
// 0x0010 (FullSize[0x0038] - InheritedSize[0x0028])
class UGameplayTaskResource : public UObject
{
public:
	int                                                ManualResourceID;                                          // 0x0028(0x0004) (Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor, Config, DisableEditOnInstance, IsPlainOldData, NoDestructor, Protected, HasGetValueTypeHash, NativeAccessSpecifierProtected)
	int8_t                                             AutoResourceID;                                            // 0x002C(0x0001) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPrivate)
	unsigned char                                      UnknownData_APLH[0x3];                                     // 0x002D(0x0003) MISSED OFFSET (FIX SPACE BETWEEN PREVIOUS PROPERTY)
	unsigned char                                      bManuallySetID : 1;                                        // 0x0030(0x0001) BIT_FIELD (Edit, DisableEditOnInstance, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	unsigned char                                      UnknownData_0SP9[0x7];                                     // 0x0031(0x0007) MISSED OFFSET (PADDING)


	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;
		if(!ptr){
			ptr = UObject::FindClass("Class GameplayTasks.GameplayTaskResource");
		}
		return ptr;
	}



};

// Class GameplayTasks.GameplayTasksComponent
// 0x0070 (FullSize[0x0148] - InheritedSize[0x00D8])
class UGameplayTasksComponent : public UActorComponent
{
public:
	unsigned char                                      UnknownData_C044[0xC];                                     // 0x00D8(0x000C) MISSED OFFSET (FIX SPACE BETWEEN PREVIOUS PROPERTY)
	unsigned char                                      UnknownData_RTBZ : 1;                                      // 0x00E4(0x0001) BIT_FIELD (PADDING)
	unsigned char                                      bIsNetDirty : 1;                                           // 0x00E4(0x0001) BIT_FIELD (NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	unsigned char                                      UnknownData_BVB4[0x3];                                     // 0x00E5(0x0003) MISSED OFFSET (FIX SPACE BETWEEN PREVIOUS PROPERTY)
	TArray<class UGameplayTask*>                       SimulatedTasks;                                            // 0x00E8(0x0010) (Net, ZeroConstructor, RepNotify, Protected, NativeAccessSpecifierProtected)
	TArray<class UGameplayTask*>                       TaskPriorityQueue;                                         // 0x00F8(0x0010) (ZeroConstructor, Protected, NativeAccessSpecifierProtected)
	unsigned char                                      UnknownData_6RK7[0x10];                                    // 0x0108(0x0010) MISSED OFFSET (FIX SPACE BETWEEN PREVIOUS PROPERTY)
	TArray<class UGameplayTask*>                       TickingTasks;                                              // 0x0118(0x0010) (ZeroConstructor, Protected, NativeAccessSpecifierProtected)
	TArray<class UGameplayTask*>                       KnownTasks;                                                // 0x0128(0x0010) (ZeroConstructor, Transient, Protected, NativeAccessSpecifierProtected)
	struct FScriptMulticastDelegate                    OnClaimedResourcesChange;                                  // 0x0138(0x0010) (BlueprintVisible, ZeroConstructor, InstancedReference, NativeAccessSpecifierPublic)


	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;
		if(!ptr){
			ptr = UObject::FindClass("Class GameplayTasks.GameplayTasksComponent");
		}
		return ptr;
	}



	void OnRep_SimulatedTasks();
	GameplayTasks_EGameplayTaskRunResult STATIC_K2_RunGameplayTask(const TScriptInterface<class UGameplayTaskOwnerInterface>& TaskOwner, class UGameplayTask* Task, unsigned char Priority, TArray<class UClass*> AdditionalRequiredResources, TArray<class UClass*> AdditionalClaimedResources);
};

// Class GameplayTasks.GameplayTask
// 0x0040 (FullSize[0x0068] - InheritedSize[0x0028])
class UGameplayTask : public UObject
{
public:
	unsigned char                                      UnknownData_P4K6[0x8];                                     // 0x0028(0x0008) MISSED OFFSET (FIX SPACE BETWEEN PREVIOUS PROPERTY)
	struct FName                                       InstanceName;                                              // 0x0030(0x0008) (ZeroConstructor, IsPlainOldData, NoDestructor, Protected, HasGetValueTypeHash, NativeAccessSpecifierProtected)
	unsigned char                                      UnknownData_C9XV[0x2];                                     // 0x0038(0x0002) MISSED OFFSET (FIX SPACE BETWEEN PREVIOUS PROPERTY)
	GameplayTasks_ETaskResourceOverlapPolicy           ResourceOverlapPolicy;                                     // 0x003A(0x0001) (ZeroConstructor, Config, IsPlainOldData, NoDestructor, Protected, HasGetValueTypeHash, NativeAccessSpecifierProtected)
	unsigned char                                      UnknownData_P6AG[0x25];                                    // 0x003B(0x0025) MISSED OFFSET (FIX SPACE BETWEEN PREVIOUS PROPERTY)
	class UGameplayTask*                               ChildTask;                                                 // 0x0060(0x0008) (ZeroConstructor, IsPlainOldData, NoDestructor, Protected, HasGetValueTypeHash, NativeAccessSpecifierProtected)


	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;
		if(!ptr){
			ptr = UObject::FindClass("Class GameplayTasks.GameplayTask");
		}
		return ptr;
	}



	void ReadyForActivation();
	void GenericGameplayTaskDelegate__DelegateSignature();
	void EndTask();
};

// Class GameplayTasks.GameplayTask_ClaimResource
// 0x0000 (FullSize[0x0068] - InheritedSize[0x0068])
class UGameplayTask_ClaimResource : public UGameplayTask
{
public:


	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;
		if(!ptr){
			ptr = UObject::FindClass("Class GameplayTasks.GameplayTask_ClaimResource");
		}
		return ptr;
	}



	class UGameplayTask_ClaimResource* STATIC_ClaimResources(const TScriptInterface<class UGameplayTaskOwnerInterface>& InTaskOwner, TArray<class UClass*> ResourceClasses, unsigned char Priority, const struct FName& TaskInstanceName);
	class UGameplayTask_ClaimResource* STATIC_ClaimResource(const TScriptInterface<class UGameplayTaskOwnerInterface>& InTaskOwner, class UClass* ResourceClass, unsigned char Priority, const struct FName& TaskInstanceName);
};

// Class GameplayTasks.GameplayTask_SpawnActor
// 0x0040 (FullSize[0x00A8] - InheritedSize[0x0068])
class UGameplayTask_SpawnActor : public UGameplayTask
{
public:
	struct FScriptMulticastDelegate                    Success;                                                   // 0x0068(0x0010) (ZeroConstructor, InstancedReference, BlueprintAssignable, NativeAccessSpecifierPublic)
	struct FScriptMulticastDelegate                    DidNotSpawn;                                               // 0x0078(0x0010) (ZeroConstructor, InstancedReference, BlueprintAssignable, NativeAccessSpecifierPublic)
	unsigned char                                      UnknownData_PUJY[0x18];                                    // 0x0088(0x0018) MISSED OFFSET (FIX SPACE BETWEEN PREVIOUS PROPERTY)
	class UClass*                                      ClassToSpawn;                                              // 0x00A0(0x0008) (ZeroConstructor, IsPlainOldData, NoDestructor, Protected, UObjectWrapper, HasGetValueTypeHash, NativeAccessSpecifierProtected)


	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;
		if(!ptr){
			ptr = UObject::FindClass("Class GameplayTasks.GameplayTask_SpawnActor");
		}
		return ptr;
	}



	class UGameplayTask_SpawnActor* STATIC_SpawnActor(const TScriptInterface<class UGameplayTaskOwnerInterface>& TaskOwner, const struct FVector& SpawnLocation, const struct FRotator& SpawnRotation, class UClass* Class, bool bSpawnOnlyOnAuthority);
	void FinishSpawningActor(class UObject* WorldContextObject, class AActor* SpawnedActor);
	bool BeginSpawningActor(class UObject* WorldContextObject, class AActor** SpawnedActor);
};

// Class GameplayTasks.GameplayTask_TimeLimitedExecution
// 0x0030 (FullSize[0x0098] - InheritedSize[0x0068])
class UGameplayTask_TimeLimitedExecution : public UGameplayTask
{
public:
	struct FScriptMulticastDelegate                    OnFinished;                                                // 0x0068(0x0010) (ZeroConstructor, InstancedReference, BlueprintAssignable, NativeAccessSpecifierPublic)
	struct FScriptMulticastDelegate                    OnTimeExpired;                                             // 0x0078(0x0010) (ZeroConstructor, InstancedReference, BlueprintAssignable, NativeAccessSpecifierPublic)
	unsigned char                                      UnknownData_Z3TE[0x10];                                    // 0x0088(0x0010) MISSED OFFSET (PADDING)


	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;
		if(!ptr){
			ptr = UObject::FindClass("Class GameplayTasks.GameplayTask_TimeLimitedExecution");
		}
		return ptr;
	}



};

// Class GameplayTasks.GameplayTask_WaitDelay
// 0x0018 (FullSize[0x0080] - InheritedSize[0x0068])
class UGameplayTask_WaitDelay : public UGameplayTask
{
public:
	struct FScriptMulticastDelegate                    OnFinish;                                                  // 0x0068(0x0010) (ZeroConstructor, InstancedReference, BlueprintAssignable, NativeAccessSpecifierPublic)
	unsigned char                                      UnknownData_LV92[0x8];                                     // 0x0078(0x0008) MISSED OFFSET (PADDING)


	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;
		if(!ptr){
			ptr = UObject::FindClass("Class GameplayTasks.GameplayTask_WaitDelay");
		}
		return ptr;
	}



	class UGameplayTask_WaitDelay* STATIC_TaskWaitDelay(const TScriptInterface<class UGameplayTaskOwnerInterface>& TaskOwner, float Time, unsigned char Priority);
	void TaskDelayDelegate__DelegateSignature();
};

// Class GameplayTasks.GameplayTaskOwnerInterface
// 0x0000 (FullSize[0x0028] - InheritedSize[0x0028])
class UGameplayTaskOwnerInterface : public UInterface
{
public:


	static UClass* StaticClass()
	{
		static UClass* ptr = nullptr;
		if(!ptr){
			ptr = UObject::FindClass("Class GameplayTasks.GameplayTaskOwnerInterface");
		}
		return ptr;
	}



};

}

#ifdef _MSC_VER
	#pragma pack(pop)
#endif
