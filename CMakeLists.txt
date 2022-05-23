cmake_minimum_required (VERSION 3.22.0)
project (brenSDL VERSION 0.1.0)

include (CTest)
enable_testing ()

set (CMAKE_C_STANDARD 17)

option (USE_FIXED_POINT_BRENDER "" ON)

find_package (SDL2 REQUIRED)
link_libraries (${SDL2_LIBRARIES})
include_directories (${SDL2_INCLUDE_DIRS})

include_directories("${PROJECT_SOURCE_DIR}/brender/inc")
add_compile_definitions(
	$<$<COMPILE_LANG_AND_ID:C,MSVC>:__VISUALC__=1>
	$<$<COMPILE_LANG_AND_ID:C,MSVC>:__TARGET_MSC__=1>)

if (USE_FIXED_POINT_BRENDER)
	message("Using the fixed-point version of BRender")
	add_compile_definitions(BASED_FIXED=1)
	add_compile_definitions(BASED_FLOAT=0)
else ()
	message("Using the floating-point version of BRender")
	add_compile_definitions(BASED_FIXED=0)
	add_compile_definitions(BASED_FLOAT=1)
endif ()


add_executable(brenSDL 
	"${PROJECT_SOURCE_DIR}/brender/std/stddiag.c"
	"${PROJECT_SOURCE_DIR}/brender/std/stderr.c"
	"${PROJECT_SOURCE_DIR}/brender/std/stdfile.c"
	"${PROJECT_SOURCE_DIR}/brender/std/stdmem.c"
	"${PROJECT_SOURCE_DIR}/main.c")

if (USE_FIXED_POINT_BRENDER)
	target_link_libraries(brenSDL
	PUBLIC
		"${PROJECT_SOURCE_DIR}/brender/lib/brfmmxr.lib"
		"${PROJECT_SOURCE_DIR}/brender/lib/brfwmxr.lib"
		"${PROJECT_SOURCE_DIR}/brender/lib/brzbmxr.lib")
else ()
	target_link_libraries(brenSDL
	PUBLIC
		"${PROJECT_SOURCE_DIR}/brender/lib/brfmmfr.lib"
		"${PROJECT_SOURCE_DIR}/brender/lib/brfwmfr.lib"
		"${PROJECT_SOURCE_DIR}/brender/lib/brzbmfr.lib")
endif ()

file(
	COPY "${PROJECT_SOURCE_DIR}/data"
	DESTINATION "${PROJECT_BINARY_DIR}")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
