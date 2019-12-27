include(vcpkg_common_functions)
vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

# else Linux desktop
vcpkg_download_distfile(
    SOURCE_ARCHIVE
    URLS https://public.highfidelity.com/dependencies/v-hacd-master.zip
    SHA512 5d9bd4872ead9eb3574e4806d6c4f490353a04036fd5f571e1e44f47cb66b709e311abcd53af30bae0015a690152170aeed93209a626c28ebcfd6591f3bb036f
    FILENAME vhacd.zip
)

vcpkg_extract_source_archive_ex(
    OUT_SOURCE_PATH SOURCE_PATH
    ARCHIVE ${SOURCE_ARCHIVE}
)

vcpkg_configure_cmake(
  SOURCE_PATH ${SOURCE_PATH}
  PREFER_NINJA
)

vcpkg_install_cmake()

file(COPY ${CMAKE_CURRENT_LIST_DIR}/copyright DESTINATION ${CURRENT_PACKAGES_DIR}/share/vhacd)
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)
if (WIN32)
    file(RENAME ${CURRENT_PACKAGES_DIR}/lib/Release/VHACD_LIB.lib ${CURRENT_PACKAGES_DIR}/lib/VHACD.lib)
    file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/lib/Release)
    file(RENAME ${CURRENT_PACKAGES_DIR}/debug/lib/Debug/VHACD_LIB.lib ${CURRENT_PACKAGES_DIR}/debug/lib/VHACD.lib)
    file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/lib/Debug)
endif()