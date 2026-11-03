param([switch]$Install)
. (Join-Path (Split-Path $PSScriptRoot) 'engine\phone.ps1')
$Phone = $script:TxPhone
$app = $PSScriptRoot
if ($Install) {

    $mf = Join-Path $app 'Package.appxmanifest'
    $m = [IO.File]::ReadAllText($mf)
    $m = [regex]::Replace($m, '(<Identity [^>]*Version=")(\d+)\.(\d+)\.(\d+)\.0"', { param($x) $x.Groups[1].Value + $x.Groups[2].Value + '.' + $x.Groups[3].Value + '.' + ([int]$x.Groups[4].Value + 1) + '.0"' })
    [IO.File]::WriteAllText($mf, $m)
}
$engine = Join-Path (Split-Path $app) 'engine'
$bat = @"
@echo off
set "PATH=%PATH%;C:\Program Files (x86)\Microsoft Visual Studio\Installer"
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat" x64_arm 10.0.19041.0 >nul
cd /d "$engine"
cl /nologo /O2 /MT /LD /W3 /wd4133 /wd4047 /wd4024 /wd4312 /wd4311 /wd4302 /DTX_DLL /D_ARM_WINAPI_PARTITION_DESKTOP_SDK_AVAILABLE=1 /D_CRT_SECURE_NO_WARNINGS /D_WINSOCK_DEPRECATED_NO_WARNINGS tx.c /Fo:txdll.obj /Fe:"$app\Payload\TxEngine.dll" /link /APPCONTAINER /OPT:REF kernel32.lib ntdll.lib ws2_32.lib || exit /b 1
del txdll.obj "$app\Payload\TxEngine.exp" "$app\Payload\TxEngine.lib"
cl /nologo /O2 /MT /W3 /wd4133 /wd4047 /wd4024 /wd4312 /wd4311 /wd4302 /D_ARM_WINAPI_PARTITION_DESKTOP_SDK_AVAILABLE=1 /D_CRT_SECURE_NO_WARNINGS /D_WINSOCK_DEPRECATED_NO_WARNINGS tx.c /Fo:txhost.obj /Fe:"$app\Payload\LuminalHost.exe" /link /SUBSYSTEM:CONSOLE,6.02 /APPCONTAINER /BASE:0x72000000 kernel32.lib ntdll.lib ws2_32.lib || exit /b 1
del txhost.obj
subst T: /d >nul 2>&1
subst T: "$(Split-Path $app)" || exit /b 1
cd /d T:\app
"C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe" Luminal.csproj /nologo /v:m /restore /p:Configuration=Release /p:Platform=ARM /p:AppxPackageDir=T:\app\out\ /p:UapAppxPackageBuildMode=SideloadOnly /p:AppxBundle=Never
set RC=%ERRORLEVEL%
cd /d "$env:TEMP"
subst T: /d
exit /b %RC%
"@
$f = Join-Path $env:TEMP 'tx-app.bat'
Set-Content -Path $f -Value $bat -Encoding ascii
cmd /c $f
$rc = $LASTEXITCODE
Remove-Item $f
if ($rc) { "BUILD FAILED $rc"; return }
$dir = Get-ChildItem "$app\out" -Directory | Sort-Object LastWriteTime | Select-Object -Last 1
"built: $($dir.Name)"
if (-not $Install) { return }
$appx = (Get-ChildItem $dir.FullName -Filter *.appx | Select-Object -First 1)
Add-Type -AssemblyName System.Net.Http
$h = New-Object Net.Http.HttpClient
$h.Timeout = [TimeSpan]::FromMinutes(5)
$mp = New-Object Net.Http.MultipartFormDataContent
$files = @($appx) + @(Get-ChildItem "$($dir.FullName)\Dependencies\ARM" -Filter *.appx -ErrorAction SilentlyContinue) + @(Get-ChildItem $app -Filter *.cer)
foreach ($x in $files) {
    $bc = New-Object Net.Http.ByteArrayContent(, [IO.File]::ReadAllBytes($x.FullName))
    $bc.Headers.TryAddWithoutValidation('Content-Disposition', "form-data; name=`"$($x.Name)`"; filename=`"$($x.Name)`"") | Out-Null
    $mp.Add($bc)
}
$r = $h.PostAsync("http://$Phone/api/app/packagemanager/package?package=$($appx.Name)", $mp).Result
"upload: $([int]$r.StatusCode)"
for ($i = 0; $i -lt 90; $i++) {
    try { $s = $h.GetAsync("http://$Phone/api/app/packagemanager/state").Result } catch { Start-Sleep -Seconds 2; continue }
    if ($s -and [int]$s.StatusCode -ne 204) { "install state $([int]$s.StatusCode): $($s.Content.ReadAsStringAsync().Result)"; break }
    Start-Sleep -Seconds 2
}
