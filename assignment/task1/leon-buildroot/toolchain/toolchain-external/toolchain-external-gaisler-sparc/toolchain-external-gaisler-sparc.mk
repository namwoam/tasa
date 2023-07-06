################################################################################
#
# toolchain-external-gaisler-sparc.mk
#
################################################################################

TOOLCHAIN_EXTERNAL_GAISLER_SPARC_VERSION = $(call qstrip,$(BR2_TOOLCHAIN_EXTERNAL_GAISLER_SPARC_LATEST_VERSION))
TOOLCHAIN_EXTERNAL_GAISLER_SPARC_SITE = https://www.gaisler.com/anonftp/linux/linux-5/toolchains
TOOLCHAIN_EXTERNAL_GAISLER_SPARC_SOURCE = sparc-gaisler-$(TOOLCHAIN_EXTERNAL_GAISLER_SPARC_VERSION).tar.xz

ifeq ($(BR2_SPARC_ERRATA_WORKAROUND_UT700),y)
TOOLCHAIN_EXTERNAL_CFLAGS += -mfix-ut700
endif

$(eval $(toolchain-external-package))
