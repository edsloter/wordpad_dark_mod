#Requires -Version 5.1
<#
.SYNOPSIS
    All-in-one build script for the WordPad dark-mode modification.

.DESCRIPTION
    1. Detects a capable C++ toolchain (MSBuild + Visual C++ compiler + MFC
       headers + Windows SDK) for this MFC based project.
    2. If the toolchain (or just MFC) is missing, the script downloads the
       official Visual Studio Build Tools bootstrapper - or re-uses an
       existing Visual Studio instance - and installs the missing
       components.  Administrator rights are requested only when an
       installation is actually required; compiling itself runs unelevated.
    3. Compiles WordPad.exe with MSBuild for every requested
       configuration/platform combination and reports the produced binaries.

    The project is a classic MFC application, therefore only Microsoft
    Visual C++ can build it (MinGW/Clang cannot).  Compatible with Windows
    PowerShell 5.1 and PowerShell 7+.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File .\Build-WordPad.ps1

.EXAMPLE
    .\Build-WordPad.ps1 -Configuration "Release Static" -Platform Win32,x64 -Clean

.EXAMPLE
    .\Build-WordPad.ps1 -ListConfigurations

.EXAMPLE
    .\Build-WordPad.ps1 -NoInstall -BinLog
#>
[CmdletBinding()]
param(
    # Any configuration from WordPad.vcxproj, e.g. "Unicode Release Static".
    [ValidateNotNullOrEmpty()]
    [string]$Configuration = 'Unicode Release Static',

    # One or more platforms: Win32 and/or x64 ("x86" is accepted as Win32).
    [ValidateNotNullOrEmpty()]
    [string[]]$Platform = @('x64'),

    # Rebuild instead of incremental build.
    [switch]$Clean,

    # Force a specific platform toolset (v143, v142, ...).  By default the
    # newest toolset installed with the detected Visual Studio instance is
    # selected automatically, because the VS2010-era project files do not
    # declare one and would otherwise target the unavailable 'v100'.
    [string]$PlatformToolset = '',

    # Write an MSBuild binary log (build-logs\<config>-<platform>.binlog).
    [switch]$BinLog,

    # Never download or install anything; fail if the toolchain is missing.
    [switch]$NoInstall,

    # Print every configuration/platform combination of the project and exit.
    [switch]$ListConfigurations,

    # Internal: used when this script relaunches itself elevated, solely to
    # install the toolchain.  Do not use interactively.
    [switch]$InstallOnly
)

Set-StrictMode -Version 2.0
$ErrorActionPreference = 'Stop'

# ---------------------------------------------------------------------------
# Sanitise PSModulePath for Windows PowerShell
# ---------------------------------------------------------------------------
# When this script is started from a PowerShell 7 session (for example a
# .bat -> cmd.exe chain), the process inherits PowerShell 7 module paths.
# Windows PowerShell then finds the PS7 copies of "Microsoft.PowerShell.*"
# first, fails to load their manifests and silently loses commands such as
# Get-FileHash or Invoke-WebRequest.  Keep only Windows PowerShell roots.
if ($PSVersionTable.PSVersion.Major -lt 6 -and $env:PSModulePath) {
    $keptPaths = @()
    foreach ($modulePath in @($env:PSModulePath -split ';')) {
        if ($modulePath -and $modulePath -match 'WindowsPowerShell') { $keptPaths += $modulePath }
    }
    if ($keptPaths.Count -gt 0) { $env:PSModulePath = $keptPaths -join ';' }
}

# ---------------------------------------------------------------------------
# Locations
# ---------------------------------------------------------------------------
$script:RepoRoot    = $PSScriptRoot
if (-not $script:RepoRoot) { $script:RepoRoot = (Get-Location).ProviderPath }
$script:ProjectFile = Join-Path $script:RepoRoot 'WordPad.vcxproj'
$script:SolutionFile = Join-Path $script:RepoRoot 'WordPad.sln'

$script:VsWhereUrl    = 'https://aka.ms/vs/17/release/vs_buildtools.exe'
$script:VsWhereUrlOld = 'https://aka.ms/vs/16/release/vs_buildtools.exe'

$script:RelPaths = @(
    'MSBuild\Current\Bin\MSBuild.exe',
    'MSBuild\15.0\Bin\MSBuild.exe',
    'MSBuild\14.0\Bin\MSBuild.exe'
)

# ---------------------------------------------------------------------------
# Console helpers
# ---------------------------------------------------------------------------
function Write-Step    { param([string]$Message) Write-Host ''; Write-Host ('==> {0}' -f $Message) -ForegroundColor White }
function Write-Note    { param([string]$Message) Write-Host ('    {0}' -f $Message) -ForegroundColor Gray }
function Write-Success { param([string]$Message) Write-Host ('    {0}' -f $Message) -ForegroundColor Green }
function Write-Caution { param([string]$Message) Write-Host ('    {0}' -f $Message) -ForegroundColor Yellow }
function Write-Failure { param([string]$Message) Write-Host ('    {0}' -f $Message) -ForegroundColor Red }

function Format-FileSize {
    param([long]$Bytes)
    if ($Bytes -ge 1MB) { return ('{0:N2} MB' -f ($Bytes / 1MB)) }
    if ($Bytes -ge 1KB) { return ('{0:N1} KB' -f ($Bytes / 1KB)) }
    return ('{0} bytes' -f $Bytes)
}

# ---------------------------------------------------------------------------
# Elevation / process helpers
# ---------------------------------------------------------------------------
function Test-IsAdministrator {
    $identity  = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($identity)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Invoke-ElevatedSelf {
    <# Relaunches this script with the supplied arguments using UAC. #>
    param([string[]]$ExtraArguments)

    $shell = Join-Path $env:SystemRoot 'System32\WindowsPowerShell\v1.0\powershell.exe'
    $argList = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', ('"{0}"' -f $PSCommandPath))
    $argList += $ExtraArguments

    Write-Note ('Requesting administrator rights: {0} {1}' -f $shell, ($argList -join ' '))
    try {
        $proc = Start-Process -FilePath $shell -ArgumentList $argList -Verb RunAs -Wait -PassThru
        return $proc.ExitCode
    } catch {
        Write-Failure ('Elevation failed or was declined: {0}' -f $_.Exception.Message)
        return -1
    }
}

function Invoke-ExternalProcess {
    <# Runs an installer/bootstrapper and returns its exit code. #>
    param(
        [Parameter(Mandatory = $true)][string]$FilePath,
        [Parameter(Mandatory = $true)][string[]]$ArgumentList
    )

    Write-Note ('{0} {1}' -f $FilePath, ($ArgumentList -join ' '))
    try {
        $proc = Start-Process -FilePath $FilePath -ArgumentList $ArgumentList -Wait -PassThru
        $code = $proc.ExitCode
    } catch {
        Write-Failure ('Failed to start process: {0}' -f $_.Exception.Message)
        return -1
    }

    if ($code -eq 0)     { Write-Success 'Installer finished successfully.' }
    elseif ($code -eq 3010) { Write-Caution 'Installer finished: a reboot is required (exit code 3010).' }
    else { Write-Caution ('Installer finished with exit code {0}.' -f $code) }
    return $code
}

# ---------------------------------------------------------------------------
# Toolchain discovery
# ---------------------------------------------------------------------------
function Get-VSWherePath {
    $candidates = @()
    if (${env:ProgramFiles(x86)}) { $candidates += (Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe') }
    if ($env:ProgramFiles)        { $candidates += (Join-Path $env:ProgramFiles 'Microsoft Visual Studio\Installer\vswhere.exe') }
    foreach ($c in $candidates) {
        if (Test-Path -LiteralPath $c) { return $c }
    }
    return $null
}

function Get-VSInstallerPath {
    $candidates = @()
    if (${env:ProgramFiles(x86)}) { $candidates += (Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\setup.exe') }
    if ($env:ProgramFiles)        { $candidates += (Join-Path $env:ProgramFiles 'Microsoft Visual Studio\Installer\setup.exe') }
    foreach ($c in $candidates) {
        if (Test-Path -LiteralPath $c) { return $c }
    }
    return $null
}

function Get-VSInstancePaths {
    <# Returns an array of Visual Studio / Build Tools installation paths. #>
    $found = @()

    $vswhere = Get-VSWherePath
    if ($vswhere) {
        try {
            $raw = & $vswhere -all -products * -format json 2>$null
            $text = (@($raw) -join [Environment]::NewLine)
            if (-not [string]::IsNullOrWhiteSpace($text)) {
                foreach ($instance in @($text | ConvertFrom-Json)) {
                    if ($instance -and $instance.installationPath) {
                        if ($found -notcontains $instance.installationPath) { $found += $instance.installationPath }
                    }
                }
            }
        } catch {
            Write-Caution ('vswhere query failed: {0}' -f $_.Exception.Message)
        }
    }

    if ($found.Count -eq 0) {
        # Fallback: scan the well known VS 2015/2017+ layout without vswhere.
        $roots = @()
        if (${env:ProgramFiles(x86)}) { $roots += (Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio') }
        if ($env:ProgramFiles)        { $roots += (Join-Path $env:ProgramFiles 'Microsoft Visual Studio') }
        foreach ($root in $roots) {
            if (-not (Test-Path -LiteralPath $root)) { continue }
            foreach ($rel in $script:RelPaths) {
                $pattern = Join-Path $root ('*\{0}' -f $rel)
                foreach ($msbuild in @(Get-ChildItem -Path $pattern -File -ErrorAction SilentlyContinue)) {
                    $dir = $msbuild.DirectoryName
                    while ($dir -and ((Split-Path $dir -Leaf) -ne 'MSBuild')) { $dir = Split-Path $dir -Parent }
                    if ($dir) {
                        $instance = Split-Path $dir -Parent
                        if ($found -notcontains $instance) { $found += $instance }
                    }
                }
            }
        }
    }

    return $found
}

function Get-InstanceToolchain {
    <# Inspects one VS instance and reports what it can build. #>
    param([Parameter(Mandatory = $true)][string]$InstallPath)

    $msbuild = $null
    foreach ($rel in $script:RelPaths) {
        $candidate = Join-Path $InstallPath $rel
        if (Test-Path -LiteralPath $candidate) { $msbuild = $candidate; break }
    }

    $compilers = @(Get-ChildItem -Path (Join-Path $InstallPath 'VC\Tools\MSVC\*\bin\Host*\*\cl.exe') -File -ErrorAction SilentlyContinue)
    $mfcInclude = @(Get-ChildItem -Path (Join-Path $InstallPath 'VC\Tools\MSVC\*\atlmfc\include\afxwin.h') -File -ErrorAction SilentlyContinue)

    $toolsetDirs = @(Get-ChildItem -LiteralPath (Join-Path $InstallPath 'VC\Tools\MSVC') -Directory -ErrorAction SilentlyContinue |
                     Sort-Object Name -Descending)

    return [pscustomobject]@{
        InstallPath     = $InstallPath
        MSBuildPath     = $msbuild
        HasMSBuild      = [bool]$msbuild
        HasCompiler     = ($compilers.Count -gt 0)
        HasMFC          = ($mfcInclude.Count -gt 0)
        MFCInclude      = if ($mfcInclude.Count -gt 0) { $mfcInclude[0].FullName } else { $null }
        VCToolsVersion  = if ($toolsetDirs.Count -gt 0) { $toolsetDirs[0].Name } else { $null }
        IsCapable       = ([bool]$msbuild -and $compilers.Count -gt 0 -and $mfcInclude.Count -gt 0)
    }
}

function Get-ToolchainReports {
    $reports = @()
    foreach ($path in @(Get-VSInstancePaths)) {
        $reports += Get-InstanceToolchain -InstallPath $path
    }
    return $reports
}

function Find-CapableToolchain {
    foreach ($report in @(Get-ToolchainReports)) {
        if ($report.IsCapable) { return $report }
    }
    return $null
}

function Test-WindowsSDK {
    $keys = @(
        'HKLM:\SOFTWARE\WOW6432Node\Microsoft\Microsoft SDKs\Windows\v10.0',
        'HKLM:\SOFTWARE\Microsoft\Microsoft SDKs\Windows\v10.0'
    )
    foreach ($key in $keys) {
        if (Test-Path -LiteralPath $key) {
            $folder = (Get-Item -LiteralPath $key).GetValue('InstallationFolder')
            if ($folder -and (Test-Path -LiteralPath (Join-Path $folder 'Include'))) { return $true }
        }
    }
    if (${env:ProgramFiles(x86)}) {
        if (Test-Path -LiteralPath (Join-Path ${env:ProgramFiles(x86)} 'Windows Kits\10\Include')) { return $true }
    }
    return $false
}

function Get-MissingRequirements {
    param($Report)
    $missing = @()
    if (-not $Report.HasMSBuild)   { $missing += 'MSBuild' }
    if (-not $Report.HasCompiler)  { $missing += 'Visual C++ compiler (MSVC)' }
    if (-not $Report.HasMFC)       { $missing += 'MFC (atlmfc headers)' }
    if (-not (Test-WindowsSDK))    { $missing += 'Windows 10/11 SDK' }
    return $missing
}

# ---------------------------------------------------------------------------
# Download / install
# ---------------------------------------------------------------------------
function Get-VSBootstrapper {
    <# Downloads (or re-uses) the official VS Build Tools bootstrapper. #>
    $workDir = Join-Path $env:TEMP 'wordpad-build-tools'
    if (-not (Test-Path -LiteralPath $workDir)) {
        New-Item -ItemType Directory -Path $workDir -Force | Out-Null
    }

    # Windows 7/8.1 cannot run VS 2022 setup; fall back to the VS 2019 channel.
    $channel = if ([Environment]::OSVersion.Version.Major -ge 10) { '17' } else { '16' }
    $url = if ($channel -eq '17') { $script:VsWhereUrl } else { $script:VsWhereUrlOld }
    $destination = Join-Path $workDir ('vs_buildtools_{0}.exe' -f $channel)

    if (-not (Test-Path -LiteralPath $destination)) {
        Write-Note ('Downloading {0}' -f $url)
        $previous = $ProgressPreference
        try {
            [Net.ServicePointManager]::SecurityProtocol = [Net.ServicePointManager]::SecurityProtocol -bor [Net.SecurityProtocolType]::Tls12
            $ProgressPreference = 'SilentlyContinue'
            Invoke-WebRequest -Uri $url -OutFile $destination -UseBasicParsing -TimeoutSec 900
        } catch {
            Write-Failure ('Download failed: {0}' -f $_.Exception.Message)
            return $null
        } finally {
            $ProgressPreference = $previous
        }
    } else {
        Write-Note ('Re-using previously downloaded {0}' -f $destination)
    }

    $item = Get-Item -LiteralPath $destination -ErrorAction SilentlyContinue
    if (-not $item -or $item.Length -lt 1MB) {
        Write-Failure 'The downloaded bootstrapper is missing or truncated.'
        Remove-Item -LiteralPath $destination -Force -ErrorAction SilentlyContinue
        return $null
    }

    $signature = Get-AuthenticodeSignature -LiteralPath $destination
    $signer = $null
    if ($signature -and $signature.SignerCertificate) { $signer = $signature.SignerCertificate.Subject }
    if ($signature.Status -ne 'Valid' -or -not $signer -or $signer -notmatch 'Microsoft') {
        Write-Failure ('Bootstrapper signature check failed ({0}); file deleted.' -f $signature.Status)
        Remove-Item -LiteralPath $destination -Force -ErrorAction SilentlyContinue
        return $null
    }
    Write-Success ('Bootstrapper verified: {0}' -f $destination)
    return $destination
}

function Install-BuildToolchain {
    <# Installs whatever is missing; returns $true when the installer ran. #>
    Write-Step 'Installing the missing C++ build tools'

    $reports  = @(Get-ToolchainReports)
    $instances = @(Get-VSInstancePaths)
    $sdkPresent = Test-WindowsSDK

    if ($instances.Count -gt 0) {
        # Re-use an existing instance; prefer one that already has MSVC.
        $target = $null
        foreach ($report in $reports) { if ($report.HasCompiler) { $target = $report; break } }
        if (-not $target -and $reports.Count -gt 0) { $target = $reports[0] }
        if (-not $target) { $target = [pscustomobject]@{ InstallPath = $instances[0]; HasMSBuild = $false; HasCompiler = $false; HasMFC = $false } }

        $setup = Get-VSInstallerPath
        if (-not $setup) {
            Write-Failure ('Visual Studio instance found at "{0}" but the VS Installer (setup.exe) is missing.' -f $target.InstallPath)
            return $false
        }

        $needWorkload = (-not $target.HasCompiler) -or (-not $target.HasMSBuild) -or (-not $sdkPresent)
        $needMfc      = (-not $target.HasMFC)

        $argList = @('modify', '--installPath', ('"{0}"' -f $target.InstallPath))
        if ($needWorkload) { $argList += @('--add', 'Microsoft.VisualStudio.Workload.VCTools', '--includeRecommended') }
        if ($needMfc)      { $argList += @('--add', 'Microsoft.VisualStudio.Component.VC.ATLMFC') }
        if ($argList.Count -le 3) {
            Write-Caution 'Nothing to install (requirements already satisfied).'
            return $true
        }
        $argList += @('--quiet', '--norestart')

        $null = Invoke-ExternalProcess -FilePath $setup -ArgumentList $argList
        return $true
    }

    # No Visual Studio at all -> download the Build Tools bootstrapper.
    $bootstrapper = Get-VSBootstrapper
    if (-not $bootstrapper) { return $false }

    $argList = @(
        '--quiet', '--norestart',
        '--add', 'Microsoft.VisualStudio.Workload.VCTools', '--includeRecommended',
        '--add', 'Microsoft.VisualStudio.Component.VC.ATLMFC'
    )
    $null = Invoke-ExternalProcess -FilePath $bootstrapper -ArgumentList $argList
    return $true
}

# ---------------------------------------------------------------------------
# Project inspection
# ---------------------------------------------------------------------------
function Get-ProjectConfigurations {
    if (-not (Test-Path -LiteralPath $script:ProjectFile)) {
        throw ('Project file not found: {0}' -f $script:ProjectFile)
    }
    [xml]$xml = Get-Content -LiteralPath $script:ProjectFile -Raw
    $ns = New-Object System.Xml.XmlNamespaceManager($xml.NameTable)
    $ns.AddNamespace('m', 'http://schemas.microsoft.com/developer/msbuild/2003')

    $list = @()
    foreach ($node in $xml.SelectNodes('//m:ProjectConfiguration', $ns)) {
        $cfgNode  = $node.SelectSingleNode('m:Configuration', $ns)
        $platNode = $node.SelectSingleNode('m:Platform', $ns)
        $cfgValue = $null
        $platValue = $null
        if ($cfgNode)  { $cfgValue  = $cfgNode.InnerText }
        if ($platNode) { $platValue = $platNode.InnerText }

        if (-not $cfgValue -or -not $platValue) {
            # Older/newer project formats only carry "Configuration|Platform"
            # in the Include attribute.
            $include = $node.GetAttribute('Include')
            if ($include) {
                $parts = $include -split '\|', 2
                if ($parts.Count -eq 2) { $cfgValue = $parts[0]; $platValue = $parts[1] }
            }
        }

        if ($cfgValue -and $platValue) {
            $list += [pscustomobject]@{
                Configuration = $cfgValue
                Platform      = $platValue
            }
        }
    }
    return $list
}

function Format-ConfigurationList {
    $known = @(Get-ProjectConfigurations)
    $lines = @()
    foreach ($cfg in @($known | Sort-Object Configuration -Unique | ForEach-Object { $_.Configuration })) {
        $platforms = @($known | Where-Object { $_.Configuration -eq $cfg } | ForEach-Object { $_.Platform })
        $lines += ('{0,-24} : {1}' -f $cfg, ($platforms -join ', '))
    }
    return $lines
}

function Resolve-PlatformName {
    param([string]$Name)
    if ($Name -ieq 'x86') { return 'Win32' }
    if ($Name -ieq 'win32') { return 'Win32' }
    if ($Name -ieq 'x64') { return 'x64' }
    return $Name
}

function Get-ExpectedOutputPath {
    param(
        [Parameter(Mandatory = $true)][string]$Config,
        [Parameter(Mandatory = $true)][string]$Plat
    )
    if ($Plat -ieq 'Win32') {
        return (Join-Path $script:RepoRoot ('{0}\WordPad.exe' -f $Config))
    }
    return (Join-Path $script:RepoRoot ('{0}\{1}\WordPad.exe' -f $Plat, $Config))
}

function Resolve-PlatformToolset {
    <#
        The project files predate VS2012 and carry no <PlatformToolset>, so
        MSBuild defaults them to 'v100', which modern Visual Studio cannot
        build.  Instead of editing the sources, the script retargets at build
        time by picking the newest toolset the detected instance really ships.
    #>
    param(
        $Toolchain,
        [Parameter(Mandatory = $true)][string]$Plat,
        [string]$MsBuildVersion = '',
        [string]$Override = ''
    )

    if ($Override) { return $Override }

    $pattern = Join-Path $Toolchain.InstallPath ('MSBuild\Microsoft\VC\v*\Platforms\{0}\PlatformToolsets\v*' -f $Plat)
    $candidates = @()
    foreach ($dir in @(Get-ChildItem -Path $pattern -Directory -ErrorAction SilentlyContinue)) {
        if ($dir.Name -match '^v\d+$') { $candidates += $dir.Name }
    }
    if ($candidates.Count -gt 0) {
        return ($candidates | Sort-Object -Descending | Select-Object -First 1)
    }

    # Last resort: map the MSBuild/Visual Studio generation to its toolset.
    if ($MsBuildVersion -match '^\s*(\d+)') {
        $major = [int]$Matches[1]
        if ($major -ge 17) { return 'v143' }
        if ($major -eq 16) { return 'v142' }
        if ($major -eq 15) { return 'v141' }
        if ($major -eq 14) { return 'v140' }
    }
    return $null
}

# ---------------------------------------------------------------------------
# Build
# ---------------------------------------------------------------------------
function Invoke-MSBuildBuild {
    param(
        [Parameter(Mandatory = $true)]$Toolchain,
        [Parameter(Mandatory = $true)][string]$Config,
        [Parameter(Mandatory = $true)][string]$Plat,
        [string]$Toolset = '',
        [switch]$Rebuild,
        [switch]$WriteBinLog
    )

    $target = if ($Rebuild) { 'Rebuild' } else { 'Build' }
    $msbuildArgs = @(
        $script:ProjectFile,
        '/nologo',
        '/m',
        '/v:minimal',
        ('/t:{0}' -f $target),
        ('/p:Configuration={0}' -f $Config),
        ('/p:Platform={0}' -f $Plat)
    )
    if ($Toolset) { $msbuildArgs += ('/p:PlatformToolset={0}' -f $Toolset) }

    if ($WriteBinLog) {
        $logDir = Join-Path $script:RepoRoot 'build-logs'
        if (-not (Test-Path -LiteralPath $logDir)) {
            New-Item -ItemType Directory -Path $logDir -Force | Out-Null
        }
        $safeName = ('{0}-{1}' -f $Config, $Plat) -replace '[^\w\.-]', '_'
        $msbuildArgs += '/bl:{0}' -f (Join-Path $logDir ('{0}.binlog' -f $safeName))
    }

    $displayArgs = ($msbuildArgs | Select-Object -Skip 1) -join ' '
    Write-Note ('MSBuild {0} {1}' -f $Toolchain.MSBuildPath, $displayArgs)

    # MSBuild reports problems on stderr; keep stderr un-redirected and relax
    # $ErrorActionPreference so native output can never become a terminating
    # error while the compiler is running.  Its stdout must be forwarded to
    # the host explicitly, otherwise the pipeline output of this function
    # would be an array of log lines instead of the exit code.
    $previousEap = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    $exitCode = -1
    try {
        & $Toolchain.MSBuildPath @msbuildArgs | ForEach-Object { Write-Host $_ }
        $exitCode = $LASTEXITCODE
    } catch {
        Write-Failure ('Could not run MSBuild: {0}' -f $_.Exception.Message)
        $exitCode = -1
    } finally {
        $ErrorActionPreference = $previousEap
    }
    return $exitCode
}

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
if (-not (Test-Path -LiteralPath $script:ProjectFile)) {
    Write-Failure ('Cannot find the project file: {0}' -f $script:ProjectFile)
    exit 1
}

if ($ListConfigurations) {
    Write-Step 'Configurations defined in WordPad.vcxproj'
    foreach ($line in @(Format-ConfigurationList)) { Write-Host ('    {0}' -f $line) }
    exit 0
}

# --- validate requested configuration / platform ---------------------------
$known = @(Get-ProjectConfigurations)
$validConfigurations = @($known | ForEach-Object { $_.Configuration } | Select-Object -Unique)
if ($validConfigurations -notcontains $Configuration) {
    Write-Failure ('Unknown configuration "{0}".' -f $Configuration)
    Write-Note 'Valid configurations:'
    foreach ($line in @(Format-ConfigurationList)) { Write-Note $line }
    exit 2
}

$platforms = @()
foreach ($entry in $Platform) {
    # "powershell -File ... -Platform x86,Win32" delivers a single string,
    # while an interactive call delivers an array - normalise both.
    foreach ($part in @($entry -split ',')) {
        $p = $part.Trim()
        if (-not $p) { continue }

        $resolved = Resolve-PlatformName -Name $p
        $match = @($known | Where-Object { $_.Configuration -eq $Configuration -and $_.Platform -eq $resolved })
        if ($match.Count -eq 0) {
            Write-Failure ('Platform "{0}" is not defined for configuration "{1}".' -f $p, $Configuration)
            $platformsForConfig = @($known | Where-Object { $_.Configuration -eq $Configuration } | ForEach-Object { $_.Platform })
            Write-Note ('Platforms available for "{0}": {1}' -f $Configuration, ($platformsForConfig -join ', '))
            exit 2
        }
        if ($platforms -notcontains $resolved) { $platforms += $resolved }
    }
}

# --- toolchain -------------------------------------------------------------
Write-Step 'Checking for a capable C++ toolchain (MSBuild + MSVC + MFC + Windows SDK)'
$toolchain = Find-CapableToolchain
$reports = @()
if (-not $toolchain) { $reports = @(Get-ToolchainReports) }

if ($toolchain) {
    $sdkPresent = Test-WindowsSDK
    Write-Success ('Found: {0}' -f $toolchain.InstallPath)
    if ($toolchain.VCToolsVersion) { Write-Note ('MSVC toolset      : v{0}' -f $toolchain.VCToolsVersion) }
    Write-Note ('MSBuild            : {0}' -f $toolchain.MSBuildPath)
    Write-Note ('MFC headers        : {0}' -f $toolchain.MFCInclude)
    Write-Note ('Windows SDK present: {0}' -f $sdkPresent)
} else {
    if ($reports.Count -gt 0) {
        Write-Caution 'Visual Studio / Build Tools found, but none of the instances can build this project:'
        foreach ($report in $reports) {
            $missing = @(Get-MissingRequirements -Report $report)
            Write-Note ('{0}' -f $report.InstallPath)
            if ($missing.Count -gt 0) {
                Write-Note ('    missing: {0}' -f ($missing -join ', '))
            } else {
                Write-Note '    missing: (unknown)'
            }
        }
    } else {
        Write-Caution 'No Visual Studio or Build Tools installation was detected.'
    }
}

if (-not $toolchain) {
    if ($NoInstall) {
        Write-Failure 'No capable toolchain available and -NoInstall was specified.'
        Write-Note 'Re-run without -NoInstall to download and install the missing components automatically.'
        exit 3
    }

    if (Test-IsAdministrator) {
        $null = Install-BuildToolchain
        $toolchain = Find-CapableToolchain
    } else {
        Write-Step 'Administrator rights are required to install the missing build tools'
        $elevatedExit = Invoke-ElevatedSelf -ExtraArguments @('-InstallOnly')
        if ($elevatedExit -eq -1) {
            Write-Failure 'Installation was not performed (elevation declined).'
        } else {
            Write-Note ('Elevated installation finished with exit code {0}; re-checking...' -f $elevatedExit)
        }
        $toolchain = Find-CapableToolchain
    }

    if (-not $toolchain) {
        Write-Failure 'The required build tools could not be installed automatically.'
        Write-Note 'Install them manually via "Visual Studio Installer":'
        Write-Note '  - Workload "Desktop development with C++" (MSBuild, MSVC, Windows SDK)'
        Write-Note '  - Component  "C++ MFC for latest v143 build tools"'
        Write-Note 'Then re-run this script.'
        exit 4
    }
    Write-Success ('Toolchain ready: {0}' -f $toolchain.InstallPath)
}

if ($InstallOnly) {
    # We were re-launched elevated just to provision the toolchain.
    Write-Success 'Toolchain installation step completed.'
    exit 0
}

# --- build -----------------------------------------------------------------
$built = @()
$failed = @()
$msbuildVersion = ''
$prevVersionEap = $ErrorActionPreference
$ErrorActionPreference = 'Continue'
try { $msbuildVersion = (@(& $toolchain.MSBuildPath -version -nologo 2>$null) | Select-Object -Last 1) } catch { }
$ErrorActionPreference = $prevVersionEap

foreach ($plat in $platforms) {
    $toolset = Resolve-PlatformToolset -Toolchain $toolchain -Plat $plat -MsBuildVersion $msbuildVersion -Override $PlatformToolset

    Write-Step ('Building {0} | {1}' -f $Configuration, $plat)
    Write-Note ('MSBuild version    : {0}' -f $msbuildVersion)
    if ($toolset) {
        Write-Note ('Platform toolset   : {0} (retargeted; the project itself declares none)' -f $toolset)
    } else {
        Write-Caution 'No platform toolset was resolved; MSBuild will use the project default.'
    }
    if (-not (Test-WindowsSDK)) {
        Write-Caution 'No Windows SDK was detected; the build will fail if that is really the case.'
    }

    $exitCode = Invoke-MSBuildBuild -Toolchain $toolchain -Config $Configuration -Plat $plat -Toolset $toolset -Rebuild:$Clean -WriteBinLog:$BinLog

    $output = Get-ExpectedOutputPath -Config $Configuration -Plat $plat
    if ($exitCode -eq 0 -and (Test-Path -LiteralPath $output)) {
        $item = Get-Item -LiteralPath $output
        Write-Success ('Build succeeded -> {0}' -f $output)
        Write-Note ('Size        : {0}' -f (Format-FileSize -Bytes $item.Length))
        Write-Note ('File version : {0}' -f $item.VersionInfo.FileVersion)
        Write-Note ('Product      : {0}' -f $item.VersionInfo.ProductName)
        # Optional extras: never let them turn a successful build into a failure.
        if (Get-Command Get-FileHash -ErrorAction SilentlyContinue) {
            try { Write-Note ('SHA256       : {0}' -f (Get-FileHash -LiteralPath $output -Algorithm SHA256).Hash) } catch { }
        }
        if ($Configuration -notmatch 'Static') {
            Write-Caution 'Dynamic MFC/runtime build: the target PC needs the VC++ 2015-2022 Redistributable (and the MFC DLL).'
        }
        $built += $output
    } else {
        Write-Failure ('Build FAILED for {0} | {1} (MSBuild exit code {2})' -f $Configuration, $plat, $exitCode)
        if (Test-Path -LiteralPath $output) {
            Write-Caution 'An older copy of the output file may exist at that location.'
        }
        $failed += ('{0}|{1}' -f $Configuration, $plat)
    }
}

Write-Step 'Summary'
if ($built.Count -gt 0) {
    Write-Success ('{0} binary/binaries produced:' -f $built.Count)
    foreach ($b in $built) { Write-Note $b }
}
if ($failed.Count -gt 0) {
    Write-Failure ('{0} target(s) failed: {1}' -f $failed.Count, ($failed -join ', '))
    exit 1
}
if ($built.Count -eq 0) {
    Write-Failure 'Nothing was built.'
    exit 1
}
exit 0
