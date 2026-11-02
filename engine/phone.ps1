$script:TxPhone = $env:LUMIA_IP

function Tx-Run([string]$Cmd, [int]$WaitSec = 15) {
    $c = New-Object Net.Sockets.TcpClient
    $c.Connect($script:TxPhone, 23)
    $s = $c.GetStream()
    $buf = New-Object byte[] 65536
    $w = New-Object IO.StreamWriter($s); $w.AutoFlush = $true
    Start-Sleep -Milliseconds 800
    while ($s.DataAvailable) { [void]$s.Read($buf, 0, $buf.Length) }

    $w.Write("echo @@%COMPUTERNAME:~0,0%BEGIN@@ & ($Cmd) & call echo rc=%^errorlevel% & echo @@%COMPUTERNAME:~0,0%END@@`r`n")
    $raw = ''
    $deadline = (Get-Date).AddSeconds($WaitSec)
    while ((Get-Date) -lt $deadline) {
        if ($s.DataAvailable) {
            $n = $s.Read($buf, 0, $buf.Length)
            $raw += [Text.Encoding]::UTF8.GetString($buf, 0, $n)
            if ($raw.Contains('@@END@@')) { break }
        } else { Start-Sleep -Milliseconds 100 }
    }
    $c.Close()
    $clean = $raw -replace '\x1b\[[0-9;?]*[A-Za-z]', "`n" -replace '[^\x09\x0A\x0D\x20-\x7E]', ''
    $b = $clean.LastIndexOf('@@BEGIN@@')
    if ($b -lt 0) { return '[no output marker]' }
    $clean = $clean.Substring($b + 9)
    $e = $clean.IndexOf('@@END@@')
    if ($e -ge 0) { $clean = $clean.Substring(0, $e) } else { $clean += "`n[timed out]" }
    ($clean -split "`r?`n" | Where-Object { $_.Trim() -ne '' -and $_ -notmatch '^C:\\windows\\system32>' }) -join "`n"
}

function Tx-Upload([string]$Local, [string]$Name, [string]$DestDir = 'U:\tx') {
    Add-Type -AssemblyName System.Net.Http
    $h = New-Object Net.Http.HttpClient
    $h.Timeout = [TimeSpan]::FromSeconds(120)
    $bytes = [IO.File]::ReadAllBytes($Local)
    $mp = New-Object Net.Http.MultipartFormDataContent
    $bc = New-Object Net.Http.ByteArrayContent(, $bytes)
    $bc.Headers.TryAddWithoutValidation('Content-Disposition', "form-data; name=`"file`"; filename=`"$Name`"") | Out-Null
    $mp.Add($bc)
    $code = [int]$h.PostAsync("http://$($script:TxPhone)/api/filesystem/apps/file?knownfolderid=PublicDocuments&path=%5C", $mp).Result.StatusCode
    if ($code -ne 200) { throw "upload of $Name failed: $code" }
    Tx-Run "move /y C:\Data\Users\Public\Documents\$Name $DestDir\$Name" 8 | Out-Null
}
