@echo off
setlocal

set comment=goto endcomment





:::endcomment

::run this on cmdline first
::adb1 root

::%comment%
adb1 shell mount -o remount, rw /

adb1 push D:\ICON\flash\mem_usage.sh /tmp/mem_usage.sh
adb1 shell chmod +x /tmp/mem_usage.sh


adb1 shell sync
pause

