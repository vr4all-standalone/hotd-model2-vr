param([string]$RomDirectory=(Join-Path $PSScriptRoot 'roms'))
$ErrorActionPreference='Stop'
$taskRoot=$PSScriptRoot
if(!(Test-Path -LiteralPath (Join-Path $taskRoot 'hotd_model2.exe'))){$taskRoot=Split-Path -Parent $PSScriptRoot}
$taskRoms=[IO.Path]::GetFullPath($RomDirectory)
if(!(Test-Path -LiteralPath (Join-Path $taskRoms 'hotdo.zip')) -and !(Test-Path -LiteralPath (Join-Path $taskRoms 'hotd.zip'))){throw 'Supply the original hotdo revision as hotdo.zip (or a verified original-revision legacy hotd.zip)'}
foreach($taskBinary in @('hotd_model2.exe','hotd_model2_xr.dll','openxr_loader.dll')){if(!(Test-Path -LiteralPath (Join-Path $taskRoot $taskBinary))){throw "Missing complete PCVR component: $taskBinary"}}
Push-Location -LiteralPath $taskRoot
try{
 & (Join-Path $taskRoot 'hotd_model2.exe') -rompath $taskRoms -verifyroms hotdo
 if($LASTEXITCODE){throw 'MAME rejected the original HOTD revision; Revision A is a distinct set'}
 $taskData=Join-Path $taskRoot 'userdata'
 foreach($taskFolder in @('nvram','cfg','diff','state','snap')){New-Item -ItemType Directory -Path (Join-Path $taskData $taskFolder) -Force | Out-Null}
 $env:HOTD_MODEL2_VR='1'
 $env:HOTD_MODEL2_XR_LIBRARY=Join-Path $taskRoot 'hotd_model2_xr.dll'
 $env:HOTD_MODEL2_ASSETS=Join-Path $taskRoot 'assets'
 $env:HOTD_MODEL2_SETTINGS=Join-Path $taskData 'settings.ini'
 $env:HOTD_MODEL2_XR_LOG=Join-Path $taskData 'hotd-model2-xr.log'
 $taskArgs=@('hotdo','-rompath',$taskRoms,'-video','none','-sound','auto','-skip_gameinfo','-noreadconfig')
 foreach($taskPair in @(@('nvram_directory','nvram'),@('cfg_directory','cfg'),@('diff_directory','diff'),@('state_directory','state'),@('snapshot_directory','snap'))){
  $taskArgs += "-$($taskPair[0])"
  $taskArgs += (Join-Path $taskData $taskPair[1])
 }
 & (Join-Path $taskRoot 'hotd_model2.exe') @taskArgs
 if($LASTEXITCODE){throw "HOTD PCVR exited with code $LASTEXITCODE"}
}finally{Pop-Location}
