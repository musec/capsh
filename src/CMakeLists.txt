add_executable(capsh
	capsh.c
	linker.c
)

target_link_libraries(capsh preopen)
