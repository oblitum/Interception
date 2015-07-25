@echo off

SET PATH=%PATH%;..\tools

%ComSpec% /c "scd . && pushd . && %WDK%\bin\setenv %WDK% fre x86 WXP no_oacr && popd && build"
