#include "pch.h"
#include "Util/PerformanceLogger.h"
#include "Util/StringUtils.h"
#include <fstream>

namespace Util
{
    std::string PerformanceLogger::s_filename;
    bool PerformanceLogger::s_initialized = false;
    std::mutex PerformanceLogger::s_fileMutex;
}