@rem this batch doesn't work for some paths which contains some special characters like the '+' character.

@echo off

setlocal
set parameters=%*

if defined parameters (endlocal && (cd %*) && call :begin .) else (endlocal && call :begin . cd)
goto end
:begin
cd %~s1
%2
:end
