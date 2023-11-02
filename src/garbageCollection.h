/*
** $Id: garbageCollection.h $
** Garbage Collector
** See Copyright Notice in venom.h
*/

#ifndef garbageCollection_h
#define garbageCollection_h


#include "object.h"
#include "state.h"

/*
** Collectable objects may have one of three colors: white, which means
** the object is not marked; gray, which means the object is marked, but
** its references may be not marked; and black, which means that the
** object and all its references are marked.  The main invariant of the
** garbage collector, while marking objects, is that a black object can
** never point to a white one. Moreover, any gray object must be in a
** "gray list" (gray, grayagain, weak, allweak, ephemeron) so that it
** can be visited again before finishing the collection cycle. (Open
** upvalues are an exception to this rule.)  These lists have no meaning
** when the invariant is not being enforced (e.g., sweep phase).
*/


/*
** Possible states of the Garbage Collector
*/
#define GCSpropagate	0
#define GCSenteratomic	1
#define GCSatomic	2
#define GCSswpallgarbageCollection	3
#define GCSswpfinobj	4
#define GCSswptobefnz	5
#define GCSswpend	6
#define GCScallfin	7
#define GCSpause	8


#define issweepphase(g)  \
	(GCSswpallgarbageCollection <= (g)->gcstate && (g)->gcstate <= GCSswpend)


/*
** macro to tell when main invariant (white objects cannot point to black
** ones) must be kept. During a collection, the sweep
** phase may break the invariant, as objects turned white may point to
** still-black objects. The invariant is restored when sweep ends and
** all objects are white again.
*/

#define keepinvariant(g)	((g)->gcstate <= GCSatomic)


/*
** some useful bit tricks
*/
#define resetbits(x,m)		((x) &= cast_byte(~(m)))
#define setbits(x,m)		((x) |= (m))
#define testbits(x,m)		((x) & (m))
#define bitmask(b)		(1<<(b))
#define bit2mask(b1,b2)		(bitmask(b1) | bitmask(b2))
#define l_setbit(x,b)		setbits(x, bitmask(b))
#define resetbit(x,b)		resetbits(x, bitmask(b))
#define testbit(x,b)		testbits(x, bitmask(b))


/*
** Layout for bit use in 'marked' field. First three bits are
** used for object "age" in generational mode. Last bit is used
** by tests.
*/
#define WHITE0BIT	3  /* object is white (type 0) */
#define WHITE1BIT	4  /* object is white (type 1) */
#define BLACKBIT	5  /* object is black */
#define FINALIZEDBIT	6  /* object has been marked for finalization */

#define TESTBIT		7



#define WHITEBITS	bit2mask(WHITE0BIT, WHITE1BIT)


#define iswhite(x)      testbits((x)->marked, WHITEBITS)
#define isblack(x)      testbit((x)->marked, BLACKBIT)
#define isgray(x)  /* neither white nor black */  \
	(!testbits((x)->marked, WHITEBITS | bitmask(BLACKBIT)))

#define tofinalize(x)	testbit((x)->marked, FINALIZEDBIT)

#define otherwhite(g)	((g)->currentwhite ^ WHITEBITS)
#define isdeadm(ow,m)	((m) & (ow))
#define isdead(g,v)	isdeadm(otherwhite(g), (v)->marked)

#define changewhite(x)	((x)->marked ^= WHITEBITS)
#define nw2black(x)  \
	check_exp(!iswhite(x), l_setbit((x)->marked, BLACKBIT))

#define venomC_white(g)	cast_byte((g)->currentwhite & WHITEBITS)


/* object age in generational mode */
#define G_NEW		0	/* created in current cycle */
#define G_SURVIVAL	1	/* created in previous cycle */
#define G_OLD0		2	/* marked old by frw. barrier in this cycle */
#define G_OLD1		3	/* first full cycle as old */
#define G_OLD		4	/* really old object (not to be visited) */
#define G_TOUCHED1	5	/* old object touched this cycle */
#define G_TOUCHED2	6	/* old object touched in previous cycle */

#define AGEBITS		7  /* all age bits (111) */

#define getage(o)	((o)->marked & AGEBITS)
#define setage(o,a)  ((o)->marked = cast_byte(((o)->marked & (~AGEBITS)) | a))
#define isold(o)	(getage(o) > G_SURVIVAL)

#define changeage(o,f,t)  \
	check_exp(getage(o) == (f), (o)->marked ^= ((f)^(t)))


/* Default Values for GC parameters */
#define VENOMI_GENMAJORMUL         100
#define VENOMI_GENMINORMUL         20

/* wait memory to double before starting new cycle */
#define VENOMI_GCPAUSE    200

/*
** some gc parameters are stored divided by 4 to allow a maximum value
** up to 1023 in a 'lu_byte'.
*/
#define getgcparam(p)	((p) * 4)
#define setgcparam(p,v)	((p) = (v) / 4)

#define VENOMI_GCMUL      100

/* how much to allocate before next GC step (log2) */
#define VENOMI_GCSTEPSIZE 13      /* 8 KB */


/*
** Check whether the declared GC mode is generational. While in
** generational mode, the collector can Venom temporarily to incremental
** mode to improve performance. This is signaled by 'g->lastatomic != 0'.
*/
#define isdecGCmodegen(g)	(g->gckind == KGC_GEN || g->lastatomic != 0)


/*
** Control when GC is running:
*/
#define GCSTPUSR	1  /* bit true when GC stopped by user */
#define GCSTPGC		2  /* bit true when GC stopped by itself */
#define GCSTPCLS	4  /* bit true when closing Venom state */
#define gcrunning(g)	((g)->gcstp == 0)


/*
** Does one step of collection when debt becomes positive. 'pre'/'pos'
** allows some adjustments to be done only when needed. macro
** 'condchangemem' is used only for heavy tests (forcing a full
** GC cycle on every opportunity)
*/
#define venomC_condGC(L,pre,pos) \
	{ if (G(L)->GCdebt > 0) { pre; venomC_step(L); pos;}; \
	  condchangemem(L,pre,pos); }

/* more often than not, 'pre'/'pos' are empty */
#define venomC_checkGC(L)		venomC_condGC(L,(void)0,(void)0)


#define venomC_barrier(L,p,v) (  \
	(iscollectable(v) && isblack(p) && iswhite(gcvalue(v))) ?  \
	venomC_barrier_(L,obj2gco(p),gcvalue(v)) : cast_void(0))

#define venomC_barrierback(L,p,v) (  \
	(iscollectable(v) && isblack(p) && iswhite(gcvalue(v))) ? \
	venomC_barrierback_(L,p) : cast_void(0))

#define venomC_objbarrier(L,p,o) (  \
	(isblack(p) && iswhite(o)) ? \
	venomC_barrier_(L,obj2gco(p),obj2gco(o)) : cast_void(0))

VENOMI_FUNC void venomC_fix (venom_State *L, GCObject *o);
VENOMI_FUNC void venomC_freealobjects (venom_State *L);
VENOMI_FUNC void venomC_step (venom_State *L);
VENOMI_FUNC void venomC_runtistate (venom_State *L, int statesmask);
VENOMI_FUNC void venomC_fulgarbageCollection (venom_State *L, int isemergency);
VENOMI_FUNC GCObject *venomC_newobj (venom_State *L, int tt, size_t sz);
VENOMI_FUNC void venomC_barrier_ (venom_State *L, GCObject *o, GCObject *v);
VENOMI_FUNC void venomC_barrierback_ (venom_State *L, GCObject *o);
VENOMI_FUNC void venomC_checkfinalizer (venom_State *L, GCObject *o, Table *mt);
VENOMI_FUNC void venomC_changemode (venom_State *L, int newmode);


#endif
