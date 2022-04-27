#pragma once
#if defined(BUILD_WINDOWS)
#include "Windows/Event.h"
#elif __linux__ || __APPLE__
#include "Unix/Event.h"
#else
#error "no build configuration defined"
#endif
