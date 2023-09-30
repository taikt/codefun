inherit base
inherit cmake
inherit lge-product-features
inherit icon-common-remote

DESCRIPTION = "install external library released by tai2.tran@lge.com"
SECTION = "async tiger lib"
LICENSE = "LGE-Proprietary"
LIC_FILES_CHKSUM = "file://${TIGER-META}/files/lge-licenses/\
${LICENSE};md5=8dc9ef8f3bb5dda9f45cebc4d1a02ac1"

PR = "r0"

PKG_DIR_NAME = "asynctiger"
FILESPATH := "${ICON_SRC_DIR}"
ICON_MODULE_NAME = "asynctiger"

DEPENDS = "boost gtest protobuf dlt-daemon servicelayer-log"

S = "${WORKDIR}/${PN}/"


FILES_${PN} += "${libdir}"


INSANE_SKIP_{PN} += "installed-vs-shipped"

INSANE_SKIP_${PN} += " ldflags"
INHIBIT_PACKAGE_STRIP = "1"
INHIBIT_SYSROOT_STRIP = "1"
SOLIBS = ".so"
FILES_SOLIBSDEV = ""

# https://stackoverflow.com/questions/57576746/how-to-install-files-in-the-native-sysroot-with-yocto-when-doing-populate-sdk
# https://stackoverflow.com/questions/50035143/in-yocto-how-to-include-header-files-from-another-recipes
#do_install() {
#    install -d ${D}${includedir}
#    install -m 0755 ${S}/lib/include/*.h ${D}${includedir}/
#    install -d ${D}/${libdir}
#    install -m 0755 ${WORKDIR}/build/lib/libasync_tiger.so ${D}${libdir}/    
#}