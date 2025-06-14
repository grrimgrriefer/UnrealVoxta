/**
 * Georgy Treshchev 2024.
 *
 * Replaces C dynamic memory management functions (calloc, malloc, free, realloc, memset, memcpy) with FMemory equivalents for Unreal Engine integration.
 * This header should be included before including third-party codec headers to ensure Unreal's memory management is used.
 */

#pragma once
#undef calloc
#undef malloc
#undef free
#undef realloc
#undef memset
#undef memcpy

#define calloc(Count, Size) [&]() { void* MemPtr = FMemory::Malloc(Count * Size); if (MemPtr) { FMemory::Memset(MemPtr, 0, Count * Size); } return MemPtr; }()
#define malloc(Count) FMemory::Malloc(Count)
#define free(Original) FMemory::Free(Original)
#define realloc(Original, Count) FMemory::Realloc(Original, Count)
#define memset(Dest, Char, Count) FMemory::Memset(Dest, Char, Count)
#define memcpy(Dest, Src, Count) FMemory::Memcpy(Dest, Src, Count)

THIRD_PARTY_INCLUDES_START

#ifdef INCLUDE_WAV
#ifndef DR_WAV_INCLUDED
#define DRWAV_API static
#define DRWAV_PRIVATE static
#include "RuntimeAudioImporter/dr_wav.h"
#define DR_WAV_INCLUDED
#endif
#endif

THIRD_PARTY_INCLUDES_END

#undef calloc
#undef malloc
#undef free
#undef realloc
#undef memset
#undef memcpy