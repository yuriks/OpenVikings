// This file should be included BEFORE the semicolon terminating the definition.

#if defined(_MSC_VER)
#pragma pack(pop)
#elif defined(__GNUC__)
__attribute__((__packed__))
#else
#error Unsupported compiler.
#endif
