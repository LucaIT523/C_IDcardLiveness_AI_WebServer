/* Copyright 2021 ID R&D Inc. All Rights Reserved. */

/**
 * @file config.h
 * Definitions for shared library's exports
 */

#pragma once

#if defined _WIN32 || defined __CYGWIN__
#define DOCSDK_API_IMPORT __declspec(dllimport)
#define DOCSDK_API_EXPORT __declspec(dllexport)
#elif __GNUC__ >= 4 || defined __clang__ && !defined(__CUDACC__)
#define DOCSDK_API_IMPORT __attribute__((visibility("default")))
#define DOCSDK_API_EXPORT __attribute__((visibility("default")))
#else
#define DOCSDK_API_IMPORT
#define DOCSDK_API_EXPORT
#endif

#ifdef DOCSDK_EXPORT
#define DOCSDK_API DOCSDK_API_EXPORT
#else
#define DOCSDK_API DOCSDK_API_IMPORT
#endif
