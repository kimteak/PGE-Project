# FreeImage is an image management library

add_library(PGE_FreeImage INTERFACE)

set(libFreeImage_Libs "${DEPENDENCIES_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}FreeImageLite${PGE_LIBS_DEBUG_SUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")

ExternalProject_Add(
    FreeImage_Local
    PREFIX ${CMAKE_BINARY_DIR}/external/FreeImage
    DOWNLOAD_COMMAND ""
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/_Libs/FreeImage
    CMAKE_ARGS
        "-DCMAKE_INSTALL_PREFIX=${DEPENDENCIES_INSTALL_DIR}"
        "-DDEPENDENCIES_INSTALL_DIR=${DEPENDENCIES_INSTALL_DIR}"
        "-DCMAKE_PROJECT_FreeImage_INCLUDE=${CMAKE_SOURCE_DIR}/_common/build_env.cmake"
        "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
        "-DCMAKE_INSTALL_PREFIX=${DEPENDENCIES_INSTALL_DIR}"
        "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
        "-DCMAKE_CONFIGURATION_TYPES=${CMAKE_CONFIGURATION_TYPES}"
        "-DCMAKE_POSITION_INDEPENDENT_CODE=ON"
        "-DFREEIMAGE_USE_SYSTEM_LIBPNG=ON"
        $<$<BOOL:APPLE>:-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}>
        ${ANDROID_CMAKE_FLAGS}
    BUILD_BYPRODUCTS
        "${libFreeImage_Libs}"
)

target_link_libraries(PGE_FreeImage INTERFACE
    "${libFreeImage_Libs}"
    PGE_libPNG
    PGE_ZLib
)
