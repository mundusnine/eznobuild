#pragma once

#include <stddef.h>

#ifndef NOBUILD__DEPRECATED
#	if defined(__GNUC__) || (defined(__clang__) && !defined(_MSC_VER))
#		define NOBUILD__DEPRECATED(func) __attribute__ ((deprecated)) func
#	elif defined(_MSC_VER)
#		define NOBUILD__DEPRECATED(func) __declspec (deprecated) func
#	endif
#endif

typedef const char * Cstr;

int cstr_ends_with(Cstr cstr, Cstr postfix);
#define ENDS_WITH(cstr, postfix) cstr_ends_with(cstr, postfix)

int cstr_starts_with(Cstr cstr, Cstr prefix);
#define STARTS_WITH(cstr, prefix) cstr_starts_with(cstr, prefix)

typedef struct {
    Cstr *elems;
    size_t count;
    size_t capacity;
} Cstr_Array;

Cstr_Array cstr_array_make(Cstr first, ...);
#define CSTR_ARRAY_MAKE(first, ...) cstr_array_make(first, ##__VA_ARGS__, NULL)

Cstr_Array cstr_array_append(Cstr_Array cstrs, Cstr cstr);

Cstr_Array cstr_array_remove(Cstr_Array cstrs, Cstr cstr);

Cstr_Array cstr_array_concat(Cstr_Array cstrs_a, Cstr_Array cstrs_b);

int cstr_array_contains(Cstr_Array cstrs, Cstr cstr);

Cstr_Array cstr_array_from_cstr(Cstr cstr, Cstr delim);
#define SPLIT(cstr, delim) cstr_array_from_cstr(cstr, delim)

Cstr cstr_array_join(Cstr sep, Cstr_Array cstrs);
#define JOIN(sep, ...) cstr_array_join(sep, cstr_array_make(__VA_ARGS__, NULL))
#define CONCAT(...) JOIN("", __VA_ARGS__)