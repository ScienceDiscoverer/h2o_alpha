@echo off
set name=h2o
cd C:\ScienceDiscoverer\PROGRAMZ\

title sd_wingw_build
g++ %name%.cpp res.coff -o %name%
IF %ERRORLEVEL% NEQ 0 pause
IF %ERRORLEVEL% EQU 0 start "" %name%.exe & taskkill /f /im cmd.exe /fi "windowtitle eq sd_wingw_build"