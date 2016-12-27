TEMPLATE = app
CONFIG += console warn_all
CONFIG -= app_bundle
CONFIG -= qt

*g++*|*clang* {
    QMAKE_CXXFLAGS *= -Wno-long-long
}

HEADERS += \
    evaluator/evaluator.h \
    evaluator/evaluator_xyz.h \
    evaluator/evaluator_operations.h \
    evaluator/evaluator_internal/type_detection.h \
    evaluator/evaluator_internal/evaluator_object.h \
    evaluator/evaluator_internal/var_container.h \
    evaluator/evaluator_internal/transition_table.h \
    evaluator/evaluator_internal/misc.h \
    evaluator/evaluator_internal/parse.h \
    evaluator/evaluator_internal/simplify.h \
    evaluator/evaluator_internal/calculate.h \
    evaluator/evaluator_internal/jit/common.h \
    evaluator/evaluator_internal/jit/opcodes.h \
    evaluator/evaluator_internal/jit/func_templates.h \
    evaluator/evaluator_internal/jit/oper_templates.h \
    evaluator/evaluator_internal/jit/real_templates.h \
    evaluator/evaluator_internal/jit/complex_templates.h \
    evaluator/evaluator_internal/jit/compile_inline.h \
    evaluator/evaluator_internal/jit/compile_extcall.h

SOURCES += \
    evaluator/evaluator_internal/transition_table.cpp \
    evaluator/evaluator_internal/jit/common.cpp \
    evaluator/evaluator_internal/jit/func_templates.cpp \
    evaluator/evaluator_internal/jit/oper_templates.cpp \
    evaluator/evaluator_internal/jit/real_templates.cpp \
    main.cpp
