
include_directories(../)

add_executable(fft-test fft-test.c fft-test.h data.h salsa20-test.c salsa20-test.h cipher-test.c cipher-test.h array-test.c array-test.h)
target_link_libraries(fft-test qcrypto)