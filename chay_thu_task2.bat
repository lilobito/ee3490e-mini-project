@echo off
rem Kiem tra toan bo phan cua Nguoi B:
rem   1. ba chuong trinh kiem tra cac ham trong common.c
rem   2. bien dich power_process (task2.c + common.c)
rem   3. chay power_process tren tung thu muc trong tests\task2
rem      va so ket qua voi thu muc mong_doi cua thu muc do.
rem Ket qua duoc ghi vao ket_qua_task2.txt ben canh file nay.
setlocal EnableDelayedExpansion
set "ROOT=%~dp0"
set "SRC=%ROOT%MSSV1_MSSV2_mini_project_20261\task_2"
set "TESTS=%ROOT%tests"
set "EXE=%TESTS%\task2\power_process.exe"
set "OUT=%ROOT%ket_qua_task2.txt"
set /a PASS=0
set /a FAIL=0

echo ===== 1. Cac ham trong common.c ===== > "%OUT%"
cd /d "%TESTS%"
for %%T in (test_buoi1 test_buoi2 test_buoi3) do (
  if exist %%T.exe del %%T.exe
  gcc -Wall -Wextra -I"%SRC%" -o %%T.exe %%T.c "%SRC%\common.c" >> "%OUT%" 2>&1
  if exist %%T.exe (
    for /f "delims=" %%L in ('%%T.exe ^| findstr /b /c:"Tong so"') do echo %%T: %%L >> "%OUT%"
    del %%T.exe
  ) else (
    echo %%T: BIEN DICH THAT BAI >> "%OUT%"
  )
)
if exist test.log del test.log

echo. >> "%OUT%"
echo ===== 2. Bien dich power_process ===== >> "%OUT%"
if exist "%EXE%" del "%EXE%"
gcc -Wall -Wextra -o "%EXE%" "%SRC%\task2.c" "%SRC%\common.c" >> "%OUT%" 2>&1
if not exist "%EXE%" (
  echo BIEN DICH THAT BAI, xem thong bao o tren. >> "%OUT%"
  goto done
)
echo Bien dich thanh cong. >> "%OUT%"

echo. >> "%OUT%"
echo ===== 3. Chay power_process tren tung truong hop ===== >> "%OUT%"
for /d %%C in ("%TESTS%\task2\*") do (
  cd /d "%%C"
  for %%F in (task2.log power_outlier.csv power_valid.csv load_condition.csv power_summary.csv) do (
    if exist %%F (
      attrib -R %%F
      del %%F
    )
  )
  set "ARGS="
  if exist lenh.txt set /p ARGS=<lenh.txt
  set "RO="
  if exist chi_doc.txt (
    set /p RO=<chi_doc.txt
    type nul > "!RO!"
    attrib +R "!RO!"
  )
  "%EXE%" !ARGS!
  set "BAD="
  for %%F in (mong_doi\*) do (
    fc /L "%%F" "%%~nxF" > nul 2>&1 || set "BAD=!BAD! %%~nxF"
  )
  if defined RO (
    attrib -R "!RO!"
    del "!RO!"
  )
  set "DESC="
  set /p DESC=<mo_ta.txt
  if defined BAD (
    set /a FAIL+=1
    echo SAI  %%~nxC: !DESC!  [khac o:!BAD!] >> "%OUT%"
  ) else (
    set /a PASS+=1
    echo DAT  %%~nxC: !DESC! >> "%OUT%"
  )
)

:done
echo. >> "%OUT%"
echo Tong: !PASS! truong hop DAT, !FAIL! truong hop SAI >> "%OUT%"
type "%OUT%"
echo.
echo Da ghi ket qua vao ket_qua_task2.txt. Bam phim bat ky de dong.
pause > nul
