#include <io.h>
#include <windows.h>
#include <process.h>
#include <direct.h>

#define getpid() GetCurrentProcessId()
#define chmod(f,m)
#define rmdir _rmdir

#ifndef S_ISDIR
#define S_ISDIR(x) (((x) & S_IFMT) == S_IFDIR)
#endif
