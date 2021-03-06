PROJECT(PA11)
CMAKE_MINIMUM_REQUIRED(VERSION 3.8)
SET(CMAKE_BUILD_TYPE Debug)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/CMakeModules")
SET(THREADS_PREFER_PTHREAD_FLAG ON)
FIND_PACKAGE(Threads REQUIRED)
FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(SDL2 REQUIRED)
FIND_PACKAGE(GLEW REQUIRED)
FIND_PACKAGE(GLM REQUIRED)
FIND_PACKAGE(Assimp REQUIRED)
FIND_PACKAGE(Bullet REQUIRED)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX17_FLAGS}")
SET(TARGET_LIBRARIES "${OPENGL_LIBRARY} ${SDL2_LIBRARY}")

IF(UNIX)
	ADD_DEFINITIONS(-DUNIX)
ENDIF(UNIX)

IF(NOT APPLE)
	IF(GLEW_FOUND)
			INCLUDE_DIRECTORIES(${GLEW_INCLUDE_DIRS})
			LINK_LIBRARIES(${GLEW_LIBRARIES})
	ENDIF(GLEW_FOUND)
ENDIF(NOT APPLE)

# Thirdparty
SET(THIRDPARTY_BASE "${PROJECT_SOURCE_DIR}/thirdparty/")

set(FASTNOISE2_NOISETOOL OFF CACHE BOOL "Build Noise Tool" FORCE) #example if don't need the graph tool
add_subdirectory("${THIRDPARTY_BASE}/FastNoise2")

#V-HACD
set(VHACD_BASE_DIR "${THIRDPARTY_BASE}v-hacd/src/VHACD_Lib")
set(CMAKE_COMMON_INC "${THIRDPARTY_BASE}v-hacd.cmake")
set(NO_OPENCL true) # No OpenCL
set(NO_OPENMP true) # No OpenMP
add_subdirectory ("${VHACD_BASE_DIR}")


SET(IMGUI_BASE "${THIRDPARTY_BASE}/dearimgui")
SET(IMGUI_INCLUDE_DIRS
	"${IMGUI_BASE}"
	"${IMGUI_BASE}/backends"
)
FILE(GLOB THIRDPARTY_SOURCES
	"${IMGUI_BASE}/*.cpp"
	"${IMGUI_BASE}/backends/imgui_impl_opengl3.cpp"
	"${IMGUI_BASE}/backends/imgui_impl_sdl.cpp"
)


INCLUDE_DIRECTORIES(
	"${PROJECT_SOURCE_DIR}/include"
	"${PROJECT_SOURCE_DIR}/thirdparty"
	"${THIRDPARTY_BASE}/FastNoise2/include"
	"${THIRDPARTY_BASE}/Compute-Shaders/src"
	"${VHACD_BASE_DIR}/inc"
	"${VHACD_BASE_DIR}/public"
	${SDL2_INCLUDE_DIR}
	${GLM_INCLUDE_DIRS}
	${IMGUI_INCLUDE_DIRS}
	${assimp_INCLUDE_DIRS}
	${BULLET_INCLUDE_DIRS}
)


# Set Includes
SET(INCLUDES ${PROJECT_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${INCLUDES})

# Set sources
FILE(GLOB_RECURSE SOURCES "src/*.cpp")
LIST(APPEND SOURCES ${THIRDPARTY_SOURCES})
ADD_EXECUTABLE(${PROJECT_NAME} ${SOURCES})

add_custom_target("${PROJECT_NAME}_SUCCESSFUL" ALL
								  DEPENDS ${PROJECT_NAME}
									COMMAND ${CMAKE_COMMAND} -E echo ""
									COMMAND ${CMAKE_COMMAND} -E echo "====================="
									COMMAND ${CMAKE_COMMAND} -E echo "	Compile complete!"
									COMMAND ${CMAKE_COMMAND} -E echo "====================="
									COMMAND ${CMAKE_COMMAND} -E echo ""
									COMMAND ${CMAKE_COMMAND} -E echo "Binaries location: ${CMAKE_CURRENT_BINARY_DIR}"
									COMMAND ${CMAKE_COMMAND} -E echo ""
									COMMAND ${CMAKE_COMMAND} -E echo "To run the app:"
									COMMAND ${CMAKE_COMMAND} -E echo "./PA11 -c config.json"
									COMMAND ${CMAKE_COMMAND} -E echo ""
								 )

target_link_libraries(${PROJECT_NAME} FastNoise vhacd)
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARY} ${SDL2_LIBRARY} ${CMAKE_DL_LIBS} ${assimp_LIBRARIES} ${BULLET_LIBRARIES} Threads::Threads)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
