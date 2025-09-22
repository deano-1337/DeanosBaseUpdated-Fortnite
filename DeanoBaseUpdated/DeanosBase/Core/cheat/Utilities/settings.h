#pragma once
#include <string>

namespace settings
{
    struct Colors
    {
        ImColor visible = ImColor(255, 255, 255, 255); // white
        ImColor notVisible = ImColor(255, 0, 0, 255);    // red
    };

    struct ESP
    {
        bool box = true;
        bool snapline = false;
        bool distance = false;
    };
    struct Aimbot
    {
        bool enabled = true;
        int smoothsize = 95;
        int targetPriority = 0;
        int fovsize = 50;
        int Hitbox = 0;

        int aimkey = VK_RBUTTON;
        bool mouse = true;
        bool prediction = false;

        bool vischeck = false;
    };

    struct UI
    {
        bool menu = false;
        bool vsync = true;
    };

    inline Colors colors{};
    inline ESP esp{};
    inline Aimbot aimbot{};

    inline UI ui{};

    inline void reset()
    {
        colors = Colors{};
        esp = ESP{};
        ui = UI{};
    }

    inline std::string to_string()
    {
        return "[Settings]\n"
            " Box: " + std::string(esp.box ? "On" : "Off") +
            " | Snapline: " + (esp.snapline ? "On" : "Off") +
            " | Distance: " + (esp.distance ? "On" : "Off") +
            " | VSync: " + (ui.vsync ? "On" : "Off");
    }
}
