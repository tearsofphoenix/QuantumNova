if (APPLE)
    link_directories(/usr/local/opt/fftw/lib)
elseif(UNIX)
    link_directories(/usr/lib)
endif (APPLE)

add_library(qcrypto QNArray.c QNArray.h QNArrayPrivate.h QNArrayPrivate.c QNRandom.c QNRandom.h QNKey.c QNKey.h QNKeyPrivate.h
        QNCipher.c QNCipher.h QNInt32Array.c QNInt32Array.h QNDoubleArray.c QNDoubleArray.h QNClass.c QNClass.h
        QNObject.c QNObject.h QNObjectPrivate.h QNCipherPrivate.h QNByteArray.c QNByteArray.h QNMessage.c QNMessage.h QNMessagePrivate.h
        QNTest.c QNTest.h QNSymmetricCipher.c QNSymmetricCipher.h vendor/salsa20.c vendor/salsa20.h)


if (APPLE)
    target_link_libraries(qcrypto fftw3 crypto tomcrypt)
elseif(UNIX)
    target_link_libraries(qcrypto fftw3 crypto tomcrypt m)
endif (APPLE)