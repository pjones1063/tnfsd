# 1. Setup paths (Using your reference paths exactly)
$AppName    = "TnfsTrayApp"
$ReleaseDir = "..\release"
$BuildDir   = "..\build"
$WinTemp    = "$ReleaseDir\win-temp"

$CompilerPath = "C:/Qt/Tools/mingw1310_64/bin"
$QtBinPath    = "C:/Qt/6.10.2/mingw_64/bin"
$ISCC         = "C:\Program Files (x86)\Inno\ISCC.exe"
$Msys2Path    = "C:\Qt\msys2"

# 2. Hard Clean
if (Test-Path $BuildDir) { Remove-Item -Recurse -Force $BuildDir }
mkdir -p $BuildDir
mkdir -p $WinTemp

# 3. Configure and Build
Write-Host "Configuring with MinGW..." -ForegroundColor Cyan
& cmake -S .. -B $BuildDir -G "MinGW Makefiles" `
    -DCMAKE_PREFIX_PATH="$Msys2Path/ucrt64;$Msys2Path/mingw64" `
    -DCMAKE_BUILD_TYPE=Release `
    -DCMAKE_MAKE_PROGRAM="$CompilerPath/mingw32-make.exe" `
    -DCMAKE_CXX_COMPILER="$CompilerPath/g++.exe" `
    -DCMAKE_C_COMPILER="$CompilerPath/gcc.exe" `
    -DCMAKE_RC_COMPILER="$CompilerPath/windres.exe" 

Write-Host "Building $AppName..." -ForegroundColor Cyan
& cmake --build $BuildDir --config Release -j ($env:NUMBER_OF_PROCESSORS)

if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed. Check the errors above."
    exit $LASTEXITCODE
}

# 4. Stage and Deploy
Write-Host "Running windeployqt..." -ForegroundColor Cyan
Copy-Item "$BuildDir\$AppName.exe" "$WinTemp\"
& "$QtBinPath\windeployqt.exe" --dir "$WinTemp" "$WinTemp\$AppName.exe"

# 5. Create Portable ZIP
Write-Host "Creating Portable ZIP..." -ForegroundColor Cyan
Compress-Archive -Path "$WinTemp\*" -DestinationPath "$ReleaseDir\$AppName-Portable.zip" -Force

# 6. Create Installer
if (Test-Path $ISCC) {
    Write-Host "Compiling Inno Setup Installer..." -ForegroundColor Cyan
    & $ISCC "TnfsTrayApp-Installer.iss"
    Write-Host "Installer created successfully in $ReleaseDir!" -ForegroundColor Green
} else {
    Write-Host "SKIPPED: Inno Setup 6 not found at $ISCC" -ForegroundColor Yellow
    Write-Host "Download from https://jrsoftware.org/isdl.php to generate the Setup.exe" -ForegroundColor Yellow
}

Write-Host "Deployment Complete!" -ForegroundColor Green