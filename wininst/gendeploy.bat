@echo off

title Создание дистрибутива Qt

mkdir ARS
copy ..\AutoRingSystemCP\AutoRingSystemCP.exe ARS
windeployqt ARS\AutoRingSystemCP.exe

pause

