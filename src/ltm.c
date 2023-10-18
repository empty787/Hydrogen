/*
** $Id: ltm.c $
** Tag methods
** See Copyright Notice in acorn.h
*/

#define ltm_c
#define ACORN_CORE

#include "lprefix.h"


#include <string.h>

#include "acorn.h"

#include "ldebug.h"
#include "ldo.h"
#include "lgc.h"
#include "lobject.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "ltm.h"
#include "lvm.h"


static const char udatatypename[] = "userdata";

ACORNI_DDEF const char *const acornT_typenames_[ACORN_TOTALTYPES] = {
  "no value",
  "nil", "boolean", udatatypename, "number",
  "string", "table", "function", udatatypename, "thread",
  "upvalue", "proto" /* these last cases are used for tests only */
};


void acornT_init (acorn_State *L) {
  static const char *const acornT_eventname[] = {  /* ORDER TM */
    "__index", "__newindex",
    "__gc", "__mode", "__len", "__eq",
    "__add", "__sub", "__mul", "__mod", "__pow",
    "__div", "__idiv",
    "__band", "__bor", "__bxor", "__shl", "__shr",
    "__unm", "__bnot", "__lt", "__le",
    "__concat", "__call", "__close"
  };
  int i;
  for (i=0; i<TM_N; i++) {
    G(L)->tmname[i] = acornS_new(L, acornT_eventname[i]);
    acornC_fix(L, obj2gco(G(L)->tmname[i]));  /* never collect these names */
  }
}


/*
** function to be used with macro "fasttm": optimized for absence of
** tag methods
*/
const TValue *acornT_gettm (Table *events, TMS event, TString *ename) {
  const TValue *tm = acornH_getshortstr(events, ename);
  acorn_assert(event <= TM_EQ);
  if (notm(tm)) {  /* no tag method? */
    events->flags |= cast_byte(1u<<event);  /* cache this fact */
    return NULL;
  }
  else return tm;
}


const TValue *acornT_gettmbyobj (acorn_State *L, const TValue *o, TMS event) {
  Table *mt;
  switch (ttype(o)) {
    case ACORN_TTABLE:
      mt = hvalue(o)->metatable;
      break;
    case ACORN_TUSERDATA:
      mt = uvalue(o)->metatable;
      break;
    default:
      mt = G(L)->mt[ttype(o)];
  }
  return (mt ? acornH_getshortstr(mt, G(L)->tmname[event]) : &G(L)->nilvalue);
}


/*
** Return the name of the type of an object. For tables and userdata
** with metatable, use their '__name' metafield, if present.
*/
const char *acornT_objtypename (acorn_State *L, const TValue *o) {
  Table *mt;
  if ((ttistable(o) && (mt = hvalue(o)->metatable) != NULL) ||
      (ttisfulluserdata(o) && (mt = uvalue(o)->metatable) != NULL)) {
    const TValue *name = acornH_getshortstr(mt, acornS_new(L, "__name"));
    if (ttisstring(name))  /* is '__name' a string? */
      return getstr(tsvalue(name));  /* use it as type name */
  }
  return ttypename(ttype(o));  /* else use standard type name */
}


void acornT_callTM (acorn_State *L, const TValue *f, const TValue *p1,
                  const TValue *p2, const TValue *p3) {
  StkId func = L->top;
  setobj2s(L, func, f);  /* push function (assume EXTRA_STACK) */
  setobj2s(L, func + 1, p1);  /* 1st argument */
  setobj2s(L, func + 2, p2);  /* 2nd argument */
  setobj2s(L, func + 3, p3);  /* 3rd argument */
  L->top = func + 4;
  /* metamethod may yield only when called from Acorn code */
  if (isAcorncode(L->ci))
    acornD_call(L, func, 0);
  else
    acornD_callnoyield(L, func, 0);
}


void acornT_callTMres (acorn_State *L, const TValue *f, const TValue *p1,
                     const TValue *p2, StkId res) {
  ptrdiff_t result = savestack(L, res);
  StkId func = L->top;
  setobj2s(L, func, f);  /* push function (assume EXTRA_STACK) */
  setobj2s(L, func + 1, p1);  /* 1st argument */
  setobj2s(L, func + 2, p2);  /* 2nd argument */
  L->top += 3;
  /* metamethod may yield only when called from Acorn code */
  if (isAcorncode(L->ci))
    acornD_call(L, func, 1);
  else
    acornD_callnoyield(L, func, 1);
  res = restorestack(L, result);
  setobjs2s(L, res, --L->top);  /* move result to its place */
}


static int callbinTM (acorn_State *L, const TValue *p1, const TValue *p2,
                      StkId res, TMS event) {
  const TValue *tm = acornT_gettmbyobj(L, p1, event);  /* try first operand */
  if (notm(tm))
    tm = acornT_gettmbyobj(L, p2, event);  /* try second operand */
  if (notm(tm)) return 0;
  acornT_callTMres(L, tm, p1, p2, res);
  return 1;
}


void acornT_trybinTM (acorn_State *L, const TValue *p1, const TValue *p2,
                    StkId res, TMS event) {
  if (l_unlikely(!callbinTM(L, p1, p2, res, event))) {
    switch (event) {
      case TM_BAND: case TM_BOR: case TM_BXOR:
      case TM_SHL: case TM_SHR: case TM_BNOT: {
        if (ttisnumber(p1) && ttisnumber(p2))
          acornG_tointerror(L, p1, p2);
        else
          acornG_opinterror(L, p1, p2, "perform bitwise operation on");
      }
      /* calls never return, but to avoid warnings: *//* FALLTHROUGH */
      default:
        acornG_opinterror(L, p1, p2, "perform arithmetic on");
    }
  }
}


void acornT_tryconcatTM (acorn_State *L) {
  StkId top = L->top;
  if (l_unlikely(!callbinTM(L, s2v(top - 2), s2v(top - 1), top - 2,
                               TM_CONCAT)))
    acornG_concaterror(L, s2v(top - 2), s2v(top - 1));
}


void acornT_trybinassocTM (acorn_State *L, const TValue *p1, const TValue *p2,
                                       int flip, StkId res, TMS event) {
  if (flip)
    acornT_trybinTM(L, p2, p1, res, event);
  else
    acornT_trybinTM(L, p1, p2, res, event);
}


void acornT_trybiniTM (acorn_State *L, const TValue *p1, acorn_Integer i2,
                                   int flip, StkId res, TMS event) {
  TValue aux;
  setivalue(&aux, i2);
  acornT_trybinassocTM(L, p1, &aux, flip, res, event);
}


/*
** Calls an order tag method.
** For lessequal, ACORN_COMPAT_LT_LE keeps compatibility with old
** behavior: if there is no '__le', try '__lt', based on l <= r iff
** !(r < l) (assuming a total order). If the metamethod yields during
** this substitution, the continuation has to know about it (to negate
** the result of r<l); bit CIST_LEQ in the call status keeps that
** information.
*/
int acornT_callorderTM (acorn_State *L, const TValue *p1, const TValue *p2,
                      TMS event) {
  if (callbinTM(L, p1, p2, L->top, event))  /* try original event */
    return !l_isfalse(s2v(L->top));
#if defined(ACORN_COMPAT_LT_LE)
  else if (event == TM_LE) {
      /* try '!(p2 < p1)' for '(p1 <= p2)' */
      L->ci->callstatus |= CIST_LEQ;  /* mark it is doing 'lt' for 'le' */
      if (callbinTM(L, p2, p1, L->top, TM_LT)) {
        L->ci->callstatus ^= CIST_LEQ;  /* clear mark */
        return l_isfalse(s2v(L->top));
      }
      /* else error will remove this 'ci'; no need to clear mark */
  }
#endif
  acornG_ordererror(L, p1, p2);  /* no metamethod found */
  return 0;  /* to avoid warnings */
}


int acornT_callorderiTM (acorn_State *L, const TValue *p1, int v2,
                       int flip, int isfloat, TMS event) {
  TValue aux; const TValue *p2;
  if (isfloat) {
    setfltvalue(&aux, cast_num(v2));
  }
  else
    setivalue(&aux, v2);
  if (flip) {  /* arguments were exchanged? */
    p2 = p1; p1 = &aux;  /* correct them */
  }
  else
    p2 = &aux;
  return acornT_callorderTM(L, p1, p2, event);
}


void acornT_adjustvarargs (acorn_State *L, int nfixparams, CallInfo *ci,
                         const Proto *p) {
  int i;
  int actual = cast_int(L->top - ci->func) - 1;  /* number of arguments */
  int nextra = actual - nfixparams;  /* number of extra arguments */
  ci->u.l.nextraargs = nextra;
  acornD_checkstack(L, p->maxstacksize + 1);
  /* copy function to the top of the stack */
  setobjs2s(L, L->top++, ci->func);
  /* move fixed parameters to the top of the stack */
  for (i = 1; i <= nfixparams; i++) {
    setobjs2s(L, L->top++, ci->func + i);
    setnilvalue(s2v(ci->func + i));  /* erase original parameter (for GC) */
  }
  ci->func += actual + 1;
  ci->top += actual + 1;
  acorn_assert(L->top <= ci->top && ci->top <= L->stack_last);
}


void acornT_getvarargs (acorn_State *L, CallInfo *ci, StkId where, int wanted) {
  int i;
  int nextra = ci->u.l.nextraargs;
  if (wanted < 0) {
    wanted = nextra;  /* get all extra arguments available */
    checkstackGCp(L, nextra, where);  /* ensure stack space */
    L->top = where + nextra;  /* next instruction will need top */
  }
  for (i = 0; i < wanted && i < nextra; i++)
    setobjs2s(L, where + i, ci->func - nextra + i);
  for (; i < wanted; i++)   /* complete required results with nil */
    setnilvalue(s2v(where + i));
}

