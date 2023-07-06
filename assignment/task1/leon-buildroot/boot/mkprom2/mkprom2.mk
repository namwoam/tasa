################################################################################
#
# mkprom2
#
################################################################################

MKPROM2_VERSION = $(call qstrip,$(BR2_TARGET_MKPROM2_VERSION))
MKPROM2_SOURCE = mkprom2-$(MKPROM2_VERSION).tar.gz

ifeq ($(MKPROM2_VERSION),custom)
# Handle custom MKPROM2 tarballs as specified by the configuration
MKPROM2_TARBALL = $(call qstrip,$(BR2_TARGET_MKPROM2_CUSTOM_TARBALL_LOCATION))
MKPROM2_SITE = $(patsubst %/,%,$(dir $(MKPROM2_TARBALL)))
MKPROM2_SOURCE = $(notdir $(MKPROM2_TARBALL))
else ifeq ($(BR2_TARGET_MKPROM2_CUSTOM_GIT),y)
MKPROM2_SITE = $(call qstrip,$(BR2_TARGET_MKPROM2_CUSTOM_REPO_URL))
MKPROM2_SITE_METHOD = git
else
MKPROM2_SITE = https://www.gaisler.com/anonftp/mkprom2
endif

MKPROM2_LICENSE = TODO
ifeq ($(BR2_TARGET_MKPROM2_LATEST_VERSION),y)
MKPROM2_LICENSE_FILES = TODO
endif

MKPROM2_INSTALL_TARGET = NO
MKPROM2_INSTALL_STAGING = NO
MKPROM2_INSTALL_IMAGES = NO

ifeq ($(BR2_TARGET_MKPROM2)$(BR2_TARGET_MKPROM2_LATEST_VERSION),y)
BR_NO_CHECK_HASH_FOR += $(MKPROM2_SOURCE)
endif

MKPROM2_MAKE_ENV = \
	CROSS_COMPILE=$(TARGET_CROSS)

MKPROM2_DEPENDENCIES += mklinuximg

MKPROM2_OPTS =

ifneq ($(BR2_TARGET_MKPROM2_FREQ),)
	MKPROM2_OPTS += -freq $(BR2_TARGET_MKPROM2_FREQ)
endif

# Not straight up qstrip-ing the variable, because it may contain spaces,
# but we must qstrip it when checking
ifneq ($(call qstrip,$(BR2_TARGET_MKPROM2_ARGS)),)
	# Strip surrounding quotation marks and convert \" to " inside
	#
	# TODO: Is already some call for this?
	MKPROM2_OPTS += $(subst \$\",$\",$(patsubst %$\",%,$(patsubst $\"%,%,$(BR2_TARGET_MKPROM2_ARGS))))
endif

MKPROM2_EXTRA_CFLAGS=-fno-stack-protector -static

TARGET_CROSS_NO_DASH=$(patsubst %-,%,$(TARGET_CROSS))
define MKPROM2_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MKPROM2_MAKE_ENV) $(MAKE) -C $(@D)/src \
		PREFIX=$(@D) TOOLCHAIN=$(TARGET_CROSS_NO_DASH) \
		XFLAGS="$(MKPROM2_EXTRA_CFLAGS)"
	$(TARGET_MAKE_ENV) $(MKPROM2_MAKE_ENV) \
		$(@D)/mkprom2 $(BINARIES_DIR)/image.ram \
		-o $(BINARIES_DIR)/image.prom \
		$(MKPROM2_OPTS) $(MKPROM2_EXTRA_CFLAGS)
endef

$(eval $(generic-package))
