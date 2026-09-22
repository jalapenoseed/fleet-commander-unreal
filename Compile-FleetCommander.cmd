@echo off
setlocal
set "ENGINE=C:\Program Files\Epic Games\UE_5.8"
set "BUILD=%ENGINE%\Engine\Build\BatchFiles\Build.bat"
set "EDITOR=%ENGINE%\Engine\Binaries\Win64\UnrealEditor.exe"
set "PROJECT=%~dp0FleetCommander.uproject"
if not exist "%PROJECT%" set "PROJECT=%~dp0FleetCommander\FleetCommander.uproject"

if not exist "%BUILD%" (
  echo Unreal Engine 5.8 Build.bat was not found at:
  echo   %BUILD%
  echo.
  echo Install UE 5.8 from Epic Launcher, same as GRIDRUNNER.
  pause
  exit /b 1
)
if not exist "%PROJECT%" (
  echo FleetCommander.uproject not found next to this script.
  pause
  exit /b 1
)

echo.
echo Compiling FleetCommanderEditor  Win64 Development
echo Project: %PROJECT%
echo This is required once. Unreal cannot load the game until this DLL exists.
echo.
call "%BUILD%" FleetCommanderEditor Win64 Development "%PROJECT%" -WaitMutex -NoHotReload -NoUBA -MaxParallelActions=3
if errorlevel 1 (
  echo.
  echo C++ compile failed. Scroll this window for the error.
  echo You need Visual Studio 2022 with "Game development with C++" — same toolchain GRIDRUNNER uses.
  pause
  exit /b 1
)

echo.
echo Compile succeeded. Opening Unreal Editor...
start "" "%EDITOR%" "%PROJECT%"
endlocal
