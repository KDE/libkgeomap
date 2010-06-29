prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=${BIN_INSTALL_DIR}
libdir=${LIB_INSTALL_DIR}
includedir=${INCLUDE_INSTALL_DIR}

Name: libkface
Description: A world map library. This library is used by digiKam and kipi-plugins.
URL: http://www.digikam.org
Requires:
Version: ${KMAP_LIB_VERSION_STRING}
Libs: -L${LIB_INSTALL_DIR} -lkmap
Cflags: -I${INCLUDE_INSTALL_DIR}
