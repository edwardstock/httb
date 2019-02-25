# Detect Redhat or Debian platform
# The following variable will be set
# $PLATFORM		Debian	Redhat		none
# $DIST_NAME	 deb	rhel	fc	none
# $BIT_MODE		32|64

if (NOT WINDOWS)

	if (${CMAKE_SYSTEM_PROCESSOR} MATCHES i386|i586|i686)
		set(BIT_MODE "32" PARENT_SCOPE)
	else ()
		set(BIT_MODE "64" PARENT_SCOPE)
	endif ()

	if (EXISTS "/etc/debian_version")
		set(IS_DEBIAN ON)
	endif (EXISTS "/etc/debian_version")

	if (EXISTS "/etc/redhat-release")
		set(IS_REDHAT ON)
	endif (EXISTS "/etc/redhat-release")
endif (NOT WINDOWS)

# ARCH
if (WITH_ARCH)
	set(PROJECT_ARCH ${WITH_ARCH} PARENT_SCOPE)
else ()
	include(cmakes/FindLinuxPlatform.cmake)
	if (IS_DEBIAN)
		if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
			set(PROJECT_ARCH "amd64")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
		elseif (CMAKE_SYSTEM_PROCESSOR STREQUAL "unknown")
			set(PROJECT_ARCH "i386")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
		else ()
			set(PROJECT_ARCH ${CMAKE_SYSTEM_PROCESSOR})

			if (CMAKE_SYSTEM_PROCESSOR STREQUAL "amd64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
				set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
			else ()
				set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
			endif ()
		endif ()
	else ()
		set(PROJECT_ARCH ${CMAKE_SYSTEM_PROCESSOR})
		if (CMAKE_SYSTEM_PROCESSOR STREQUAL "amd64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
		else ()
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
		endif ()
	endif ()
endif ()