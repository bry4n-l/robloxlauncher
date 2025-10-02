# Roblox and Chrome Setup Utilities

## Overview
These C programs (`roblox.c` and `chrome.c`) are Windows GUI utilities that display a top-most progress bar window during setup tasks for Roblox and Chrome. Both create a simple "Please wait..." dialog with a smooth progress bar that updates in stages (33%, 66%, 100%). They are designed to run silently in the background while performing administrative tasks.

Key shared features:
- Automatically relaunches as administrator if not already elevated.
- Registers the "roblox-player" URL protocol handler to point to the latest `RobloxPlayerBeta.exe` (for deep linking into Roblox).
- Overwrites Roblox storage files (`appStorage.json`, `memProfStorage*.json`, `*.dat`) for **all users** to force autologout of accounts by clearing persistent login/cookie data.
- Handles file overwrites with retries for sharing violations (e.g., if Roblox is running).

Differences:
- **`roblox.c`**: Focuses on Roblox setup—launches the latest `RobloxPlayerBeta.exe` after tasks complete.
- **`chrome.c`**: Extends Roblox tasks with Chrome-specific cleanup—deletes the Default browsing profile data, then launches `chrome.exe`.

**Note:** Paths are hardcoded (e.g., `W:\Roblox\AppData\Local\Roblox` for Roblox, `C:\Program Files\Google\Chrome` for Chrome). Adjust as needed. These tools affect all user profiles—use cautiously in multi-user systems.

## Requirements
- Windows OS (tested on Windows 10+).
- C compiler: Visual Studio (`cl.exe`) or MinGW.
- Linked libraries: `advapi32.lib`, `user32.lib`, `shell32.lib`, `shlwapi.lib`, `ole32.lib`, `oleaut32.lib`, `comctl32.lib`.
- No external dependencies beyond Windows SDK.


This produces `roblox.exe` and `chrome.exe`. Run the executable directly (e.g., double-click or via shortcut).

## Usage
1. Compile as above to generate the `.exe` files.
2. Run `roblox.exe` or `chrome.exe` (it will prompt for admin if needed).
3. The progress window appears centered and always on top:
   - **33%**: Protocol registration complete.
   - **66%**: File overwrites (autologout) complete.
   - **100%**: Target application (Roblox or Chrome) launched; window closes after 2 seconds.
4. Tasks run asynchronously; the window provides visual feedback.

**Example Behavior (roblox.exe):**
- Registers `roblox-player://` protocol to open latest Roblox Beta.
- Clears login data for all users.
- Launches `RobloxPlayerBeta.exe` from the most recent `version-*` folder.

**Example Behavior (chrome.exe):**
- Deletes Chrome's Default profile (history, cookies, etc.).
- Performs Roblox protocol registration and autologout.
- Launches `chrome.exe` from Chrome's Application folder.

## How It Works
### Shared Components
1. **Admin Check & Relaunch** (`is_admin()`, `run_as_admin()`):
   - Uses `IsUserAnAdmin()` to verify elevation.
   - If not admin, relaunches via `ShellExecuteW` with "runas".

2. **Roblox Protocol Registration** (`register_roblox_protocol()`, `run_uri_logic()`):
   - Scans `W:\Roblox\AppData\Local\Roblox\Versions\version-*` for the first folder with `RobloxPlayerBeta.exe`.
   - Registers `roblox-player` in `HKEY_CLASSES_ROOT`:
     - Sets default icon and command: `"path\to\RobloxPlayerBeta.exe" %1`.
     - Stores version folder name for reference.
   - Updates progress to 33%.

3. **File Overwrites for Autologout** (`overwrite_file()`, `overwrite_files()`):
   - Scans `C:\Users\*` for user directories (skips `.`, `..`).
   - For each user's `AppData\Local\Roblox\LocalStorage`:
     - Overwrites `appStorage.json` and `memProfStorage*.json` with `{"placeholder": true}`.
     - Overwrites `*.dat` files with `{"CookiesVersion":"1","CookiesData":""}`.
   - Retries up to 5 times on `ERROR_SHARING_VIOLATION` (sleeps 100ms).
   - Updates progress to 66%.

### roblox.c Specific: Launch Roblox (`launch_latest_roblox()`)
- Scans `version-*` folders for `RobloxPlayerBeta.exe`.
- Launches the first found executable via `CreateProcessA` (no window).
- Updates progress to 100%, sleeps 2s, quits.

### chrome.c Specific
1. **Chrome Data Deletion** (`deleteChromeData()`, `deleteFolder()`):
   - Gets `%LOCALAPPDATA%\Google\Chrome\User Data\Default`.
   - Uses `SHFileOperation` to silently delete the folder (`FO_DELETE` with no UI flags).

2. **Launch Chrome** (`launch_latest_roblox()`—misnamed):
   - Hardcoded to `C:\Program Files\Google\Chrome\Application\chrome.exe`.
   - Launches via `CreateProcessA` (no window).
   - Updates progress to 100%, sleeps 2s, quits.

### GUI Setup (`WindowProc()`, `WinMain()`)
- Registers a custom window class (`RobloxProgressClass`).
- Creates a top-most, tool window (400x150px) with label and progress bar.
- Initializes common controls for the progress bar.
- Message loop handles creation/destruction.

## Potential Issues & Notes
- **Admin Rights:** Always required for registry writes and multi-user file access—UAC prompt may appear.
- **Hardcoded Paths:** Roblox assumes `W:\Roblox`; Chrome assumes standard install. Edit source if customized.
- **File Locks:** Retries help with open files, but force-close Roblox/Chrome if persistent issues.
- **Chrome Deletion:** Only targets Default profile—other profiles (e.g., Profile 1) are untouched. Data loss is permanent.
- **Protocol Registration:** Overwrites existing `roblox-player` handler; uninstall Roblox to revert.
- **No Error Handling:** Basic; check console (if compiled with console) or add logging for debugging.
- **Customization:**
  - Add more Chrome profiles to `deleteChromeData()`.
  - Change placeholders in `overwrite_files()` for different autologout behavior.
  - Modify launch paths for portable installs.

## License
Feel free to use/modify for personal use. Credit if shared!

---

*Last Updated: October 02, 2025*
