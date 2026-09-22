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

rem UE 5.8 still launches local UBA after -NoUBA. Kill it in the project config.
if not exist "Saved\UnrealBuildTool" mkdir "Saved\UnrealBuildTool"
copy /Y "Build\UnrealBuildTool\BuildConfiguration.xml" "Saved\UnrealBuildTool\BuildConfiguration.xml" >nul

set FREEGB=0
for /f %%A in ('powershell -NoProfile -Command "[int]((Get-PSDrive -Name C).Free/1GB)"') do set FREEGB=%%A
echo C: free space: %FREEGB% GB
if %FREEGB% LSS 20 (
  echo.
  echo DISK FULL — Unreal cannot compile.
  echo C: has %FREEGB% GB free. Need about 20 GB.
  echo.
  echo Delete these, empty Recycle Bin, then run this script again:
  echo   1. This folder's Intermediate\  and  Saved\
  echo   2. C:\ProgramData\Epic\UnrealBuildAccelerator
  echo   3. C:\Users\ty\Downloads\fleet-commander-unreal-main  (old cube zip)
  echo   4. Recycle Bin
  echo.
  echo %DATE% %TIME% > "%LOG%"
  echo DISK FULL: C: has %FREEGB% GB free. Need ~20 GB.>> "%LOG%"
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
echo FREEGB=%FREEGB%>> "%LOG%"
echo.

call "%BUILD%" FleetCommanderEditor Win64 Development "%PROJECT%" -WaitMutex -NoHotReload -NoUBA -Executor=Parallel -UBAStoreCapacityGb=2 -MaxParallelActions=3 >> "%LOG%" 2>&1
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
