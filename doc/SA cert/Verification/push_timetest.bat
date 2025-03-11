@echo off
setlocal

set comment=goto endcomment

::%comment%
adb1 shell mount -o remount, rw /


adb1 push D:\ICON\flash\test_blocking_time_async.sh /tmp/test_blocking_time_async.sh
adb1 shell chmod +x /tmp/test_blocking_time_async.sh

adb1 push D:\ICON\flash\test_response_time.sh /tmp/test_response_time.sh
adb1 shell chmod +x /tmp/test_response_time.sh

adb1 push D:\ICON\flash\test_response_time_async.sh /tmp/test_response_time_async.sh
adb1 shell chmod +x /tmp/test_response_time_async.sh

adb1 shell sync
pause

