# CMakeList.txt : CMake project for Group43_Project, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Group43_Project")

# Set the file location
set(FILE_PATH_DIR "${CMAKE_SOURCE_DIR}/credentials.dat")

# Pass it to the compiler as a definition
add_definitions(-DFILE_PATH_DIR=\"${FILE_PATH_DIR}\")

# Include OpenSSL header directory
include_directories(${CMAKE_SOURCE_DIR}/include)

# Link OpenSSL dynamic libraries
link_directories(${CMAKE_SOURCE_DIR}/lib)
link_directories(${CMAKE_SOURCE_DIR}/bin/windows)


# Add source to this project's executable.
add_executable (Group43_Project "main.cpp" "mylibrary.cpp" "mylibrary.h")

target_link_libraries(Group43_Project PRIVATE
						${CMAKE_SOURCE_DIR}/lib/libssl.lib
						${CMAKE_SOURCE_DIR}/lib/libcrypto.lib)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Group43_Project PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
