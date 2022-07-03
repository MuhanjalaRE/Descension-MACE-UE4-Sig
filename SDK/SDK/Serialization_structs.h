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
// Script Structs
//---------------------------------------------------------------------------

// ScriptStruct Serialization.StructSerializerNumericTestStruct
// 0x0030
struct FStructSerializerNumericTestStruct
{
	int8_t                                             Int8;                                                      // 0x0000(0x0001) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	unsigned char                                      UnknownData_UDAW[0x1];                                     // 0x0001(0x0001) MISSED OFFSET (FIX SPACE BETWEEN PREVIOUS PROPERTY)
	int16_t                                            Int16;                                                     // 0x0002(0x0002) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	int                                                Int32;                                                     // 0x0004(0x0004) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	int64_t                                            Int64;                                                     // 0x0008(0x0008) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	unsigned char                                      UInt8;                                                     // 0x0010(0x0001) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	unsigned char                                      UnknownData_V6AU[0x1];                                     // 0x0011(0x0001) MISSED OFFSET (FIX SPACE BETWEEN PREVIOUS PROPERTY)
	uint16_t                                           UInt16;                                                    // 0x0012(0x0002) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	uint32_t                                           UInt32;                                                    // 0x0014(0x0004) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	uint64_t                                           UInt64;                                                    // 0x0018(0x0008) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	float                                              Float;                                                     // 0x0020(0x0004) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	unsigned char                                      UnknownData_4DQS[0x4];                                     // 0x0024(0x0004) MISSED OFFSET (FIX SPACE BETWEEN PREVIOUS PROPERTY)
	double                                             Double;                                                    // 0x0028(0x0008) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)

};

// ScriptStruct Serialization.StructSerializerBooleanTestStruct
// 0x0003
struct FStructSerializerBooleanTestStruct
{
	bool                                               BoolFalse;                                                 // 0x0000(0x0001) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	bool                                               BoolTrue;                                                  // 0x0001(0x0001) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	unsigned char                                      Bitfield0 : 1;                                             // 0x0002(0x0001) BIT_FIELD (NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	unsigned char                                      Bitfield1 : 1;                                             // 0x0002(0x0001) BIT_FIELD (NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	unsigned char                                      Bitfield2Set : 1;                                          // 0x0002(0x0001) BIT_FIELD (NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	unsigned char                                      Bitfield3 : 1;                                             // 0x0002(0x0001) BIT_FIELD (NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	unsigned char                                      Bitfield4Set : 1;                                          // 0x0002(0x0001) BIT_FIELD (NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	unsigned char                                      Bitfield5Set : 1;                                          // 0x0002(0x0001) BIT_FIELD (NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	unsigned char                                      Bitfield6 : 1;                                             // 0x0002(0x0001) BIT_FIELD (NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	unsigned char                                      Bitfield7Set : 1;                                          // 0x0002(0x0001) BIT_FIELD (NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)

};

// ScriptStruct Serialization.StructSerializerBuiltinTestStruct
// 0x0090
struct FStructSerializerBuiltinTestStruct
{
	struct FGuid                                       Guid;                                                      // 0x0000(0x0010) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	struct FName                                       Name;                                                      // 0x0010(0x0008) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	struct FString                                     String;                                                    // 0x0018(0x0010) (ZeroConstructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	struct FText                                       Text;                                                      // 0x0028(0x0018) (NativeAccessSpecifierPublic)
	struct FVector                                     Vector;                                                    // 0x0040(0x000C) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	unsigned char                                      UnknownData_9T4E[0x4];                                     // 0x004C(0x0004) MISSED OFFSET (FIX SPACE BETWEEN PREVIOUS PROPERTY)
	struct FVector4                                    Vector4;                                                   // 0x0050(0x0010) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	struct FRotator                                    Rotator;                                                   // 0x0060(0x000C) (ZeroConstructor, IsPlainOldData, NoDestructor, NativeAccessSpecifierPublic)
	unsigned char                                      UnknownData_SRFX[0x4];                                     // 0x006C(0x0004) MISSED OFFSET (FIX SPACE BETWEEN PREVIOUS PROPERTY)
	struct FQuat                                       Quat;                                                      // 0x0070(0x0010) (IsPlainOldData, NoDestructor, NativeAccessSpecifierPublic)
	struct FColor                                      Color;                                                     // 0x0080(0x0004) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	unsigned char                                      UnknownData_N1TO[0xC];                                     // 0x0084(0x000C) MISSED OFFSET (PADDING)

};

// ScriptStruct Serialization.StructSerializerObjectTestStruct
// 0x00A0
struct FStructSerializerObjectTestStruct
{
	class UClass*                                      Class;                                                     // 0x0000(0x0008) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	class UClass*                                      SubClass;                                                  // 0x0008(0x0008) (ZeroConstructor, IsPlainOldData, NoDestructor, UObjectWrapper, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	unsigned char                                      SoftClass[0x28];                                           // 0x0010(0x0028) UNKNOWN PROPERTY: SoftClassProperty Serialization.StructSerializerObjectTestStruct.SoftClass
	class UObject*                                     Object;                                                    // 0x0038(0x0008) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	TWeakObjectPtr<class UMetaData>                    WeakObject;                                                // 0x0040(0x0008) (ZeroConstructor, IsPlainOldData, NoDestructor, UObjectWrapper, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	unsigned char                                      SoftObject[0x28];                                          // 0x0048(0x0028) UNKNOWN PROPERTY: SoftObjectProperty Serialization.StructSerializerObjectTestStruct.SoftObject
	struct FSoftClassPath                              ClassPath;                                                 // 0x0070(0x0018) (ZeroConstructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	struct FSoftObjectPath                             ObjectPath;                                                // 0x0088(0x0018) (ZeroConstructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)

};

// ScriptStruct Serialization.StructSerializerArrayTestStruct
// 0x0050
struct FStructSerializerArrayTestStruct
{
	TArray<int>                                        Int32Array;                                                // 0x0000(0x0010) (ZeroConstructor, NativeAccessSpecifierPublic)
	int                                                StaticSingleElement;                                       // 0x0010(0x0004) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	int                                                StaticInt32Array[0x3];                                     // 0x0014(0x000C) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	float                                              StaticFloatArray[0x3];                                     // 0x0020(0x000C) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	unsigned char                                      UnknownData_RMZR[0x4];                                     // 0x002C(0x0004) MISSED OFFSET (FIX SPACE BETWEEN PREVIOUS PROPERTY)
	TArray<struct FVector>                             VectorArray;                                               // 0x0030(0x0010) (ZeroConstructor, NativeAccessSpecifierPublic)
	TArray<struct FStructSerializerBuiltinTestStruct>  StructArray;                                               // 0x0040(0x0010) (ZeroConstructor, NativeAccessSpecifierPublic)

};

// ScriptStruct Serialization.StructSerializerMapTestStruct
// 0x0140
struct FStructSerializerMapTestStruct
{
	TMap<int, struct FString>                          IntToStr;                                                  // 0x0000(0x0050) (ZeroConstructor, NativeAccessSpecifierPublic)
	TMap<struct FString, struct FString>               StrToStr;                                                  // 0x0050(0x0050) (ZeroConstructor, NativeAccessSpecifierPublic)
	TMap<struct FString, struct FVector>               StrToVec;                                                  // 0x00A0(0x0050) (ZeroConstructor, NativeAccessSpecifierPublic)
	TMap<struct FString, struct FStructSerializerBuiltinTestStruct> StrToStruct;                                               // 0x00F0(0x0050) (ZeroConstructor, NativeAccessSpecifierPublic)

};

// ScriptStruct Serialization.StructSerializerSetTestStruct
// 0x0140
struct FStructSerializerSetTestStruct
{
	unsigned char                                      StrSet[0x50];                                              // 0x0000(0x0050) UNKNOWN PROPERTY: SetProperty Serialization.StructSerializerSetTestStruct.StrSet
	unsigned char                                      IntSet[0x50];                                              // 0x0050(0x0050) UNKNOWN PROPERTY: SetProperty Serialization.StructSerializerSetTestStruct.IntSet
	unsigned char                                      NameSet[0x50];                                             // 0x00A0(0x0050) UNKNOWN PROPERTY: SetProperty Serialization.StructSerializerSetTestStruct.NameSet
	unsigned char                                      StructSet[0x50];                                           // 0x00F0(0x0050) UNKNOWN PROPERTY: SetProperty Serialization.StructSerializerSetTestStruct.StructSet

};

// ScriptStruct Serialization.StructSerializerTestStruct
// 0x0440
struct FStructSerializerTestStruct
{
	struct FStructSerializerNumericTestStruct          Numerics;                                                  // 0x0000(0x0030) (NoDestructor, NativeAccessSpecifierPublic)
	struct FStructSerializerBooleanTestStruct          Booleans;                                                  // 0x0030(0x0003) (NoDestructor, NativeAccessSpecifierPublic)
	unsigned char                                      UnknownData_9RRM[0x5];                                     // 0x0033(0x0005) MISSED OFFSET (FIX SPACE BETWEEN PREVIOUS PROPERTY)
	struct FStructSerializerObjectTestStruct           Objects;                                                   // 0x0038(0x00A0) (NativeAccessSpecifierPublic)
	unsigned char                                      UnknownData_58CQ[0x8];                                     // 0x00D8(0x0008) MISSED OFFSET (FIX SPACE BETWEEN PREVIOUS PROPERTY)
	struct FStructSerializerBuiltinTestStruct          Builtins;                                                  // 0x00E0(0x0090) (HasGetValueTypeHash, NativeAccessSpecifierPublic)
	struct FStructSerializerArrayTestStruct            Arrays;                                                    // 0x0170(0x0050) (NativeAccessSpecifierPublic)
	struct FStructSerializerMapTestStruct              Maps;                                                      // 0x01C0(0x0140) (NativeAccessSpecifierPublic)
	struct FStructSerializerSetTestStruct              Sets;                                                      // 0x0300(0x0140) (NativeAccessSpecifierPublic)

};

}

#ifdef _MSC_VER
	#pragma pack(pop)
#endif