@echo off
setlocal

set comment=goto endcomment


::%comment%
scp -P 43149 worker@tai2-tran-bmw-icon.vbee.lge.com:/home/worker/build-icon/nad/sa525m/SA525M_apps/apps_proc/build/tmp-glibc/work/aarch64-oe-linux/asynctiger/1.0-r0/image/usr/lib/libasync_tiger.so D:\ICON\flash\async_tiger\.
scp -P 43149 worker@tai2-tran-bmw-icon.vbee.lge.com:/home/worker/build-icon/nad/sa525m/SA525M_apps/apps_proc/build/tmp-glibc/work/aarch64-oe-linux/asynctiger/1.0-r0/image/usr/include/* D:\ICON\flash\async_tiger\include\.
::scp -P 55900 worker@hoachdang-bmw-icon.vbee.lge.com:/home/worker/build-icon/nad/sa525m/SA525M_apps/apps_proc/build/tmp-glibc/work/aarch64-oe-linux/asynctestapp/1.0-r0/image/usr/bin/asynctest_app D:\ICON\flash\async_tiger\.


:::endcomment

::run this on cmdline first
::adb1 root

::%comment%
adb1 shell mount -o remount, rw /
adb1 push D:\ICON\flash\async_tiger\libasync_tiger.so /usr/lib/libasync_tiger.so
adb1 shell chmod +x /usr/lib/libasync_tiger.so

:: https://stackoverflow.com/questions/12901240/adb-push-multiple-files-with-the-same-extension-with-a-single-command
adb1 push D:\ICON\flash\async_tiger\include /usr/include/

::adb1 push D:\ICON\flash\async_tiger\asynctest_app /usr/bin/asynctest_app
:: adb1 shell chmod +x /usr/bin/asynctest_app





adb1 shell sync
pause

:::endcomment

:: target
:: /etc/systemd/system/multi-user.target.wants/
:: systemctl disable someipconsumer
