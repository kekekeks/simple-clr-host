#include "DotNetClassInfo.h"

#include "RuntimeManager.h"


void DotNetClassInfo::ConvertUProperty(UObject* Outer, FDotNetPropertyInfo p, int offset)
{
	FProperty* resultProperty = nullptr;
	auto flags = CPF_NativeAccessSpecifierPublic | CPF_BlueprintVisible;
	if (p.Flags == BlueprintEditable)
		flags |= CPF_Edit;

	//TODO: Marshaling arrays and obejct pointers
	
	switch(p.PropertyType.Type)
	{
		case Boolean:
			resultProperty = new FBoolProperty(Outer, FName(p.PropertyName),
				RF_Public | RF_MarkAsNative | RF_Transient, offset, flags, 0, 1, true);
			break;
		case Byte:
			resultProperty = new FInt8Property(Outer, FName(p.PropertyName),
				RF_Public | RF_MarkAsNative | RF_Transient, offset, flags);
			break;
		case UShort:
			resultProperty = new FUInt16Property(Outer, FName(p.PropertyName),
				RF_Public | RF_MarkAsNative | RF_Transient, offset, flags);
			break;
		case Short:
			resultProperty = new FInt16Property(Outer, FName(p.PropertyName),
				RF_Public | RF_MarkAsNative | RF_Transient, offset, flags);
			break;
		case Int:
			resultProperty = new FIntProperty(Outer, FName(p.PropertyName),
				RF_Public | RF_MarkAsNative | RF_Transient, offset, flags);
			break;
		case UInt:
			resultProperty = new FUInt32Property(Outer, FName(p.PropertyName),
				RF_Public | RF_MarkAsNative | RF_Transient, offset, flags);
			break;
		case Long:
			resultProperty = new FInt64Property(Outer, FName(p.PropertyName),
				RF_Public | RF_MarkAsNative | RF_Transient, offset, flags);
			break;
		case ULong:
			resultProperty = new FUInt64Property(Outer, FName(p.PropertyName),
				RF_Public | RF_MarkAsNative | RF_Transient, offset, flags);
			break;
		case Float:
			resultProperty = new FFloatProperty(Outer, FName(p.PropertyName),
				RF_Public | RF_MarkAsNative | RF_Transient, offset, flags);
			break;
		case Double:
			resultProperty = new FDoubleProperty(Outer, FName(p.PropertyName),
				RF_Public | RF_MarkAsNative | RF_Transient, offset, flags);
			break;
		case String:
			resultProperty = new FTextProperty(Outer, FName(p.PropertyName),
				RF_Public | RF_MarkAsNative | RF_Transient, offset, flags);
			break;
		default:

			break;
	}
	resultProperty->ArrayDim = 1;
}


UClass* DotNetClassInfo::ConstructClass()
{
	if(!ConstructedClass)
	{
		auto Outer = GetPrivateStaticClass();
		ConstructedClass = Outer;
		UObjectForceRegistration(Outer);

		UClass* SuperClass = Outer->GetSuperClass();
		if (SuperClass)
		{
			Outer->ClassFlags |= (SuperClass->ClassFlags & CLASS_Inherit);
		}

		Outer->ClassFlags |= CLASS_Constructed;

		if (Info.NumProperties > 0)
		{
			//Properties registration
			auto offset = URuntimeManager::Interop_GetWrapperSize(Info.NativeWrapperLinkedName);
			for (int i = 0; i < Info.NumProperties;i++)
			{
				ConvertUProperty(Outer, Info.Properties[i], offset);
				offset += Info.Properties[i].PropertyType.SizeInMemory;
			}
		}

		if(Info.NumFunctions > 0)
		{
			//Function registration required binding of generated static calls
		}

		Outer->StaticLink();
		Outer->SetSparseClassDataStruct(Outer->GetSparseClassDataArchetypeStruct());
	}
	return ConstructedClass;
}

DotNetClassInfo::DotNetClassInfo(NetRuntime::Import_ObjectToStaticWrapper CreateWrapperPtr, FDotNetClassInfo info, const TCHAR* PackageName): PackageName(PackageName), Info(info)
{
	CreateWrapper = CreateWrapperPtr;
	//Generate wrappers to avoid using static methods only
	CreateClassCallback = (RegisterCallback)CreateWrapper(this, (void*)&DotNetClassInfo::GetStaticClassFor);
	ConstructClassCallback = (RegisterCallback)CreateWrapper(this, (void*)&DotNetClassInfo::ConstructClassFor);
	RegisterNativesCallback = (::RegisterNativesCallback)CreateWrapper(this, (void*)&DotNetClassInfo::RegisterNativesFor);
	CalculatedSize = URuntimeManager::Interop_GetWrapperSize(info.NativeWrapperLinkedName) + CalculateTotalPropertiesSize();
	CreatedClass = nullptr;
	ConstructedClass = nullptr;
}

UClass* DotNetClassInfo::GetPrivateStaticClass()
{
	if(!CreatedClass)
	{
		//Get wrapper info from generated tables
		int align = URuntimeManager::Interop_GetClassAlign(Info.NativeWrapperLinkedName);
		auto refCallback = URuntimeManager::Interop_GetAddRefCollectorPtr(Info.NativeWrapperLinkedName);
		
		//TODO: Automatically resolve UClass of DotNet super class
		auto superClass = URuntimeManager::Interop_GetSuperClass(Info.NativeWrapperLinkedName);	  //Now use only direct class without inheritance

		
		const NetRuntime::Import_GenerateConstructor generator = NetRuntime::Import_GenerateConstructor(NetRuntime::Imports[3]);	//Generator
		auto wrapperConstructor = (void*)URuntimeManager::Interop_GetDefaultConstructor(Info.NativeWrapperLinkedName); //Wrapper constructor which returns void*
		auto constructorPtr = (void(*)(const FObjectInitializer&))generator(Info.Name, wrapperConstructor);
			
		//Generate class
		GetPrivateStaticClassBody(
			PackageName,
			(TCHAR*)(Info.Name) + 1,
			CreatedClass,
			RegisterNativesCallback,
			CalculatedSize,
			align,
			CLASS_Intrinsic | CLASS_MatchedSerializers | CLASS_NoExport | CLASS_Abstract,
			CASTCLASS_AActor,
			TEXT("Engine"),
			constructorPtr,
			nullptr,
			refCallback,
			superClass,
			&UObject::StaticClass, false, nullptr);
	}
	return CreatedClass;
}

int DotNetClassInfo::CalculateTotalPropertiesSize()
{
	if (Info.NumProperties == 0) return 0;
	int totalSize = 0;
	for(int i=0;i<Info.NumProperties;i++)
	{
		totalSize += Info.Properties[i].PropertyType.SizeInMemory;
	}
	return totalSize;
}

extern FString RemoveClassPrefix(const TCHAR* ClassName);

void DotNetClassInfo::Register()
{

	auto info = new FDotNetCompiledInfo(PackageName, CalculatedSize, this);
	FString Name = FString(Info.Name);
	UClassCompiledInDefer(info, (TCHAR*)Info.Name, CalculatedSize, 0xDEADBEEF);
	//Registration
	//Hack to allow registration
	//NotifyRegistrationEvent(PackageName, *Name, ENotifyRegistrationType::NRT_Class, ENotifyRegistrationPhase::NRP_Added, (UObject*(*)())ConstructClassCallback, false);
	//NotifyRegistrationEvent(PackageName, *(FString(DEFAULT_OBJECT_PREFIX) + NoPrefix), ENotifyRegistrationType::NRT_ClassCDO, ENotifyRegistrationPhase::NRP_Added, (UObject*(*)())ConstructClassCallback, false);
	//Add class construction to queue
	UObjectCompiledInDefer(ConstructClassCallback, CreateClassCallback, (TCHAR*)Info.Name, PackageName, false, nullptr, nullptr);
}

//Wrapper
UClass* DotNetClassInfo::ConstructClassFor(DotNetClassInfo* obj)
{
	return obj->ConstructClass();
}
//Wrapper
UClass* DotNetClassInfo::GetStaticClassFor(DotNetClassInfo* obj)
{
	return obj->GetPrivateStaticClass();
}

void DotNetClassInfo::RegisterNatives()
{
	
}


void DotNetClassInfo::RegisterNativesFor(DotNetClassInfo* obj)
{
	return obj->RegisterNatives();
}







