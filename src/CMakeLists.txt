link_directories(/usr/local/opt/fftw/lib)
add_library(qcrypto arithmetic.c arithmetic.h fft.c fft.h helper.c helper.h)
target_link_libraries(qcrypto fftw3)