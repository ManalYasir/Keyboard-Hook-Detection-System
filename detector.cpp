#include "project.h"

// ---------------- LOGGER ----------------
void Logger::logEvent(const string &msg) {
    ofstream log("detector_log.txt", ios::app);
    log << msg << endl;
    log.close();
    cout << msg << endl;
}

// ---------------- KEYBOARD DETECTOR ----------------
void KeyboardHookDetector::detect() {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe = { sizeof(PROCESSENTRY32) };

    if (Process32First(snap, &pe)) {
        do {
            HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe.th32ProcessID);

            if (hProc) {
                HMODULE modules[1024];
                DWORD needed;

                if (EnumProcessModules(hProc, modules, sizeof(modules), &needed)) {
                    int count = needed / sizeof(HMODULE);

                    for (int i = 0; i < count; i++) {
                        char modName[260];
                        GetModuleFileNameExA(hProc, modules[i], modName, 260);

                        string name = modName;

                        if (name.find("key") != string::npos ||
                            name.find("hook") != string::npos ||
                            name.find("log") != string::npos)
                        {
                            Logger::logEvent("[!] Suspicious module: " + name);
                        }
                    }
                }
                CloseHandle(hProc);
            }
        } while (Process32Next(snap, &pe));
    }

    CloseHandle(snap);
}

// ---------------- HIDDEN WINDOWS ----------------
BOOL CALLBACK HiddenWindowDetector::EnumWindowsProc(HWND hwnd, LPARAM) {
    if (!IsWindowVisible(hwnd)) {
        char title[256];
        GetWindowTextA(hwnd, title, sizeof(title));

        if (strlen(title) > 0) {
            Logger::logEvent("[!] Hidden window: " + string(title));
        }
    }
    return TRUE;
}

void HiddenWindowDetector::detect() {
    Logger::logEvent("[+] Scanning hidden windows...");
    EnumWindows(EnumWindowsProc, 0);
}

// ---------------- PROCESS DETECTOR ----------------
void ProcessDetector::detect() {
    Logger::logEvent("[+] Scanning suspicious processes...");

    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe = { sizeof(PROCESSENTRY32) };

    if (Process32First(snap, &pe)) {
        do {
            HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe.th32ProcessID);

            if (hProc) {
                char path[MAX_PATH] = {0};

                if (GetModuleFileNameExA(hProc, NULL, path, MAX_PATH)) {
                    string filePath = path;

                    if ((string(pe.szExeFile) == "svchost.exe" ||
                         string(pe.szExeFile) == "explorer.exe") &&
                        filePath.find("System32") == string::npos)
                    {
                        Logger::logEvent("[!] Suspicious " + string(pe.szExeFile) + " at: " + filePath);
                    }
                }
                CloseHandle(hProc);
            }
        } while (Process32Next(snap, &pe));
    }

    CloseHandle(snap);
}

// ---------------- MONITOR ----------------
Monitor::Monitor(string p) : path(p) {}

void Monitor::monitorFileWrites() {
    FILETIME lastWrite = {0};

    while (true) {
        HANDLE file = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

        if (file != INVALID_HANDLE_VALUE) {
            FILETIME writeTime;
            GetFileTime(file, NULL, NULL, &writeTime);

            if (CompareFileTime(&writeTime, &lastWrite) == 1) {
                Logger::logEvent("[!] Rapid write detected in: " + path);
                lastWrite = writeTime;
            }

            CloseHandle(file);
        }

        Sleep(1000);
    }
}

void Monitor::start() {
    thread t(&Monitor::monitorFileWrites, this);
    t.detach();
}

// ---------------- SYSTEM ----------------
SystemController::SystemController() : monitor("keylog.txt") {
    modules.push_back(new KeyboardHookDetector());
    modules.push_back(new HiddenWindowDetector());
    modules.push_back(new ProcessDetector());
}

void SystemController::run() {
    Logger::logEvent("[+] Detector started\n");

    for (auto module : modules) {
        module->detect();   // 
    }

    Logger::logEvent("[+] Monitoring file writes...");
    monitor.start();

    while (true) Sleep(1000);
}

SystemController::~SystemController() {
    for (auto m : modules) {
        delete m;
    }
}