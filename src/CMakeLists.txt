if (APPLE)
    link_directories(/usr/local/opt/fftw/lib)
elseif(UNIX)
    link_directories(/usr/lib)
endif (APPLE)

add_library(qcrypto QCArray.c QCArray.h QCArrayPrivate.h QCArrayPrivate.c QNRandom.c QNRandom.h QCKey.c QCKey.h QCKeyPrivate.h
        QCCipher.c QCCipher.h QCInt32Array.c QCInt32Array.h QCDoubleArray.c QCDoubleArray.h QCClass.c QCClass.h
        QCObject.c QCObject.h QCObjectPrivate.h QCCipherPrivate.h QCByteArray.c QCByteArray.h QCMessage.c QCMessage.h QCMessagePrivate.h
        QNTest.c QNTest.h QNSymmetricCipher.c QNSymmetricCipher.h vendor/salsa20.c vendor/salsa20.h)


if (APPLE)
    target_link_libraries(qcrypto fftw3 crypto tomcrypt)
elseif(UNIX)
    target_link_libraries(qcrypto fftw3 crypto tomcrypt m)
endif (APPLE)