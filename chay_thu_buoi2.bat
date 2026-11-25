@echo off
rem Bien dich va chay chuong trinh kiem tra cua buoi 1 va buoi 2.
rem Chay ca buoi 1 de chac chan sua common.c khong lam hong ham cu.
rem Ket qua duoc ghi vao ket_qua_buoi2.txt ben canh file nay.
set "ROOT=%~dp0"
set "SRC=%ROOT%MSSV1_MSSV2_mini_project_20261\task_2"
set "OUT=%ROOT%ket_qua_buoi2.txt"
cd /d "%ROOT%tests"
if exist test.log del test.log
echo. > "%OUT%"

for %%T in (test_buoi1 test_buoi2) do (
  if exist %%T.exe del %%T.exe
  echo ================ %%T ================ >> "%OUT%"
  gcc -Wall -Wextra -I"%SRC%" -o %%T.exe %%T.c "%SRC%\common.c" >> "%OUT%" 2>&1
  if exist %%T.exe (
    echo Bien dich thanh cong. >> "%OUT%"
    %%T.exe >> "%OUT%" 2>&1
    del %%T.exe
  ) else (
    echo Bien dich THAT BAI, xem thong bao o tren. >> "%OUT%"
  )
  echo. >> "%OUT%"
)
if exist test.log del test.log

type "%OUT%"
echo.
echo Da ghi ket qua vao ket_qua_buoi2.txt. Bam phim bat ky de dong.
pause > nul
