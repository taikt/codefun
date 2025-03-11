@echo off
setlocal

set comment=goto endcomment

::%comment%
adb1 shell mount -o remount, rw /


adb1 pull /tmp/test_cpu.txt D:\ICON\flash\cpu_test\test_cpu.txt
adb1 pull /tmp/test_cpu_async.txt D:\ICON\flash\cpu_test\test_cpu_async.txt


adb1 shell sync
pause

