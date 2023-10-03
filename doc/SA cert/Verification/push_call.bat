@echo off
setlocal

set comment=goto endcomment

::%comment%
adb1 shell mount -o remount, rw /

adb1 push D:\ICON\flash\call_async.sh /tmp/call_async.sh
adb1 shell chmod +x /tmp/call_async.sh

adb1 push D:\ICON\flash\call_sync.sh /tmp/call_sync.sh
adb1 shell chmod +x /tmp/call_sync.sh

adb1 shell sync
pause

