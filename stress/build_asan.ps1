Set-Location "C:\Users\Administrator\Documents\GitHub\MisaMino\stress"
$bat = "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars32.bat"
$clLine = 'cl /nologo /EHsc /fsanitize=address /Zi /MD /Od /D_CRT_SECURE_NO_WARNINGS /I..\dllai ..\dllai\ai.cpp ..\dllai\genmove.cpp ..\dllai\tetris_gem.cpp ..\dllai\dllai.cpp ub_stress.cpp /Fe:ub_stress.exe'
$full = '"' + $bat + '" && ' + $clLine
& cmd /c $full 2>&1 | Select-Object -Last 20
Write-Output ("build exit=" + $LASTEXITCODE)
if ($LASTEXITCODE -eq 0) {
  .\ub_stress.exe 2>&1 | Select-Object -Last 40
  Write-Output ("run exit=" + $LASTEXITCODE)
}
