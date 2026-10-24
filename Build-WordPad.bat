@echo off
setlocal EnableExtensions

rem ---------------------------------------------------------------------------
rem Build-WordPad.bat
rem   Enables script execution for the current user (RemoteSigned) and then
rem   runs the all-in-one build script Build-WordPad.ps1 that sits next to
rem   this file.  Any arguments are passed straight through, e.g.:
rem
rem     Build-WordPad.bat -Configuration "Release Static" -Platform Win32,x64
rem ---------------------------------------------------------------------------

set "SCRIPT=%~dp0Build-WordPad.ps1"

if not exist "%SCRIPT%" (
    echo ERROR: Build-WordPad.ps1 was not found in:
    echo        %~dp0
    pause
    exit /b 1
)

rem Drop PowerShell 7 module paths that may have been inherited from a pwsh
rem session; Windows PowerShell cannot load those manifests and would then
rem lose built-in commands such as Set-ExecutionPolicy / Get-FileHash.
powershell.exe -NoProfile -Command "$keep = @($env:PSModulePath -split ';' | Where-Object { $_ -match 'WindowsPowerShell' }); if ($keep.Count -gt 0) { $env:PSModulePath = $keep -join ';' }; Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser -Force"
if errorlevel 1 (
    echo WARNING: Could not change the execution policy; continuing anyway.
)

powershell.exe -NoProfile -File "%SCRIPT%" %*
set "RC=%ERRORLEVEL%"

echo.
echo Build script exit code: %RC%
pause
exit /b %RC%
