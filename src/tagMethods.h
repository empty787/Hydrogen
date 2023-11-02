/*
** $Id: tagMethods.h $
** Tag methods
** See Copyright Notice in venom.h
*/

#ifndef tagMethods_h
#define tagMethods_h


#include "object.h"


/*
* WARNING: if you change the order of this enumeration,
* grep "ORDER TM" and "ORDER OP"
*/
typedef enum {
  TM_INDEX,
  TM_NEWINDEX,
  TM_GC,
  TM_MODE,
  TM_LEN,
  TM_EQ,  /* last tag method with fast access */
  TM_ADD,
  TM_SUB,
  TM_MUL,
  TM_MOD,
  TM_POW,
  TM_DIV,
  TM_IDIV,
  TM_BAND,
  TM_BOR,
  TM_BXOR,
  TM_SHL,
  TM_SHR,
  TM_UNM,
  TM_BNOT,
  TM_LT,
  TM_LE,
  TM_CONCAT,
  TM_CALL,
  TM_CLOSE,
  TM_N		/* number of elements in the enum */
} TMS;


/*
** Mask with 1 in all fast-access methods. A 1 in any of these bits
** in the flag of a (meta)table means the metatable does not have the
** corresponding metamethod field. (Bit 7 of the flag is used for
** 'isrealasize'.)
*/
#define maskflags	(~(~0u << (TM_EQ + 1)))


/*
** Test whether there is no tagmethod.
** (Because tagmethods use raw accesses, the result may be an "empty" nil.)
*/
#define notm(tm)	ttisnil(tm)


#define gfasttm(g,et,e) ((et) == NULL ? NULL : \
  ((et)->flags & (1u<<(e))) ? NULL : venomT_gettm(et, e, (g)->tmname[e]))

#define fasttm(l,et,e)	gfasttm(G(l), et, e)

#define ttypename(x)	venomT_typenames_[(x) + 1]

VENOMI_DDEC(const char *const venomT_typenames_[VENOM_TOTALTYPES];)


VENOMI_FUNC const char *venomT_objtypename (venom_State *L, const TValue *o);

VENOMI_FUNC const TValue *venomT_gettm (Table *events, TMS event, TString *ename);
VENOMI_FUNC const TValue *venomT_gettmbyobj (venom_State *L, const TValue *o,
                                                       TMS event);
VENOMI_FUNC void venomT_init (venom_State *L);

VENOMI_FUNC void venomT_caltagMethods (venom_State *L, const TValue *f, const TValue *p1,
                            const TValue *p2, const TValue *p3);
VENOMI_FUNC void venomT_caltagMethodsres (venom_State *L, const TValue *f,
                            const TValue *p1, const TValue *p2, StkId p3);
VENOMI_FUNC void venomT_trybinTM (venom_State *L, const TValue *p1, const TValue *p2,
                              StkId res, TMS event);
VENOMI_FUNC void venomT_tryconcatTM (venom_State *L);
VENOMI_FUNC void venomT_trybinassocTM (venom_State *L, const TValue *p1,
       const TValue *p2, int inv, StkId res, TMS event);
VENOMI_FUNC void venomT_trybiniTM (venom_State *L, const TValue *p1, venom_Integer i2,
                               int inv, StkId res, TMS event);
VENOMI_FUNC int venomT_callorderTM (venom_State *L, const TValue *p1,
                                const TValue *p2, TMS event);
VENOMI_FUNC int venomT_callorderiTM (venom_State *L, const TValue *p1, int v2,
                                 int inv, int isfloat, TMS event);

VENOMI_FUNC void venomT_adjustvarargs (venom_State *L, int nfixparams,
                                   struct CallInfo *ci, const Proto *p);
VENOMI_FUNC void venomT_getvarargs (venom_State *L, struct CallInfo *ci,
                                              StkId where, int wanted);


#endif
