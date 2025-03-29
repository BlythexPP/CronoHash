cmake_minimum_required(VERSION 3.10)
project(CronoHash VERSION 1.0 LANGUAGES CXX)

# Setze den C++ Standard auf C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Überprüfe, ob eine 64-Bit-Plattform verwendet wird
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(FATAL_ERROR "This project requires a 64-bit platform.")
endif()

# Setze den Buildtyp, falls nicht angegeben (Standard: Release)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Füge zusätzliche Compiler-Warnungen hinzu (optional, für Produktion)
if(MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Finde liboqs (stelle sicher, dass liboqs installiert ist)
find_package(OQS REQUIRED)

# Include-Verzeichnisse
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${OQS_INCLUDE_DIRS})

# Sammle alle Quellcodes aus dem src-Verzeichnis
file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")

# Füge den CLI-Einstiegspunkt (CronoHash.cpp) hinzu
list(APPEND SOURCES "${CMAKE_SOURCE_DIR}/CronoHash.cpp")

# Erstelle das ausführbare Programm
add_executable(cronohash ${SOURCES})

# Verlinke liboqs
target_link_libraries(cronohash ${OQS_LIBRARIES})

# Plattformabhängige Einstellungen
if(WIN32)
  # Windows-spezifische Einstellungen, z.B. zusätzliche Bibliotheken
  target_link_libraries(cronohash ws2_32)
endif()

if(UNIX AND NOT APPLE)
  # Linux-spezifische Anpassungen, z.B. pthread (falls benötigt)
  target_link_libraries(cronohash pthread)
endif()

# Optionale Installation (z.B. in /usr/local/bin)
install(TARGETS cronohash RUNTIME DESTINATION bin)

# Optionale Testintegration mit GoogleTest
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
  enable_testing()
  # Hier wird davon ausgegangen, dass im tests-Verzeichnis eine eigene CMakeLists.txt für GoogleTest liegt
  add_subdirectory(tests)
endif()
