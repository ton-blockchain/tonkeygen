# This file is part of TON Key Generator,
# a desktop application for the TON Blockchain project.
#
# For license and copyright information please follow this link:
# https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL

add_executable(Keygen WIN32 MACOSX_BUNDLE)
init_target(Keygen)

if (${CMAKE_GENERATOR} MATCHES "(Visual Studio|Xcode)")
    set(output_folder ${CMAKE_BINARY_DIR})
elseif (DESKTOP_APP_SPECIAL_TARGET STREQUAL "")
    set(output_folder ${CMAKE_BINARY_DIR}/bin)
else()
    set(output_folder ${CMAKE_BINARY_DIR}/$<IF:$<CONFIG:Debug>,Debug,Release>)
endif()
set(update_packer_output_folder ${output_folder})

add_subdirectory(lib_rpl)
add_subdirectory(lib_crl)
add_subdirectory(lib_base)
add_subdirectory(lib_ui)
add_subdirectory(lib_tl)
add_subdirectory(lib_storage)
add_subdirectory(lib_lottie)
add_subdirectory(lib_ton)
add_subdirectory(codegen)

include(CheckCXXSourceCompiles)
include(lib_ui/cmake/generate_styles.cmake)

get_filename_component(src_loc SourceFiles REALPATH)
get_filename_component(res_loc Resources REALPATH)

set(style_files
    ui/keygen.style
)

set(dependent_style_files
    ${submodules_loc}/lib_ui/ui/colors.palette
    ${submodules_loc}/lib_ui/ui/basic.style
    ${submodules_loc}/lib_ui/ui/layers/layers.style
    ${submodules_loc}/lib_ui/ui/widgets/widgets.style
)

generate_styles(Keygen ${src_loc} "${style_files}" "${dependent_style_files}")

set_target_properties(Keygen PROPERTIES AUTORCC ON)

target_link_libraries(Keygen
PRIVATE
    desktop-app::lib_crl
    desktop-app::lib_ui
    desktop-app::lib_tl
    desktop-app::lib_ton
    desktop-app::lib_storage
    desktop-app::lib_lottie
    desktop-app::lib_base
    desktop-app::lib_base_crash_report_writer
    desktop-app::external_rlottie
    desktop-app::external_qt
    desktop-app::external_crash_reports
    desktop-app::external_auto_updates
    desktop-app::external_openssl
)

if (DESKTOP_APP_USE_PACKAGED)
    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    find_package(Threads)

    target_link_libraries(Keygen
    PRIVATE
        ${CMAKE_DL_LIBS}
        Threads::Threads
    )
endif()

target_precompile_headers(Keygen PRIVATE ${src_loc}/core/pch.h)
nice_target_sources(Keygen ${src_loc}
PRIVATE
    ${style_files}

    core/base_integration.cpp
    core/base_integration.h
    core/launcher.cpp
    core/launcher.h
    core/main.cpp
    core/qt_static_plugins.cpp
    core/sandbox.cpp
    core/sandbox.h
    core/ui_integration.cpp
    core/ui_integration.h
    keygen/application.cpp
    keygen/application.h
    keygen/phrases.cpp
    keygen/phrases.h
    keygen/steps/check.cpp
    keygen/steps/check.h
    keygen/steps/created.cpp
    keygen/steps/created.h
    keygen/steps/done.cpp
    keygen/steps/done.h
    keygen/steps/intro.cpp
    keygen/steps/intro.h
    keygen/steps/manager.cpp
    keygen/steps/manager.h
    keygen/steps/random_seed.cpp
    keygen/steps/random_seed.h
    keygen/steps/step.cpp
    keygen/steps/step.h
    keygen/steps/view.cpp
    keygen/steps/view.h
    keygen/tr.h
    ui/lottie_widget.cpp
    ui/lottie_widget.h
    ui/message_box.cpp
    ui/message_box.h
    ui/public_key_label.cpp
    ui/public_key_label.h
    ui/word_suggestions.cpp
    ui/word_suggestions.h
)

if (DESKTOP_APP_SPECIAL_TARGET)
    target_compile_definitions(Keygen PRIVATE KEYGEN_OFFICIAL_BUILD)
endif()

if (DESKTOP_APP_USE_PACKAGED)
    message(FATAL_ERROR "Packaged build is not supported yet.")
    nice_target_sources(Keygen ${src_loc} PRIVATE core/qt_functions.cpp)
else()
    nice_target_sources(Keygen ${src_loc} PRIVATE core/qt_static_plugins.cpp)
endif()

nice_target_sources(Keygen ${res_loc}
PRIVATE
    qrc/keygen/keygen.qrc
    win/Keygen.rc
    win/Keygen.manifest
)

if (WIN32)
elseif (APPLE)
    set(icons_path ${res_loc}/art/Images.xcassets)
    set_target_properties(Keygen PROPERTIES RESOURCE ${icons_path})
    target_sources(Keygen PRIVATE ${icons_path})
elseif (LINUX)
    #if (NOT KEYGEN_DISABLE_GTK_INTEGRATION)
        find_package(PkgConfig REQUIRED)

        pkg_search_module(GTK REQUIRED gtk+-2.0 gtk+-3.0)
        target_include_directories(Keygen PRIVATE ${GTK_INCLUDE_DIRS})
        target_compile_options(Keygen PRIVATE -Wno-register)

        if (DESKTOP_APP_USE_PACKAGED)
            find_library(X11_LIBRARY X11)
            target_link_libraries(Keygen PRIVATE ${X11_LIBRARY})
        endif()
    #endif()
endif()

if (build_macstore)
    message(FATAL_ERROR "Mac App Store build is not supported.")
else()
    set(bundle_identifier "org.ton.TonKeyGenerator$<$<CONFIG:Debug>:Debug>")
    set(bundle_entitlements "Keygen.entitlements")
    if (LINUX AND DESKTOP_APP_USE_PACKAGED)
        set(output_name "keygen-desktop")
    else()
        set(output_name "Keygen")
    endif()
endif()

set_target_properties(Keygen PROPERTIES
    OUTPUT_NAME ${output_name}
    MACOSX_BUNDLE_GUI_IDENTIFIER ${bundle_identifier}
    MACOSX_BUNDLE_INFO_PLIST ${res_loc}/mac/Keygen.plist
    XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${res_loc}/mac/${bundle_entitlements}"
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${bundle_identifier}
    XCODE_ATTRIBUTE_CURRENT_PROJECT_VERSION ${desktop_app_version_string}
    XCODE_ATTRIBUTE_PRODUCT_NAME ${output_name}
    XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT $<$<NOT:$<CONFIG:Debug>>:dwarf-with-dsym>
    XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME AppIcon
    XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES
    XCODE_ATTRIBUTE_COMBINE_HIDPI_IMAGES YES
    XCODE_ATTRIBUTE_COPY_PHASE_STRIP NO
    XCODE_ATTRIBUTE_ALWAYS_SEARCH_USER_PATHS NO
    XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY libc++
    XCODE_ATTRIBUTE_OTHER_CODE_SIGN_FLAGS --deep
)
set(entitlement_sources
    "${res_loc}/mac/Keygen.entitlements"
)
target_sources(Keygen PRIVATE ${entitlement_sources})
source_group(TREE ${res_loc} PREFIX Resources FILES ${entitlement_sources})

target_include_directories(Keygen PRIVATE ${src_loc})

set_target_properties(Keygen PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${output_folder})
