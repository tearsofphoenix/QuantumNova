
include_directories(../)

add_executable(fft-test fft-test.c fft-test.h data.h salsa20-test.c salsa20-test.h)
target_link_libraries(fft-test qcrypto)