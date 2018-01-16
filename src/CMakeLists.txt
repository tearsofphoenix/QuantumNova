if (APPLE)
    link_directories(/usr/local/opt/fftw/lib /usr/local/opt/libtasn1/lib)
elseif(UNIX)
    link_directories(/usr/lib)
endif (APPLE)

add_library(qcrypto QCArray.c QCArray.h QCArrayPrivate.h QCArrayPrivate.c QCRandom.c QCRandom.h QCKey.c QCKey.h QCKeyPrivate.h
        QCCipher.c QCCipher.h QCPrivate.c QCPrivate.h QCInt32Array.c QCInt32Array.h QCDoubleArray.c QCDoubleArray.h QCClass.c QCClass.h
        QCObject.c QCObject.h QCObjectPrivate.h QCCipherPrivate.h QCByteArray.c QCByteArray.h)

add_subdirectory(vendor)

if (APPLE)
    target_link_libraries(qcrypto fftw3 ext-crypto tasn1 crypto tomcrypt)
elseif(UNIX)
    target_link_libraries(qcrypto fftw3 ext-crypto tasn1 crypto tomcrypt m)
endif (APPLE)