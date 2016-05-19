@ECHO OFF
CD %~dp0
CD
start "" "http://127.0.0.1:8080/?fmt=html"
.\php.exe -S 127.0.0.1:8080
