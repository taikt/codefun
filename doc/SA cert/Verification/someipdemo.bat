@echo off
setlocal

set comment=goto endcomment


::%comment%


scp -P 43149 worker@tai2-tran-bmw-icon.vbee.lge.com:/home/worker/build-icon/nad/sa525m/SA525M_apps/apps_proc/build/tmp-glibc/work/aarch64-oe-linux/demosomeip/1.0-r0/image/usr/bin/myClient D:\ICON\flash\someipdemo\.

scp -P 43149 worker@tai2-tran-bmw-icon.vbee.lge.com:/home/worker/build-icon/nad/sa525m/SA525M_apps/apps_proc/build/tmp-glibc/work/aarch64-oe-linux/demosomeip/1.0-r0/image/usr/bin/myService D:\ICON\flash\someipdemo\.

:::endcomment

::run this on cmdline first
::adb1 root

::%comment%
adb1 shell mount -o remount, rw /




adb1 push D:\ICON\flash\someipdemo\myClient /usr/bin/myClient
adb1 shell chmod +x /usr/bin/myClient

adb1 push D:\ICON\flash\someipdemo\myService /usr/bin/myService
adb1 shell chmod +x /usr/bin/myService


::adb1 pull /etc/vsomeip/vsomeip_std.json
::adb1 push D:\ICON\flash\vsomeip_std.json /etc/vsomeip/vsomeip_std.json


adb1 shell sync
pause

