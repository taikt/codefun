@echo off
setlocal

set comment=goto endcomment

::%comment%
adb1 shell mount -o remount, rw /


adb1 pull /tmp/time_execution_sync.txt D:\ICON\flash\time_test\time_execution_sync.txt
adb1 pull /tmp/time_execution_async.txt D:\ICON\flash\time_test\time_execution_async.txt

adb1 pull /tmp/time_blocking_async.txt D:\ICON\flash\time_test\time_blocking_async.txt

adb1 shell sync
pause

