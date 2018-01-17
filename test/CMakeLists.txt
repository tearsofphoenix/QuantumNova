
include_directories(../)

add_executable(fft-test fft-test.c fft-test.h data.h cipher-test.c cipher-test.h array-test.c array-test.h key-test.c key-test.h main.c)

#configure_file(data.txt ${CMAKE_CURRENT_BINARY_DIR}/data.txt COPYONLY)
#configure_file(enc.data ${CMAKE_CURRENT_BINARY_DIR}/enc.data COPYONLY)
#configure_file(priv.key ${CMAKE_CURRENT_BINARY_DIR}/priv.key COPYONLY)
#configure_file(pub.key ${CMAKE_CURRENT_BINARY_DIR}/pub.key COPYONLY)
#configure_file(aux ${CMAKE_CURRENT_BINARY_DIR}/aux COPYONLY)
file(COPY aux DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(fft-test qcrypto)