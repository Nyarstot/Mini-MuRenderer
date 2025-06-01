#pragma once

#include "GameCore.h"


class CGpuTimeManager
{
private:
    ID3D12QueryHeap* sm_QueryHeap = nullptr;
    ID3D12Resource* sm_ReadBackBuffer = nullptr;
    uint64_t* sm_TimeStampBuffer = nullptr;
    uint64_t sm_Fence = 0;
    uint32_t sm_MaxNumTimers = 0;
    uint32_t sm_NumTimers = 1;
    uint64_t sm_ValidTimeStart = 0;
    uint64_t sm_ValidTimeEnd = 0;
    double sm_GpuTickDelta = 0.0;

    ID3D12Device* m_parentDevice;
    CommandListManager* m_parentCommandListManager;

public:
    CGpuTimeManager() = default;
    ~CGpuTimeManager() = default;

    void Initialize(CommandListManager* cmdListManager, ID3D12Device* Device, uint32_t MaxNumTimers = 4096);
    void Shutdown();

    // Reserve a unique timer index
    uint32_t NewTimer(void);

    // Write start and stop time stamps on the GPU timeline
    void StartTimer(CommandContext& Context, uint32_t TimerIdx);
    void StopTimer(CommandContext& Context, uint32_t TimerIdx);

    // Bookend all calls to GetTime() with Begin/End which correspond to Map/Unmap.  This
    // needs to happen either at the very start or very end of a frame.
    void BeginReadBack(void);
    void EndReadBack(void);

    // Returns the time in milliseconds between start and stop queries
    float GetTime(uint32_t TimerIdx);

};