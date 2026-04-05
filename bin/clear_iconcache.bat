@echo off
taskkill /IM explorer.exe /F
del /A %userprofile%\AppData\Local\IconCache.db
start explorer.exe
echo on