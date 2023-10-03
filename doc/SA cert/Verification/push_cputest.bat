@echo off
setlocal

set comment=goto endcomment

::%comment%
adb1 shell mount -o remount, rw /

adb1 push D:\ICON\flash\test_cpu_sync.sh /tmp/test_cpu_sync.sh
adb1 shell chmod +x /tmp/test_cpu_sync.sh

adb1 push D:\ICON\flash\test_cpu_async.sh /tmp/test_cpu_async.sh
adb1 shell chmod +x /tmp/test_cpu_async.sh

adb1 shell sync
pause

