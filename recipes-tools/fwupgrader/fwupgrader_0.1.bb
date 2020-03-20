#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#

SUMMARY = "Simple helloworld application"
SECTION = "examples"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://main.c \
           file://Driver/Bus/types.h \
           file://Driver/Bus/SPI.c \
           file://Driver/Bus/SPI.h \
           file://card_slot.c \
           file://card_slot.h \
           file://dvCard.c \
           file://dvCard.h \
           file://utilHostAPI.c \
           file://utilHostAPI.h \
           file://utilHexToBin.c \
           file://utilHexToBinAPI.h \
           file://dvCardUpgrade.c \
           file://dvCardUpgrade.h \
          "

S = "${WORKDIR}"

do_compile() {
	     ${CC} -I=/Driver/Bus  ${LDFLAGS} main.c card_slot.c dvCard.c utilHostAPI.c utilHexToBin.c dvCardUpgrade.c Driver/Bus/SPI.c -o fwupgrader
}

#do_install() {
#	     install -d ${D}${bindir}
#	     install -m 0755 fwupgrader ${D}${bindir}
#}
