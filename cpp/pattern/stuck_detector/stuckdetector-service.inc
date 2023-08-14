PKG_DIR_NAME = "stuckdetector"

FILESPATH := "${TIGER-WORKSPACE}/../gm/services:"
#FILESPATH := "${PATH_GM_SERVICE}:"


include ../../tg_common.inc

SRC_URI = "file://${PKG_DIR_NAME}/${SRC_SUB_PATH}"

S = "${@oe.utils.conditional('TIGER_CORE_SRC', 'git', '${S_GIT}', '${S_FILE}', d)}"
