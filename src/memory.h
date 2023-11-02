/*
** $Id: memory.h $
** Interface to Memory Manager
** See Copyright Notice in venom.h
*/

#ifndef memory_h
#define memory_h


#include <stddef.h>

#include "limits.h"
#include "venom.h"


#define venomM_error(L)	venomD_throw(L, VENOM_ERRMEM)


/*
** This macro tests whether it is safe to multiply 'n' by the size of
** type 't' without overflows. Because 'e' is always constant, it avoids
** the runtime division MAX_SIZET/(e).
** (The macro is somewhat complex to avoid warnings:  The 'sizeof'
** comparison avoids a runtime comparison when overflow cannot occur.
** The compiler should be able to optimize the real test by itself, but
** when it does it, it may give a warning about "comparison is always
** false due to limited range of data type"; the +1 tricks the compiler,
** avoiding this warning but also this optimization.)
*/
#define venomM_testsize(n,e)  \
	(sizeof(n) >= sizeof(size_t) && cast_sizet((n)) + 1 > MAX_SIZET/(e))

#define venomM_checksize(L,n,e)  \
	(venomM_testsize(n,e) ? venomM_toobig(L) : cast_void(0))


/*
** Computes the minimum between 'n' and 'MAX_SIZET/sizeof(t)', so that
** the result is not larger than 'n' and cannot overflow a 'size_t'
** when multiplied by the size of type 't'. (Assumes that 'n' is an
** 'int' or 'unsigned int' and that 'int' is not larger than 'size_t'.)
*/
#define venomM_limitN(n,t)  \
  ((cast_sizet(n) <= MAX_SIZET/sizeof(t)) ? (n) :  \
     cast_uint((MAX_SIZET/sizeof(t))))


/*
** Arrays of chars do not need any test
*/
#define venomM_reallocvchar(L,b,on,n)  \
  cast_charp(venomM_saferealloc_(L, (b), (on)*sizeof(char), (n)*sizeof(char)))

#define venomM_freemem(L, b, s)	venomM_free_(L, (b), (s))
#define venomM_free(L, b)		venomM_free_(L, (b), sizeof(*(b)))
#define venomM_freearray(L, b, n)   venomM_free_(L, (b), (n)*sizeof(*(b)))

#define venomM_new(L,t)		cast(t*, venomM_malloc_(L, sizeof(t), 0))
#define venomM_newvector(L,n,t)	cast(t*, venomM_malloc_(L, (n)*sizeof(t), 0))
#define venomM_newvectorchecked(L,n,t) \
  (venomM_checksize(L,n,sizeof(t)), venomM_newvector(L,n,t))

#define venomM_newobject(L,tag,s)	venomM_malloc_(L, (s), tag)

#define venomM_growvector(L,v,nelems,size,t,limit,e) \
	((v)=cast(t *, venomM_growaux_(L,v,nelems,&(size),sizeof(t), \
                         venomM_limitN(limit,t),e)))

#define venomM_reallocvector(L, v,oldn,n,t) \
   (cast(t *, venomM_realloc_(L, v, cast_sizet(oldn) * sizeof(t), \
                                  cast_sizet(n) * sizeof(t))))

#define venomM_shrinkvector(L,v,size,fs,t) \
   ((v)=cast(t *, venomM_shrinkvector_(L, v, &(size), fs, sizeof(t))))

VENOMI_FUNC l_noret venomM_toobig (venom_State *L);

/* not to be called directly */
VENOMI_FUNC void *venomM_realloc_ (venom_State *L, void *block, size_t oldsize,
                                                          size_t size);
VENOMI_FUNC void *venomM_saferealloc_ (venom_State *L, void *block, size_t oldsize,
                                                              size_t size);
VENOMI_FUNC void venomM_free_ (venom_State *L, void *block, size_t osize);
VENOMI_FUNC void *venomM_growaux_ (venom_State *L, void *block, int nelems,
                               int *size, int size_elem, int limit,
                               const char *what);
VENOMI_FUNC void *venomM_shrinkvector_ (venom_State *L, void *block, int *nelem,
                                    int final_n, int size_elem);
VENOMI_FUNC void *venomM_malloc_ (venom_State *L, size_t size, int tag);

#endif

