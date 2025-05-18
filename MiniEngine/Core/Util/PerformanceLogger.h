#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <stdexcept>
#include <mutex>

namespace Util
{
    class PerformanceLogger final
    {
        struct MeasureEntry {
            float cpuTime;
            float gpuTime;
            float frameTime;

            MeasureEntry(float ct, float gt, float ft)
                : cpuTime(ct), gpuTime(gt), frameTime(ft) {}
        };

    private:
        static std::mutex s_fileMutex;
        static std::string s_filename;
        static bool s_initialized;

    public:
        static void Initialize(const std::string& filename) {
            std::lock_guard<std::mutex> lock(s_fileMutex);
            if (s_initialized == true) {
                throw std::runtime_error("PerformanceLogger already initialize!");
            }

            PerformanceLogger::s_filename = filename;

            std::ofstream file(s_filename, std::ios::out);
            if (!file.is_open()) {
                throw std::runtime_error("Failed to open file: " + s_filename);
            }

            file << "CPUTime(ms),GPUTime(ms),FrameTime(ms)\n";
            s_initialized = true;
        }

        static void WriteImmediate(float cpuTime, float gpuTime, float frameTime) {
            if (s_initialized == false) {
                throw std::runtime_error("PerformanceLogger already initialize!");
            }

            std::lock_guard<std::mutex> lock(s_fileMutex);
            std::ofstream file(s_filename, std::ios::app); // Append mode
            if (!file.is_open()) {
                throw std::runtime_error("Failed to open file: " + s_filename);
            }

            file << cpuTime << ","
                << gpuTime << ","
                << frameTime << "\n";
        }
    };
}