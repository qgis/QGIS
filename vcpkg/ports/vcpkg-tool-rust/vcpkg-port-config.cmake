function(vcpkg_get_rust OUT)
    set(version "1.82.0")
    set(tool_name "cargo")
    set(tool_subdirectory "rust-${version}-x64")
    set(rustup_version "1.28.1")
    set(download_urls "https://raw.githubusercontent.com/rust-lang/rustup/refs/tags/${rustup_version}/rustup-init.sh")
    set(download_filename "rustup-${rustup_version}-x64")
    set(download_sha512 2482d1ca4b052f4452e06a43c472a2cf5430b4ff5e4bb1e80bb65608ca581c191e12cf85f07115ce75628f30b5e0ca09c9c5e74711111ed638f6d550c52ac725)

    set(tool_path "${DOWNLOADS}/tools/${tool_subdirectory}")

    find_program(CARGO NAMES "${tool_name}" PATHS "${tool_path}/bin" NO_DEFAULT_PATH)

    set(ENV{RUSTUP_HOME} "${tool_path}")
    set(ENV{CARGO_HOME} "${tool_path}")

    vcpkg_download_distfile(rustup_path
        URLS ${download_urls}
        SHA512 "${download_sha512}"
        FILENAME "${download_filename}"
    )

    if (VCPKG_TARGET_IS_WINDOWS AND NOT VCPKG_TARGET_IS_MINGW)
        vcpkg_acquire_msys(MSYS_ROOT PACKAGES)
        vcpkg_add_to_path("${MSYS_ROOT}/usr/bin")
        set(BASH "${MSYS_ROOT}/usr/bin/bash")
    else()
        set(BASH "bash")
        file(CHMOD "${rustup_path}" PERMISSIONS OWNER_EXECUTE OWNER_READ)
    endif()

    if(VCPKG_TARGET_ARCHITECTURE MATCHES "(amd|AMD|x|X)64")
        set(host_cpu_fam x86_64)
    elseif(VCPKG_TARGET_ARCHITECTURE MATCHES "(x|X)86")
        set(host_cpu_fam x86)
    elseif(VCPKG_TARGET_ARCHITECTURE MATCHES "riscv64")
        set(host_cpu_fam riscv64)
    elseif(VCPKG_TARGET_ARCHITECTURE MATCHES "^(ARM|arm)64$")
        set(host_cpu_fam aarch64)
    elseif(VCPKG_TARGET_ARCHITECTURE MATCHES "^(ARM|arm)$")
        set(host_cpu_fam arm)
    elseif(VCPKG_TARGET_ARCHITECTURE MATCHES "loongarch64")
        set(host_cpu_fam loongarch64)
    elseif(VCPKG_TARGET_ARCHITECTURE MATCHES "wasm32")
        set(host_cpu_fam wasm32)
    else()
        message(WARNING "Unsupported target architecture ${VCPKG_TARGET_ARCHITECTURE} for rustup!" )
    endif()
    if(VCPKG_TARGET_IS_WINDOWS)
        set(system_name "unknown-windows-msvc")
    elseif(VCPKG_TARGET_IS_OSX)
        set(system_name "apple-darwin")
    elseif(VCPKG_TARGET_IS_LINUX)
        set(system_name "unknown-linux-gnu")
    else()
        message(WARNING "Unsupported target for rustup!" )
    endif()

    set(RUST_TARGET "${host_cpu_fam}-${system_name}")

    file(MAKE_DIRECTORY "${tool_path}")
    message(STATUS "Running rustup ...")
    vcpkg_execute_in_download_mode(
                    COMMAND "${BASH}" -c "${rustup_path} -y --profile minimal --default-toolchain=${version} --target ${RUST_TARGET}"
                    WORKING_DIRECTORY "${tool_path}"
                    OUTPUT_FILE "${CURRENT_BUILDTREES_DIR}/rustup-${TARGET_TRIPLET}-out.log"
                    ERROR_FILE "${CURRENT_BUILDTREES_DIR}/rustup-${TARGET_TRIPLET}-err.log"
                    RESULT_VARIABLE error_code
                )
    if(error_code)
        message(FATAL_ERROR "Couldn't install rust with rustup!")
    endif()
    message(STATUS "Installing rust ... finished!")
    set(CARGO "${tool_path}/bin/cargo.exe")

    set(${OUT} "${CARGO}" PARENT_SCOPE)
endfunction()
