// 一个非常方便的debug 函数
#if 1
#define fshow(fmt, args...)    \
        printf("==> UDEBUG [%s +%d][%s] " fmt CLOSE, __FILE__, __LINE__, __FUNCTION__, ##args)
#else
#define fshow(fmt, args...)
#endif
