#ifndef PROJECT_H
#define PROJECT_H

#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

#pragma comment(lib, "psapi.lib")

using namespace std;

// ---------------- LOGGER ----------------
class Logger {
public:
    static void logEvent(const string &msg);
};

// ---------------- BASE CLASS ----------------
class DetectionModule {
public:
    virtual void detect() = 0;   
    virtual ~DetectionModule() {}
};

// ---------------- DERIVED CLASSES ----------------
class KeyboardHookDetector : public DetectionModule {
public:
    void detect() override;
};

class HiddenWindowDetector : public DetectionModule {
public:
    void detect() override;
    static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM);
};

class ProcessDetector : public DetectionModule {
public:
    void detect() override;
};

// ---------------- MONITOR ----------------
class Monitor {
private:
    string path;

public:
    Monitor(string p);
    void monitorFileWrites();
    void start();
};

// ---------------- SYSTEM ----------------
class SystemController {
private:
    vector<DetectionModule*> modules;
    Monitor monitor;

public:
    SystemController();
    void run();
    ~SystemController();
};

#endif