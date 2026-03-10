set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)

set(VCPKG_CMAKE_SYSTEM_NAME Darwin)
set(VCPKG_OSX_ARCHITECTURES arm64)
set(VCPKG_BUILD_TYPE release)

set(VCPKG_FIXUP_MACHO_RPATH ON)

# Needs to be aligned with .github/workflows/build-macos-qt6.yml
set(VCPKG_OSX_DEPLOYMENT_TARGET 11.0)
# See https://github.com/microsoft/vcpkg/issues/10038
set(VCPKG_C_FLAGS -mmacosx-version-min=${VCPKG_OSX_DEPLOYMENT_TARGET})
set(VCPKG_CXX_FLAGS -mmacosx-version-min=${VCPKG_OSX_DEPLOYMENT_TARGET})