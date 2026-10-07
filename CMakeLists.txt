cmake_minimum_required(VERSION 3.25.1)
project(tsg-billing
        VERSION 0.4.3
        LANGUAGES CXX
        HOMEPAGE_URL "https://github.com/khromenokroman/tsg-billing")

set(CMAKE_CXX_STANDARD 20)

# fmt и cpp-httplib линкуются статически: в Debian нет .a, а soname
# libfmt/libcpp-httplib отличается между Debian 12 и 13.
include(FetchContent)
set(BUILD_SHARED_LIBS OFF)
set(FMT_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(fmt
        URL https://github.com/fmtlib/fmt/archive/refs/tags/11.1.4.tar.gz
        URL_HASH SHA256=ac366b7b4c2e9f0dde63a59b3feb5ee59b67974b14ee5dc9ea8ad78aa2c1ee1e
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# httplib в header-only режиме, без OpenSSL/zlib/brotli (используется только http-сервер)
set(HTTPLIB_COMPILE OFF CACHE BOOL "" FORCE)
set(HTTPLIB_INSTALL OFF CACHE BOOL "" FORCE)
set(HTTPLIB_USE_OPENSSL_IF_AVAILABLE OFF CACHE BOOL "" FORCE)
set(HTTPLIB_USE_ZLIB_IF_AVAILABLE OFF CACHE BOOL "" FORCE)
set(HTTPLIB_USE_BROTLI_IF_AVAILABLE OFF CACHE BOOL "" FORCE)
FetchContent_Declare(httplib
        URL https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.18.7.tar.gz
        URL_HASH SHA256=b7b1e9e4e77565a5a9bc95e761d5df3e7c0e8ca37c90fd78b1b031bc6cb90fc1
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(fmt httplib)

find_package(PkgConfig REQUIRED)
pkg_check_modules(QRENCODE REQUIRED libqrencode)

configure_file(Version.in Version @ONLY)

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/tsg-billing.cpp
)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt httplib::httplib ${QRENCODE_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${QRENCODE_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${QRENCODE_LIBRARY_DIRS})

install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES systemd/tsg-billing.service
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/systemd/system
)
install(FILES db/tsg-billing.json
        DESTINATION /var/tsg-billing/
)
install(FILES configuration/cfg.json
        DESTINATION /etc/tsg-billing/
)

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR "khromenokroman")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_CONTACT "khromenokroman")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "TSG billing")
set(CPACK_PACKAGE_HOMEPAGE_URL "${PROJECT_HOMEPAGE_URL}")

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "khromenokroman")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_SOURCE_DIR}/postinst")


include(CPack)
