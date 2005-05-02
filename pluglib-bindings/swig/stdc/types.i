/*
 * file:	stdc/types.i
 * author:	Martin Lazar <mlazar@suse.cz>
 *
 * basic data types typemas and helpers
 *
 * $Id$
 */

%{
extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
}

/* convert any to SV */

bool FROM_PACK(SV *sv, void *ptr, int size, swig_type_info *t) {
    SWIG_MakePackedObj(sv, ptr, size, t);
    return true;
}

bool TO_PACK(SV *sv, void *ptr, int size, swig_type_info *t) {
    return SWIG_ConvertPacked(sv, ptr, size, t, 0) == 0;
}

bool FROM_BOOL(SV *sv, const bool *x, int size, const swig_type_info *t) {
    sv_setiv(sv, *x);
    return true;
}

bool FROM_CHAR(SV *sv, const signed char *x, int size, const swig_type_info *t) {
    sv_setiv(sv, *x);
    return true;
}

bool FROM_SHORT(SV *sv, const short int *x, int size, const swig_type_info *t) {
    sv_setiv(sv, *x);
    return true;
}

bool FROM_INT(SV *sv, const int *x, int size, const swig_type_info *t) {
    sv_setiv(sv, *x);
    return true;
}

bool FROM_LONG(SV *sv, const long int *x, int size, const swig_type_info *t) {
    sv_setiv(sv, *x);
    return true;
}

bool FROM_QUAD(SV *sv, const long long int *x, int size, const swig_type_info *t) {
    char temp[256]; 
    sprintf(temp,"%lld", *x); 
    sv_setpv(sv, temp);
    return true;
}

bool FROM_UCHAR(SV *sv, const unsigned char *x, int size, const swig_type_info *t) {
    sv_setuv(sv, *x);
    return true;
}

bool FROM_USHORT(SV *sv, const unsigned short *x, int size, const swig_type_info *t) {
    sv_setuv(sv, *x);
    return true;
}

bool FROM_UINT(SV *sv, const unsigned int *x, int size, const swig_type_info *t) {
    sv_setuv(sv, *x);
    return true;
}

bool FROM_ULONG(SV *sv, const unsigned long *x, int size, const swig_type_info *t) {
    sv_setuv(sv, *x);
    return true;
}

bool FROM_UQUAD(SV *sv, const unsigned long long *x, int size, const swig_type_info *t) {
    char temp[256]; 
    sprintf(temp,"%llu", *x); 
    sv_setpv(sv, temp);
    return true;
}

bool FROM_DOUBLE(SV *sv, const double *x, int size, const swig_type_info *t) {
    sv_setnv(sv, *x);
    return true;
}

bool FROM_FLOAT(SV *sv, const float *x, int size, const swig_type_info *t) {
    sv_setnv(sv, *x);
    return true;
}


/* convert SV to any */
bool TO_BOOL(SV* sv, bool *x, int size, const swig_type_info *t) { 
    *x = SvIV(sv);
    return true;
}

bool TO_INT(SV* sv, int32_t *x, int size, const swig_type_info *t) { 
    *x = SvIV(sv);
    return true;
}

bool TO_INT64(SV* sv, int64_t *x, int size, const swig_type_info *t) {
    *x = strtoll(SvPV_nolen(sv), 0, 0);
    return true;
}

bool TO_UINT(SV* sv, uint32_t *x, int size, const swig_type_info *t) {
    *x = SvUV(sv);
    return true;
}

bool TO_UINT64(SV* sv, uint64_t *x, int size, const swig_type_info *t) { 
    *x = strtoull(SvPV_nolen(sv), 0, 0);
    return true;
}

bool TO_FLOAT(SV* sv, double *x, int size, const swig_type_info *t) { 
    *x = SvNV(sv);
    return true;
}

/* check SV */
bool CHECK() {
    return false;
}


%}

%define apply_c_types(WHAT, ...)
WHAT(__VA_ARGS__, bool, TO_BOOL, FROM_BOOL, CHECK)

WHAT(__VA_ARGS__, signed char, TO_CHAR, FROM_CHAR, CHECK)
WHAT(__VA_ARGS__, signed short, TO_SHORT, FROM_SHORT, CHECK)
WHAT(__VA_ARGS__, signed int, TO_INT, FROM_INT, CHECK)
WHAT(__VA_ARGS__, signed long, TO_LONG, FROM_LONG, CHECK)
WHAT(__VA_ARGS__, signed long long, TO_QUAD, FROM_QUAD, CHECK)

WHAT(__VA_ARGS__, unsigned char, TO_UCHAR, FROM_UCHAR, CHECK)
WHAT(__VA_ARGS__, unsigned short, TO_USHORT, FROM_USHORT, CHECK)
WHAT(__VA_ARGS__, unsigned int, TO_UINT, FROM_UINT, CHECK)
WHAT(__VA_ARGS__, unsigned long, TO_ULONG, FROM_ULONG, CHECK)
WHAT(__VA_ARGS__, unsigned long long, TO_UQUAD, FROM_UQUAD, CHECK)

WHAT(__VA_ARGS__, float, TO_FLOAT, FROM_FLOAT, CHECK)
WHAT(__VA_ARGS__, double, TO_DOUBLE, FROM_DOUBLE, CHECK)
%enddef

%define apply_c_keytypes(WHAT, ...)
WHAT(__VA_ARGS__, signed char, TO_CHAR, FROM_CHAR, CHECK)
WHAT(__VA_ARGS__, signed short, TO_SHORT, FROM_SHORT, CHECK)
WHAT(__VA_ARGS__, signed int, TO_INT, FROM_INT, CHECK)
WHAT(__VA_ARGS__, signed long, TO_LONG, FROM_LONG, CHECK)
WHAT(__VA_ARGS__, signed long long, TO_QUAD, FROM_QUAD, CHECK)

WHAT(__VA_ARGS__, unsigned char, TO_UCHAR, FROM_UCHAR, CHECK)
WHAT(__VA_ARGS__, unsigned short, TO_USHORT, FROM_USHORT, CHECK)
WHAT(__VA_ARGS__, unsigned int, TO_UINT, FROM_UINT, CHECK)
WHAT(__VA_ARGS__, unsigned long, TO_ULONG, FROM_ULONG, CHECK)
WHAT(__VA_ARGS__, unsigned long long, TO_UQUAD, FROM_UQUAD, CHECK)
%enddef
