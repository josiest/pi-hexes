include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

function(install_interface_targets TARGET_NAME)
    target_include_directories(
            ${TARGET_NAME} INTERFACE
            "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/${CMAKE_INSTALL_INCLUDEDIR}>"
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

    install(TARGETS ${TARGET_NAME}
            EXPORT ${TARGET_NAME}-targets
            FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_LIBDIR})

    export(EXPORT ${TARGET_NAME}-targets
            FILE ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}-targets.cmake
            NAMESPACE ${TARGET_NAME}::)

    install(EXPORT ${TARGET_NAME}-targets
            FILE ${TARGET_NAME}-targets.cmake
            NAMESPACE ${TARGET_NAME}::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${TARGET_NAME})
endfunction()

function(install_package TARGET_NAME)
    write_basic_package_version_file(
            ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}-config-version.cmake
            VERSION ${PROJECT_VERSION}
            COMPATIBILITY AnyNewerVersion)

    configure_package_config_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/${TARGET_NAME}-config.cmake.in
            ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}-config.cmake
            INSTALL_DESTINATION ${CMAKE_INSTALL_LIBIDIR}/cmake/${TARGET_NAME})

    install(FILES
            ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}-config.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}-config-version.cmake
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${TARGET_NAME})

    export(PACKAGE ${TARGET_NAME})
endfunction()
