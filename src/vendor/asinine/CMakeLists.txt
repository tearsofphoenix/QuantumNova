include_directories(include)

add_library(asinine src/asn1-parser.c src/asn1-oid.c src/asn1-types.c src/x509.c src/x509-name.c src/x509-path.c
        src/x509-pubkey.c
        include/asinine/asn1.h include/asinine/dsl.h include/asinine/errors.h include/asinine/macros.h
        include/asinine/x509.h)