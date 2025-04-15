#pragma once

#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx12.h"
#include "imgui.h"

namespace ImGuiRenderer
{
    void Initialize(void);
    void Shutdown(void);
}