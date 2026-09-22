@echo off
setlocal EnableExtensions
cd /d "%~dp0"

set "ENGINE=C:\Program Files\Epic Games\UE_5.8"
if not exist "%ENGINE%\Engine\Build\BatchFiles\Build.bat" set "ENGINE=D:\Program Files\Epic Games\UE_5.8"
if not exist "%ENGINE%\Engine\Build\BatchFiles\Build.bat" set "ENGINE=C:\Epic Games\UE_5.8"

set "BUILD=%ENGINE%\Engine\Build\BatchFiles\Build.bat"
set "EDITOR=%ENGINE%\Engine\Binaries\Win64\UnrealEditor.exe"
set "PROJECT=%cd%\FleetCommander.uproject"
set "LOG=%cd%\CompileLog.txt"

if not exist "%BUILD%" (
  echo Unreal 5.8 Build.bat not found. Install UE 5.8 like GRIDRUNNER.
  echo Tried: C:\Program Files\Epic Games\UE_5.8
  pause
  exit /b 1
)
if not exist "%PROJECT%" (
  echo FleetCommander.uproject is not in this folder:
  echo   %cd%
  echo Put this script next to the uproject, then run it again.
  pause
  exit /b 1
)

echo Compiling FleetCommanderEditor  Win64 Development
echo Project: %PROJECT%
echo Log:     %LOG%
echo.
echo %DATE% %TIME% > "%LOG%"
echo ENGINE=%ENGINE%>> "%LOG%"
echo PROJECT=%PROJECT%>> "%LOG%"
echo.

call "%BUILD%" FleetCommanderEditor Win64 Development "%PROJECT%" -WaitMutex -NoHotReload -NoUBA -MaxParallelActions=3 >> "%LOG%" 2>&1
set "ERR=%ERRORLEVEL%"
type "%LOG%"
echo.
if not "%ERR%"=="0" (
  echo COMPILE FAILED  exit %ERR%
  echo Full log saved to:
  echo   %LOG%
  echo Open that file and send the last error block if you want it fixed.
  pause
  exit /b %ERR%
)

echo COMPILE OK. Opening editor...
start "" "%EDITOR%" "%PROJECT%"
endlocal
