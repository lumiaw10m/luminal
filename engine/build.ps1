$here = $PSScriptRoot
$bat = @"
@echo off
set "PATH=%PATH%;C:\Program Files (x86)\Microsoft Visual Studio\Installer"
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat" x64_arm 10.0.19041.0 >nul
cd /d "$here"
cl /nologo /O2 /MT /W3 /wd4133 /wd4047 /wd4024 /wd4312 /wd4311 /wd4302 /D_ARM_WINAPI_PARTITION_DESKTOP_SDK_AVAILABLE=1 /D_CRT_SECURE_NO_WARNINGS /D_WINSOCK_DEPRECATED_NO_WARNINGS tx.c /Fe:tx.exe /link /SUBSYSTEM:CONSOLE,6.02 kernel32.lib ntdll.lib ws2_32.lib /BASE:0x72000000
"@
$f = Join-Path $env:TEMP 'tx-build.bat'
Set-Content -Path $f -Value $bat -Encoding ascii
cmd /c $f
$code = $LASTEXITCODE
Remove-Item $f
Remove-Item (Join-Path $here 'tx.obj') -ErrorAction SilentlyContinue
exit $code
