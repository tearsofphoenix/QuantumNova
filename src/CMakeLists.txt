link_directories(/usr/local/opt/fftw/lib)
add_library(qcrypto QCArray.c QCArray.h QCArrayPrivate.h)
target_link_libraries(qcrypto fftw3)