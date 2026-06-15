param(
  [string]$ToolsRoot,
  [string]$MetadataPath
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($ToolsRoot)) {
  if ([string]::IsNullOrWhiteSpace($env:RUNNER_TEMP)) {
    $ToolsRoot = Join-Path ([System.IO.Path]::GetTempPath()) "strata-windows-signing"
  } else {
    $ToolsRoot = Join-Path $env:RUNNER_TEMP "strata-windows-signing"
  }
}

if ([string]::IsNullOrWhiteSpace($MetadataPath)) {
  $MetadataPath = Join-Path $ToolsRoot "metadata.json"
}

function Get-RequiredEnv {
  param([string]$Name)

  $value = [Environment]::GetEnvironmentVariable($Name)
  if ([string]::IsNullOrWhiteSpace($value)) {
    throw "Missing required environment variable '$Name' for Windows code signing."
  }

  return $value
}

function Add-CiEnvironment {
  param(
    [string]$Name,
    [string]$Value
  )

  Set-Item -Path "Env:$Name" -Value $Value

  if (-not [string]::IsNullOrWhiteSpace($env:GITHUB_ENV)) {
    "$Name=$Value" | Out-File -FilePath $env:GITHUB_ENV -Append -Encoding utf8
  }
}

function Add-CiOutput {
  param(
    [string]$Name,
    [string]$Value
  )

  if (-not [string]::IsNullOrWhiteSpace($env:GITHUB_OUTPUT)) {
    "$Name=$Value" | Out-File -FilePath $env:GITHUB_OUTPUT -Append -Encoding utf8
  }
}

function Get-NuGetExe {
  $existing = Get-Command nuget.exe -ErrorAction SilentlyContinue
  if ($existing) {
    return $existing.Source
  }

  $nugetExe = Join-Path $ToolsRoot "nuget.exe"
  if (-not (Test-Path -LiteralPath $nugetExe)) {
    Write-Host "Downloading nuget.exe..."
    Invoke-WebRequest -Uri "https://dist.nuget.org/win-x86-commandline/latest/nuget.exe" -OutFile $nugetExe
  }

  return $nugetExe
}

function Install-NuGetPackage {
  param(
    [string]$NuGetExe,
    [string]$PackageId
  )

  $packagesRoot = Join-Path $ToolsRoot "packages"
  New-Item -ItemType Directory -Force -Path $packagesRoot | Out-Null

  & $NuGetExe install $PackageId -OutputDirectory $packagesRoot -NonInteractive -DirectDownload | Write-Host
  if ($LASTEXITCODE -ne 0) {
    throw "NuGet package install failed for '$PackageId'."
  }

  $package = Get-ChildItem -LiteralPath $packagesRoot -Directory |
    Where-Object { $_.Name -like "$PackageId.*" -or $_.Name -eq $PackageId } |
    Sort-Object Name -Descending |
    Select-Object -First 1

  if (-not $package) {
    throw "NuGet package '$PackageId' was installed but could not be found under '$packagesRoot'."
  }

  return $package.FullName
}

function Ensure-DotNet8Runtime {
  $hasDotNet8 = $false
  $dotnet = Get-Command dotnet -ErrorAction SilentlyContinue
  if ($dotnet) {
    $runtimes = & $dotnet.Source --list-runtimes
    $hasDotNet8 = ($runtimes | Where-Object { $_ -match '^Microsoft\.NETCore\.App 8\.' } | Measure-Object).Count -gt 0
  }

  if ($hasDotNet8) {
    Write-Host ".NET 8 runtime is available."
    return
  }

  $choco = Get-Command choco.exe -ErrorAction SilentlyContinue
  if (-not $choco) {
    throw ".NET 8 runtime is required for Azure Artifact Signing, and Chocolatey is not available to install it."
  }

  Write-Host "Installing .NET 8 runtime..."
  & $choco.Source install dotnet-8.0-runtime -y --no-progress
  if ($LASTEXITCODE -ne 0) {
    throw "Failed to install .NET 8 runtime."
  }
}

function Resolve-LatestFile {
  param(
    [string]$Root,
    [string]$Filter,
    [string]$RequiredPathFragment
  )

  $candidates = @(Get-ChildItem -LiteralPath $Root -Recurse -File -Filter $Filter |
    Where-Object {
      [string]::IsNullOrWhiteSpace($RequiredPathFragment) -or
      $_.FullName -like "*$RequiredPathFragment*"
    } |
    Sort-Object FullName -Descending)

  if (($candidates | Measure-Object).Count -eq 0) {
    throw "Could not find '$Filter' under '$Root'."
  }

  return $candidates[0].FullName
}

New-Item -ItemType Directory -Force -Path $ToolsRoot | Out-Null

$endpoint = Get-RequiredEnv "AZURE_ARTIFACT_SIGNING_ENDPOINT"
$accountName = Get-RequiredEnv "AZURE_ARTIFACT_SIGNING_ACCOUNT_NAME"
$certificateProfileName = Get-RequiredEnv "AZURE_ARTIFACT_SIGNING_CERT_PROFILE_NAME"

Ensure-DotNet8Runtime

$nugetExe = Get-NuGetExe
$sdkPackage = Install-NuGetPackage -NuGetExe $nugetExe -PackageId "Microsoft.Windows.SDK.BuildTools"
$clientPackage = Install-NuGetPackage -NuGetExe $nugetExe -PackageId "Microsoft.ArtifactSigning.Client"

$signToolPath = Resolve-LatestFile -Root $sdkPackage -Filter "signtool.exe" -RequiredPathFragment "\x64\"
$dlibPath = Resolve-LatestFile -Root $clientPackage -Filter "Azure.CodeSigning.Dlib.dll" -RequiredPathFragment "\x64\"

$correlationId = [guid]::NewGuid().ToString()
if (-not [string]::IsNullOrWhiteSpace($env:GITHUB_RUN_ID)) {
  $repo = if ([string]::IsNullOrWhiteSpace($env:GITHUB_REPOSITORY)) { "local" } else { $env:GITHUB_REPOSITORY }
  $correlationId = "$repo/actions/runs/$($env:GITHUB_RUN_ID)"
}

$metadata = [ordered]@{
  Endpoint = $endpoint
  CodeSigningAccountName = $accountName
  CertificateProfileName = $certificateProfileName
  CorrelationId = $correlationId
  ExcludeCredentials = @(
    "EnvironmentCredential",
    "ManagedIdentityCredential",
    "WorkloadIdentityCredential",
    "SharedTokenCacheCredential",
    "VisualStudioCredential",
    "VisualStudioCodeCredential",
    "AzurePowerShellCredential",
    "AzureDeveloperCliCredential",
    "InteractiveBrowserCredential"
  )
}

New-Item -ItemType Directory -Force -Path (Split-Path -Parent $MetadataPath) | Out-Null
$metadata | ConvertTo-Json -Depth 4 | Out-File -FilePath $MetadataPath -Encoding utf8

Add-CiEnvironment -Name "STRATA_SIGNTOOL_PATH" -Value $signToolPath
Add-CiEnvironment -Name "STRATA_AZURE_CODESIGN_DLIB_PATH" -Value $dlibPath
Add-CiEnvironment -Name "STRATA_AZURE_CODESIGN_METADATA_PATH" -Value $MetadataPath
Add-CiEnvironment -Name "STRATA_WINDOWS_CODESIGN_READY" -Value "true"

Add-CiOutput -Name "signtool" -Value $signToolPath
Add-CiOutput -Name "dlib" -Value $dlibPath
Add-CiOutput -Name "metadata" -Value $MetadataPath

Write-Host "Windows code signing tools ready."
Write-Host "SignTool: $signToolPath"
Write-Host "Artifact Signing dlib: $dlibPath"
Write-Host "Metadata: $MetadataPath"
