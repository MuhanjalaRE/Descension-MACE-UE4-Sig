﻿// Name: mace, Version: 1.9.1.12285

#include "../pch.h"

/*!!DEFINE!!*/

/*!!HELPER_DEF!!*/

/*!!HELPER_INC!!*/

#ifdef _MSC_VER
	#pragma pack(push, 0x01)
#endif

namespace CG
{
//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

// Function WBP_PunishVoteDisplay.WBP_PunishVoteDisplay_C.Get_VoteOptionNo_Text_1
// (Public, HasOutParms, HasDefaults, BlueprintCallable, BlueprintEvent, BlueprintPure)
// Parameters:
// struct FText                   ReturnValue                    (Parm, OutParm, ReturnParm)
struct FText UWBP_PunishVoteDisplay_C::Get_VoteOptionNo_Text_1()
{
	static UFunction* fn = nullptr;
	if(!fn){
		fn = UObject::FindObject<UFunction>("Function WBP_PunishVoteDisplay.WBP_PunishVoteDisplay_C.Get_VoteOptionNo_Text_1");
	}

	UWBP_PunishVoteDisplay_C_Get_VoteOptionNo_Text_1_Params params;

	auto flags = fn->FunctionFlags;

	UObject::ProcessEvent(fn, &params);
	fn->FunctionFlags = flags;


	return params.ReturnValue;
}


// Function WBP_PunishVoteDisplay.WBP_PunishVoteDisplay_C.Get_VoteOptionYes_Text_1
// (Public, HasOutParms, HasDefaults, BlueprintCallable, BlueprintEvent, BlueprintPure)
// Parameters:
// struct FText                   ReturnValue                    (Parm, OutParm, ReturnParm)
struct FText UWBP_PunishVoteDisplay_C::Get_VoteOptionYes_Text_1()
{
	static UFunction* fn = nullptr;
	if(!fn){
		fn = UObject::FindObject<UFunction>("Function WBP_PunishVoteDisplay.WBP_PunishVoteDisplay_C.Get_VoteOptionYes_Text_1");
	}

	UWBP_PunishVoteDisplay_C_Get_VoteOptionYes_Text_1_Params params;

	auto flags = fn->FunctionFlags;

	UObject::ProcessEvent(fn, &params);
	fn->FunctionFlags = flags;


	return params.ReturnValue;
}


// Function WBP_PunishVoteDisplay.WBP_PunishVoteDisplay_C.Get_VoteDisplayNameText_Text_1
// (Public, HasOutParms, HasDefaults, BlueprintCallable, BlueprintEvent, BlueprintPure)
// Parameters:
// struct FText                   ReturnValue                    (Parm, OutParm, ReturnParm)
struct FText UWBP_PunishVoteDisplay_C::Get_VoteDisplayNameText_Text_1()
{
	static UFunction* fn = nullptr;
	if(!fn){
		fn = UObject::FindObject<UFunction>("Function WBP_PunishVoteDisplay.WBP_PunishVoteDisplay_C.Get_VoteDisplayNameText_Text_1");
	}

	UWBP_PunishVoteDisplay_C_Get_VoteDisplayNameText_Text_1_Params params;

	auto flags = fn->FunctionFlags;

	UObject::ProcessEvent(fn, &params);
	fn->FunctionFlags = flags;


	return params.ReturnValue;
}


// Function WBP_PunishVoteDisplay.WBP_PunishVoteDisplay_C.Construct
// (BlueprintCosmetic, Event, Public, BlueprintEvent)
void UWBP_PunishVoteDisplay_C::Construct()
{
	static UFunction* fn = nullptr;
	if(!fn){
		fn = UObject::FindObject<UFunction>("Function WBP_PunishVoteDisplay.WBP_PunishVoteDisplay_C.Construct");
	}

	UWBP_PunishVoteDisplay_C_Construct_Params params;

	auto flags = fn->FunctionFlags;

	UObject::ProcessEvent(fn, &params);
	fn->FunctionFlags = flags;

}


// Function WBP_PunishVoteDisplay.WBP_PunishVoteDisplay_C.Tick
// (BlueprintCosmetic, Event, Public, BlueprintEvent)
// Parameters:
// struct FGeometry               MyGeometry                     (BlueprintVisible, BlueprintReadOnly, Parm, IsPlainOldData, NoDestructor)
// float                          InDeltaTime                    (BlueprintVisible, BlueprintReadOnly, Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
void UWBP_PunishVoteDisplay_C::Tick(const struct FGeometry& MyGeometry, float InDeltaTime)
{
	static UFunction* fn = nullptr;
	if(!fn){
		fn = UObject::FindObject<UFunction>("Function WBP_PunishVoteDisplay.WBP_PunishVoteDisplay_C.Tick");
	}

	UWBP_PunishVoteDisplay_C_Tick_Params params;
	params.MyGeometry = MyGeometry;
	params.InDeltaTime = InDeltaTime;

	auto flags = fn->FunctionFlags;

	UObject::ProcessEvent(fn, &params);
	fn->FunctionFlags = flags;

}


// Function WBP_PunishVoteDisplay.WBP_PunishVoteDisplay_C.ConditionallyContinueAnimation
// (BlueprintCallable, BlueprintEvent)
void UWBP_PunishVoteDisplay_C::ConditionallyContinueAnimation()
{
	static UFunction* fn = nullptr;
	if(!fn){
		fn = UObject::FindObject<UFunction>("Function WBP_PunishVoteDisplay.WBP_PunishVoteDisplay_C.ConditionallyContinueAnimation");
	}

	UWBP_PunishVoteDisplay_C_ConditionallyContinueAnimation_Params params;

	auto flags = fn->FunctionFlags;

	UObject::ProcessEvent(fn, &params);
	fn->FunctionFlags = flags;

}


// Function WBP_PunishVoteDisplay.WBP_PunishVoteDisplay_C.ExecuteUbergraph_WBP_PunishVoteDisplay
// (Final, HasDefaults)
// Parameters:
// int                            EntryPoint                     (BlueprintVisible, BlueprintReadOnly, Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
void UWBP_PunishVoteDisplay_C::ExecuteUbergraph_WBP_PunishVoteDisplay(int EntryPoint)
{
	static UFunction* fn = nullptr;
	if(!fn){
		fn = UObject::FindObject<UFunction>("Function WBP_PunishVoteDisplay.WBP_PunishVoteDisplay_C.ExecuteUbergraph_WBP_PunishVoteDisplay");
	}

	UWBP_PunishVoteDisplay_C_ExecuteUbergraph_WBP_PunishVoteDisplay_Params params;
	params.EntryPoint = EntryPoint;

	auto flags = fn->FunctionFlags;

	UObject::ProcessEvent(fn, &params);
	fn->FunctionFlags = flags;

}


}

#ifdef _MSC_VER
	#pragma pack(pop)
#endif
