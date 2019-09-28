@echo off
call env-vc64.bat

if not exist build mkdir build

pushd build
..\bin\lua ..\build.lua %~dp0 %*
popd

if %ERRORLEVEL% geq 1 (
	echo Done with errors.
	exit /B 2
) else (
	echo Done.
)