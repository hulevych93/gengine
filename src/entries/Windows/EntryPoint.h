#pragma once

#define IMPLEMENT_ENTRY                                                       \
  void* g_module_instance = nullptr;                                          \
  extern "C" int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, \
                                LPSTR lpCmdLine, int nShowCmd) {              \
    g_module_instance = hInstance;                                            \
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);                            \
    Main mainObject;                                                          \
    auto args = WinArgs(hInstance, hPrevInstance, lpCmdLine, nShowCmd);       \
    auto result = mainObject.Run(args);                                       \
    CoUninitialize();                                                         \
    return result;                                                            \
  }

#define IMPLEMENT_ENTRY_NO_COM                                                \
  void* g_module_instance = nullptr;                                          \
  extern "C" int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, \
                                LPSTR lpCmdLine, int nShowCmd) {              \
    g_module_instance = hInstance;                                            \
    Main mainObject;                                                          \
    auto args = WinArgs(hInstance, hPrevInstance, lpCmdLine, nShowCmd);       \
    auto result = mainObject.Run(args);                                       \
    return result;                                                            \
  }

#if defined(UNICODE)
#define IMPLEMENT_CONSOLE_ENTRY                \
  void* g_module_instance = nullptr;           \
  int wmain(int argc, wchar_t** argv) {        \
    Main mainObject;                           \
    auto args = BaseArgs<wchar_t>(argc, argv); \
    return mainObject.Run(args);               \
  }

#else
#define IMPLEMENT_CONSOLE_ENTRY             \
  void* g_module_instance = nullptr;        \
  int main(int argc, char** argv) {         \
    Main mainObject;                        \
    auto args = BaseArgs<char>(argc, argv); \
    return mainObject.Run(args);            \
  }
#endif
