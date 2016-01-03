TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += warn_all

SOURCES += \
#    main.cpp \
    fake.cpp \
    benchmark.cpp

HEADERS += parser.h \
    parser_operations.h \
    parser_internal.h \
    parser_opcodes.h \
    parser_compiler_inline.h \
    parser_compiler_extcall.h \
    parser_templates.h
