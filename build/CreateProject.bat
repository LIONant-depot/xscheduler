rmdir /s /q xscheduler_unit_test.vs2022
rem rmdir /s /q ..\dependencies
cmake ../ -G "Visual Studio 17 2022" -A x64 -B xscheduler_unit_test.vs2022
pause