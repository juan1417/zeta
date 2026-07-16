<#
.SYNOPSIS
    Zeta Language Uninstaller for Windows

.PARAMETER Prefix
    Installation directory to remove. Default: C:\Zeta
#>

param(
    [string]$Prefix = "C:\Zeta"
)

$ErrorActionPreference = "Stop"

function Write-Info  { Write-Host "[INFO]  $args" -ForegroundColor Blue }
function Write-Ok    { Write-Host "[OK]    $args" -ForegroundColor Green }

Write-Host ""
Write-Host "=== Zeta Language Uninstaller (Windows) ===" -ForegroundColor Cyan
Write-Host ""

$BinDir = Join-Path $Prefix "bin"

Write-Info "Install prefix: $Prefix"

# Remove binaries
Write-Info "Removing binaries..."
foreach ($bin in @("zeta", "zeta_server", "zeta_dashboard", "zeta_term")) {
    $exe = Join-Path $BinDir "$bin.exe"
    if (Test-Path $exe) {
        Remove-Item $exe -Force
        Write-Ok "  Removed $exe"
    }
}

# Remove directory
if (Test-Path $Prefix) {
    Write-Info "Removing $Prefix..."
    Remove-Item -Recurse -Force $Prefix
    Write-Ok "  Removed $Prefix"
}

# Remove from PATH
Write-Info "Removing from PATH..."
$UserPath = [Environment]::GetEnvironmentVariable("Path", "User")
if ($UserPath -like "*$BinDir*") {
    $NewPath = ($UserPath -split ";" | Where-Object { $_ -ne $BinDir }) -join ";"
    [Environment]::SetEnvironmentVariable("Path", $NewPath, "User")
    Write-Ok "  Removed from PATH"
}

Write-Host ""
Write-Host "=== Uninstall complete! ===" -ForegroundColor Green
