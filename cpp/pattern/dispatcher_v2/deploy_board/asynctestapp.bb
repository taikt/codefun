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

PKG_DIR_NAME = "asynctestapp"
FILESPATH := "${ICON_SRC_DIR}"
ICON_MODULE_NAME = "asynctestapp"

DEPENDS = "boost dlt-daemon gtest protobuf asynctiger servicelayer-log"

S = "${WORKDIR}/${PN}/"


FILES_${PN} += "${libdir}"

INSANE_SKIP_{PN} += "installed-vs-shipped"


#do_install() {
#    install -d ${D}${bindir}
#    install -m 0755 ${WORKDIR}/build/asynctest_app ${D}${bindir}/    
#}



