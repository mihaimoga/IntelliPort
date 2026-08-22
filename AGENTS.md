# Agent Guidelines for IntelliPort Solution

This document provides instructions, architectural context, conventions, and workflows for AI agents working within this repository.

---

## 1. Project Overview & Architecture

**IntelliPort** is a Windows C++/MFC desktop application serving as a high-performance serial port (COM) and TCP/UDP socket logger/terminal (an alternative to HyperTerminal). It is designed with pure Win32 API, MFC, and modern C++ for low CPU utilization, high execution speed, and small binary footprint.

### Solution Layout (`IntelliPort.sln`)

- **`IntelliPort.vcxproj`** (Main Application):
  - Target: Windows Executable (`.exe`), statically linked MFC (`UseOfMfc = Static`), Unicode character set.
  - Architecture: MFC Document/View architecture with Ribbon interface (`CMainFrame`, `CIntelliPortDoc`, `CIntelliPortView`).
  - Core Modules:
	- `SerialPort.h` / `SerialPort.cpp`: Serial port communication handling (baud rates, parity, data/stop bits, flow control).
	- `enumser.h` / `enumser.cpp`: Serial port enumeration across Windows systems.
	- `SocMFC.h` / `SocMFC.cpp`: Winsock-based TCP/UDP socket communication (client & server modes).
	- `RingBuffer.h`: Lock-free / efficient memory ring buffer for incoming/outgoing stream data.
	- `EdgeWebBrowser.h` / `EdgeWebBrowser.cpp`: Embedded Microsoft Edge WebView2 control integration.
	- `ConfigureDlg`, `IncomingDlg`, `InputDlg`, `CheckForUpdatesDlg`, `WebBrowserDlg`: UI dialogs for settings, connection alerts, input prompts, and update workflows.
- **`genUp4win/genUp4win.vcxproj`** (Updater Module):
  - Generic Windows software updater DLL / library.
  - Handles HTTPS downloading (`URLDownloadToFile`), XML configuration parsing (`AppSettings`), and SHA-256 checksum verification (`SHA256.h` / `SHA256.cpp`).
- **`Setup/Setup.vdproj`**:
  - Visual Studio Installer project to create MSI deployment packages (`IntelliPortSetup.msi`).

---

## 2. Technology Stack & Dependencies

- **Language**: C++ (C++17 standard)
- **Frameworks**: Microsoft Foundation Classes (MFC), Win32 API, Winsock2
- **External & NuGet Dependencies** (`packages.config`):
  - `Microsoft.Web.WebView2` (Native WebView2 control for embedded browsing)
  - `Microsoft.Windows.ImplementationLibrary` (WIL - RAII resource management & error handling)
- **Tooling & Platform**:
  - Visual Studio 2022+ (`v143`/`v145` Platform Toolsets)
  - Windows SDK 10.0+
  - Target Architectures: `Win32` (x86) and `x64`
  - Build Configurations: `Debug` and `Release`

---

## 3. Build & Verification Commands

### Building via MSBuild

Restore NuGet packages before building:
```cmd
nuget restore IntelliPort.sln
```

Build the entire solution or specific project via MSBuild:
```cmd
msbuild IntelliPort.sln /p:Configuration=Release /p:Platform=x64
msbuild IntelliPort.sln /p:Configuration=Debug /p:Platform=x64
msbuild IntelliPort.sln /p:Configuration=Release /p:Platform=Win32
msbuild IntelliPort.sln /p:Configuration=Debug /p:Platform=Win32
```

### Resource Compilation
When modifying dialogs, icons, strings, or menus:
- Resources are located in `IntelliPort.rc`, `res/IntelliPort.rc2`, and `resource.h`.
- Ribbon UI definitions reside in `res/*.mfcribbon-ms`.
- Ensure new resource IDs in `resource.h` match definitions in `.rc` and do not collide with MFC standard command IDs.

---

## 4. Coding Standards & Conventions

Agents must adhere to the project's established conventions (see also `CONTRIBUTING.md`):

### Formatting & Bracing
- **Allman Bracing Style**: Opening and closing curly braces on their own lines.
  ```cpp
  void CMainFrame::OnConfigure()
  {
	  if (m_bConnected)
	  {
		  // Action
	  }
  }
  ```
  *(Exception: single-line inline method implementations inside header files).*
- **Indentation**: Use tabs (or 4-space equivalent). Do not mix arbitrary indentation styles.
- **Spacing**:
  - Exactly one space around binary and ternary operators (`a == 10 && b == 42`).
  - Exactly one space after semicolons in `for` loops (`for (int i = 0; i < count; ++i)`).
  - No space between function name and opening parenthesis (`foo(arg)`).
  - One space between control keywords and opening parenthesis (`if (condition)`, `while (condition)`).
- **Comments**: Prefer C++ style `//` comments over C-style `/* */` block comments in implementation files.

### Naming Conventions
- **Classes / Structs**: `PascalCase`. MFC-derived classes prefixed with `C` (e.g. `CIntelliPortApp`, `CMainFrame`, `CConfigureDlg`).
- **Methods**: `camelCase` for internal methods; `PascalCase` / `On<Event>` for MFC message handlers and virtual overrides.
- **Member Variables**: MFC Hungarian notation (`m_strText`, `m_nBaudRate`, `m_bConnected`, `m_pSocket`) or leading underscore (`_variable`).
- **Constants / Enums**: `UPPER_SNAKE_CASE` or scoped enums (`enum class`).

### Modern C++ Best Practices
- **Memory & Resource Management**:
  - Avoid raw `new` and `delete`. Use automatic stack allocation or RAII wrappers.
  - Prefer `std::unique_ptr` over raw pointers. Avoid `std::shared_ptr` unless multi-owner semantics are explicitly needed.
  - Utilize RAII helpers (`AutoHandle.h`, `AutoHeapAlloc.h`, `AutoHModule.h`, and WIL wrappers) for Windows handles.
- **Casts**: Always use C++ style casts (`static_cast`, `reinterpret_cast`, `const_cast`) instead of C-style casts `(type)val`.
- **String Handling**:
  - Prefer `.empty()` / `.IsEmpty()` to test for empty strings (`!str.empty()`, `!m_strName.IsEmpty()`).
  - Use `utf8_to_wstring` and `wstring_to_utf8` for cross-encoding conversions.
- **Headers**: Never place `using namespace` directives in header (`.h`) files.
- **Increments**: Prefer prefix increment `++i` over postfix increment `i++`.

---

## 5. Agent Instructions for Code Modifications

1. **Investigate Context First**:
   - Read the relevant header (`.h`) and implementation (`.cpp`) files before editing.
   - For UI changes, inspect `resource.h`, `IntelliPort.rc`, and ribbon files (`res/*.mfcribbon-ms`).
2. **Preserve Compatibility**:
   - Maintain static MFC compatibility and Unicode character set support.
   - Respect Windows 10/11 APIs while avoiding hard dependencies that break portability without feature detection.
3. **Thread Safety & Sockets/Serial**:
   - Serial communication (`CSerialPort`) and socket operations (`SocMFC`) involve asynchronous / threaded I/O.
   - Synchronize shared buffers (`RingBuffer`) carefully to prevent deadlocks or race conditions.
4. **Validation**:
   - Run compilation via build tools after making modifications.
   - Verify that all project configurations (Debug/Release, x86/x64) compile cleanly without introducing compiler warnings.
