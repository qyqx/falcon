####################################################################
# The Falcon Programming language
#
# CMake configuration file for module SDL_ttf
####################################################################

FALCON_DEFINE_MODULE( SDLTTF ttf )

if(COMMAND cmake_policy)
   cmake_policy(SET CMP0003 OLD)
endif(COMMAND cmake_policy)

Find_Package ( SDL_ttf REQUIRED )

# Inclusion settings
INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${SDLTTF_INCLUDE_DIR} 
)

# Target
ADD_LIBRARY( ${SDLTTF} MODULE
   sdlttf.cpp
   sdlttf_ext.cpp
   sdlttf_mod.cpp
)

#Link
TARGET_LINK_LIBRARIES( ${SDLTTF} falcon_engine ${SDLTTF_LIBRARY} ${SDL_LIBRARY})

SET_TARGET_PROPERTIES( ${SDLTTF}
   PROPERTIES PREFIX "")
#Install
FALCON_INSTALL_MODULE2( ${SDLTTF} sdl )
