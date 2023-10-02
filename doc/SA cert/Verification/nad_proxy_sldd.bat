@echo off
setlocal

set comment=goto endcomment


::%comment%

scp -P 43149 worker@tai2-tran-bmw-icon.vbee.lge.com:/home/worker/build-icon/nad/sa525m/SA525M_apps/apps_proc/build/tmp-glibc/work/aarch64-oe-linux/nadproxy/1.0-r0/image/usr/bin/sldd_nadp D:\ICON\flash\nadproxy\.

:::endcomment

::run this on cmdline first
::adb1 root

::%comment%
adb1 shell mount -o remount, rw /



adb1 shell rm /usr/bin/sldd_nadp




adb1 push D:\ICON\flash\nadproxy\sldd_nadp /usr/bin/sldd_nadp
adb1 shell chmod +x /usr/bin/sldd_nadp




adb1 shell sync
pause

:::endcomment

:: target
:: /etc/systemd/system/multi-user.target.wants/
:: systemctl disable someipconsumer
