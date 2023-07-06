################################################################################
#
# mklinuximg
#
################################################################################

MKLINUXIMG_VERSION = $(call qstrip,$(BR2_TARGET_MKLINUXIMG_VERSION))
MKLINUXIMG_COMPR = bz2
MKLINUXIMG_SOURCE = mklinuximg-$(MKLINUXIMG_VERSION).tar.$(MKLINUXIMG_COMPR)

ifeq ($(MKLINUXIMG_VERSION),custom)
# Handle custom MKLINUXIMG tarballs as specified by the configuration
MKLINUXIMG_TARBALL = $(call qstrip,$(BR2_TARGET_MKLINUXIMG_CUSTOM_TARBALL_LOCATION))
MKLINUXIMG_SITE = $(patsubst %/,%,$(dir $(MKLINUXIMG_TARBALL)))
MKLINUXIMG_SOURCE = $(notdir $(MKLINUXIMG_TARBALL))
else ifeq ($(BR2_TARGET_MKLINUXIMG_CUSTOM_GIT),y)
MKLINUXIMG_SITE = $(call qstrip,$(BR2_TARGET_MKLINUXIMG_CUSTOM_REPO_URL))
MKLINUXIMG_SITE_METHOD = git
MKLINUXIMG_COMPR = gz
else
MKLINUXIMG_SITE = https://www.gaisler.com/anonftp/linux/linux-5/kernel
endif

MKLINUXIMG_LICENSE = GPL-2.0+
ifeq ($(BR2_TARGET_MKLINUXIMG_LATEST_VERSION),y)
MKLINUXIMG_LICENSE_FILES = doc/gpl-2.0.txt
endif

MKLINUXIMG_INSTALL_TARGET = NO
MKLINUXIMG_INSTALL_STAGING = NO
MKLINUXIMG_INSTALL_IMAGES = NO

ifeq ($(BR2_TARGET_MKLINUXIMG)$(BR2_TARGET_MKLINUXIMG_LATEST_VERSION),y)
BR_NO_CHECK_HASH_FOR += $(MKLINUXIMG_SOURCE)
endif

MKLINUXIMG_MAKE_ENV = \
	CROSS_COMPILE=$(TARGET_CROSS)

MKLINUXIMG_DEPENDENCIES += linux

MKLINUXIMG_OPTS =

ifneq ($(BR2_TARGET_MKLINUXIMG_BASE),)
	MKLINUXIMG_OPTS += -base $(BR2_TARGET_MKLINUXIMG_BASE)
endif

# Not qstrip-ing the variable, because it may contain spaces,
# but we must qstrip it when checking
ifneq ($(call qstrip,$(BR2_TARGET_MKLINUXIMG_CMDLINE)),)
	MKLINUXIMG_OPTS += -cmdline $(BR2_TARGET_MKLINUXIMG_CMDLINE)
endif

MKLINUXIMG_XML = $(call qstrip,$(BR2_TARGET_MKLINUXIMG_XML))
ifneq ($(MKLINUXIMG_XML),)
	MKLINUXIMG_OPTS += -xml $(MKLINUXIMG_XML)
endif

# The toolchain wrapper handles this, so avoid mklinuximg's default -mcpu=leon3
MKLINUXIMG_OPTS += -mcpu -

# Not straight up qstrip-ing the variable, because it may contain spaces,
# but we must qstrip it when checking
ifneq ($(call qstrip,$(BR2_TARGET_MKLINUXIMG_ARGS)),)
	# Strip surrounding quotation marks and convert \" to " inside
	#
	# TODO: Is already some call for this?
	MKLINUXIMG_OPTS += $(subst \$\",$\",$(patsubst %$\",%,$(patsubst $\"%,%,$(BR2_TARGET_MKLINUXIMG_ARGS))))
endif

define MKLINUXIMG_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MKLINUXIMG_MAKE_ENV) $(MAKE) -C $(@D)
	$(TARGET_MAKE_ENV) $(MKLINUXIMG_MAKE_ENV) \
		$(@D)/mklinuximg "$(BINARIES_DIR)/vmlinux" "$(BINARIES_DIR)/image.ram" $(MKLINUXIMG_OPTS)
endef

$(eval $(generic-package))
