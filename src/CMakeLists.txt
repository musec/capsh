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


target_link_libraries(capsh preopen)
install(TARGETS capsh DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
