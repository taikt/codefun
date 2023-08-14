inherit autotools
inherit tiger-features
inherit debug-feature

DESCRIPTION = "stuck detector daemon"
SECTION = "stuckdetector-service"

LICENSE = "LGE-Proprietary"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta-tiger/files/lge-licenses/\
${LICENSE};md5=8dc9ef8f3bb5dda9f45cebc4d1a02ac1"

PR = "r0"

#include adadiag-service_version.inc


SRC_SUB_PATH = ""
SRC_CFG_DIR = ""

require stuckdetector-service.inc

do_unpack[postfuncs] += "report_doc_and_sample_app_code"


SRC_FILE_CONFIG_AC = "${PATH_FRAMEWORK_VARIANT}/material/services_configure_f.src"
do_unpack[postfuncs] += "set_config_ac_file"



DEPENDS += "servicelayer-interface"
DEPENDS += "stuckdetector-interface"
DEPENDS += "gtest"
DEPENDS += "boost"
