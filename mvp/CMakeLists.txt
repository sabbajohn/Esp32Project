cmake_minimum_required(VERSION 3.16)
project(rf_capture_mvp C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(subghz_core STATIC
    ../core/subghz_core/src/detector.c
)
target_include_directories(subghz_core PUBLIC
    ../core/subghz_core/include
)

add_library(rf_record STATIC
    ../core/record/src/record.c
    ../core/record/src/codec.c
)
target_include_directories(rf_record PUBLIC
    ../core/record/include
    ../core/subghz_core/include
)

file(GLOB LEGACY_PROTOCOL_SRCS ../core/subghz_legacy/protocols/*.c)
file(GLOB LEGACY_BLOCK_SRCS ../core/subghz_legacy/blocks/*.c)

add_library(subghz_legacy_bridge STATIC
    ../core/subghz_legacy/registry.c
    ../core/subghz_legacy/environment.c
    ../core/subghz_legacy/subghz_keystore.c
    ../core/subghz_legacy/subghz_file_encoder_worker.c
    ../core/subghz_legacy/receiver.c
    ../core/subghz_legacy_bridge/src/bridge.c
    ../core/subghz_legacy/compat/src/furi_compat.c
    ../core/subghz_legacy/compat/src/flipper_format_stub.c
    ../core/subghz_legacy/compat/lib/toolbox/manchester_decoder.c
    ../core/subghz_legacy/compat/lib/toolbox/manchester_encoder.c
    ${LEGACY_BLOCK_SRCS}
    ${LEGACY_PROTOCOL_SRCS}
)
target_include_directories(subghz_legacy_bridge PUBLIC
    ../core/subghz_legacy
    ../core/subghz_legacy/protocols
    ../core/subghz_legacy/blocks
    ../core/subghz_legacy/compat
    ../core/subghz_legacy_bridge/include
    ../core/subghz_core/include
)

add_library(rf_platform_linux STATIC
    ../platform/linux/src/radio_hal_linux.c
    ../platform/linux/src/cc1101_hal_linux.c
    ../platform/linux/src/storage_fs.c
)
target_include_directories(rf_platform_linux PUBLIC
    ../platform/linux/include
    ../platform/common/include
    ../core/subghz_core/include
    ../include
)

add_executable(rf_firmware_cli
    ../firmware/cli/mvp_firmware_cli.c
    ../firmware/cli/profile_store.c
)
target_include_directories(rf_firmware_cli PRIVATE
    ../include
    ../core/subghz_core/include
    ../core/subghz_legacy_bridge/include
    ../core/record/include
    ../platform/common/include
)
target_link_libraries(rf_firmware_cli PRIVATE
    subghz_core
    subghz_legacy_bridge
    rf_record
    rf_platform_linux
)

add_executable(rf_client
    ../tools/client/src/rf_client.c
)
target_include_directories(rf_client PRIVATE
    ../core/record/include
    ../core/subghz_core/include
    ../core/subghz_legacy_bridge/include
)
target_link_libraries(rf_client PRIVATE rf_record subghz_legacy_bridge)
