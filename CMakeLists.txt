cmake_minimum_required(VERSION 3.0)
project(gbUtils)

include(gbCMake/gbCommon.cmake)

set(GB_UTILS_VERSION_MAJOR 0)
set(GB_UTILS_VERSION_MINOR 0)

option(GB_UTILS_LUA_API "bind lua api?" ON)
option(GB_UTILS_STATIC_LIB "build static lib?" ON)
option(GB_UTILS_MULTI_THREADS "support multi-threads" ON)

configure_file(${PROJECT_SOURCE_DIR}/config.h.in ${PROJECT_SOURCE_DIR}/src/config.h)

set(srcs)
gb_add_class(ns src srcs)
gb_add_class(common src srcs)
gb_add_class(logger src srcs)
gb_add_class(time src srcs)
gb_add_class(string src srcs)
# gb_add_class(vector src srcs)
gb_add_class(concurrency src srcs)
gb_add_class(filesystem src srcs)
gb_add_class(callback src srcs)

if(GB_UTILS_LUA_API)
  gb_add_class(luastate src srcs)
  gb_add_class(luatable src srcs)
endif()

# find_package(lua REQUIRED)
# include_directories(SYSTEM ${lua_INCLUDE_DIRS})

# gb_add_class(luacpp_type_conversion src srcs)
# gb_add_class(luacpp src srcs)
gb_add_class(args src srcs)
gb_add_class(file src srcs)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(GB_UTILS_STATIC_LIB)
  add_library(gbUtils
    ${srcs}
    )
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGB_UTILS_SHARED_LIB=1")
  add_library(gbUtils SHARED
    ${srcs}
    )
endif()

set_target_properties(gbUtils
  PROPERTIES
  DEBUG_OUTPUT_NAME gbUtilsd
  RELEASE_OUTPUT_NAME gbUtils
  )

set(CMAKE_INSTALL_PREFIX ${GB_DEFAULT_INSTALL_DIR}/gbUtils)


install(TARGETS gbUtils ARCHIVE DESTINATION .)

set(headers)
foreach(src IN LISTS srcs)
  if(${src} MATCHES ".*\\.h") 
    set(headers ${headers} ${src})
    message("header: " ${src})
  endif()
endforeach()
set(headers ${headers} src/config.h)

install(FILES ${headers} DESTINATION .)
