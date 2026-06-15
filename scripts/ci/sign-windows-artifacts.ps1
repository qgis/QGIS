param(
  [ValidateSet("staging", "package", "verify")]
  [string]$Mode = "staging",
  [string]$Path,
  [string[]]$PackageFiles,
  [string]$PackageManifest,
  [switch]$Recurse,
  [switch]$VerifyOnly,
  [string]$SignToolPath = $env:STRATA_SIGNTOOL_PATH,
  [string]$DlibPath = $env:STRATA_AZURE_CODESIGN_DLIB_PATH,
  [string]$MetadataPath = $env:STRATA_AZURE_CODESIGN_METADATA_PATH
)

$ErrorActionPreference = "Stop"
$TimestampUrl = "http://timestamp.acs.microsoft.com"
$SignableExtensions = @(".exe", ".dll", ".pyd")

$script:Seen = 0
$script:Signed = 0
$script:Verified = 0
$script:SkippedNonPe = 0

function Resolve-RequiredFile {
  param(
    [string]$FilePath,
    [string]$Label
  )

  if ([string]::IsNullOrWhiteSpace($FilePath)) {
    throw "$Label path is not configured."
  }

  $resolved = Resolve-Path -LiteralPath $FilePath -ErrorAction SilentlyContinue
  if (-not $resolved) {
    throw "$Label not found: $FilePath"
  }

  return $resolved.ProviderPath
}

function Test-PortableExecutable {
  param([string]$FilePath)

  $stream = [System.IO.File]::Open($FilePath, [System.IO.FileMode]::Open, [System.IO.FileAccess]::Read, [System.IO.FileShare]::ReadWrite)
  try {
    if ($stream.Length -lt 2) {
      return $false
    }

    $buffer = New-Object byte[] 2
    [void]$stream.Read($buffer, 0, 2)
    return $buffer[0] -eq 0x4d -and $buffer[1] -eq 0x5a
  }
  finally {
    $stream.Dispose()
  }
}

function Test-AuthenticodeSignature {
  param([string]$FilePath)

  & $script:SignTool verify /pa /all /q $FilePath *> $null
  return $LASTEXITCODE -eq 0
}

function Invoke-SignableFile {
  param(
    [string]$FilePath,
    [switch]$OnlyVerify
  )

  $extension = [System.IO.Path]::GetExtension($FilePath).ToLowerInvariant()
  if ($SignableExtensions -notcontains $extension) {
    return
  }

  $script:Seen += 1

  if (-not (Test-PortableExecutable -FilePath $FilePath)) {
    $script:SkippedNonPe += 1
    Write-Host "Skipping non-PE file: $FilePath"
    return
  }

  if (Test-AuthenticodeSignature -FilePath $FilePath) {
    $script:Verified += 1
    Write-Host "Already signed: $FilePath"
    return
  }

  if ($OnlyVerify) {
    throw "Unsigned or invalid Authenticode signature: $FilePath"
  }

  Write-Host "Signing: $FilePath"
  & $script:SignTool sign /v /debug /fd SHA256 /tr $TimestampUrl /td SHA256 /dlib $script:Dlib /dmdf $script:Metadata /d "Strata" /du "https://github.com/francemazzi/strata" $FilePath
  if ($LASTEXITCODE -ne 0) {
    throw "SignTool failed for: $FilePath"
  }

  if (-not (Test-AuthenticodeSignature -FilePath $FilePath)) {
    throw "Signed file failed Authenticode verification: $FilePath"
  }

  $script:Signed += 1
}

function Get-SignableFiles {
  param(
    [string]$RootPath,
    [switch]$Recursive
  )

  $resolved = Resolve-Path -LiteralPath $RootPath -ErrorAction SilentlyContinue
  if (-not $resolved) {
    throw "Path not found: $RootPath"
  }

  $item = Get-Item -LiteralPath $resolved.ProviderPath
  if (-not $item.PSIsContainer) {
    if ($SignableExtensions -contains $item.Extension.ToLowerInvariant()) {
      return @($item)
    }
    return @()
  }

  $items = if ($Recursive) {
    Get-ChildItem -LiteralPath $item.FullName -Recurse -File
  } else {
    Get-ChildItem -LiteralPath $item.FullName -File
  }

  return @($items | Where-Object { $SignableExtensions -contains $_.Extension.ToLowerInvariant() })
}

function Invoke-SignablePath {
  param(
    [string]$RootPath,
    [switch]$Recursive,
    [switch]$OnlyVerify
  )

  $files = Get-SignableFiles -RootPath $RootPath -Recursive:$Recursive
  foreach ($file in $files) {
    Invoke-SignableFile -FilePath $file.FullName -OnlyVerify:$OnlyVerify
  }
}

function Invoke-ZipVerification {
  param([string]$ZipPath)

  $tempRoot = Join-Path ([System.IO.Path]::GetTempPath()) ("strata-verify-" + [guid]::NewGuid().ToString("N"))
  New-Item -ItemType Directory -Force -Path $tempRoot | Out-Null

  try {
    Write-Host "Verifying signed contents in ZIP: $ZipPath"
    Expand-Archive -LiteralPath $ZipPath -DestinationPath $tempRoot -Force
    Invoke-SignablePath -RootPath $tempRoot -Recursive -OnlyVerify
  }
  finally {
    Remove-Item -LiteralPath $tempRoot -Recurse -Force -ErrorAction SilentlyContinue
  }
}

function Get-PackageFileList {
  $files = @()

  if ($PackageFiles) {
    $files += $PackageFiles
  }

  if (-not [string]::IsNullOrWhiteSpace($PackageManifest)) {
    if (-not (Test-Path -LiteralPath $PackageManifest)) {
      throw "Package manifest not found: $PackageManifest"
    }
    $files += Get-Content -LiteralPath $PackageManifest | Where-Object { -not [string]::IsNullOrWhiteSpace($_) }
  }

  return @($files)
}

$script:SignTool = Resolve-RequiredFile -FilePath $SignToolPath -Label "SignTool"
$script:Dlib = Resolve-RequiredFile -FilePath $DlibPath -Label "Azure Artifact Signing dlib"
$script:Metadata = Resolve-RequiredFile -FilePath $MetadataPath -Label "Azure Artifact Signing metadata"

if ($Mode -eq "staging" -or $Mode -eq "verify") {
  if ([string]::IsNullOrWhiteSpace($Path)) {
    throw "Mode '$Mode' requires -Path."
  }

  Invoke-SignablePath -RootPath $Path -Recursive:$Recurse -OnlyVerify:($VerifyOnly -or $Mode -eq "verify")
} elseif ($Mode -eq "package") {
  $files = Get-PackageFileList
  if (($files | Measure-Object).Count -eq 0) {
    throw "Mode 'package' requires -PackageFiles or -PackageManifest."
  }

  foreach ($file in $files) {
    $resolved = Resolve-Path -LiteralPath $file -ErrorAction SilentlyContinue
    if (-not $resolved) {
      throw "Package file not found: $file"
    }

    $fullName = $resolved.ProviderPath
    $extension = [System.IO.Path]::GetExtension($fullName).ToLowerInvariant()

    if ($extension -eq ".zip") {
      Invoke-ZipVerification -ZipPath $fullName
    } elseif ($extension -eq ".exe") {
      Invoke-SignableFile -FilePath $fullName -OnlyVerify:$VerifyOnly
    } else {
      Write-Host "Skipping package file that does not need Authenticode signing: $fullName"
    }
  }
}

if ($script:Seen -eq 0) {
  throw "No Windows PE files were found for mode '$Mode'."
}

Write-Host "Windows signing summary: seen=$script:Seen signed=$script:Signed alreadySigned=$script:Verified skippedNonPe=$script:SkippedNonPe mode=$Mode verifyOnly=$VerifyOnly"
