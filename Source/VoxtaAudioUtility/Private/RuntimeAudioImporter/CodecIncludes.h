/**
 * Georgy Treshchev 2024.
 * 
 * Replacing C dynamic memory management functions
 * (calloc, malloc, free, realloc, memset, memcpy) with FMemory ones
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
#define DRWAV_API static
#define DRWAV_PRIVATE static
#include "RuntimeAudioImporter/dr_wav.h"
#endif

THIRD_PARTY_INCLUDES_END

#undef calloc
#undef malloc
#undef free
#undef realloc
#undef memset
#undef memcpy