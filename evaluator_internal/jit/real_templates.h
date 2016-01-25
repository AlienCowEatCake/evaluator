#ifndef REAL_TEMPLATES_H
#define REAL_TEMPLATES_H

#include "opcodes.h"

namespace evaluator_internal_jit
{

// Input:  st(0) = X , st(1) = Y
// Output: st(0) = X ^ Y
void real_pow(char *& code_curr);

// Input:  st(0) = X
// Output: st(0) = asin(X)
void real_asin(char *& code_curr);

// Input:  st(0) = X
// Output: st(0) = acos(X)
void real_acos(char *& code_curr);

// Input:  st(0) = X
// Output: st(0) = log2(X)
void real_log2(char *& code_curr);

// Input:  st(0) = X
// Output: st(0) = log(X)
void real_log(char *& code_curr);

// Input:  st(0) = X
// Output: st(0) = log10(X)
void real_log10(char *& code_curr);

// Input:  st(0) = X
// Output: st(0) = exp(X)
void real_exp(char *& code_curr);

// Input:  st(0) = X
// Output: st(0) = sinh(X)
void real_sinh(char *& code_curr);

// Input:  st(0) = X
// Output: st(0) = cosh(X)
void real_cosh(char *& code_curr);

// Input:  st(0) = X
// Output: st(0) = tanh(X)
void real_tanh(char *& code_curr);

// Input:  st(0) = X
// Output: st(0) = asinh(X)
void real_asinh(char *& code_curr);

// Input:  st(0) = X
// Output: st(0) = acosh(X)
void real_acosh(char *& code_curr);

// Input:  st(0) = X
// Output: st(0) = atanh(X)
void real_atanh(char *& code_curr);

// Input:  st(0) = X
// Output: st(0) = arg(X)
void real_arg(char *& code_curr);

}

#endif // REAL_TEMPLATES_H

