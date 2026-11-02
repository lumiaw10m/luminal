. (Join-Path (Split-Path $PSScriptRoot) 'engine\phone.ps1')
$script:Ls = 'U:\Users\DefApps\APPDATA\Local\Packages\Luminal_h5t9p3hk10bwg\LocalState'
$script:ShotDir = Join-Path $PSScriptRoot 'shots'

function Tx-Auto([string[]]$Lines) {
    $f = Join-Path $env:TEMP 'auto.txt'
    [IO.File]::WriteAllText($f, ($Lines -join "`n") + "`n")
    Tx-Upload $f 'auto.txt' $script:Ls
    Tx-Run 'kill.exe Luminal.exe' 5 | Out-Null
    Tx-Run 'TestNavigationApi.exe LaunchModernApplication Luminal_h5t9p3hk10bwg!App App auto' 8 | Out-Null
}

function Tx-Shot([string]$Name) {
    New-Item -ItemType Directory -Force $script:ShotDir | Out-Null
    Tx-Run "ScreenCapture.exe C:\Data\Users\Public\Documents\$Name.png" 8 | Out-Null
    $dest = Join-Path $script:ShotDir "$Name.png"
    Invoke-WebRequest "http://$($script:TxPhone)/api/filesystem/apps/file?knownfolderid=PublicDocuments&filename=$Name.png" -OutFile $dest -UseBasicParsing
    Tx-Run "del C:\Data\Users\Public\Documents\$Name.png" 5 | Out-Null
    $dest
}
