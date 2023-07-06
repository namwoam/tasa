
if ! grep "BR2_TARGET_MKLINUXIMG=y" < $BR2_CONFIG > /dev/null ; then
    exit 0
fi

eval `grep ^BR2_TARGET_MKLINUXIMG_VERSION= $BR2_CONFIG`

if [ -n "$BR2_TARGET_MKLINUXIMG_VERSION" ] ; then
    PKGBDIR="$BUILD_DIR/mklinuximg-$BR2_TARGET_MKLINUXIMG_VERSION"
    for f in .files-list.before .files-list-staging.before \
        .files-list-host.before .files-list-images.before ; \
    do
	touch $PKGBDIR/$f
    done
fi

make -C $CONFIG_DIR mklinuximg-rebuild


if ! grep "BR2_TARGET_MKPROM2=y" < $BR2_CONFIG > /dev/null ; then
    exit 0
fi
eval `grep ^BR2_TARGET_MKPROM2_VERSION= $BR2_CONFIG`
if [ -n "$BR2_TARGET_MKPROM2_VERSION" ] ; then
    PKGBDIR="$BUILD_DIR/mkprom2-$BR2_TARGET_MKPROM2_VERSION"
    for f in .files-list.before .files-list-staging.before \
        .files-list-host.before .files-list-images.before ; \
    do
	touch $PKGBDIR/$f
    done
fi
make -C $CONFIG_DIR mkprom2-rebuild
