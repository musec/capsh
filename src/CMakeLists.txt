include_directories(.)

add_subdirectory(platform)

add_executable(capsh
        CommandLine.cc
        Error.cc
        File.cc
        OSError.cc
        UserError.cc
        capsh.cc

        $<TARGET_OBJECTS:platform-lib>
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/file.conf
          ${CMAKE_CURRENT_SOURCE_DIR}/vi.conf
          ${CMAKE_CURRENT_SOURCE_DIR}/pwd.conf
          ${CMAKE_CURRENT_SOURCE_DIR}/clang.conf
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(capsh preopen)
install(TARGETS capsh DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

