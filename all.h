#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAKESURE(what, x) typedef char make_sure_##what[(x)?1:-1]
#define die(...) die_(__FILE__, __VA_ARGS__)

typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned long long bits;

typedef struct BSet BSet;
typedef struct Ref Ref;
typedef struct OpDesc OpDesc;
typedef struct Ins Ins;
typedef struct Phi Phi;
typedef struct Blk Blk;
typedef struct Use Use;
typedef struct Alias Alias;
typedef struct Tmp Tmp;
typedef struct Con Con;
typedef struct Addr Mem;
typedef struct Fn Fn;
typedef struct Typ Typ;
typedef struct Seg Seg;
typedef struct Dat Dat;
typedef struct Target Target;

enum {
	NString = 32,
	NPred   = 63,
	NIns    = 8192,
	NAlign  = 3,
	NSeg    = 32,
	NTyp    = 128,
	NBit    = CHAR_BIT * sizeof(bits),
};

struct Target {
	int gpr0;   /* first general purpose reg */
	int ngpr;
	int fpr0;   /* first floating point reg */
	int nfpr;
	bits rglob; /* globally live regs (e.g., sp, fp) */
	int nrglob;
	int *rsave; /* caller-save */
	int nrsave[2];
	bits (*retregs)(Ref, int[2]);
	bits (*argregs)(Ref, int[2]);
};

#define BIT(n) ((bits)1 << (n))

enum {
	RXX = 0,
	Tmp0 = NBit, /* first non-reg temporary */
};

struct BSet {
	uint nt;
	bits *t;
};

struct Ref {
	uint type:3;
	uint val:29;
};

enum {
	RTmp,
	RCon,
	RType,
	RSlot,
	RCall,
	RMem,
};

#define R        (Ref){0, 0}
#define TMP(x)   (Ref){RTmp, x}
#define CON(x)   (Ref){RCon, x}
#define CON_Z    CON(0)          /* reserved zero constant */
#define SLOT(x)  (Ref){RSlot, x}
#define TYPE(x)  (Ref){RType, x}
#define CALL(x)  (Ref){RCall, x}
#define MEM(x)   (Ref){RMem, x}

static inline int req(Ref a, Ref b)
{
	return a.type == b.type && a.val == b.val;
}

static inline int rtype(Ref r)
{
	if (req(r, R))
		return -1;
	return r.type;
}

static inline int isreg(Ref r)
{
	return rtype(r) == RTmp && r.val < Tmp0;
}

enum CmpI {
	Cieq,
	Cine,
	Cisge,
	Cisgt,
	Cisle,
	Cislt,
	Ciuge,
	Ciugt,
	Ciule,
	Ciult,
	NCmpI,
};

enum CmpF {
	Cfeq,
	Cfge,
	Cfgt,
	Cfle,
	Cflt,
	Cfne,
	Cfo,
	Cfuo,
	NCmpF,
	NCmp = NCmpI + NCmpF,
};

enum Op {
	Oxxx,
#define OP(op, x, y) O##op,
	#include "ops.h"
	NOp,
};

enum Jmp {
	Jxxx,
#define JMPS(X)                                 \
	X(ret0)   X(retw)   X(retl)   X(rets)   \
	X(retd)   X(retc)   X(jmp)    X(jnz)    \
	X(jfieq)  X(jfine)  X(jfisge) X(jfisgt) \
	X(jfisle) X(jfislt) X(jfiuge) X(jfiugt) \
	X(jfiule) X(jfiult) X(jffeq)  X(jffge)  \
	X(jffgt)  X(jffle)  X(jfflt)  X(jffne)  \
	X(jffo)   X(jffuo)
#define X(j) J##j,
	JMPS(X)
#undef X
	NJmp
};

enum {
	Ocmpw = Oceqw,
	Ocmpw1 = Ocultw,
	Ocmpl = Oceql,
	Ocmpl1 = Ocultl,
	Ocmps = Oceqs,
	Ocmps1 = Ocuos,
	Ocmpd = Oceqd,
	Ocmpd1 = Ocuod,
	Oalloc = Oalloc4,
	Oalloc1 = Oalloc16,
	Oflag = Oflagieq,
	Oflag1 = Oflagfuo,
	NPubOp = Onop,
	Jjf = Jjfieq,
	Jjf1 = Jjffuo,
};

#define isstore(o) (Ostoreb <= o && o <= Ostored)
#define isload(o) (Oloadsb <= o && o <= Oload)
#define isext(o) (Oextsb <= o && o <= Oextuw)
#define ispar(o) (Opar <= o && o <= Opare)
#define isarg(o) (Oarg <= o && o <= Oarge)
#define isret(j) (Jret0 <= j && j <= Jretc)

enum Class {
	Kx = -1, /* "top" class (see usecheck() and clsmerge()) */
	Kw,
	Kl,
	Ks,
	Kd
};

#define KWIDE(k) ((k)&1)
#define KBASE(k) ((k)>>1)

struct OpDesc {
	char *name;
	short argcls[2][4];
	uint cfold:1; /* can fold */
	uint sflag:1; /* sets the zero flag */
	uint lflag:1; /* leaves flags */
	int nmem;
};

struct Ins {
	uint op:30;
	Ref to;
	Ref arg[2];
	uint cls:2;
};

struct Phi {
	Ref to;
	Ref arg[NPred];
	Blk *blk[NPred];
	uint narg;
	int cls;
	Phi *link;
};

struct Blk {
	Phi *phi;
	Ins *ins;
	uint nins;
	struct {
		short type;
		Ref arg;
	} jmp;
	Blk *s1;
	Blk *s2;
	Blk *link;

	uint id;
	uint visit;

	Blk *idom;
	Blk *dom, *dlink;
	Blk **fron;
	uint nfron;

	Blk **pred;
	uint npred;
	BSet in[1], out[1], gen[1];
	int nlive[2];
	int loop;
	char name[NString];
};

struct Use {
	enum {
		UXXX,
		UPhi,
		UIns,
		UJmp,
	} type;
	int bid;
	union {
		Ins *ins;
		Phi *phi;
	} u;
};

enum {
	NoAlias,
	MayAlias,
	MustAlias
};

struct Alias {
	enum {
		ABot = 0,
		ALoc = 1, /* stack local */
		ACon = 2,
		AEsc = 3, /* stack escaping */
		ASym = 4,
		AUnk = 6,
	} type;
	Ref base;
	char label[NString];
	int64_t offset;
	Alias *slot;
};

struct Tmp {
	char name[NString];
	Use *use;
	uint ndef, nuse;
	uint cost;
	short slot; /* -1 for unset */
	short cls;
	struct {
		int r;
		bits m;
	} hint;
	int phi;
	Alias alias;
	enum {
		WFull,
		Wsb, /* must match Oload/Oext order */
		Wub,
		Wsh,
		Wuh,
		Wsw,
		Wuw
	} width;
	int visit;
};

struct Con {
	enum {
		CUndef,
		CBits,
		CAddr,
	} type;
	char label[NString];
	union {
		int64_t i;
		double d;
		float s;
	} bits;
	char flt; /* 1 to print as s, 2 to print as d */
	char local;
};

typedef struct Addr Addr;

struct Addr { /* x64 addressing */
	Con offset;
	Ref base;
	Ref index;
	int scale;
};

struct Fn {
	Blk *start;
	Tmp *tmp;
	Con *con;
	Mem *mem;
	int ntmp;
	int ncon;
	int nmem;
	uint nblk;
	int retty; /* index in typ[], -1 if no aggregate return */
	Ref retr;
	Blk **rpo;
	bits reg;
	int slot;
	char export;
	char vararg;
	char name[NString];
};

struct Typ {
	char name[NString];
	int dark;
	int align;
	size_t size;
	int nunion;
	struct Seg {
		enum {
			SEnd,
			SPad,
			SInt,
			SFlt,
			STyp,
		} type;
		uint len; /* index in typ[] for Styp */
	} (*seg)[NSeg+1];
};

struct Dat {
	enum {
		DStart,
		DEnd,
		DName,
		DAlign,
		DB,
		DH,
		DW,
		DL,
		DZ
	} type;
	union {
		int64_t num;
		double fltd;
		float flts;
		char *str;
		struct {
			char *nam;
			int64_t off;
		} ref;
	} u;
	char isref;
	char isstr;
	char export;
};

/* available targets */
extern Target T_x64_sysv;

/* main.c */
extern Target T;
extern char debug['Z'+1];

/* util.c */
typedef enum {
	Pheap, /* free() necessary */
	Pfn, /* discarded after processing the function */
} Pool;

extern Typ typ[NTyp];
extern Ins insb[NIns], *curi;
void die_(char *, char *, ...) __attribute__((noreturn));
void *emalloc(size_t);
void *alloc(size_t);
void freeall(void);
void emit(int, int, Ref, Ref, Ref);
void emiti(Ins);
void idup(Ins **, Ins *, ulong);
Ins *icpy(Ins *, Ins *, ulong);
void *vnew(ulong, size_t, Pool);
void vfree(void *);
void vgrow(void *, ulong);
int cmpop(int);
int cmpneg(int);
int clsmerge(short *, short);
int phicls(int, Tmp *);
Ref newtmp(char *, int, Fn *);
void chuse(Ref, int, Fn *);
Ref getcon(int64_t, Fn *);
void addcon(Con *, Con *);
void dumpts(BSet *, Tmp *, FILE *);

void bsinit(BSet *, uint);
void bszero(BSet *);
uint bscount(BSet *);
void bsset(BSet *, uint);
void bsclr(BSet *, uint);
void bscopy(BSet *, BSet *);
void bsunion(BSet *, BSet *);
void bsinter(BSet *, BSet *);
void bsdiff(BSet *, BSet *);
int bsequal(BSet *, BSet *);
int bsiter(BSet *, int *);

static inline int
bshas(BSet *bs, uint elt)
{
	assert(elt < bs->nt * NBit);
	return (bs->t[elt/NBit] & BIT(elt%NBit)) != 0;
}

/* parse.c */
extern OpDesc opdesc[NOp];
void parse(FILE *, char *, void (Dat *), void (Fn *));
void printfn(Fn *, FILE *);
void printref(Ref, Fn *, FILE *);
void err(char *, ...) __attribute__((noreturn));

/* cfg.c */
Blk *blknew(void);
void edgedel(Blk *, Blk **);
void fillpreds(Fn *);
void fillrpo(Fn *);
void filldom(Fn *);
int sdom(Blk *, Blk *);
int dom(Blk *, Blk *);
void fillfron(Fn *);
void loopiter(Fn *, void (*)(Blk *, Blk *));
void fillloop(Fn *);
void simpljmp(Fn *);

/* mem.c */
void memopt(Fn *);

/* alias.c */
void fillalias(Fn *);
int alias(Ref, int, Ref, int, int *, Fn *);
int escapes(Ref, Fn *);

/* load.c */
int loadsz(Ins *);
int storesz(Ins *);
void loadopt(Fn *);

/* ssa.c */
void filluse(Fn *);
void fillpreds(Fn *);
void fillrpo(Fn *);
void ssa(Fn *);
void ssacheck(Fn *);

/* simpl.c */
void simpl(Fn *);

/* copy.c */
void copy(Fn *);

/* fold.c */
void fold(Fn *);

/* live.c */
void liveon(BSet *, Blk *, Blk *);
void filllive(Fn *);

/* spill.c */
void fillcost(Fn *);
void spill(Fn *);

/* rega.c */
void rega(Fn *);
