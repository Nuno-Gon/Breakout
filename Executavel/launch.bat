@echo off
cd "C:\Users\hugo\Desktop\SO2\Breakout\Servidor\x64\Debug"
start Servidor.exe
timeout /t 1
start Gateway.exe
timeout /t 1
start ClienteGrafico.exe