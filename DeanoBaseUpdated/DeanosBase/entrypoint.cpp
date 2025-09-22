#include <Windows.h>
#include <iostream>
#include <chrono>
#include <thread>
#include "Core/cheat/Renderer/render.h"
#include "Core/cheat/Visuals/esp.h"

namespace cheat
{
    constexpr auto targetProcess = L"FortniteClient-Win64-Shipping.exe";

    void setConsoleTitle(const char* title)
    {
        ::SetConsoleTitleA(title);
    }

    void clearConsole()
    {
        HANDLE hConsole = ::GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        DWORD count, cellCount;
        COORD homeCoords = { 0, 0 };

        if (hConsole == INVALID_HANDLE_VALUE) return;
        if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) return;

        cellCount = csbi.dwSize.X * csbi.dwSize.Y;
        FillConsoleOutputCharacter(hConsole, ' ', cellCount, homeCoords, &count);
        FillConsoleOutputAttribute(hConsole, csbi.wAttributes, cellCount, homeCoords, &count);

        SetConsoleCursorPosition(hConsole, homeCoords);
    }

    bool waitForProcess()
    {
        std::wstring wsTarget(targetProcess);
        std::string sTarget(wsTarget.begin(), wsTarget.end());

        std::cout << "\n [*] Waiting for " << sTarget << "...\n";

        while (true)
        {
            Driver::ProcessID = Driver::FindProcess(targetProcess);

            if (Driver::ProcessID != 0)
            {
                clearConsole();
                //std::cout << " [+] Found target process (PID: " << Driver::ProcessID << ")\n";
                Driver::bWindowHandle = Driver::GetProcessWND(Driver::ProcessID);
                return true;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    bool initDriver()
    {
        if (!Driver::Init())
        {
            std::cerr << " [!] Driver not loaded. Exiting...\n";
            std::this_thread::sleep_for(std::chrono::seconds(5));
            return false;
        }
        return true;
    }

    bool initBase()
    {
        Base = Driver::GetBase();
        if (!Base)
        {
            std::cerr << " [!] Failed to fetch base image address.\n";
            return false;
        }

        if (!Driver::CR3())
        {
            std::cerr << " [!] Failed to fetch CR3.\n";
            return false;
        }

        //std::cout << " [+] Base and CR3 fetched successfully.\n";
        return true;
    }

    int run()
    {
        setConsoleTitle("Deano's Base");

        if (!initDriver())
            return EXIT_FAILURE;

        if (!waitForProcess())
            return EXIT_FAILURE;

        if (!initBase())
            return EXIT_FAILURE;

        setup(); // Your ESP/render setup
        return EXIT_SUCCESS;
    }
}

int main()
{
    return cheat::run();
}
