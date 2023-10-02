@echo off
setlocal

set comment=goto endcomment


::%comment%

scp -P 43149 worker@tai2-tran-bmw-icon.vbee.lge.com:/home/worker/build-icon/nad/sa525m/SA525M_apps/apps_proc/build/tmp-glibc/work/aarch64-oe-linux/config-service/1.0-r0/image/usr/bin/ConfigurationManagerService D:\ICON\flash\configmgr\.



:::endcomment

::run this on cmdline first
::adb1 root

::%comment%
adb1 shell mount -o remount, rw /


adb1 shell systemctl stop configurationManager
adb1 shell rm /usr/bin/ConfigurationManagerService


adb1 push D:\ICON\flash\configmgr\ConfigurationManagerService /usr/bin/ConfigurationManagerService
adb1 shell chmod +x /usr/bin/ConfigurationManagerService



adb1 shell systemctl restart configurationManager

adb1 shell systemctl daemon-reload



adb1 shell sync
pause

:::endcomment

:: target
:: /etc/systemd/system/multi-user.target.wants/
:: systemctl disable someipconsumer
