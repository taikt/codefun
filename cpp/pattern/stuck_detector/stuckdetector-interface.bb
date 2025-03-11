inherit autotools
inherit tiger-features
inherit debug-feature

DESCRIPTION = "stuckdetector's interface"

LICENSE = "LGE-Proprietary"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta-tiger/files/lge-licenses/\
${LICENSE};md5=8dc9ef8f3bb5dda9f45cebc4d1a02ac1"

PR = "r0"


#include stuckdetector-service.inc

SRC_SUB_PATH = ""
SRC_CFG_DIR = "interface"

require stuckdetector-service.inc


# for sldd
#INTERFACE_LIB_NAME = "libistuckdetector"   
#do_compile[postfuncs] += "make_sldd"

# for tcli
#do_unpack[postfuncs] += "make_tcli"

# if you want to start from systemd , you uncomments the following line
# if you do not want to start from systemd , you add # in front of line to do as comments.
do_unpack[postfuncs] += "make_start_list"


SRC_FILE_CONFIG_AC = "${PATH_FRAMEWORK_VARIANT}/material/service_i_configure_f.src"
do_unpack[postfuncs] += "set_config_ac_file"

#do_install_append() {
#    install -d ${D}/usr/include/services/ADADiagManagerService/pal
#}



DEPENDS += "boost"
DEPENDS += "servicelayer-core"
RDEPENDS_${PN} = "stuckdetector-service"
