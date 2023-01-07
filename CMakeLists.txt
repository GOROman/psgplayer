cmake_minimum_required(VERSION 3.13)
project(mxp CXX C)

add_subdirectory(portaudio)
add_subdirectory(emu2149)

set(SRC
  src/psgplayer.h
  src/psgplayer.cpp
  src/psg.h
  src/psg.cpp
)

include_directories(
    emu2149
    portaudio/include
)

add_executable(psgplayer ${SRC})
set(LIBS PortAudio emu2149 dl)

target_link_libraries(psgplayer ${LIBS})
#target_compile_features(psgplayer)

#INSTALL(TARGETS psgplayer RUNTIME DESTINATION bin)
