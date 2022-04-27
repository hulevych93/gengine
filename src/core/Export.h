#pragma once

#if defined(_WIN32)

#define API_IMPORT __declspec(dllimport)
#define API_EXPORT __declspec(dllexport)
#define API_HIDDEN

#elif __GNUC__ >= 4

#define API_IMPORT __attribute__((visibility("default")))
#define API_EXPORT __attribute__((visibility("default")))
#define API_HIDDEN __attribute__((visibility("hidden")))

#else

#define API_IMPORT
#define API_EXPORT
#define API_HIDDEN

#endif

#if !defined(EXPORT_SYMBOLS)
#define EXPORT_SYMBOLS 0
#endif

#if EXPORT_SYMBOLS
#define GENGINE_API API_EXPORT
#else
#define GENGINE_API API_IMPORT
#endif

#define PRIVATE_SYMBOL API_HIDDEN
