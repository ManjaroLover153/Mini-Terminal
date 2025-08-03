#include <iostream>
#include <string>
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#include <direct.h>
#include <iomanip>
#include <locale>
#include <sstream>

using namespace std;

wstring intToWstring(int val) {
    wstringstream wss;
    wss << val;
    return wss.str();
}

void printWin7Header() {
    wcout << L"Microsoft Windows [Wersja 6.2.9200]\n";
    wcout << L"(c) Microsoft Corporation. All rights reserved.\n\n";
}

void clearScreen() {
    system("cls");
}

void printPWD() {
    wchar_t buffer[FILENAME_MAX];
    if (_wgetcwd(buffer, FILENAME_MAX))
        wcout << buffer << L"\n";
    else
        wcout << L"Error getting current directory\n";
}

void unameCommand(const wstring& option) {
    if (option == L"-r") wcout << L"5.15.0-arch-custom\n";
    else wcout << L"FakeLinuxShell 1.0\n";
}

wstring getRAM() {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);
    DWORDLONG totalPhys = statex.ullTotalPhys;
    double gb = (double)totalPhys / (1024.0 * 1024.0 * 1024.0);
    return intToWstring((int)gb) + L" GB";
}

void neofetchCommand() {
    SYSTEM_INFO siSysInfo;
    GetSystemInfo(&siSysInfo);

    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osvi);

    wcout << LR"(
      ___       _       _
     | __|_ _  (_)__ __| |__ ___
     | _|| ' \ | |\ V /| / /(_-<
     |___|_||_||_| \_/ |_\_\/__/
)" << L"\n";

    wcout << L"OS: Windows " << osvi.dwMajorVersion << L"." << osvi.dwMinorVersion << L"\n";
    wcout << L"Kernel: 5.15.0-arch-custom\n";
    wcout << L"CPU: " << siSysInfo.dwNumberOfProcessors << L" cores (x64)\n";
    wcout << L"RAM: " << getRAM() << L"\n";
}

void lsblkCommand() {
    DWORD drives = GetLogicalDrives();
    if (drives == 0) {
        wcout << L"Error getting drives.\n";
        return;
    }

    wcout << left << setw(8) << L"NAME" << setw(8) << L"SIZE" << setw(8) << L"FSTYPE" << L"MOUNTPOINT" << endl;

    char letter = 'A';
    int diskIndex = 0;
    while (drives) {
        if (drives & 1) {
            wstring root = wstring(1, wchar_t(letter)) + L":\\";
            UINT type = GetDriveTypeW(root.c_str());
            if (type == DRIVE_FIXED) {
                ULARGE_INTEGER freeBytes, totalBytes, freeTotal;
                if (GetDiskFreeSpaceExW(root.c_str(), &freeBytes, &totalBytes, &freeTotal)) {
                    double sizeGB = (double)totalBytes.QuadPart / (1024.0 * 1024.0 * 1024.0);

                    wchar_t fsName[MAX_PATH];
                    GetVolumeInformationW(root.c_str(), NULL, 0, NULL, NULL, NULL, fsName, MAX_PATH);

                    wcout << left << setw(8) << (L"sd" + wstring(1, wchar_t('a' + diskIndex)))
                          << setw(8) << (intToWstring((int)sizeGB) + L"G")
                          << setw(8) << fsName
                          << root << endl;
                }
            }
            diskIndex++;
        }
        drives >>= 1;
        letter++;
    }
}

void partedCommand() {
    wcout << L"GNU Parted 3.4\n";
    wcout << L"Using fake disk table\n";
    wstring input;
    while (true) {
        wcout << L"(parted) ";
        getline(wcin, input);
        if (input == L"quit") break;
        else if (input == L"print") wcout << L"Model: ATA FakeDisk (scsi)\nDisk /dev/sda: 500GB\nPartition Table: gpt\n";
        else wcout << L"Command not recognized\n";
    }
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    #ifdef _O_U8TEXT
    _setmode(_fileno(stdout), _O_U8TEXT);
    _setmode(_fileno(stdin), _O_U8TEXT);
    #else
    _setmode(_fileno(stdout), _O_TEXT);
    _setmode(_fileno(stdin), _O_TEXT);
    #endif

    printWin7Header();

    wstring command;

    while (true) {
        wcout << L"$ ";
        getline(wcin, command);

        if (command == L"exit") break;
        else if (command == L"clear") clearScreen();
        else if (command == L"pwd") printPWD();
        else if (command.rfind(L"cd ", 0) == 0) {
            wstring path = command.substr(3);
            if (_wchdir(path.c_str()) != 0)
                wcout << L"No such directory\n";
        }
        else if (command == L"ls") system("dir");
        else if (command.rfind(L"echo ", 0) == 0) wcout << command.substr(5) << endl;
        else if (command.rfind(L"uname", 0) == 0) unameCommand(command.size() > 5 ? command.substr(6) : L"");
        else if (command == L"neofetch") neofetchCommand();
        else if (command == L"lsblk") lsblkCommand();
        else if (command == L"sudo") wcout << L"Permission granted (fake sudo)\n";
        else if (command == L"parted") partedCommand();
        else wcout << L"Command not found: " << command << endl;
    }

    return 0;
}
