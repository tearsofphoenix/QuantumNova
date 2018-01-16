cmake_minimum_required(VERSION 3.8)
project(pqc-crypto)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)

if (APPLE)
    include_directories(/usr/local/opt/fftw/include /usr/local/opt/libtasn1/include /usr/local/opt/openssl/include)
    link_directories(/usr/local/opt/fftw/lib /usr/local/opt/libtasn1/lib /usr/local/opt/openssl/lib)
elseif(UNIX)

    include_directories(/usr/include)
    link_directories(/usr/lib)
endif (APPLE)


add_subdirectory(src)
add_subdirectory(test)


enable_testing()

