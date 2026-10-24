@echo off
setlocal
cd /d "%~dp0"
where py >nul 2>&1
if not errorlevel 1 (
    py -3 "tools\windows_cli.py" build
    goto result
)
where python >nul 2>&1
if not errorlevel 1 (
    python "tools\windows_cli.py" build
    goto result
)
echo Python 3 is missing. Install it from https://www.python.org/downloads/windows/
set "ERR=1"
goto finish
:result
set "ERR=%errorlevel%"
:finish
echo.
if not "%ERR%"=="0" echo Build failed. Review the error above.
pause
exit /b %ERR%
