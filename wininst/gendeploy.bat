@echo off

title Создание дистрибутива Qt

mkdir ARS
copy ..\AutoRingSystemCP\AutoRingSystem.exe ARS
windeployqt ARS\AutoRingSystem.exe

pause

