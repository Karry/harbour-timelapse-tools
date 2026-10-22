find_program(MAKE_EXECUTABLE NAMES gmake make REQUIRED)

include(ProcessorCount)
ProcessorCount(CPU_COUNT)
if(CPU_COUNT EQUAL 0)
    set(CPU_COUNT 1)
endif()

add_custom_target(lib64link
    COMMAND mkdir -p "${CMAKE_CURRENT_BINARY_DIR}/installroot/lib"
    COMMAND ln -sf lib "${CMAKE_CURRENT_BINARY_DIR}/installroot/lib64")

# ==================================================================================================
# libltdl (libtool)

set(libltdl_LIBRARY
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libltdl.so
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libltdl.so.7
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libltdl.so.7.3.1)

ExternalProject_Add(libltdl
    URL "${CMAKE_CURRENT_SOURCE_DIR}/libtool-2.4.6.tar.gz"
    URL_HASH SHA256=e3bd4d5d3d025a36c21dd6af7ea818a2afcd4dfc1ea5a17b39d7854bcd0c06e3
    INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/installroot"
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix <INSTALL_DIR>
    BUILD_COMMAND ${MAKE_EXECUTABLE} -j${CPU_COUNT}
    INSTALL_COMMAND ${MAKE_EXECUTABLE} -j${CPU_COUNT} install
    TEST_COMMAND ""
    BUILD_BYPRODUCTS ${libltdl_LIBRARY}
    )

# ==================================================================================================
# libgd

set(libgd_LIBRARY
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libgd.so
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libgd.so.3
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libgd.so.3.0.11)

ExternalProject_Add(libgd
    URL "${CMAKE_CURRENT_SOURCE_DIR}/libgd-gd-2.3.3.tar.gz"
    URL_HASH SHA256=24429f9d0dbe0f865aaa4b1a63558242396ba9134e6cfd32ca5e486a84483350
    INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/installroot"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_PREFIX_PATH=${CMAKE_CURRENT_BINARY_DIR}/installroot
    BUILD_COMMAND ${MAKE_EXECUTABLE} -j${CPU_COUNT}
    INSTALL_COMMAND ${MAKE_EXECUTABLE} -j${CPU_COUNT} install
    TEST_COMMAND ""
    BUILD_BYPRODUCTS ${libgd_LIBRARY}
    DEPENDS lib64link
    )

# ==================================================================================================
# libjpeg

set(libjpeg_LIBRARY
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libjpeg.so
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libjpeg.so.62
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libjpeg.so.62.4.0)

ExternalProject_Add(libjpeg
    URL "${CMAKE_CURRENT_SOURCE_DIR}/libjpeg-turbo-3.2.0.tar.gz"
    URL_HASH SHA256=6f30092cef9fb839779646608f4ee14ae3cbac989c47fa05e841b0841f09878e
    INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/installroot"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_PREFIX_PATH=${CMAKE_CURRENT_BINARY_DIR}/installroot
        # Bundled zlib/libspng object libraries are also linked into libturbojpeg.so.
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    BUILD_COMMAND ${MAKE_EXECUTABLE} -j${CPU_COUNT}
    INSTALL_COMMAND ${MAKE_EXECUTABLE} -j${CPU_COUNT} install
    TEST_COMMAND ""
    BUILD_BYPRODUCTS ${libjpeg_LIBRARY}
    DEPENDS lib64link
    )

# ==================================================================================================
# libusb

set(libusb_LIBRARY
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libusb-1.0.so
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libusb-1.0.so.0
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libusb-1.0.so.0.3.0)

ExternalProject_Add(libusb
    URL "${CMAKE_CURRENT_SOURCE_DIR}/libusb-1.0.26.tar.bz2"
    URL_HASH SHA256=12ce7a61fc9854d1d2a1ffe095f7b5fac19ddba095c259e6067a46500381b5a5
    INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/installroot"
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix <INSTALL_DIR> --disable-udev
    BUILD_COMMAND ${MAKE_EXECUTABLE} -j${CPU_COUNT}
    INSTALL_COMMAND ${MAKE_EXECUTABLE} -j${CPU_COUNT} install
    TEST_COMMAND ""
    BUILD_BYPRODUCTS ${libusb_LIBRARY}
    )

# ==================================================================================================
# ImageMagick

set(ImageMagick_LIBRARY
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libMagick++-7.Q16HDRI.so
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libMagick++-7.Q16HDRI.so.5
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libMagick++-7.Q16HDRI.so.5.0.0
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libMagickCore-7.Q16HDRI.so
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libMagickCore-7.Q16HDRI.so.10
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libMagickCore-7.Q16HDRI.so.10.0.3
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libMagickWand-7.Q16HDRI.so
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libMagickWand-7.Q16HDRI.so.10
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libMagickWand-7.Q16HDRI.so.10.0.3
    )

ExternalProject_Add(ImageMagick
    URL "${CMAKE_CURRENT_SOURCE_DIR}/ImageMagick-7.1.2-34.tar.gz"
    URL_HASH SHA256=f5163a573a1c1bff16358d0a592b2fb168ad0fd3b9dc5ff57dc7820eebc5cf79
    INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/installroot"
    CONFIGURE_COMMAND cmake -E env
        "PKG_CONFIG_PATH=<INSTALL_DIR>/lib/pkgconfig"
        "CFLAGS=-I<INSTALL_DIR>/include   -DMAGICKCORE_CONFIGURE_PATH=\\\"/usr/share/harbour-timelapse-tools/etc/ImageMagick-7/\\\" -DMAGICKCORE_LIBRARY_PATH=\\\"/usr/share/harbour-timelapse-tools/lib/ImageMagick-7.1.2/\\\" -DMAGICKCORE_SHARE_PATH=\\\"/usr/share/harbour-timelapse-tools/share/ImageMagick-7/\\\" -DMAGICKCORE_DOCUMENTATION_PATH=\\\"/usr/share/harbour-timelapse-tools/share/doc/ImageMagick-7/\\\""
        "CPPFLAGS=-I<INSTALL_DIR>/include -DMAGICKCORE_CONFIGURE_PATH=\\\"/usr/share/harbour-timelapse-tools/etc/ImageMagick-7/\\\" -DMAGICKCORE_LIBRARY_PATH=\\\"/usr/share/harbour-timelapse-tools/lib/ImageMagick-7.1.2/\\\" -DMAGICKCORE_SHARE_PATH=\\\"/usr/share/harbour-timelapse-tools/share/ImageMagick-7/\\\" -DMAGICKCORE_DOCUMENTATION_PATH=\\\"/usr/share/harbour-timelapse-tools/share/doc/ImageMagick-7/\\\""
        "LDFLAGS=-L<INSTALL_DIR>/lib -Wl,-rpath=/usr/share/harbour-timelapse-tools/lib"
        <SOURCE_DIR>/configure --prefix <INSTALL_DIR> --with-sysroot=<INSTALL_DIR> --with-modules --without-freetype --disable-openmp
    BUILD_COMMAND ${MAKE_EXECUTABLE} -j${CPU_COUNT}
    INSTALL_COMMAND ${MAKE_EXECUTABLE} -j${CPU_COUNT} install
    TEST_COMMAND ""
    BUILD_BYPRODUCTS ${ImageMagick_LIBRARY}
    DEPENDS libgd libjpeg libltdl
    )

add_library(ImageMagick::ImageMagick INTERFACE IMPORTED GLOBAL)
target_include_directories(ImageMagick::ImageMagick INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/installroot/include/ImageMagick-7)
target_link_libraries(ImageMagick::ImageMagick INTERFACE "${ImageMagick_LIBRARY}")  # need the quotes to expand list
# howto pickup CFlags from pkg config file?
target_compile_options(ImageMagick::ImageMagick INTERFACE -fopenmp -DMAGICKCORE_HDRI_ENABLE=0 -DMAGICKCORE_QUANTUM_DEPTH=16)
add_dependencies(ImageMagick::ImageMagick ImageMagick)

# ==================================================================================================
# x265

set(x265_LIBRARY
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libx265.so
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libx265.so.217)

ExternalProject_Add(x265
    URL "${CMAKE_CURRENT_SOURCE_DIR}/x265_4.3.tar.gz"
    URL_HASH SHA256=83c53e4c8bbb8f1e33ed59e10a7d621d1d7801ca853910c3eb41f038b8ffb121
    INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/installroot"
    PATCH_COMMAND sed "s/-mcpu=native//" -i source/CMakeLists.txt
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DENABLE_PIC=ON
    SOURCE_SUBDIR "source"
    BUILD_BYPRODUCTS ${x265_LIBRARY}
    TEST_COMMAND ""
    )

# ==================================================================================================
# x264

set(x264_LIBRARY
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libx264.so
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libx264.so.165)

ExternalProject_Add(x264
    URL "${CMAKE_CURRENT_SOURCE_DIR}/x264-master20261022.tar.bz2"
    URL_HASH SHA256=b5989ad76e0022b114e42162780a28e202dd525b42eede49bb8a025b373fcab4
    INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/installroot"
    CONFIGURE_COMMAND cmake -E env
        "PKG_CONFIG_PATH=<INSTALL_DIR>/lib/pkgconfig"
        "CFLAGS=-I<INSTALL_DIR>/include"
        "CPPFLAGS=-I<INSTALL_DIR>/include"
        "LDFLAGS=-L<INSTALL_DIR>/lib -Wl,-rpath=/usr/share/harbour-timelapse-tools/lib"
        <SOURCE_DIR>/configure --prefix=<INSTALL_DIR> --enable-shared --disable-asm
    BUILD_COMMAND ${MAKE_EXECUTABLE} -j${CPU_COUNT}
    INSTALL_COMMAND ${MAKE_EXECUTABLE} -j${CPU_COUNT} install
    BUILD_BYPRODUCTS ${x264_LIBRARY}
    TEST_COMMAND ""
    )

# ==================================================================================================
# ffmpeg

set(ffmpeg_BIN
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/bin/ffmpeg)

ExternalProject_Add(ffmpeg
    URL "${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg-7.0.2.tar.xz"
    URL_HASH SHA256=8646515b638a3ad303e23af6a3587734447cb8fc0a0c064ecdb8e95c4fd8b389
    INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/installroot"
    CONFIGURE_COMMAND cmake -E env
        "PKG_CONFIG_PATH=<INSTALL_DIR>/lib/pkgconfig"
        "CFLAGS=-I<INSTALL_DIR>/include"
        "CPPFLAGS=-I<INSTALL_DIR>/include"
        "LDFLAGS=-L<INSTALL_DIR>/lib -Wl,-rpath=/usr/share/harbour-timelapse-tools/lib"
        <SOURCE_DIR>/configure --prefix=<INSTALL_DIR> --disable-x86asm --enable-gpl --enable-libx264 --enable-libx265 --disable-doc
    BUILD_COMMAND ${MAKE_EXECUTABLE} -j${CPU_COUNT}
    INSTALL_COMMAND ${MAKE_EXECUTABLE} -j${CPU_COUNT} install
    TEST_COMMAND ""
    BUILD_BYPRODUCTS ${ffmpeg_BIN}
    DEPENDS libjpeg x264 x265
    )

# ==================================================================================================
# vidstab

set(vidstab_LIBRARY
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libvidstab.so.1.4)

ExternalProject_Add(vidstab
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DUSE_OMP=OFF -DCMAKE_POLICY_VERSION_MINIMUM=3.5
    URL "${CMAKE_CURRENT_SOURCE_DIR}/vidstab"
    INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/installroot"
    BUILD_BYPRODUCTS ${vidstab_LIBRARY}
    DEPENDS lib64link)

add_library(vidstab::vidstab INTERFACE IMPORTED GLOBAL)
target_include_directories(vidstab::vidstab INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/installroot/include)
target_link_libraries(vidstab::vidstab INTERFACE "${vidstab_LIBRARY}")  # need the quotes to expand list
add_dependencies(vidstab::vidstab vidstab)

# ==================================================================================================
# v4l

set(v4lUtils_LIBRARY
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libv4l2.so.0
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libv4l2.so.0.0.0
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libv4lconvert.so.0
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libv4lconvert.so.0.0.0)

ExternalProject_Add(v4lUtils
    URL "${CMAKE_CURRENT_SOURCE_DIR}/v4l-utils-1.22.1.tar.bz2"
    URL_HASH SHA256=65c6fbe830a44ca105c443b027182c1b2c9053a91d1e72ad849dfab388b94e31
    INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/installroot"
    CONFIGURE_COMMAND cmake -E env
        "PKG_CONFIG_PATH=<INSTALL_DIR>/lib/pkgconfig"
        "CFLAGS=-I<INSTALL_DIR>/include"
        "CPPFLAGS=-I<INSTALL_DIR>/include"
        "LDFLAGS=-L<INSTALL_DIR>/lib -Wl,-rpath=/usr/share/harbour-timelapse-tools/lib"
        <SOURCE_DIR>/configure
        --prefix <INSTALL_DIR>
        --disable-v4l2-compliance-libv4l
        --disable-qv4l2 --disable-qvidcap
        --disable-v4l-utils
        --with-udevdir=<INSTALL_DIR>/udev
    BUILD_COMMAND ${MAKE_EXECUTABLE} -j${CPU_COUNT}
    INSTALL_COMMAND ${MAKE_EXECUTABLE} -j${CPU_COUNT} install
    TEST_COMMAND ""
    BUILD_BYPRODUCTS ${v4lUtils_LIBRARY}
    DEPENDS libjpeg libgd
    )

add_library(v4lUtils::v4lUtils INTERFACE IMPORTED GLOBAL)
target_include_directories(v4lUtils::v4lUtils INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/installroot/include)
target_link_libraries(v4lUtils::v4lUtils INTERFACE "${v4lUtils_LIBRARY}")  # need the quotes to expand list
add_dependencies(v4lUtils::v4lUtils v4lUtils)

# ==================================================================================================
# GPhoto2

set(GPhoto2_LIBRARY
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libgphoto2_port.so
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libgphoto2_port.so.12
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libgphoto2_port.so.12.0.0
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libgphoto2.so
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libgphoto2.so.6
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libgphoto2.so.6.2.0)

ExternalProject_Add(GPhoto2
    URL "${CMAKE_CURRENT_SOURCE_DIR}/libgphoto2-2.5.29.tar.gz"
    URL_HASH SHA256=467eaf95e5385e48786ee115802e24ece00f34ae88ac02f5dfa7c836846c6294
    INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/installroot"
    CONFIGURE_COMMAND cmake -E env
        "PKG_CONFIG_PATH=<INSTALL_DIR>/lib/pkgconfig"
        DEFAULT_CAMLIBS=/usr/share/harbour-timelapse-tools/lib/libgphoto2/2.5.29
        DEFAULT_IOLIBS=/usr/share/harbour-timelapse-tools/lib/libgphoto2_port/0.12.0
        "CFLAGS=-I<INSTALL_DIR>/include"
        "CPPFLAGS=-I<INSTALL_DIR>/include"
        "LDFLAGS=-L<INSTALL_DIR>/lib -Wl,-rpath=/usr/share/harbour-timelapse-tools/lib"
        <SOURCE_DIR>/configure --prefix <INSTALL_DIR>
    BUILD_COMMAND ${MAKE_EXECUTABLE} -j${CPU_COUNT}
    INSTALL_COMMAND ${MAKE_EXECUTABLE} -j${CPU_COUNT} install
    TEST_COMMAND ""
    BUILD_BYPRODUCTS ${GPhoto2_LIBRARY}
    DEPENDS libltdl libgd libjpeg libusb
    )

add_library(GPhoto2::GPhoto2 INTERFACE IMPORTED GLOBAL)
target_include_directories(GPhoto2::GPhoto2 INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/installroot/include)
target_link_libraries(GPhoto2::GPhoto2 INTERFACE "${GPhoto2_LIBRARY}")  # need the quotes to expand list
add_dependencies(GPhoto2::GPhoto2 GPhoto2)

# ==================================================================================================
# TimeLapse

set(TimeLapse_LIBRARY
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libtimelapse.so
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libjpeg.so.62 # workaround for v4l linker issue on armv7hl
    )

ExternalProject_Add(TimeLapse
    URL "${CMAKE_CURRENT_SOURCE_DIR}/TimeLapse"
    # CONFIGURE_COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/installroot"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_PREFIX_PATH=${CMAKE_CURRENT_BINARY_DIR}/installroot
        -DCMAKE_EXE_LINKER_FLAGS=<INSTALL_DIR>/lib/libjpeg.so.62 # workaround for v4l linker issue on armv7hl
    BUILD_BYPRODUCTS ${TimeLapse_LIBRARY}
    DEPENDS GPhoto2 v4lUtils vidstab ImageMagick
    )

# older CMake is complaining when include did not exist at configure time
execute_process(COMMAND mkdir -p "${CMAKE_CURRENT_BINARY_DIR}/installroot/include/ImageMagick-7")

add_library(TimeLapse::TimeLapse INTERFACE IMPORTED GLOBAL)
target_include_directories(TimeLapse::TimeLapse INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/installroot/include)
target_link_libraries(TimeLapse::TimeLapse INTERFACE
    "${TimeLapse_LIBRARY}"
    Qt5::Core
    Qt5::Multimedia
    GPhoto2::GPhoto2
    v4lUtils::v4lUtils
    vidstab::vidstab
    ImageMagick::ImageMagick)  # need the quotes to expand list
add_dependencies(TimeLapse::TimeLapse TimeLapse)

# ==================================================================================================

set(DEP_LIBS
    ${x264_LIBRARY}
    ${x265_LIBRARY}
    ${vidstab_LIBRARY}
    ${ImageMagick_LIBRARY}
    ${v4lUtils_LIBRARY}
    ${GPhoto2_LIBRARY}
    ${TimeLapse_LIBRARY}
    ${libltdl_LIBRARY}
    ${libgd_LIBRARY}
    ${libjpeg_LIBRARY}
    ${libusb_LIBRARY}
    PARENT_SCOPE)

find_program(TAR bsdtar REQUIRED)

add_custom_target(tarFfmpeg ALL
    # unpacker supports just old posix tar format
    # for POSIX tar format we need to specify --format=ustar
    COMMAND ${TAR} --format=ustar -cf ffmpeg.tar ffmpeg
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/installroot/bin
    BYPRODUCTS "${ffmpeg_BIN}.tar"
    DEPENDS ffmpeg)

set(ffmpeg_TAR
    "${ffmpeg_BIN}.tar"
    PARENT_SCOPE)
