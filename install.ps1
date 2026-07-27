<#
.SYNOPSIS
    Zeta Language Installer for Windows

.DESCRIPTION
    Compiles and installs Zeta Language binaries on Windows.
    Requires clang++ (LLVM) or cl.exe (MSVC) in PATH.

.PARAMETER Prefix
    Installation directory. Default: C:\Zeta

.PARAMETER Help
    Show help message

.EXAMPLE
    .\install.ps1
    .\install.ps1 -Prefix "$env:LOCALAPPDATA\Zeta"
    .\install.ps1 -Help
#>

param(
    [string]$Prefix = "C:\Zeta",
    [switch]$Help
)

$ErrorActionPreference = "Stop"

# Colors
function Write-Info  { Write-Host "[INFO]  $args" -ForegroundColor Blue }
function Write-Ok    { Write-Host "[OK]    $args" -ForegroundColor Green }
function Write-Warn  { Write-Host "[WARN]  $args" -ForegroundColor Yellow }
function Write-Err   { Write-Host "[ERROR] $args" -ForegroundColor Red }

# Show help
if ($Help) {
    Write-Host ""
    Write-Host "Zeta Language Installer (Windows)" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Uso:" -ForegroundColor Yellow
    Write-Host "  .\install.ps1                         Instalar en C:\Zeta"
    Write-Host "  .\install.ps1 -Prefix 'C:\MiRuta'     Instalar en ruta custom"
    Write-Host "  .\install.ps1 -Help                   Mostrar esta ayuda"
    Write-Host ""
    Write-Host "Requisitos:" -ForegroundColor Yellow
    Write-Host "  - clang++ (LLVM) o cl.exe (MSVC) en PATH"
    Write-Host "  - Windows 10/11 x64"
    Write-Host ""
    Write-Host "Despues de instalar, reinicia la terminal para que PATH surta efecto." -ForegroundColor Gray
    Write-Host ""
    exit 0
}

Write-Host ""
Write-Host "=== Zeta Language Installer (Windows) ===" -ForegroundColor Cyan
Write-Host ""

# ── Detect compiler ──────────────────────────────────────────────
$CXX = $null
$CompilerName = ""

# Try clang++ first
if (Get-Command "clang++" -ErrorAction SilentlyContinue) {
    $CXX = "clang++"
    $CompilerName = "clang++"
    Write-Info "Using: $(& $CXX --version 2>&1 | Select-Object -First 1)"
}
# Try cl.exe (MSVC)
elseif (Get-Command "cl.exe" -ErrorAction SilentlyContinue) {
    $CXX = "cl.exe"
    $CompilerName = "MSVC (cl.exe)"
    Write-Info "Using: MSVC compiler"
}
else {
    Write-Err "No C++ compiler found."
    Write-Host ""
    Write-Host "Install one of:" -ForegroundColor Yellow
    Write-Host "  LLVM/Clang: https://releases.llvm.org/download.html" -ForegroundColor Gray
    Write-Host "  MSVC:       Install 'Visual Studio Build Tools' from visualstudio.microsoft.com" -ForegroundColor Gray
    Write-Host "  Or: winget install LLVM.LLVM" -ForegroundColor Gray
    exit 1
}

# ── Detect architecture ──────────────────────────────────────────
$Arch = if ([Environment]::Is64BitOperatingSystem) { "x64" } else { "x86" }
Write-Info "Architecture: $Arch"

# ── Setup directories ────────────────────────────────────────────
$BinDir  = Join-Path $Prefix "bin"
$LibDir  = Join-Path $Prefix "lib"
$DocDir  = Join-Path $Prefix "docs"
$TmpDir  = Join-Path $Prefix "tmp_build"

Write-Info "Install prefix: $Prefix"

# Create directories
New-Item -ItemType Directory -Force -Path $BinDir | Out-Null
New-Item -ItemType Directory -Force -Path $LibDir | Out-Null
New-Item -ItemType Directory -Force -Path $DocDir | Out-Null
New-Item -ItemType Directory -Force -Path $TmpDir | Out-Null

# ── Source paths ─────────────────────────────────────────────────
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path

$CoreSrcs = @(
    "src/core\valor_zeta.cpp",
    "src/core\tabla_simbolos.cpp",
    "src/core\estadisticas.cpp",
    "src/core\errores.cpp",
    "src\core\serializador.cpp",
    "src\core\grafo_json.cpp",
    "src\core\xlsx_reader.cpp",
    "deps\pugixml\src\pugixml.cpp",
    "src\lexer\lexer.cpp",
    "src\parser\parser.cpp",
    "src\interpreter\interpreter.cpp",
    "src\dl_loader\dl_loader.cpp"
)

$Includes = @(
    "-std=c++20",
    "-I", "include",
    "-I", ".",
    "-I", "deps\pugixml\src",
    "-I", "deps\miniz"
)

$Flags = @(
    "-O2",
    "-Wno-deprecated-literal-operator",
    "-Wno-unused-variable",
    "-Wno-unused-but-set-variable"
)

# Miniz C sources
$MinizCSrcs = @(
    "deps\miniz\miniz.c",
    "deps\miniz\miniz_zip.c",
    "deps\miniz\miniz_tdef.c",
    "deps\miniz\miniz_tinfl.c"
)

# ── Build function ───────────────────────────────────────────────
function Build-Binary {
    param(
        [string]$Name,
        [string]$MainSrc,
        [string[]]$ExtraSrcs = @(),
        [string[]]$ExtraLibs = @()
    )

    Write-Info "Compiling $Name..."

    $OutFile = Join-Path $BinDir "$Name.exe"

    if ($CompilerName -eq "clang++") {
        $MinizObjs = @()
        foreach ($src in $MinizCSrcs) {
            $obj = Join-Path $TmpDir ([IO.Path]::GetFileNameWithoutExtension($src) + ".o")
            & cc -c -O2 -I deps\miniz -o $obj $src 2>$null
            $MinizObjs += $obj
        }

        $allSrcs = @($MainSrc) + $ExtraSrcs + $CoreSrcs
        & $CXX @Includes @Flags $allSrcs $MinizObjs @ExtraLibs -o $OutFile -lz 2>&1
    }
    else {
        # MSVC (cl.exe) - simplified
        $allSrcs = @($MainSrc) + $ExtraSrcs + $CoreSrcs
        & $CXX /std:c++20 /O2 /EHsc /I "include" /I "." /I "deps\pugixml\src" /I "deps\miniz" `
            $allSrcs /Fe:$OutFile 2>&1
    }

    if ($LASTEXITCODE -ne 0) {
        Write-Warn "Failed to compile $Name"
        return $false
    }

    Remove-Item -Path $OutFile -ErrorAction SilentlyContinue
    # If using MSVC, output may be in different location
    if (-not (Test-Path $OutFile)) {
        $OutFile = Join-Path $TmpDir "$Name.exe"
        if (Test-Path $OutFile) {
            Copy-Item $OutFile (Join-Path $BinDir "$Name.exe") -Force
        }
    }

    Write-Ok "$Name -> $BinDir\$Name.exe"
    return $true
}

# ── Build binaries ───────────────────────────────────────────────
Push-Location $ScriptDir

$Built = @()

# zeta CLI
if (Build-Binary -Name "zeta" -MainSrc "src\main.cpp") {
    $Built += "zeta"
}

# zeta_server
if (Build-Binary -Name "zeta_server" -MainSrc "src\server_main.cpp" -ExtraLibs @("-lpthread")) {
    $Built += "zeta_server"
}

# zeta_term
if (Build-Binary -Name "zeta_term" -MainSrc "src\term\main.cpp") {
    $Built += "zeta_term"
}

# zeta-lsp
$LspSrcs = @(
    "lsp\zeta-lsp.cpp",
    "lsp\transport.cpp",
    "lsp\builtins.cpp",
    "lsp\analyzer.cpp",
    "src\lexer\lexer.cpp",
    "src\parser\parser.cpp",
    "src\core\valor_zeta.cpp",
    "src\core\errores.cpp",
    "src\core\estadisticas.cpp"
)
if (Build-Binary -Name "zeta-lsp" -MainSrc $LspSrcs[0] -ExtraSrcs $LspSrcs[1..($LspSrcs.Count-1)]) {
    $Built += "zeta-lsp"
}

Pop-Location

# ── Install .zl library files ────────────────────────────────────
Write-Info "Installing Zeta libraries..."
$ZlFiles = Get-ChildItem -Path (Join-Path $ScriptDir "lib") -Filter "*.zl" -ErrorAction SilentlyContinue
if ($ZlFiles) {
    foreach ($f in $ZlFiles) {
        Copy-Item $f.FullName -Destination $LibDir -Force
    }
    Write-Ok "Library files installed -> $LibDir"
}
else {
    Write-Warn "No .zl library files found"
}

# ── Install documentation ────────────────────────────────────────
Write-Info "Installing documentation..."
$DocFiles = Get-ChildItem -Path (Join-Path $ScriptDir "docs") -Filter "*.md" -ErrorAction SilentlyContinue
if ($DocFiles) {
    foreach ($f in $DocFiles) {
        Copy-Item $f.FullName -Destination $DocDir -Force
    }
    Write-Ok "Documentation installed -> $DocDir"
}

# ── Cleanup build temp ───────────────────────────────────────────
Remove-Item -Recurse -Force $TmpDir -ErrorAction SilentlyContinue

# ── Add to PATH ──────────────────────────────────────────────────
Write-Info "Adding $BinDir to user PATH..."
$UserPath = [Environment]::GetEnvironmentVariable("Path", "User")
if ($UserPath -notlike "*$BinDir*") {
    [Environment]::SetEnvironmentVariable("Path", "$UserPath;$BinDir", "User")
    $env:Path = "$env:Path;$BinDir"
    Write-Ok "Added to PATH (restart terminal to apply)"
}
else {
    Write-Ok "$BinDir already in PATH"
}

# ── Verify ───────────────────────────────────────────────────────
Write-Host ""
Write-Host "=== Installation complete! ===" -ForegroundColor Green
Write-Host ""
Write-Host "Installed to:    $Prefix"
Write-Host "Binaries:        $BinDir"
Write-Host "Libraries:       $LibDir"
Write-Host ""
Write-Host "Quick start:" -ForegroundColor Cyan
Write-Host "  zeta --help                    # Show CLI options"
Write-Host "  zeta script.zl                 # Run a script"
Write-Host "  zeta_server --port 8080        # Start HTTP server"
Write-Host "  zeta_term --host localhost      # Terminal renderer"
Write-Host ""
Write-Host "Note: Restart your terminal for PATH changes to take effect." -ForegroundColor Yellow
