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
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libjpeg.so.62.3.0)

ExternalProject_Add(libjpeg
    URL "${CMAKE_CURRENT_SOURCE_DIR}/libjpeg-turbo-2.1.3.tar.gz"
    URL_HASH SHA256=467b310903832b033fe56cd37720d1b73a6a3bd0171dbf6ff0b620385f4f76d0
    INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/installroot"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_PREFIX_PATH=${CMAKE_CURRENT_BINARY_DIR}/installroot
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
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libMagick++-6.Q16.so
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libMagick++-6.Q16.so.9
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libMagick++-6.Q16.so.9.0.0
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libMagickCore-6.Q16.so
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libMagickCore-6.Q16.so.7
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libMagickCore-6.Q16.so.7.0.0
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libMagickWand-6.Q16.so
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libMagickWand-6.Q16.so.7
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libMagickWand-6.Q16.so.7.0.0)

ExternalProject_Add(ImageMagick
    URL "${CMAKE_CURRENT_SOURCE_DIR}/ImageMagick-6.9.12-50.tar.gz"
    URL_HASH SHA256=178dcf059963840e35ae034ecb9dc82c8549692c33d98ff0b6a7f3aadc9a1605
    INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/installroot"
    CONFIGURE_COMMAND cmake -E env
        "PKG_CONFIG_PATH=<INSTALL_DIR>/lib/pkgconfig"
        "CFLAGS=-I<INSTALL_DIR>/include   -DMAGICKCORE_CONFIGURE_PATH=\\\"/usr/share/harbour-timelapse-tools/etc/ImageMagick-6/\\\" -DMAGICKCORE_LIBRARY_PATH=\\\"/usr/share/harbour-timelapse-tools/lib/ImageMagick-6.9.12/\\\" -DMAGICKCORE_SHARE_PATH=\\\"/usr/share/harbour-timelapse-tools/share/ImageMagick-6/\\\" -DMAGICKCORE_DOCUMENTATION_PATH=\\\"/usr/share/harbour-timelapse-tools/share/doc/ImageMagick-6/\\\""
        "CPPFLAGS=-I<INSTALL_DIR>/include -DMAGICKCORE_CONFIGURE_PATH=\\\"/usr/share/harbour-timelapse-tools/etc/ImageMagick-6/\\\" -DMAGICKCORE_LIBRARY_PATH=\\\"/usr/share/harbour-timelapse-tools/lib/ImageMagick-6.9.12/\\\" -DMAGICKCORE_SHARE_PATH=\\\"/usr/share/harbour-timelapse-tools/share/ImageMagick-6/\\\" -DMAGICKCORE_DOCUMENTATION_PATH=\\\"/usr/share/harbour-timelapse-tools/share/doc/ImageMagick-6/\\\""
        "LDFLAGS=-L<INSTALL_DIR>/lib -Wl,-rpath=/usr/share/harbour-timelapse-tools/lib"
        <SOURCE_DIR>/configure --prefix <INSTALL_DIR> --with-sysroot=<INSTALL_DIR> --with-modules --without-freetype --disable-openmp
    BUILD_COMMAND ${MAKE_EXECUTABLE} -j${CPU_COUNT}
    INSTALL_COMMAND ${MAKE_EXECUTABLE} -j${CPU_COUNT} install
    TEST_COMMAND ""
    BUILD_BYPRODUCTS ${ImageMagick_LIBRARY}
    DEPENDS libgd libjpeg
    )

add_library(ImageMagick::ImageMagick INTERFACE IMPORTED GLOBAL)
target_include_directories(ImageMagick::ImageMagick INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/installroot/include/ImageMagick-6)
target_link_libraries(ImageMagick::ImageMagick INTERFACE "${ImageMagick_LIBRARY}")  # need the quotes to expand list
# howto pickup CFlags from pkg config file?
target_compile_options(ImageMagick::ImageMagick INTERFACE -fopenmp -DMAGICKCORE_HDRI_ENABLE=0 -DMAGICKCORE_QUANTUM_DEPTH=16)
add_dependencies(ImageMagick::ImageMagick ImageMagick)

# ==================================================================================================
# ffmpeg

set(ffmpeg_BIN
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/bin/ffmpeg)

ExternalProject_Add(ffmpeg
    URL "${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg-5.0.1.tar.gz"
    URL_HASH SHA256=840c012766ceb1a95ec92fcbe51d80024a74b1b3134c92c2ff64a6f8165e9059
    INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/installroot"
    CONFIGURE_COMMAND cmake -E env
        "PKG_CONFIG_PATH=<INSTALL_DIR>/lib/pkgconfig"
        "CFLAGS=-I<INSTALL_DIR>/include"
        "CPPFLAGS=-I<INSTALL_DIR>/include"
        "LDFLAGS=-L<INSTALL_DIR>/lib -Wl,-rpath=/usr/share/harbour-timelapse-tools/lib"
        <SOURCE_DIR>/configure --prefix=<INSTALL_DIR> --disable-x86asm --enable-gpl
    BUILD_COMMAND ${MAKE_EXECUTABLE} -j${CPU_COUNT}
    INSTALL_COMMAND ${MAKE_EXECUTABLE} -j${CPU_COUNT} install
    TEST_COMMAND ""
    BUILD_BYPRODUCTS ${ffmpeg_BIN}
    DEPENDS libjpeg
    )

# ==================================================================================================
# vidstab

set(vidstab_LIBRARY
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libvidstab.so.1.2)

ExternalProject_Add(vidstab
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DUSE_OMP=OFF
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
    ${CMAKE_CURRENT_BINARY_DIR}/installroot/lib/libtimelapse.so)

ExternalProject_Add(TimeLapse
    URL "${CMAKE_CURRENT_SOURCE_DIR}/TimeLapse"
    # CONFIGURE_COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/installroot"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_PREFIX_PATH=${CMAKE_CURRENT_BINARY_DIR}/installroot
    BUILD_BYPRODUCTS ${TimeLapse_LIBRARY}
    DEPENDS GPhoto2 v4lUtils vidstab ImageMagick
    )

# older CMake is complaining when include did not exist at configure time
execute_process(COMMAND mkdir -p "${CMAKE_CURRENT_BINARY_DIR}/installroot/include/ImageMagick-6")

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

add_custom_target(tarFfmpeg ALL
    COMMAND tar -cf ffmpeg.tar ffmpeg
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/installroot/bin
    BYPRODUCTS "${ffmpeg_BIN}.tar"
    DEPENDS ffmpeg)

set(ffmpeg_TAR
    "${ffmpeg_BIN}.tar"
    PARENT_SCOPE)