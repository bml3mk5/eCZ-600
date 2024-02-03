@echo off
rem %1 = $(SolutionDir) %2 = $(ConfigurationName)
xcopy /s /c /i /q /y %1\..\src\res\common\*.* %1%2\res\
xcopy /s /c /i /q /y %1\..\locale\*.xml %1%2\locale\
xcopy /s /c /i /q /y %1\..\locale\*.mo %1%2\locale\
