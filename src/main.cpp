#include <windows.h>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

std::wstring quote(const std::wstring& value) {
    std::wstring result = L"\"";
    size_t slashes = 0;

    for (wchar_t ch : value) {
        if (ch == L'\\') {
            ++slashes;
        } else if (ch == L'"') {
            result.append(slashes * 2 + 1, L'\\');
            result += L'"';
            slashes = 0;
        } else {
            result.append(slashes, L'\\');
            slashes = 0;
            result += ch;
        }
    }

    result.append(slashes * 2, L'\\');
    result += L'"';
    return result;
}

std::wstring findExecutable(const std::wstring& name) {
    std::vector<wchar_t> buffer(32768);
    DWORD length = SearchPathW(nullptr, name.c_str(), nullptr, static_cast<DWORD>(buffer.size()), buffer.data(), nullptr);

    if (length == 0 || length >= buffer.size()) {
        return {};
    }

    return std::wstring(buffer.data(), length);
}

bool runProcess(const std::wstring& executable, const std::wstring& arguments) {
    std::wstring commandLine = quote(executable) + L" " + arguments;
    std::vector<wchar_t> mutableCommand(commandLine.begin(), commandLine.end());
    mutableCommand.push_back(L'\0');

    STARTUPINFOW startupInfo{};
    startupInfo.cb = sizeof(startupInfo);

    PROCESS_INFORMATION processInfo{};

    if (!CreateProcessW(executable.c_str(), mutableCommand.data(), nullptr, nullptr, TRUE, 0, nullptr, nullptr, &startupInfo, &processInfo)) {
        std::wcerr << L"error: failed to start " << executable << L" (Windows error " << GetLastError() << L")\n";
        return false;
    }

    WaitForSingleObject(processInfo.hProcess, INFINITE);

    DWORD exitCode = 1;
    GetExitCodeProcess(processInfo.hProcess, &exitCode);

    CloseHandle(processInfo.hThread);
    CloseHandle(processInfo.hProcess);

    return exitCode == 0;
}

void printUsage() {
    std::wcout
        << L"AT&T Style ASM Compile\n"
        << L"Usage:\n"
        << L"  atandt-compile <input.s|input.asm> [-o output.exe]\n\n"
        << L"Options:\n"
        << L"  -o <file>   Set output executable path\n"
        << L"  -h, --help  Show this help\n";
}

int wmain(int argc, wchar_t* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    fs::path input;
    fs::path output;

    for (int i = 1; i < argc; ++i) {
        std::wstring argument = argv[i];

        if (argument == L"-h" || argument == L"--help") {
            printUsage();
            return 0;
        }

        if (argument == L"-o") {
            if (i + 1 >= argc) {
                std::wcerr << L"error: -o requires an output path\n";
                return 1;
            }

            output = argv[++i];
            continue;
        }

        if (!input.empty()) {
            std::wcerr << L"error: unexpected argument: " << argument << L"\n";
            printUsage();
            return 1;
        }

        input = argument;
    }

    if (input.empty()) {
        std::wcerr << L"error: no input file specified\n";
        return 1;
    }

    if (!fs::exists(input) || !fs::is_regular_file(input)) {
        std::wcerr << L"error: input file does not exist: " << input.wstring() << L"\n";
        return 1;
    }

    if (output.empty()) {
        output = input;
        output.replace_extension(L".exe");
    }

    std::wstring llvmMc = findExecutable(L"llvm-mc.exe");
    if (llvmMc.empty()) {
        llvmMc = findExecutable(L"llvm-mc");
    }

    std::wstring clang = findExecutable(L"clang.exe");
    if (clang.empty()) {
        clang = findExecutable(L"clang");
    }

    if (llvmMc.empty()) {
        std::wcerr << L"error: llvm-mc was not found in PATH\n";
        return 1;
    }

    if (clang.empty()) {
        std::wcerr << L"error: clang was not found in PATH\n";
        return 1;
    }

    fs::path object = output;
    object.replace_extension(L".obj");

    std::error_code error;
    fs::create_directories(output.parent_path().empty() ? fs::current_path() : output.parent_path(), error);

    std::wstring assembleArguments =
        L"-triple=x86_64-pc-windows-msvc -filetype=obj " +
        quote(fs::absolute(input).wstring()) +
        L" -o " +
        quote(fs::absolute(object).wstring());

    std::wcout << L"[1/2] Assembling " << input.wstring() << L"\n";

    if (!runProcess(llvmMc, assembleArguments)) {
        std::wcerr << L"error: assembly failed\n";
        return 1;
    }

    std::wstring linkArguments =
        quote(fs::absolute(object).wstring()) +
        L" -o " +
        quote(fs::absolute(output).wstring());

    std::wcout << L"[2/2] Linking " << output.wstring() << L"\n";

    if (!runProcess(clang, linkArguments)) {
        std::wcerr << L"error: linking failed\n";
        return 1;
    }

    fs::remove(object, error);
    std::wcout << L"Successfully created " << fs::absolute(output).wstring() << L"\n";
    return 0;
}
