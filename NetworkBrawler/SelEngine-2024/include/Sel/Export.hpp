#pragma once

#ifdef _WIN32

#ifdef SEL_ENGINE_BUILD
	#define SEL_ENGINE_API __declspec(dllexport)
#else
	#define SEL_ENGINE_API __declspec(dllimport)
#endif

#else
	#define SEL_ENGINE_API __attribute__((visibility("default")))
#endif
