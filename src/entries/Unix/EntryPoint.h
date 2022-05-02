#pragma once

#define IMPLEMENT_ENTRY \
    void* g_module_instance = nullptr;\
    int main(int argc, char** argv)\
    {\
        static const int SomeData = 10u;\
        g_module_instance = (void*)&SomeData; \
        Main mainObject;\
        auto args = BaseArgs<char>(argc, argv);\
        return mainObject.Run(args);\
    }

#define IMPLEMENT_CONSOLE_ENTRY IMPLEMENT_ENTRY
