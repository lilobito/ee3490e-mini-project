@echo off
rem Bien dich va chay chuong trinh kiem tra cua buoi 1.
rem Ket qua duoc ghi vao ket_qua_buoi1.txt ben canh file nay.
set "ROOT=%~dp0"
set "SRC=%ROOT%MSSV1_MSSV2_mini_project_20261\task_2"
set "OUT=%ROOT%ket_qua_buoi1.txt"
cd /d "%ROOT%tests"

if exist test_buoi1.exe del test_buoi1.exe
if exist test.log del test.log

echo === Bien dich: gcc -Wall -Wextra test_buoi1.c common.c === > "%OUT%"
gcc -Wall -Wextra -I"%SRC%" -o test_buoi1.exe test_buoi1.c "%SRC%\common.c" >> "%OUT%" 2>&1
if not exist test_buoi1.exe (
  echo Bien dich THAT BAI, xem thong bao o tren. >> "%OUT%"
  goto done
)
echo Bien dich thanh cong. >> "%OUT%"
echo. >> "%OUT%"
test_buoi1.exe >> "%OUT%" 2>&1
echo. >> "%OUT%"
echo === Noi dung test.log do log_error ghi === >> "%OUT%"
type test.log >> "%OUT%"
del test_buoi1.exe

:done
type "%OUT%"
echo.
echo Da ghi ket qua vao ket_qua_buoi1.txt. Bam phim bat ky de dong.
pause > nul
