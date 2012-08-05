#if defined(_MSC_VER)
// Warning C4103: alignment changed after including header, may be due to missing #pragma pack(pop)
#pragma warning(disable : 4103)
#pragma pack(push, 1)
#elif defined(__GNUC__)
#else
#error Unsupported compiler.
#endif
