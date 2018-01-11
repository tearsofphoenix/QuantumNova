cmake_minimum_required(VERSION 3.8)
project(pqc-crypto)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)

include_directories(/usr/local/opt/fftw/include /usr/local/opt/libtasn1/include)
link_directories(/usr/local/opt/fftw/lib /usr/local/opt/libtasn1/lib)

add_subdirectory(src)
add_subdirectory(test)


enable_testing()

