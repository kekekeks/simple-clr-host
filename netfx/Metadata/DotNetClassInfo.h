#pragma once

#include "MetadataStructs.h"
#include "CoreMinimal.h"
#include "RuntimeManager.h"

typedef UClass*(*RegisterCallback)();
typedef void(*RegisterNativesCallback)();

class DotNetClassInfo;





class DotNetClassInfo
{
public:
	DotNetClassInfo(NetRuntime::Import_ObjectToStaticWrapper CreateWrapperPtr, FDotNetClassInfo info, const TCHAR* PackageName);

	UClass* GetPrivateStaticClass();
	UClass* ConstructClass();
	void RegisterNatives();

	void Register();

	int CalculateTotalPropertiesSize();

	void ConvertUProperty(UObject* Outer, FDotNetPropertyInfo p, int offset);

	static UClass* GetStaticClassFor(DotNetClassInfo* obj);
	static UClass* ConstructClassFor(DotNetClassInfo* obj);
	static void RegisterNativesFor(DotNetClassInfo* obj);

private:
	NetRuntime::Import_ObjectToStaticWrapper CreateWrapper;
	const TCHAR* PackageName;
	FDotNetClassInfo Info;
	int CalculatedSize;

	UClass* CreatedClass;
	UClass* ConstructedClass;

	RegisterCallback CreateClassCallback;
	RegisterCallback ConstructClassCallback;
	RegisterNativesCallback RegisterNativesCallback;
};

struct FDotNetCompiledInfo : public FFieldCompiledInInfo
{
	FDotNetCompiledInfo(const TCHAR* package, int size, DotNetClassInfo* info) : FFieldCompiledInInfo(size, 0xDEADBEEF), Package(package), Info(info)
	{

	}

	UClass* Register() const override
	{
		LLM_SCOPE(ELLMTag::UObject);
		return Info->GetPrivateStaticClass();
	}

	const TCHAR* ClassPackage() const override
	{
		return Package;
	}

	const TCHAR* Package;
	DotNetClassInfo* Info;
};