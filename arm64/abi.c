#include "all.h"

typedef struct Class_ Class;
typedef struct Insl Insl;

enum {
	Cstk = 1, /* pass on the stack */
	Cptr = 2, /* replaced by a pointer */
};

struct Class_ {
	char class;
	char ishfa;
	struct {
		char base;
		uchar size;
	} hfa;
	uint size;
	Typ *t;
	uchar nreg;
	uchar ngp;
	uchar nfp;
	int reg[4];
	int cls[4];
};

struct Insl {
	Ins i;
	Insl *link;
};

static int gpreg[12] = {R0, R1, R2, R3, R4, R5, R6, R7};
static int fpreg[12] = {V0, V1, V2, V3, V4, V5, V6, V7};

/* layout of call's second argument (RCall)
 *
 *  29   13    9    5   2  0
 *  |0.00|x|xxxx|xxxx|xxx|xx|                  range
 *        |    |    |   |  ` gp regs returned (0..2)
 *        |    |    |   ` fp regs returned    (0..4)
 *        |    |    ` gp regs passed          (0..8)
 *        |     ` fp regs passed              (0..8)
 *        ` is x8 used                        (0..1)
 */

static int
isfloatv(Typ *t, char *cls)
{
	Field *f;
	uint n;

	for (n=0; n<t->nunion; n++)
		for (f=t->fields[n]; f->type != FEnd; f++)
			switch (f->type) {
			case Fs:
				if (*cls == Kd)
					return 0;
				*cls = Ks;
				break;
			case Fd:
				if (*cls == Ks)
					return 0;
				*cls = Kd;
				break;
			case FTyp:
				if (isfloatv(&typ[f->len], cls))
					break;
			default:
				return 0;
			}
	return 1;
}

static void
typclass(Class *c, Typ *t, int *gp, int *fp)
{
	uint64_t sz;
	uint n;

	sz = (t->size + 7) & -8;
	c->t = t;
	c->class = 0;
	c->ngp = 0;
	c->nfp = 0;

	if (t->align > 4)
		err("alignments larger than 16 are not supported");

	if (t->dark || sz > 16 || sz == 0) {
		/* large structs are replaced by a
		 * pointer to some caller-allocated
		 * memory */
		c->class |= Cptr;
		c->size = 8;
		return;
	}

	c->size = sz;
	c->hfa.base = Kx;
	c->ishfa = isfloatv(t, &c->hfa.base);
	c->hfa.size = t->size/(KWIDE(c->hfa.base) ? 8 : 4);

	if (c->ishfa)
		for (n=0; n<c->hfa.size; n++, c->nfp++) {
			c->reg[n] = *fp++;
			c->cls[n] = c->hfa.base;
		}
	else
		for (n=0; n<sz/8; n++, c->ngp++) {
			c->reg[n] = *gp++;
			c->cls[n] = Kl;
		}

	c->nreg = n;
}

static void
blit8(Ref rstk, uint soff, Ref rsrc, uint sz, Fn *fn)
{
	Ref r, r1;
	uint boff;

	/* it's an impolite blit, we might go across the end
	 * of the source object a little bit... */
	for (boff=0; sz>0; sz-=8, soff+=8, boff+=8) {
		r = newtmp("abi", Kl, fn);
		r1 = newtmp("abi", Kl, fn);
		emit(Ostorel, 0, R, r, r1);
		emit(Oadd, Kl, r1, rstk, getcon(soff, fn));
		r1 = newtmp("abi", Kl, fn);
		emit(Oload, Kl, r, r1, R);
		emit(Oadd, Kl, r1, rsrc, getcon(boff, fn));
	}
}

static void
sttmps(Ref tmp[], int cls[], int n, Ref mem, Fn *fn)
{
	static int st[] = {
		[Kw] = Ostorew, [Kl] = Ostorel,
		[Ks] = Ostores, [Kd] = Ostored
	};
	int i;
	uint64_t off;
	Ref r;

	assert(n <= 4);
	off = 0;
	for (i=0; i<n; i++) {
		tmp[n] = newtmp("abi", cls[i], fn);
		r = newtmp("abi", Kl, fn);
		emit(st[cls[i]], 0, R, tmp[n], r);
		emit(Oadd, Kl, r, mem, getcon(off, fn));
		off += KWIDE(cls[i]) ? 8 : 4;
	}
}

static void
ldregs(int reg[], int cls[], int n, Ref mem, Fn *fn)
{
	int i;
	uint64_t off;
	Ref r;

	off = 0;
	for (i=0; i<n; i++) {
		r = newtmp("abi", Kl, fn);
		emit(Oload, cls[i], TMP(reg[i]), r, R);
		emit(Oadd, Kl, r, mem, getcon(off, fn));
		off += KWIDE(cls[i]) ? 8 : 4;
	}
}

static void
selret(Blk *b, Fn *fn)
{
	int j, k, cty;
	Ref r;
	Class cr;

	j = b->jmp.type;

	if (!isret(j) || j == Jret0)
		return;

	r = b->jmp.arg;
	b->jmp.type = Jret0;

	if (j == Jretc) {
		typclass(&cr, &typ[fn->retty], gpreg, fpreg);
		cty = (cr.nfp << 2) | cr.ngp;
		if (cr.class & Cptr) {
			assert(rtype(fn->retr) == RTmp);
			blit8(fn->retr, 0, r, cr.t->size, fn);
		} else
			ldregs(cr.reg, cr.cls, cr.nreg, r, fn);
	} else {
		k = j - Jretw;
		if (KBASE(k) == 0) {
			emit(Ocopy, k, TMP(R0), r, R);
			cty = 1;
		} else {
			emit(Ocopy, k, TMP(V0), r, R);
			cty = 1 << 2;
		}
	}

	b->jmp.arg = CALL(cty);
}

static int
argsclass(Ins *i0, Ins *i1, Class *carg, Ref *env)
{
	int ngp, nfp, *gp, *fp;
	Class *c;
	Ins *i;

	gp = gpreg;
	fp = fpreg;
	ngp = 8;
	nfp = 8;
	for (i=i0, c=carg; i<i1; i++, c++)
		switch (i->op) {
		case Opar:
		case Oarg:
			c->cls[0] = i->cls;
			c->size = 8;
			if (KBASE(i->cls) == 0 && ngp > 0) {
				ngp--;
				c->reg[0] = *gp++;
				break;
			}
			if (KBASE(i->cls) == 1 && nfp > 0) {
				nfp--;
				c->reg[0] = *fp++;
				break;
			}
			c->class |= Cstk;
			break;
		case Oparc:
		case Oargc:
			typclass(c, &typ[i->arg[0].val], gp, fp);
			if (c->class & Cptr) {
				if (ngp > 0) {
					ngp--;
					c->reg[0] = *gp++;
					c->cls[0] = Kl;
					break;
				}
			} else if (c->ngp <= ngp) {
				if (c->nfp <= nfp) {
					ngp -= c->ngp;
					nfp -= c->nfp;
					gp += c->ngp;
					fp += c->nfp;
					break;
				} else
					nfp = 0;
			} else
				ngp = 0;
			c->class |= Cstk;
			break;
		case Opare:
			*env = i->to;
			break;
		case Oarge:
			*env = i->arg[0];
			break;
		}

	return ((gp-gpreg) << 5) | ((fp-fpreg) << 9);
}

bits
arm64_retregs(Ref r, int p[2])
{
	bits b;
	int ngp, nfp;

	assert(rtype(r) == RCall);
	ngp = r.val & 3;
	nfp = (r.val >> 2) & 7;
	if (p) {
		p[0] = ngp;
		p[1] = nfp;
	}
	b = 0;
	while (ngp--)
		b |= BIT(R0+ngp);
	while (nfp--)
		b |= BIT(V0+nfp);
	return b;
}

bits
arm64_argregs(Ref r, int p[2])
{
	bits b;
	int ngp, nfp, x8;

	assert(rtype(r) == RCall);
	ngp = (r.val >> 5) & 15;
	nfp = (r.val >> 9) & 15;
	x8 = (r.val >> 13) & 1;
	if (p) {
		p[0] = ngp + x8;
		p[1] = nfp;
	}
	b = 0;
	while (ngp--)
		b |= BIT(R0+ngp);
	while (nfp--)
		b |= BIT(V0+nfp);
	return b | ((bits)x8 << R8);
}

static void
stkblob(Ref r, Class *c, Fn *fn, Insl **ilp)
{
	Insl *il;
	int al;

	il = alloc(sizeof *il);
	al = c->t->align - 2; /* NAlign == 3 */
	if (al < 0)
		al = 0;
	il->i = (Ins){
		Oalloc + al, r,
		{getcon(c->t->size, fn)}, Kl
	};
	il->link = *ilp;
	*ilp = il;
}

static void
selcall(Fn *fn, Ins *i0, Ins *i1, Insl **ilp)
{
	Ins *i;
	Class *ca, *c, cr;
	int cty, envc;
	uint n;
	uint64_t stk, off;
	Ref r, rstk, env, tmp[4];

	env = R;
	ca = alloc((i1-i0) * sizeof ca[0]);
	cty = argsclass(i0, i1, ca, &env);

	stk = 0;
	for (i=i0, c=ca; i<i1; i++, c++) {
		if (c->class & Cptr) {
			i->arg[0] = newtmp("abi", Kl, fn);
			stkblob(i->arg[0], c, fn, ilp);
			i->op = Oarg;
		}
		if (c->class & Cstk)
			stk += c->size;
	}
	stk += stk & 15;
	rstk = getcon(stk, fn);
	if (stk)
		emit(Oadd, Kl, TMP(SP), TMP(SP), rstk);

	if (!req(i1->arg[1], R)) {
		stkblob(i1->to, &cr, fn, ilp);
		typclass(&cr, &typ[i1->arg[1].val], gpreg, fpreg);
		cty |= (cr.nfp << 2) | cr.ngp;
		if (cr.class & Cptr)
			cty |= 1 << 13;
		else {
			sttmps(tmp, c->cls, c->nreg, i1->to, fn);
			for (n=0; n<c->nreg; n++) {
				r = TMP(c->reg[n]);
				emit(Ocopy, c->cls[n], tmp[n], r, R);
			}
		}
	} else {
		if (KBASE(i1->cls) == 0) {
			emit(Ocopy, i1->cls, i1->to, TMP(R0), R);
			cty |= 1;
		} else {
			emit(Ocopy, i1->cls, i1->to, TMP(V0), R);
			cty |= 1 << 2;
		}
	}

	envc = !req(R, env);
	if (envc)
		die("todo (arm abi): env calls");
	emit(Ocall, 0, R, i1->arg[0], CALL(cty));

	if (cty & (1 << 13))
		/* struct return argument */
		emit(Ocopy, Kl, TMP(R8), i1->to, R);

	for (i=i0, c=ca; i<i1; i++, c++) {
		if ((c->class & Cstk) != 0)
			continue;
		if (i->op != Oargc)
			emit(Ocopy, *c->cls, TMP(*c->reg), i->arg[0], R);
		else
			ldregs(c->reg, c->cls, c->nreg, i->arg[1], fn);
	}

	off = 0;
	for (i=i0, c=ca; i<i1; i++, c++) {
		if ((c->class & Cstk) == 0)
			continue;
		if (i->op != Oargc) {
			r = newtmp("abi", Kl, fn);
			emit(Ostorel, 0, R, i->arg[0], r);
			emit(Oadd, Kl, r, TMP(SP), getcon(off, fn));
		} else
			blit8(TMP(SP), off, i->arg[1], c->size, fn);
		off += c->size;
	}
	if (stk)
		emit(Osub, Kl, TMP(SP), TMP(SP), rstk);

	for (i=i0, c=ca; i<i1; i++, c++)
		if (c->class & Cptr)
			blit8(i->arg[0], 0, i->arg[1], c->t->size, fn);
}

static int
selpar(Fn *fn, Ins *i0, Ins *i1)
{
	Class *ca, *c, cr;
	Insl *il;
	Ins *i;
	int n, s;
	Ref r, env, tmp[16], *t;

	env = R;
	ca = alloc((i1-i0) * sizeof ca[0]);
	curi = &insb[NIns];

	argsclass(i0, i1, ca, &env);

	il = 0;
	t = tmp;
	for (i=i0, c=ca; i<i1; i++, c++) {
		if (i->op != Oparc || (c->class & (Cptr|Cstk)))
			continue;
		sttmps(t, c->cls, c->nreg, i->to, fn);
		stkblob(i->to, c, fn, &il);
		t += c->nreg;
	}
	for (; il; il=il->link)
		emiti(il->i);

	if (fn->retty >= 0) {
		typclass(&cr, &typ[fn->retty], gpreg, fpreg);
		if (cr.class & Cptr) {
			fn->retr = newtmp("abi", Kl, fn);
			emit(Ocopy, Kl, fn->retr, TMP(R8), R);
		}
	}

	t = tmp;
	for (i=i0, c=ca, s=4; i<i1; i++, c++) {
		if (i->op == Oparc
		&& (c->class & Cptr) == 0) {
			if (c->class & Cstk) {
				fn->tmp[i->to.val].slot = -s;
				s += c->size / 4;
			} else
				for (n=0; n<c->nreg; n++) {
					r = TMP(c->reg[n]);
					emit(Ocopy, Kd, *t++, r, R);
				}
		} else if (c->class & Cstk) {
			emit(Oload, *c->cls, i->to, SLOT(-s), R);
			s += 2;
		} else {
			r = TMP(*c->reg);
			emit(Ocopy, *c->cls, i->to, r, R);
		}
	}

	if (!req(R, env))
		die("todo (arm abi): env calls");

	return 0;
}

void
arm64_abi(Fn *fn)
{
	Blk *b;
	Ins *i, *i0, *ip;
	Insl *il;
	int n;

	for (b=fn->start; b; b=b->link)
		b->visit = 0;

	/* lower parameters */
	for (b=fn->start, i=b->ins; i-b->ins<b->nins; i++)
		if (!ispar(i->op))
			break;
	selpar(fn, b->ins, i);
	n = b->nins - (i - b->ins) + (&insb[NIns] - curi);
	i0 = alloc(n * sizeof(Ins));
	ip = icpy(ip = i0, curi, &insb[NIns] - curi);
	ip = icpy(ip, i, &b->ins[b->nins] - i);
	b->nins = n;
	b->ins = i0;

	/* lower calls, returns, and vararg instructions */
	il = 0;
	b = fn->start;
	do {
		if (!(b = b->link))
			b = fn->start; /* do it last */
		if (b->visit)
			continue;
		curi = &insb[NIns];
		selret(b, fn);
		for (i=&b->ins[b->nins]; i!=b->ins;)
			switch ((--i)->op) {
			default:
				emiti(*i);
				break;
			case Ocall:
			case Ovacall:
				for (i0=i; i0>b->ins; i0--)
					if (!isarg((i0-1)->op))
						break;
				selcall(fn, i0, i, &il);
				i = i0;
				break;
			case Ovastart:
			case Ovaarg:
				die("todo (arm abi): vastart and vaarg");
				break;
			case Oarg:
			case Oargc:
				die("unreachable");
			}
		if (b == fn->start)
			for (; il; il=il->link)
				emiti(il->i);
		b->nins = &insb[NIns] - curi;
		idup(&b->ins, curi, b->nins);
	} while (b != fn->start);

	if (debug['A']) {
		fprintf(stderr, "\n> After ABI lowering:\n");
		printfn(fn, stderr);
	}
}
