if (${CMAKE_SYSTEM_NAME} MATCHES FreeBSD)
	set(PLATFORM_SRCS
		Capsicum.cc
		FileDescriptor.cc
		FreeBSD.cc
		PosixError.cc
	)
endif ()

add_library(platform-lib OBJECT
	Platform.cc
	PlatformFile.cc

	${PLATFORM_SRCS}
)
