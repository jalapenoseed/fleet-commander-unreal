@echo off
setlocal
set "UE_EDITOR=C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe"
set "PROJECT=%~dp0FleetCommander.uproject"
if not exist "%UE_EDITOR%" (
  echo Could not find UE 5.8 at:
  echo   %UE_EDITOR%
  echo Open FleetCommander.uproject from the Epic Launcher or Unreal Hub instead.
  pause
  exit /b 1
)
start "" "%UE_EDITOR%" "%PROJECT%"
endlocal
