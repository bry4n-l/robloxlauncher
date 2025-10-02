@echo off
setlocal
:: Load Visual Studio build tools environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

:: Clean previous build artifacts
del *.obj *.exe *.res 2>nul

:: Compile
cl chrome.c advapi32.lib user32.lib shell32.lib shlwapi.lib ole32.lib oleaut32.lib comctl32.lib /link /SUBSYSTEM:WINDOWS
cl roblox.c advapi32.lib user32.lib shell32.lib shlwapi.lib ole32.lib oleaut32.lib comctl32.lib /link /SUBSYSTEM:WINDOWS

:: Clean build artifacts
del *.obj *.res 2>nul

endlocal
pause
