#pragma once

#include <boost/config.hpp> 
#include <boost/dll/alias.hpp> 

#define EXPORT_GLOBAL_SHARED_SERVICE(impl) \
extern "C" BOOST_SYMBOL_EXPORT impl impl##_service; \
impl impl##_service;

#define EXPORT_CREATOR_SHARED_SERVICE(implementation) \
static I##implementation* Create##implementation##Service() \
{ \
    return new implementation(); \
} \
BOOST_DLL_ALIAS( \
    Create##implementation##Service, \
    create_##implementation##_service \
)

//todo

#define EXPORT_SHARED_ENTRY_POINT(implementation) \
static boost::shared_ptr<IEntry> Create##implementation##Service(std::unique_ptr<IEntryToolsFactory>&& factory) \
{ \
    return boost::shared_ptr<implementation>(new implementation(std::move(factory))); \
} \
BOOST_DLL_ALIAS( \
    Create##implementation##Service, \
    create_shared_entry \
)

#if __WIN32__
#define IMPLEMENT_SHARED_ENTRY \
void* g_module_instance = nullptr;\
BOOL APIENTRY DllMain(HMODULE ModuleInstance, DWORD ReasonForCall, LPVOID Reserved)\
{\
switch (ReasonForCall)\
{\
case DLL_PROCESS_ATTACH:\
    g_module_instance = (HINSTANCE)ModuleInstance;\
    break;\
case DLL_PROCESS_DETACH:\
    g_module_instance = nullptr;\
    break;\
}\
return TRUE;\
}
#else
#define IMPLEMENT_SHARED_ENTRY \
static const int SomeAddress = 0u; \
void* g_module_instance = (void*)&SomeAddress;
#endif
