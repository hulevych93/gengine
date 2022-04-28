#pragma once
#if defined(_WIN32)
#include "Windows/Event.h"
#elif __linux__ || __APPLE__
#include "Unix/Event.h"
#else
#error "no build configuration defined"
#endif
