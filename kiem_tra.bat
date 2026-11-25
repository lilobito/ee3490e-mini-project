@echo off
rem Kiem tra moi truong cho bai tap lon EE3490E.
rem Ket qua duoc ghi vao ket_qua_kiem_tra.txt ben canh file nay.
cd /d "%~dp0"
set "OUT=%~dp0ket_qua_kiem_tra.txt"
set "PROJ=%~dp0MSSV1_MSSV2_mini_project_20261"

if not exist "%PROJ%\task_1" mkdir "%PROJ%\task_1"
if not exist "%PROJ%\task_2" mkdir "%PROJ%\task_2"
if not exist "%PROJ%\task_3" mkdir "%PROJ%\task_3"

echo === 1. Vi tri gcc === > "%OUT%"
where gcc >> "%OUT%" 2>&1
echo. >> "%OUT%"
echo === 2. gcc --version === >> "%OUT%"
gcc --version >> "%OUT%" 2>&1
echo. >> "%OUT%"
echo === 3. Bien dich thu common.h === >> "%OUT%"
cd /d "%PROJ%\task_2"
gcc -Wall -Wextra -o check check.c >> "%OUT%" 2>&1
if exist check.exe (
  echo Bien dich thanh cong, khong co canh bao o tren. >> "%OUT%"
  echo. >> "%OUT%"
  echo === 4. Chay thu: check load_specs.csv data.csv === >> "%OUT%"
  check.exe load_specs.csv data.csv >> "%OUT%" 2>&1
  del check.exe
) else (
  echo Bien dich THAT BAI. >> "%OUT%"
)
if exist check.c del check.c
cd /d "%~dp0"
echo. >> "%OUT%"
echo === XONG === >> "%OUT%"

type "%OUT%"
echo.
echo Da ghi ket qua vao ket_qua_kiem_tra.txt. Bam phim bat ky de dong.
pause > nul
