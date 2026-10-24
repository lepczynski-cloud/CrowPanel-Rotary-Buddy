@echo off
setlocal
cd /d "%~dp0"
where py >nul 2>&1
if not errorlevel 1 (
    py -3 "tools\windows_cli.py" upload
    goto result
)
where python >nul 2>&1
if not errorlevel 1 (
    python "tools\windows_cli.py" upload
    goto result
)
echo.
echo ERROR: Python 3 is required on Windows.
echo Install Python from https://www.python.org/downloads/windows/
echo Enable "Add Python to PATH" and restart the terminal.
set "ERR=1"
goto finish
:result
set "ERR=%errorlevel%"
:finish
echo.
if not "%ERR%"=="0" echo Operation failed. Review the error above.
pause
exit /b %ERR%
