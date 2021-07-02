cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project(B2M CXX)

option(RTMIDI17_HEADER_ONLY "Header-only mode" OFF)
option(RTMIDI17_NO_COREMIDI "Disable CoreMidi back-end" OFF)
option(RTMIDI17_NO_WINMM "Disable WinMM back-end" OFF)
option(RTMIDI17_NO_WINUWP "Disable UWP back-end" ON)
option(RTMIDI17_NO_JACK "Disable JACK back-end" OFF)
option(RTMIDI17_NO_ALSA "Disable ALSA back-end" OFF)
option(RTMIDI17_EXAMPLES "Enable examples" ON)

include(CheckSymbolExists)
### Main library ###
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(RTMIDI17_HEADER_ONLY)
  add_library(RtMidi17 INTERFACE)
  set(_public INTERFACE)
  target_compile_definitions(RtMidi17 ${_public} RTMIDI17_HEADER_ONLY)
else()
  add_library(RtMidi17
    include/rtmidi17/detail/alsa.hpp
    include/rtmidi17/detail/coreaudio.hpp
    include/rtmidi17/detail/dummy.hpp
    include/rtmidi17/detail/jack.hpp
    include/rtmidi17/detail/midi_api.hpp
    include/rtmidi17/detail/semaphore.hpp
    include/rtmidi17/detail/winmm.hpp
    include/rtmidi17/detail/winuwp.hpp

    include/rtmidi17/message.hpp
    include/rtmidi17/reader.hpp
    include/rtmidi17/writer.hpp
    include/rtmidi17/rtmidi17.hpp

    include/rtmidi17/rtmidi17.cpp
    include/rtmidi17/reader.cpp
    include/rtmidi17/writer.cpp
  )
  set(_public PUBLIC)
endif()

target_compile_features(RtMidi17 ${_public} cxx_std_17)

find_package(Threads)
target_link_libraries(RtMidi17 ${_public} ${CMAKE_THREAD_LIBS_INIT})

target_include_directories(RtMidi17 ${_public}
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

if(APPLE)
  ## CoreMIDI support ##
  if(NOT RTMIDI17_NO_COREMIDI)
    target_compile_definitions(RtMidi17 ${_public} RTMIDI17_COREAUDIO)

    find_library(COREMIDI_LIBRARY CoreMIDI)
    find_library(COREAUDIO_LIBRARY CoreAudio)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)

    target_link_libraries(RtMidi17
      ${_public}
        ${COREFOUNDATION_LIBRARY}
        ${COREAUDIO_LIBRARY}
        ${COREMIDI_LIBRARY}
     )
  endif()
endif()

## JACK support ##
if(NOT RTMIDI17_NO_JACK)
  find_path(WEAKJACK_PATH weakjack/weak_libjack.h HINTS ${WEAKJACK_FOLDER})
  find_path(JACK_PATH jack/jack.h)
  if(WEAKJACK_PATH AND JACK_PATH)
    message(" -- RtMidi17 : Using WeakJACK")
    set(HAS_JACK 1)
    target_include_directories(RtMidi17 ${_public} $<BUILD_INTERFACE:${WEAKJACK_PATH}> $<BUILD_INTERFACE:${JACK_PATH}>)
  elseif(JACK_PATH)
    message(" -- RtMidi17 : Using linked JACK")
    find_library(JACK_LIBRARIES jack)
    if(JACK_LIBRARIES)
      set(HAS_JACK 1)
      target_link_libraries(RtMidi17 ${_public} ${JACK_LIBRARIES})
      target_include_directories(RtMidi17 ${_public} $<BUILD_INTERFACE:${JACK_PATH}>)
    endif()
  endif()

  if(HAS_JACK)
    target_compile_definitions(RtMidi17 ${_public} RTMIDI17_JACK)

    check_symbol_exists(jack_port_rename jack/jack.h HAS_JACK_PORT_RENAME)
    if(HAS_JACK_PORT_RENAME)
      target_compile_definitions(RtMidi17 ${_public} RTMIDI17_JACK_HAS_PORT_RENAME)
    endif()
  endif()
endif()

### Install  ###
if(NOT RTMIDI17_HEADER_ONLY)
  install(TARGETS RtMidi17
          EXPORT rtmidi17-exports
          ARCHIVE DESTINATION lib/static
          RUNTIME DESTINATION bin
          LIBRARY DESTINATION lib
          )
else()
    install(TARGETS RtMidi17
            EXPORT rtmidi17-exports
            )
endif()
install(EXPORT rtmidi17-exports
        DESTINATION lib/cmake/rtmidi17)
export(EXPORT rtmidi17-exports)






find_package(Torch REQUIRED)
find_library(cs CoreServices REQUIRED)
find_library(cf CoreFoundation REQUIRED)
find_library(au AudioUnit REQUIRED)
find_library(at AudioToolbox REQUIRED)
find_library(ca CoreAudio REQUIRED)

add_library(stft_nn_lib STATIC stft_nn.cpp)
target_link_libraries(stft_nn_lib "${TORCH_LIBRARIES}")
set_property(TARGET stft_nn_lib PROPERTY CXX_STANDARD 14)

add_library(midiout STATIC midiout.cpp)
target_link_libraries(midiout RtMidi17) 
set_property(TARGET midiout PROPERTY CXX_STANDARD 14)

add_executable(test test_lib.cpp)
target_link_libraries(test stft_nn_lib midiout
                      stft_nn_lib ${cs} ${cf} ${au} ${at} ${ca} 
                      "/usr/local/lib/libportaudio.a")
set_property(TARGET test PROPERTY CXX_STANDARD 14)