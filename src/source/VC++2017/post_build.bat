@echo off
rem %1 = $(SolutionDir) %2 = $(ConfigurationName)
xcopy /s /c /i /q /y %1\..\src\res\common\*.* %1%2\res\
rem xcopy /s /c /i /q /y %1\..\locale\*.* %1%2\locale\
