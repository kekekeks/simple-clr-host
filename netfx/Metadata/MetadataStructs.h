#pragma once

enum EDotNetType
{
	Byte = 0,
	UShort,
	Short,
	Int,
	UInt,
	Long,
	ULong,
	Float,
	Double,
	Boolean,
	String,
	ValueStruct,
	ObjectRef
};

enum EDotNetPropertyFlags
{
	BlueprintReadOnly = 0,
	BlueprintEditable = 1,
	BlueprintAssignable = 2,
};

enum EDotNetFunctionType
{
	BlueprintCallable = 0,
	BlueprintEvent = 1,
};

struct FDotNetTypeInfo
{
	EDotNetType Type;	//Type for marshaling
	int SizeInMemory;	//Size in bytes
	wchar_t* TypeName; 	//Type specifier for Class/Structs
};

struct FDotNetFunctionParameterInfo
{
	FDotNetTypeInfo ParameterType;
	bool IsReference;
	wchar_t* ParameterName;
};

struct FDotNetPropertyInfo
{
	FDotNetTypeInfo PropertyType;
	wchar_t* PropertyName;
	EDotNetPropertyFlags Flags;
};

struct FDotNetFunctionInfo
{
	wchar_t* Name;
	EDotNetFunctionType Type;
	bool IsStatic;
	bool HasReturnValue;
	FDotNetTypeInfo ReturnType;
	FDotNetFunctionParameterInfo* Parameters;
	int NumParameters;
};

struct FDotNetStructInfo
{
	wchar_t* Name;
	FDotNetPropertyInfo* Properties;
	int NumProperties;
};

struct FDotNetClassInfo
{
	wchar_t* Name;
	wchar_t* NativeWrapperLinkedName;
	wchar_t* SuperClass;
	FDotNetPropertyInfo* Properties;
	int NumProperties;
	FDotNetFunctionInfo* Functions;
	int NumFunctions;
};

struct FDotNetReflectionInfo
{
	FDotNetStructInfo* Structs;
	int NumStructs;
	FDotNetClassInfo* Classes;
	int NumClasses;
};