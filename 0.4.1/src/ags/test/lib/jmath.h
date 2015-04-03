#ifndef __J_Math_H__
#define __J_Math_H__

#include "jtypes.h"
#include "jlist.h"
//#include "jtext.h"

int j_math_or(int a, int b);
int j_math_exclusive_or(int a, int b);
int j_math_and(int a, int b);

jboolean j_math_more(int a, int b);
jboolean j_math_less(int a, int b);
int* j_math_sort_array(int *array, int size);
void j_math_put_null_at_beginning(int *array, uint size);
void j_math_put_null_at_end(int *array, uint size);

uint j_math_width(int a);
JList* j_math_width_list(JList *list);
JList* j_math_cut_leading_zeros(JList *value);

int j_math_exp2(int a);
int j_math_exp(int a, int b);
int j_math_root2(int a);
int j_math_root(int a, int b);

void j_math_make_valid(char *string);
jboolean j_math_is_operator(char *string);
jboolean j_math_is_function(char *string);
jboolean j_math_is_function_backward(char *start, char *string);
char* j_math_get_first_level_operator(char *string);
char* j_math_get_second_level_operator(char *string);

int j_math_solve_arithmetic(char *string);
//int j_math_solve_arithmetic_text(JText *text);

char* j_math_algebra_brackeds_out(char *string);
char* j_math_algebra_operations_out(char *string);
char* j_math_algebra_functions_out(char *string);
int* j_math_algebra_solve(char *string, char *operation);
//int* j_math_algebra_solve_text(JText *text);

int* j_math_gauss(char **string, uint count);

#endif /*__J_Math_H__*/
