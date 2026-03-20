param(
  [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot ".."))
)

$ErrorActionPreference = "Stop"

$root = (Resolve-Path $RepoRoot).Path
$failures = New-Object System.Collections.Generic.List[string]

$allowedTopLevel = @(
  ".git",
  ".github",
  ".gitignore",
  ".out",
  ".worktrees",
  "Arch.md",
  "assets",
  "CMakeLists.txt",
  "CMakePresets.json",
  "docs",
  "README.md",
  "scripts",
  "src",
  "vcpkg.json"
)

$forbiddenRoot = @(
  "build",
  "vcpkg",
  "designdocs",
  "build_log.txt",
  "build_error_log.txt",
  "screensaver_debug.log"
)

foreach ($name in $forbiddenRoot) {
  $path = Join-Path $root $name
  if (Test-Path $path) {
    $failures.Add("Forbidden root path present: $name")
  }
}

$rootEntries = Get-ChildItem -Path $root -Force
foreach ($entry in $rootEntries) {
  if ($allowedTopLevel -notcontains $entry.Name) {
    $failures.Add("Unexpected root path (not in SSOT allowlist): $($entry.Name)")
  }
}

$allBuildDirs = Get-ChildItem -Path $root -Directory -Recurse -Force |
  Where-Object {
    $_.Name -eq "build" -and
    -not $_.FullName.StartsWith((Join-Path $root ".out"), [System.StringComparison]::OrdinalIgnoreCase) -and
    -not $_.FullName.StartsWith((Join-Path $root ".worktrees"), [System.StringComparison]::OrdinalIgnoreCase) -and
    -not $_.FullName.StartsWith((Join-Path $root ".git"), [System.StringComparison]::OrdinalIgnoreCase)
  }

if ($allBuildDirs.Count -gt 0) {
  foreach ($d in $allBuildDirs) {
    $failures.Add("Non-SSOT build directory found: $($d.FullName)")
  }
}

if (-not (Test-Path (Join-Path $root ".out\build"))) {
  $failures.Add("Missing required SSOT build directory: .out/build")
}

$outEntries = Get-ChildItem -Path (Join-Path $root ".out") -Force -ErrorAction SilentlyContinue
$allowedOut = @("build", "logs", "artifacts", "toolchains")
foreach ($entry in $outEntries) {
  if ($allowedOut -notcontains $entry.Name) {
    $failures.Add("Unexpected .out entry: $($entry.Name)")
  }
}

if ($failures.Count -gt 0) {
  Write-Host "SSOT path verification FAILED" -ForegroundColor Red
  foreach ($f in $failures) {
    Write-Host " - $f" -ForegroundColor Red
  }
  exit 1
}

Write-Host "SSOT path verification PASSED" -ForegroundColor Green
Write-Host " - canonical build root: .out/build"
