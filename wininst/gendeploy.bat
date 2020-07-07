@echo off

title Создание дистрибутива Qt

mkdir ARS
copy ..\src\AutoRingSystem.exe ARS
windeployqt ARS\AutoRingSystem.exe

pause

