@echo off
setlocal

set comment=goto endcomment

::%comment%
adb1 shell mount -o remount, rw /

adb1 shell rm -rf /tmp/dispatcher_list.txt
adb1 shell rm -rf /tmp/running_list.txt
adb1 shell rm -rf /tmp/elapsed_list.txt
::adb1 shell ./tmp/call_async.sh &
timeout /t 20
adb1 pull /tmp/dispatcher_list.txt D:\ICON\flash\thread_pool\dispatcher_list.txt
adb1 pull /tmp/running_list.txt D:\ICON\flash\thread_pool\running_list.txt
adb1 pull /tmp/elapsed_list.txt D:\ICON\flash\thread_pool\elapsed_list.txt



adb1 shell sync
pause

