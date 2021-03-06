cmake_minimum_required(VERSION 2.8.8)
project(sdr-wspr)
# required for original wspr code
enable_language(Fortran)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)
include(InstallHeadersWithDirectory)

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5WebKitWidgets REQUIRED)

find_package(FFTW REQUIRED)
find_package(FFTWSingle)
find_package(PortAudio REQUIRED)
find_package(RTLSDR REQUIRED)

find_package(Libsdr)
find_package(LibsdrGui)

ADD_DEFINITIONS(${Qt5Widgets_DEFINITIONS})

INCLUDE_DIRECTORIES(${Qt5Core_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${Qt5Declarative_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${Qt5Widgets_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${Qt5WebKitWidgets_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${PORTAUDIO_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src)

LINK_DIRECTORIES(${PROJECT_BINARY_DIR}/src)
set(LIBS ${FFTW_LIBRARIES} ${FFTWSingle_LIBRARIES} ${PORTAUDIO_LIBRARIES}
         ${RTLSDR_LIBRARIES} "pthread")

# If libsdr cannot be found, compile and link to libsdr shipped along
# with this sources...
IF(NOT LIBSDR_FOUND)
 INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/libsdr/src")
 INCLUDE_DIRECTORIES("${PROJECT_BINARY_DIR}/libsdr/src")
 LINK_DIRECTORIES("${PROJECT_BINARY_DIR}/libsdr/src")
 SET(LIBS ${LIBS} libsdr)
 SET(SDR_WITH_QT5 ON)
 SET(SDR_WITH_FFTW ON)
 SET(SDR_WITH_PORTAUDIO ON)
 SET(SDR_WITH_RTLSDR ON)
 CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/libsdr/src/config.hh.in
                ${CMAKE_CURRENT_BINARY_DIR}/libsdr/src/config.hh)
ELSE(NOT LIBSDR_FOUND)
 INCLUDE_DIRECTORIES(${LIBSDR_INCLUDE_DIRS})
 SET(LIBS ${LIBS} ${LIBSDR_LIBRARIES})
ENDIF(NOT LIBSDR_FOUND)

IF(NOT LIBSDR_GUI_FOUND)
 MESSAGE(STATUS "libsdr-gui not found: Build them too...")
 INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/libsdr-gui/src")
 INCLUDE_DIRECTORIES("${PROJECT_BINARY_DIR}/libsdr-gui/src")
 LINK_DIRECTORIES("${PROJECT_BINARY_DIR}/libsdr-gui/src")
 SET(LIBS ${LIBS} libsdr-gui)
 SET(SDR_WITH_FFTW ON)
ELSE(NOT LIBSDR_GUI_FOUND)
 INCLUDE_DIRECTORIES(${LIBSDR_GUI_INCLUDE_DIRS})
 SET(LIBS ${LIBS} ${LIBSDR_GUI_LIBRARIES})
ENDIF(NOT LIBSDR_GUI_FOUND)

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS} -Wall -fPIC")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -ggdb")


# FFLAGS depend on the compiler
get_filename_component(Fortran_COMPILER_NAME ${CMAKE_Fortran_COMPILER} NAME)

if(Fortran_COMPILER_NAME MATCHES "gfortran.*")
  # gfortran
  set(CMAKE_Fortran_FLAGS_RELEASE "-funroll-all-loops -fno-f2c -O3")
  set(CMAKE_Fortran_FLAGS_DEBUG   "-fno-f2c -O0 -g")
elseif(Fortran_COMPILER_NAME MATCHES "ifort.*")
  # ifort (untested)
  set(CMAKE_Fortran_FLAGS_RELEASE "-f77rtl -O3")
  set(CMAKE_Fortran_FLAGS_DEBUG   "-f77rtl -O0 -g")
elseif(Fortran_COMPILER_NAME MATCHES "g77")
  # g77
  set(CMAKE_Fortran_FLAGS_RELEASE "-funroll-all-loops -fno-f2c -O3 -m32")
  set(CMAKE_Fortran_FLAGS_DEBUG   "-fno-f2c -O0 -g -m32")
else(Fortran_COMPILER_NAME MATCHES "gfortran.*")
  message("CMAKE_Fortran_COMPILER full path: " ${CMAKE_Fortran_COMPILER})
  message("Fortran compiler: " ${Fortran_COMPILER_NAME})
  message("No optimized Fortran compiler flags are known, we just try -O2...")
  set(CMAKE_Fortran_FLAGS_RELEASE "-O2")
  set(CMAKE_Fortran_FLAGS_DEBUG   "-O0 -g")
endif(Fortran_COMPILER_NAME MATCHES "gfortran.*")

#
# Get default install directories under Linux
#
IF(UNIX AND NOT APPLE)
 INCLUDE(GNUInstallDirs)
ELSE(UNIX AND NOT APPLE)
 SET(CMAKE_INSTALL_LIBDIR "lib")
 SET(CMAKE_INSTALL_FULL_LIBDIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
 SET(CMAKE_INSTALL_INCLUDEDIR "include")
 SET(CMAKE_INSTALL_FULL_INCLUDEDIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}")
 # Set RPATH under MacOS
 SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
 SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
 SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
ENDIF(UNIX AND NOT APPLE)

# SDR-WSPR sources...
add_subdirectory(libwspr)
add_subdirectory(src)
# ... also compile libsdr if not found:
IF(NOT LIBSDR_FOUND)
  add_subdirectory(libsdr/src)
ENDIF(NOT LIBSDR_FOUND)
# ... and compile libsdr-gui if not found:
IF(NOT LIBSDR_GUI_FOUND)
  add_subdirectory(libsdr-gui/src)
ENDIF(NOT LIBSDR_GUI_FOUND)

#
# Install logo and .desktop file under Linux:
#
IF(UNIX AND NOT APPLE)
 INSTALL(FILES shared/sdr-wspr.svg
         DESTINATION share/icons)
 INSTALL(FILES shared/sdr-wspr.desktop
         DESTINATION share/applications)
ENDIF(UNIX AND NOT APPLE)

# Source distribution packages:
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_PACKAGE_FILE_NAME
  "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_SOURCE_IGNORE_FILES
  "/build/;/doc/;/dist/;/.git/;.dat$;.wav$;~$;.qm$;${CPACK_SOURCE_IGNORE_FILES}")

include(CPack)
