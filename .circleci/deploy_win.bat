@echo off
set CONAN_LOCAL=1

for /f %%i in ('type ..\version') do set VERS=%%i

@rem default config
conan create . edwardstock/latest -s build_type=Debug --build=missing
conan create . edwardstock/latest -s build_type=Release --build=missing

@rem dll
conan create . edwardstock/latest -s build_type=Debug --build=missing -o shared=True
conan create . edwardstock/latest -s build_type=Release --build=missing -o shared=True

@rem conan upload httb/%VERS%@edwardstock/latest --all -r=edwardstock