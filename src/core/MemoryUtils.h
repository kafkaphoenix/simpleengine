#include <cstddef>
#if defined(_WIN32)
#  include <windows.h>
#  include <psapi.h>
#else
#  include <fstream>
#  include <string>
#  include <unistd.h>
#endif

inline size_t getProcessMemoryUsageKB() {
#if defined(_WIN32)
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize / 1024;
    }
    return 0;
#else
    std::ifstream statm("/proc/self/statm");
    size_t size = 0, resident = 0;
    statm >> size >> resident;
    long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024;
    return resident * page_size_kb;
#endif
}
