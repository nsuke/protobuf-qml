:: source this to setup envvars needed to build and run artifacts
@echo off

if "%1"=="" (
    echo Usage: setup_env.bat [Debug/Relase]
    goto :eof
)

set CUR=%~dp0
set DEPS_DIR=%~dp0..\build\deps\%1\

set PATH=%DEPS_DIR%bin;%DEPS_DIR%lib;%PATH%
set INCLUDE=%DEPS_DIR%include;%INCLUDE%
set LIBPATH=%DEPS_DIR%lib;%LIBPATH%
set LIB=%DEPS_DIR%lib;%LIB%
set PYTHONPATH=%PYTHONPATH%:"%DEPS_DIR%pylib"

:eof
