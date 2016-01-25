TEMPLATE = app
CONFIG += console warn_all
CONFIG -= app_bundle
CONFIG -= qt

HEADERS += \
    evaluator.h \
    evaluator_operations.h \
    evaluator_internal/evaluator_object.h \
    evaluator_internal/var_container.h \
    evaluator_internal/transition_table.h \
    evaluator_internal/misc.h \
    evaluator_internal/parse.h \
    evaluator_internal/simplify.h \
    evaluator_internal/calculate.h \
    evaluator_internal/jit/common.h \
    evaluator_internal/jit/opcodes.h \
    evaluator_internal/jit/func_templates.h \
    evaluator_internal/jit/oper_templates.h \
    evaluator_internal/jit/real_templates.h \
    evaluator_internal/jit/complex_templates.h \
    evaluator_internal/jit/compile_inline.h \
    evaluator_internal/jit/compile_extcall.h

SOURCES += \
    evaluator_internal/transition_table.cpp \
    evaluator_internal/jit/common.cpp \
    evaluator_internal/jit/func_templates.cpp \
    evaluator_internal/jit/oper_templates.cpp \
    evaluator_internal/jit/real_templates.cpp \
    main.cpp
