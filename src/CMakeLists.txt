link_directories(/usr/local/opt/fftw/lib /usr/local/opt/libtasn1/lib)
add_library(qcrypto QCArray.c QCArray.h QCArrayPrivate.h QCArrayPrivate.c QCRandom.c QCRandom.h QCKey.c QCKey.h QCKeyPrivate.c QCKeyPrivate.h QCCipher.c QCCipher.h QCPrivate.c QCPrivate.h)
target_link_libraries(qcrypto fftw3 tasn1)