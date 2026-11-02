. (Join-Path $PSScriptRoot 'phone.ps1')
$pause = Join-Path $env:TEMP 'tx-keepawake.pause'
$stop = Join-Path $env:TEMP 'tx-keepawake.stop'
$log = Join-Path $env:TEMP 'tx-keepawake.log'
Remove-Item $stop -ErrorAction SilentlyContinue
$end = (Get-Date).AddHours(8)
while ((Get-Date) -lt $end -and -not (Test-Path $stop)) {
    if (-not (Test-Path $pause)) {
        try {
            if ((Tx-Run 'tlist.exe | findstr /i luminal.exe' 6) -notmatch 'Luminal\.exe') {
                Tx-Run 'TestNavigationApi.exe LaunchModernApplication Luminal_h5t9p3hk10bwg!App App awake' 8 | Out-Null
                Add-Content $log "$(Get-Date -Format s) launched"
            }
        } catch {
            Add-Content $log "$(Get-Date -Format s) failed: $_"
        }
    }
    for ($i = 0; $i -lt 12 -and -not (Test-Path $stop); $i++) { Start-Sleep -Seconds 5 }
}
Add-Content $log "$(Get-Date -Format s) ended"
