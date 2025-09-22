#pragma once
#include "render.h"
#include <Windows.h>
#include <d3d11.h>
#include <iostream>
#include "../Utilities/util.h"
#include "../Visuals/esp.h"

namespace resources
{
    struct D3DResources
    {
        HWND hwnd{ nullptr };
        ID3D11Device* device{ nullptr };
        ID3D11DeviceContext* device_context{ nullptr };
        ID3D11RenderTargetView* render_target_view{ nullptr };
        IDXGISwapChain* swap_chain{ nullptr };
    };

    inline D3DResources gfx{};
}

namespace overlay
{
    inline void CreateWindowOverlay()
    {
        WNDCLASSEXA wc{};
        wc.cbSize = sizeof(WNDCLASSEXA);
        wc.lpfnWndProc = DefWindowProcA;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = "DeanoOverlay";
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);

        RegisterClassExA(&wc);

        RECT screen_rect{};
        GetWindowRect(GetDesktopWindow(), &screen_rect);

        resources::gfx.hwnd = CreateWindowExA(
            WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW,
            wc.lpszClassName,
            "DeanoBase",
            WS_POPUP,
            screen_rect.left, screen_rect.top,
            screen_rect.right, screen_rect.bottom,
            nullptr, nullptr,
            wc.hInstance,
            nullptr
        );

        MARGINS margins = { -1 };
        DwmExtendFrameIntoClientArea(resources::gfx.hwnd, &margins);
        SetLayeredWindowAttributes(resources::gfx.hwnd, RGB(0, 0, 0), 255, LWA_ALPHA);
        SetWindowPos(resources::gfx.hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        ShowWindow(resources::gfx.hwnd, SW_SHOW);
        UpdateWindow(resources::gfx.hwnd);
    }
}

namespace d3d
{
    inline void ApplyCustomStyle()
    {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.11f, 0.15f, 1.00f);
        colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.25f, 0.60f);
        colors[ImGuiCol_Text] = ImVec4(0.90f, 0.92f, 0.95f, 1.00f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.09f, 0.12f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.18f, 0.25f, 1.00f);

        colors[ImGuiCol_Button] = ImVec4(0.20f, 0.35f, 0.60f, 0.70f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.45f, 0.80f, 0.90f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.30f, 0.50f, 1.00f);

        colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.18f, 0.22f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.35f, 0.60f, 0.70f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.45f, 0.80f, 0.80f);

        colors[ImGuiCol_SliderGrab] = ImVec4(0.30f, 0.55f, 0.90f, 0.80f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.40f, 0.65f, 1.00f, 0.90f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.35f, 0.60f, 0.95f, 1.00f);

        colors[ImGuiCol_Tab] = ImVec4(0.14f, 0.18f, 0.25f, 1.00f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.30f, 0.55f, 0.90f, 0.80f);
        colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.35f, 0.60f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.18f, 0.28f, 0.45f, 1.00f);

        style.WindowPadding = ImVec2(15, 15);
        style.FramePadding = ImVec2(6, 4);
        style.ItemSpacing = ImVec2(10, 6);
        style.WindowRounding = 8.0f;
        style.FrameRounding = 6.0f;
        style.GrabRounding = 6.0f;
        style.ScrollbarRounding = 9.0f;
        style.ChildRounding = 8.0f;
        style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    }

    inline bool Setup()
    {
        DXGI_SWAP_CHAIN_DESC scd{};
        scd.BufferCount = 2;
        scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        scd.BufferDesc.RefreshRate.Numerator = 60;
        scd.BufferDesc.RefreshRate.Denominator = 1;
        scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scd.OutputWindow = resources::gfx.hwnd;
        scd.SampleDesc.Count = 1;
        scd.SampleDesc.Quality = 0;
        scd.Windowed = TRUE;
        scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
        D3D_FEATURE_LEVEL selected_level;

        if (FAILED(D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            0,
            feature_levels,
            _countof(feature_levels),
            D3D11_SDK_VERSION,
            &scd,
            &resources::gfx.swap_chain,
            &resources::gfx.device,
            &selected_level,
            &resources::gfx.device_context)))
        {
            return false;
        }

        ID3D11Texture2D* back_buffer = nullptr;
        if (FAILED(resources::gfx.swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer))) || !back_buffer)
            return false;

        if (FAILED(resources::gfx.device->CreateRenderTargetView(back_buffer, nullptr, &resources::gfx.render_target_view)))
            return false;

        back_buffer->Release();

        ImGui::CreateContext();
        ApplyCustomStyle();

        ImGui_ImplWin32_Init(resources::gfx.hwnd);
        ImGui_ImplDX11_Init(resources::gfx.device, resources::gfx.device_context);

        resources::gfx.device->Release();
        return true;
    }
}

namespace menu
{
    inline void Render()
    {
        const DWORD color_picker_flags = ImGuiColorEditFlags_NoSidePreview
            | ImGuiColorEditFlags_AlphaBar
            | ImGuiColorEditFlags_NoInputs
            | ImGuiColorEditFlags_AlphaPreview;

        ImGui::SetNextWindowSize({ 700.f, 500.f }, ImGuiCond_FirstUseEver);
        ImGui::Begin("Deano Overlay Menu", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

        if (ImGui::BeginTabBar("##tabs"))
        {
            // ----- Aimbot Tab -----
            if (ImGui::BeginTabItem("Aimbot"))
            {
                ImGui::Checkbox("Enable Aimbot", &settings::aimbot.enabled);
                ImGui::SliderInt("Smoothness", &settings::aimbot.smoothsize, 1, 100);
                ImGui::SliderInt("FOV Size", &settings::aimbot.fovsize, 1, 500);
                ImGui::Combo("Target Priority", &settings::aimbot.targetPriority, "Closest Distance\0Closest Crosshair\0\0");
                ImGui::Combo("Hitbox", &settings::aimbot.Hitbox, "Head\0Chest\0Body\0Legs\0Random\0\0");
                ImGui::Checkbox("Prediction", &settings::aimbot.prediction);
                ImGui::Checkbox("Visibility Check", &settings::aimbot.vischeck);

                // --- HOTKEY ---



                ImGui::EndTabItem(); // <-- Must be called!
            }

            // ----- ESP Tab -----
            if (ImGui::BeginTabItem("ESP"))
            {
                ImGui::Checkbox("Box", &settings::esp.box);
                ImGui::Checkbox("Snapline", &settings::esp.snapline);
                ImGui::Checkbox("Distance", &settings::esp.distance);

                ImGui::Separator();
                ImGui::Text("ESP Colors");
                ImGui::ColorEdit4("Visible", reinterpret_cast<float*>(&settings::colors.visible), color_picker_flags);
                ImGui::ColorEdit4("Not Visible", reinterpret_cast<float*>(&settings::colors.notVisible), color_picker_flags);

                ImGui::EndTabItem(); // <-- Must be called!
            }

            // ----- UI Tab -----
            if (ImGui::BeginTabItem("UI"))
            {
                ImGui::Checkbox("VSync", &settings::ui.vsync);
                ImGui::EndTabItem(); // <-- Must be called!
            }

            ImGui::EndTabBar(); // <-- Must match BeginTabBar
        }

        ImGui::End(); // End window
    }
}

namespace core
{
    inline void Run()
    {
        constexpr float clear_color[4] = { 0.f, 0.f, 0.f, 0.f };
        MSG msg{};

        while (msg.message != WM_QUIT)
        {
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            auto& io = ImGui::GetIO();
            io.DeltaTime = 1.0f / 60.0f;

            if (GetAsyncKeyState(VK_INSERT) & 1)
                settings::ui.menu = !settings::ui.menu;

            if (settings::ui.menu)
            {
                POINT cursor{};
                GetCursorPos(&cursor);
                io.MousePos = ImVec2(static_cast<float>(cursor.x), static_cast<float>(cursor.y));
                io.MouseDown[0] = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
            }

            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            ActorLoop();

            if (settings::ui.menu)
                menu::Render();

            ImGui::Render();

            resources::gfx.device_context->OMSetRenderTargets(1, &resources::gfx.render_target_view, nullptr);
            resources::gfx.device_context->ClearRenderTargetView(resources::gfx.render_target_view, clear_color);

            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
            resources::gfx.swap_chain->Present(settings::ui.vsync, 0);
        }

        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }
}

inline void setup()
{
    overlay::CreateWindowOverlay();
    d3d::Setup();
    core::Run();
}
