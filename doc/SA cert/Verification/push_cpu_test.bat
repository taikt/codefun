@echo off
setlocal

set comment=goto endcomment





:::endcomment

::run this on cmdline first
::adb1 root

::%comment%
adb1 shell mount -o remount, rw /

adb1 push D:\ICON\flash\cpu_usage_async.sh /tmp/cpu_usage_async.sh
adb1 shell chmod +x /tmp/cpu_usage_async.sh

adb1 push D:\ICON\flash\cpu_usage_sync.sh /tmp/cpu_usage_sync.sh
adb1 shell chmod +x /tmp/cpu_usage_sync.sh

adb1 shell sync
pause

