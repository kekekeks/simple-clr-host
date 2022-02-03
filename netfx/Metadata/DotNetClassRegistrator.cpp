#include "DotNetClassRegistrator.h"


DotNetClassRegistrator::DotNetClassRegistrator(NetRuntime::Import_ObjectToStaticWrapper CreateWrapperPtr)
{
	CreateWrapperFunc = CreateWrapperPtr;
}


void DotNetClassRegistrator::ProcessRegistration(FDotNetReflectionInfo info, const TCHAR* Package)
{
	if(info.NumClasses > 0)
	{
		for(int i=0;i<info.NumClasses;i++)
		{
			auto clsInfo = new DotNetClassInfo(CreateWrapperFunc, info.Classes[i], Package);
			Classes.push_back(clsInfo);
			clsInfo->Register();
		}
	}
}
