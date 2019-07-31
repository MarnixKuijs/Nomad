#if defined(__clang__)
#define POP_COUNT(x)  __builtin_popcount(x)
#elif defined(__GNUC__) || defined(__GNUG__)
#define POP_COUNT(x)  __builtin_popcount(x)
#elif defined(_MSC_VER)
#include <intrin.h>
#define POP_COUNT(x) __popcnt(x)
#endif // 
