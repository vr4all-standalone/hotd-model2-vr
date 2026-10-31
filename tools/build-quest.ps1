param(
 [string]$AndroidSdk='E:/AndroidSdk',
 [string]$JavaHome='E:/.gradle/jdks/eclipse_adoptium-21-amd64-windows.2',
 [string]$WorkRoot='D:/VR4ALL-HOTD-Model2-Build/build/quest',
 [string]$EngineLibrary='D:/VR4ALL-HOTD-Model2-Build/engine/android-project/app/src/main/libs/arm64-v8a/libmain.so',
 [string]$HostLibrary='D:/VR4ALL-HOTD-Model2-Build/build/host-android/libhotd_model2_xr.so',
 [string]$SdlRoot='D:/StorageRelief/ege19/2026-10-25/vr4all-workspace-cache/carnevil-port/sdl-android',
 [string]$OpenXRLoader='D:/StorageRelief/ege19/2026-10-25/vr4all-workspace-cache/carnevil-port/openxr-android/src/loader/libopenxr_loader.so',
 [string]$PythonPath='D:/Python314/python.exe',
 [switch]$CheckOnly,
 [switch]$Offline
)
$ErrorActionPreference='Stop'
$taskProject=Split-Path -Parent $PSScriptRoot
$taskNdk=Join-Path $AndroidSdk 'ndk/29.0.14206865'
$taskNm=Join-Path $taskNdk 'toolchains/llvm/prebuilt/windows-x86_64/bin/llvm-nm.exe'
$taskSigner=Join-Path $AndroidSdk 'build-tools/36.0.0/apksigner.bat'
$taskLibraries=@{
 'libmain.so'=$EngineLibrary; 'libhotd_model2_xr.so'=$HostLibrary;
 'libSDL2.so'=(Join-Path $SdlRoot 'lib/libSDL2.so'); 'libopenxr_loader.so'=$OpenXRLoader;
 'libc++_shared.so'=(Join-Path $taskNdk 'toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/lib/aarch64-linux-android/libc++_shared.so')
}
if($CheckOnly){$taskLibraries.GetEnumerator()|ForEach-Object{[PSCustomObject]@{Library=$_.Key;Path=$_.Value;Exists=(Test-Path -LiteralPath $_.Value)}};return}
foreach($taskFile in @($PythonPath,$taskNm,$taskSigner,(Join-Path $JavaHome 'bin/javac.exe'))+$taskLibraries.Values){if(!(Test-Path -LiteralPath $taskFile -PathType Leaf)){throw "Required HOTD build dependency missing: $taskFile"}}
$taskExports=& $taskNm -D --defined-only $HostLibrary
if($LASTEXITCODE){throw 'Cannot inspect HOTD host exports'}
foreach($taskSymbol in @('hotd_model2_xr_start','hotd_model2_xr_stop','hotd_model2_xr_android_init')){if(!($taskExports -match "\b$taskSymbol$")){throw "Missing HOTD export: $taskSymbol"}}
$taskDestination=Join-Path $WorkRoot 'jniLibs/arm64-v8a'
New-Item -ItemType Directory -Path $taskDestination -Force | Out-Null
foreach($taskName in $taskLibraries.Keys){Copy-Item -LiteralPath $taskLibraries[$taskName] -Destination (Join-Path $taskDestination $taskName) -Force}
$taskKey=Join-Path $taskProject 'private/hotd-model2.keystore'
if(!(Test-Path -LiteralPath $taskKey)){
 & (Join-Path $JavaHome 'bin/keytool.exe') -genkeypair -keystore $taskKey -storepass android -keypass android -alias hotdmodel2 -keyalg RSA -keysize 3072 -validity 10000 -dname 'CN=HOTD Model 2 VR Local Development'
 if($LASTEXITCODE){throw 'Independent local HOTD signing-key creation failed'}
}
$taskOldJava,$taskOldAndroid,$taskOldTemp,$taskOldTmp=$env:JAVA_HOME,$env:ANDROID_HOME,$env:TEMP,$env:TMP
$taskTemp=Join-Path $WorkRoot 'tmp'
New-Item -ItemType Directory -Path $taskTemp -Force | Out-Null
try{
 $env:JAVA_HOME=$JavaHome;$env:ANDROID_HOME=$AndroidSdk;$env:TEMP=$taskTemp;$env:TMP=$taskTemp
 $taskArgs=@('-p',(Join-Path $taskProject 'android'),':app:assembleRelease','--project-cache-dir',(Join-Path $WorkRoot 'gradle-cache'),"-PhotdModel2NativeDir=$(Split-Path -Parent $taskDestination)",'--no-daemon','--console=plain')
 if($Offline){$taskArgs+='--offline'}
 & (Join-Path $taskProject 'android/gradlew.bat') @taskArgs
 if($LASTEXITCODE){throw 'HOTD Quest APK assembly failed'}
 $taskApk=Join-Path $taskProject 'build/android/app/outputs/apk/release/app-release.apk'
 & $PythonPath (Join-Path $PSScriptRoot 'package.py') --check-apk $taskApk --apksigner $taskSigner
 if($LASTEXITCODE){throw 'HOTD APK structural/signature verification failed'}
 $taskBadging=& (Join-Path $AndroidSdk 'build-tools/36.0.0/aapt2.exe') dump badging $taskApk
 if($LASTEXITCODE -or !($taskBadging -match "package: name='com.vrlightgunhub.hotdmodel2'")){throw 'Wrong HOTD APK identity'}
 Write-Output $taskApk
}finally{$env:JAVA_HOME=$taskOldJava;$env:ANDROID_HOME=$taskOldAndroid;$env:TEMP=$taskOldTemp;$env:TMP=$taskOldTmp}
