#pragma once
#include "DotNetClassInfo.h"
#include <list>

struct FDotNetReflectionInfo;

class DotNetClassRegistrator
{
public:

	DotNetClassRegistrator(NetRuntime::Import_ObjectToStaticWrapper CreateWrapperPtr);
	
	void ProcessRegistration(FDotNetReflectionInfo info, const TCHAR* Package);

private:
	std::list<DotNetClassInfo*> Classes;
	NetRuntime::Import_ObjectToStaticWrapper CreateWrapperFunc;
};
