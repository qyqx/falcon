##
#   Falcon MongoDB CMake file
#
#   Author(s): Stanislas Marquis <stnsls@gmail.com>
#

falcon_define_module( FALCON_MODULE mongo )

message( STATUS "Adding mongodb module" )

##
#   Find Falcon
#
list( APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake )
find_package( Falcon
    HINTS "$ENV{FALCON_ACTIVE_TREE}/*/*/cmake"
          "$ENV{FALCON_ACTIVE_TREE}/*/cmake"
          "$ENV{FALCON_BIN_PATH}/cmake"
    REQUIRED )

include_directories( ${Falcon_INCLUDE_DIRS} )

##
#   Third-party pure C driver (mongoc)
#
add_subdirectory( src )

if ( MSVC )
    add_definitions( -DMONGO_USE__INT64 )
else()
    #add_definitions( -DMONGO_HAVE_STDINT )
    add_definitions( -DMONGO_USE_LONG_LONG_INT )
endif()

##
#   Target
#
add_library( ${FALCON_MODULE} MODULE
    mongodb_fm.cpp
    mongodb_ext.cpp
    mongodb_mod.cpp
    mongodb_srv.cpp
    mongodb_st.cpp )

add_dependencies( ${FALCON_MODULE} mongoc )

##
#   Linking
#
if ( WIN32 )
    target_link_libraries( ${FALCON_MODULE} falcon_engine mongoc wsock32 )
else()
    target_link_libraries( ${FALCON_MODULE} falcon_engine mongoc )
endif()

##
#   Install
#
falcon_install_module( ${FALCON_MODULE} )