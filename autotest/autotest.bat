@echo off

call :test stdlibtest.c
if %errorlevel% neq 0 goto :error

call :test testint16.c
if %errorlevel% neq 0 goto :error

call :test testint32.c
if %errorlevel% neq 0 goto :error

call :test testint16mul.c
if %errorlevel% neq 0 goto :error

call :test recursiontest.c
if %errorlevel% neq 0 goto :error

call :test strcmptest.c
if %errorlevel% neq 0 goto :error

call :test strcmptest2.c
if %errorlevel% neq 0 goto :error

call :test arraytest.c
if %errorlevel% neq 0 goto :error

call :test arraytestfloat.c
if %errorlevel% neq 0 goto :error

call :test optiontest.c
if %errorlevel% neq 0 goto :error

call :test floatcmptest.c
if %errorlevel% neq 0 goto :error

call :test floatmultest.c
if %errorlevel% neq 0 goto :error

call :test staticconsttest.c
if %errorlevel% neq 0 goto :error

call :test arrayinittest.c
if %errorlevel% neq 0 goto :error

call :test array2stringinittest.c
if %errorlevel% neq 0 goto :error

call :test testint16cmp.c
if %errorlevel% neq 0 goto :error

call :test testint8cmp.c
if %errorlevel% neq 0 goto :error

call :test testint32cmp.c
if %errorlevel% neq 0 goto :error

call :test testinterval.c
if %errorlevel% neq 0 goto :error

call :test floatstringtest.c
if %errorlevel% neq 0 goto :error

call :test qsorttest.c
if %errorlevel% neq 0 goto :error

call :test loopdomtest.c
if %errorlevel% neq 0 goto :error

call :test byteindextest.c
if %errorlevel% neq 0 goto :error

call :test asmtest.c
if %errorlevel% neq 0 goto :error

call :testb bitshifttest.c
if %errorlevel% neq 0 goto :error

call :test arrparam.c
if %errorlevel% neq 0 goto :error

call :test bsstest.c
if %errorlevel% neq 0 goto :error

call :test copyintvec.c
if %errorlevel% neq 0 goto :error

call :test divmodtest.c
if %errorlevel% neq 0 goto :error

call :test enumswitch.c
if %errorlevel% neq 0 goto :error

call :test incvector.c
if %errorlevel% neq 0 goto :error

call :test structoffsettest2.c
if %errorlevel% neq 0 goto :error

call :test funcvartest.c
if %errorlevel% neq 0 goto :error

call :test structassigntest.c
if %errorlevel% neq 0 goto :error

call :test structmembertest.c
if %errorlevel% neq 0 goto :error

call :test randsumtest.c
if %errorlevel% neq 0 goto :error

exit /b 0

:error
echo Failed with error #%errorlevel%.
exit /b %errorlevel%

:test
..\release\oscar64 -e %~1
if %errorlevel% neq 0 goto :error

..\release\oscar64 -e -n %~1
if %errorlevel% neq 0 goto :error

..\release\oscar64 -e -O2 %~1
if %errorlevel% neq 0 goto :error

..\release\oscar64 -e -O2 -n %~1
if %errorlevel% neq 0 goto :error

..\release\oscar64 -e -O0 %~1
if %errorlevel% neq 0 goto :error

..\release\oscar64 -e -O0 -n %~1
if %errorlevel% neq 0 goto :error

..\release\oscar64 -e -O3 %~1
if %errorlevel% neq 0 goto :error

..\release\oscar64 -e -O3 -n %~1
if %errorlevel% neq 0 goto :error

exit /b 0

:testb
..\release\oscar64 -e %~1
if %errorlevel% neq 0 goto :error

..\release\oscar64 -e -O2 %~1
if %errorlevel% neq 0 goto :error

..\release\oscar64 -e -O0 %~1
if %errorlevel% neq 0 goto :error

..\release\oscar64 -e -O3 %~1
if %errorlevel% neq 0 goto :error

exit /b 0
