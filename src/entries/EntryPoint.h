#pragma once

#if defined(_WIN32)
#include "Windows/EntryPoint.h"
#elif __linux__ || __APPLE__
#include "Unix/EntryPoint.h"
#endif
