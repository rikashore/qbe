#include "../all.h"
#include "arm64.h"

typedef struct TClass Class;
typedef struct Insl Insl;

struct TClass {
	char inmem; /* 0, 1, or 2 for scalar argument on the stack */
	char ishfa;
	struct {
		char base;
		char size;
	} hfa;
	int align;
	uint64_t size;
};

struct Insl {
	Ins i;
	Insl *link;
};

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
typclass(Class *c, Typ *t)
{
	uint64_t sz;
	int al;

	sz = t->size;
	al = 1 << t->align;

	/* the ABI requires sizes to be rounded
	 * up to the nearest multiple of 8
	 */
	if (al < 8)
		al = 8;
	sz = (sz + al-1) & -al;

	c->size = sz;
	c->align = t->align;

	if (t->dark || sz > 16 || sz == 0) {
		/* large or unaligned structures are
		 * required to be passed in memory
		 */
		c->inmem = 1;
		return;
	}

	c->hfa.base = -1;
	c->ishfa = isfloatv(t, &c->hfa.base);
	if (c->hfa.base == Ks)
		c->hfa.size = t->size / 4;
	else
		c->hfa.size = t->size / 8;
}

static void
blit(Ref rstk, uint soff, Ref rsrc, uint sz, Fn *fn)
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

#if 0
static int
retr(Ref reg[4], Class *cret)
{
	int n, ca;

	if (cret->ishfa) {
		ca = cret->hfa.size << 2;
		for (n=0; n<cret->hfa.size; n++)
			reg[n] = TMP(V0+n);
	} else {
		ca = cret->size / 8;
		reg[0] = TMP(R0);
		reg[1] = TMP(R1);
	}
	return ca;
}
#endif

static void
selret(Blk *b, Fn *fn)
{
	int i, j, k, s, ca;
	Ref r, r0;
	Class cret;

	j = b->jmp.type;

	if (!isret(j) || j == Jret0)
		return;

	r0 = b->jmp.arg;
	b->jmp.type = Jret0;

	if (j == Jretc) {
		typclass(&cret, &typ[fn->retty]);
		if (cret.inmem) {
			assert(rtype(fn->retr) == RTmp);
			blit(fn->retr, 0, r0, cret.size, fn);
			ca = 0;
		} else if (cret.ishfa) {
			k = cret.hfa.base;
			s = 4 + 4 * (k == Kd);
			for (i=0; i<cret.hfa.size; i++) {
				r = newtmp("abi", k, fn);
				emit(Oload, k, TMP(V0+i), r, R);
				emit(Oadd, Kl, r, r0, getcon(i*s, fn));
			}
			ca = cret.hfa.size << 2;
		} else {
			if (cret.size > 8) {
				r = newtmp("abi", Kl, fn);
				emit(Oload, Kl, TMP(R1), r, R);
				emit(Oadd, Kl, r, r0, getcon(8, fn));
			}
			emit(Oload, Kl, TMP(R0), r0, R);
			ca = cret.size / 8;
		}
	} else {
		k = j - Jretw;
		if (KBASE(k) == 0) {
			emit(Ocopy, k, TMP(R0), r0, R);
			ca = 1;
		} else {
			emit(Ocopy, k, TMP(V0), r0, R);
			ca = 1 << 2;
		}
	}

	b->jmp.arg = CALL(ca);
}

static int
argsclass(Ins *i0, Ins *i1, Class *carg, Ref *env)
{
	int ngp, nfp, *pn;
	Class *c;
	Ins *i;

	ngp = 8;
	nfp = 8;
	for (i=i0, c=carg; i<i1; i++, c++)
		switch (i->op) {
		case Opar:
		case Oarg:
			if (KBASE(i->cls) == 0)
				pn = &ngp;
			else
				pn = &nfp;
			if (*pn > 0) {
				*pn -= 1;
				c->inmem = 0;
			} else
				c->inmem = 2;
			c->align = 3;
			c->size = 8;
			break;
		case Oparc:
		case Oargc:
			typclass(c, &typ[i->arg[0].val]);
			if (c->inmem)
				break;
			if (c->ishfa) {
				nfp -= c->hfa.size;
				pn = &nfp;
			} else {
				ngp -= c->size / 8;
				pn = &ngp;
			}
			if (*pn < 0) {
				*pn = 0;
				c->inmem = 1;
			}
			break;
		case Opare:
			*env = i->to;
			break;
		case Oarge:
			*env = i->arg[0];
			break;
		}

	return ((8-ngp) << 5) | ((8-nfp) << 9);
}

int a_rsave[] = {
	R0,  R1,  R2,  R3,  R4,  R5,  R6,  R7,
	R8,  R9,  R10, R11, R12, R13, R14, R15,
	IP0, IP1, R18,
	V0,  V1,  V2,  V3,  V4,  V5,  V6,  V7,
	V16, V17, V18, V19, V20, V21, V22, V23,
	V24, V25, V26, V27, V28, V29, V30,
	-1
};
int a_rclob[] = {
	R19, R20, R21, R22, R23, R24, R25, R26,
	R27, R28,
	V8,  V9,  V10, V11, V12, V13, V14, V15,
	-1
};
MAKESURE(arrays_ok,
	sizeof a_rsave == (NGPS+NFPS+1) * sizeof(int) &&
	sizeof a_rclob == (NCLR+1) * sizeof(int)
);

/* layout of call's second argument (RCall)
 *
 *  29   13    9    5   2  0
 *  |0...00|xxxx|xxxx|xxx|xx|                  range
 *             |    |   |  ` gp regs returned (0..2)
 *             |    |   ` fp regs returned    (0..4)
 *             |    ` gp regs passed          (0..8)
 *              ` fp regs passed              (0..8)
 */

bits
a_retregs(Ref r, int p[2])
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
a_argregs(Ref r, int p[2])
{
	bits b;
	int ngp, nfp;

	assert(rtype(r) == RCall);
	ngp = (r.val >> 5) & 15;
	nfp = (r.val >> 9) & 15;
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

#if 0

static Ref
rarg(int ty, int *ni, int *ns)
{
	if (KBASE(ty) == 0)
		return TMP(xv_rsave[(*ni)++]);
	else
		return TMP(XMM0 + (*ns)++);
}

static void
selcall(Fn *fn, Ins *i0, Ins *i1, RAlloc **rap)
{
	Ins *i;
	AClass *ac, *a, aret;
	int ca, ni, ns, al, varc, envc;
	uint stk, off;
	Ref r, r1, r2, reg[2], env;
	RAlloc *ra;

	env = R;
	ac = alloc((i1-i0) * sizeof ac[0]);

	if (!req(i1->arg[1], R)) {
		assert(rtype(i1->arg[1]) == RType);
		typclass(&aret, &typ[i1->arg[1].val]);
		ca = argsclass(i0, i1, ac, Oarg, &aret, &env);
	} else
		ca = argsclass(i0, i1, ac, Oarg, 0, &env);

	for (stk=0, a=&ac[i1-i0]; a>ac;)
		if ((--a)->inmem) {
			if (a->align > 4)
				err("sysv abi requires alignments of 16 or less");
			stk += a->size;
			if (a->align == 4)
				stk += stk & 15;
		}
	stk += stk & 15;
	if (stk) {
		r = getcon(-(int64_t)stk, fn);
		emit(Osalloc, Kl, R, r, R);
	}

	if (!req(i1->arg[1], R)) {
		if (aret.inmem) {
			/* get the return location from eax
			 * it saves one callee-save reg */
			r1 = newtmp("abi", Kl, fn);
			emit(Ocopy, Kl, i1->to, TMP(RAX), R);
			ca += 1;
		} else {
			if (aret.size > 8) {
				r = newtmp("abi", Kl, fn);
				aret.ref[1] = newtmp("abi", aret.cls[1], fn);
				emit(Ostorel, 0, R, aret.ref[1], r);
				emit(Oadd, Kl, r, i1->to, getcon(8, fn));
			}
			aret.ref[0] = newtmp("abi", aret.cls[0], fn);
			emit(Ostorel, 0, R, aret.ref[0], i1->to);
			ca += retr(reg, &aret);
			if (aret.size > 8)
				emit(Ocopy, aret.cls[1], aret.ref[1], reg[1], R);
			emit(Ocopy, aret.cls[0], aret.ref[0], reg[0], R);
			r1 = i1->to;
		}
		/* allocate return pad */
		ra = alloc(sizeof *ra);
		/* specific to NAlign == 3 */
		al = aret.align >= 2 ? aret.align - 2 : 0;
		ra->i = (Ins){Oalloc+al, r1, {getcon(aret.size, fn)}, Kl};
		ra->link = (*rap);
		*rap = ra;
	} else {
		ra = 0;
		if (KBASE(i1->cls) == 0) {
			emit(Ocopy, i1->cls, i1->to, TMP(RAX), R);
			ca += 1;
		} else {
			emit(Ocopy, i1->cls, i1->to, TMP(XMM0), R);
			ca += 1 << 2;
		}
	}
	envc = !req(R, env);
	varc = i1->op == Ovacall;
	if (varc && envc)
		err("sysv abi does not support variadic env calls");
	ca |= (varc | envc) << 12;
	emit(Ocall, i1->cls, R, i1->arg[0], CALL(ca));
	if (envc)
		emit(Ocopy, Kl, TMP(RAX), env, R);
	if (varc)
		emit(Ocopy, Kw, TMP(RAX), getcon((ca >> 8) & 15, fn), R);

	ni = ns = 0;
	if (ra && aret.inmem)
		emit(Ocopy, Kl, rarg(Kl, &ni, &ns), ra->i.to, R); /* pass hidden argument */
	for (i=i0, a=ac; i<i1; i++, a++) {
		if (a->inmem)
			continue;
		r1 = rarg(a->cls[0], &ni, &ns);
		if (i->op == Oargc) {
			if (a->size > 8) {
				r2 = rarg(a->cls[1], &ni, &ns);
				r = newtmp("abi", Kl, fn);
				emit(Oload, a->cls[1], r2, r, R);
				emit(Oadd, Kl, r, i->arg[1], getcon(8, fn));
			}
			emit(Oload, a->cls[0], r1, i->arg[1], R);
		} else
			emit(Ocopy, i->cls, r1, i->arg[0], R);
	}

	if (!stk)
		return;

	r = newtmp("abi", Kl, fn);
	for (i=i0, a=ac, off=0; i<i1; i++, a++) {
		if (!a->inmem)
			continue;
		if (i->op == Oargc) {
			if (a->align == 4)
				off += off & 15;
			blit(r, off, i->arg[1], a->size, fn);
		} else {
			r1 = newtmp("abi", Kl, fn);
			emit(Ostorel, 0, R, i->arg[0], r1);
			emit(Oadd, Kl, r1, r, getcon(off, fn));
		}
		off += a->size;
	}
	emit(Osalloc, Kl, r, getcon(stk, fn), R);
}

static int
selpar(Fn *fn, Ins *i0, Ins *i1)
{
	AClass *ac, *a, aret;
	Ins *i;
	int ni, ns, s, al, fa;
	Ref r, env;

	env = R;
	ac = alloc((i1-i0) * sizeof ac[0]);
	curi = &insb[NIns];
	ni = ns = 0;

	if (fn->retty >= 0) {
		typclass(&aret, &typ[fn->retty]);
		fa = argsclass(i0, i1, ac, Opar, &aret, &env);
	} else
		fa = argsclass(i0, i1, ac, Opar, 0, &env);

	for (i=i0, a=ac; i<i1; i++, a++) {
		if (i->op != Oparc || a->inmem)
			continue;
		if (a->size > 8) {
			r = newtmp("abi", Kl, fn);
			a->ref[1] = newtmp("abi", Kl, fn);
			emit(Ostorel, 0, R, a->ref[1], r);
			emit(Oadd, Kl, r, i->to, getcon(8, fn));
		}
		a->ref[0] = newtmp("abi", Kl, fn);
		emit(Ostorel, 0, R, a->ref[0], i->to);
		/* specific to NAlign == 3 */
		al = a->align >= 2 ? a->align - 2 : 0;
		emit(Oalloc+al, Kl, i->to, getcon(a->size, fn), R);
	}

	if (fn->retty >= 0 && aret.inmem) {
		r = newtmp("abi", Kl, fn);
		emit(Ocopy, Kl, r, rarg(Kl, &ni, &ns), R);
		fn->retr = r;
	}

	for (i=i0, a=ac, s=4; i<i1; i++, a++) {
		switch (a->inmem) {
		case 1:
			if (a->align > 4)
				err("sysv abi requires alignments of 16 or less");
			if (a->align == 4)
				s = (s+3) & -4;
			fn->tmp[i->to.val].slot = -s;
			s += a->size / 4;
			continue;
		case 2:
			emit(Oload, i->cls, i->to, SLOT(-s), R);
			s += 2;
			continue;
		}
		r = rarg(a->cls[0], &ni, &ns);
		if (i->op == Oparc) {
			emit(Ocopy, Kl, a->ref[0], r, R);
			if (a->size > 8) {
				r = rarg(a->cls[1], &ni, &ns);
				emit(Ocopy, Kl, a->ref[1], r, R);
			}
		} else
			emit(Ocopy, i->cls, i->to, r, R);
	}

	if (!req(R, env))
		emit(Ocopy, Kl, env, TMP(RAX), R);

	return fa | (s*4)<<12;
}

static Blk *
split(Fn *fn, Blk *b)
{
	Blk *bn;

	++fn->nblk;
	bn = blknew();
	bn->nins = &insb[NIns] - curi;
	idup(&bn->ins, curi, bn->nins);
	curi = &insb[NIns];
	bn->visit = ++b->visit;
	snprintf(bn->name, NString, "%s.%d", b->name, b->visit);
	bn->loop = b->loop;
	bn->link = b->link;
	b->link = bn;
	return bn;
}

static void
chpred(Blk *b, Blk *bp, Blk *bp1)
{
	Phi *p;
	uint a;

	for (p=b->phi; p; p=p->link) {
		for (a=0; p->blk[a]!=bp; a++)
			assert(a+1<p->narg);
		p->blk[a] = bp1;
	}
}

void
selvaarg(Fn *fn, Blk *b, Ins *i)
{
	Ref loc, lreg, lstk, nr, r0, r1, c4, c8, c16, c, ap;
	Blk *b0, *bstk, *breg;
	int isint;

	c4 = getcon(4, fn);
	c8 = getcon(8, fn);
	c16 = getcon(16, fn);
	ap = i->arg[0];
	isint = KBASE(i->cls) == 0;

	/* @b [...]
	       r0 =l add ap, (0 or 4)
	       nr =l loadsw r0
	       r1 =w cultw nr, (48 or 176)
	       jnz r1, @breg, @bstk
	   @breg
	       r0 =l add ap, 16
	       r1 =l loadl r0
	       lreg =l add r1, nr
	       r0 =w add nr, (8 or 16)
	       r1 =l add ap, (0 or 4)
	       storew r0, r1
	   @bstk
	       r0 =l add ap, 8
	       lstk =l loadl r0
	       r1 =l add lstk, 8
	       storel r1, r0
	   @b0
	       %loc =l phi @breg %lreg, @bstk %lstk
	       i->to =(i->cls) load %loc
	*/

	loc = newtmp("abi", Kl, fn);
	emit(Oload, i->cls, i->to, loc, R);
	b0 = split(fn, b);
	b0->jmp = b->jmp;
	b0->s1 = b->s1;
	b0->s2 = b->s2;
	if (b->s1)
		chpred(b->s1, b, b0);
	if (b->s2 && b->s2 != b->s1)
		chpred(b->s2, b, b0);

	lreg = newtmp("abi", Kl, fn);
	nr = newtmp("abi", Kl, fn);
	r0 = newtmp("abi", Kw, fn);
	r1 = newtmp("abi", Kl, fn);
	emit(Ostorew, Kw, R, r0, r1);
	emit(Oadd, Kl, r1, ap, isint ? CON_Z : c4);
	emit(Oadd, Kw, r0, nr, isint ? c8 : c16);
	r0 = newtmp("abi", Kl, fn);
	r1 = newtmp("abi", Kl, fn);
	emit(Oadd, Kl, lreg, r1, nr);
	emit(Oload, Kl, r1, r0, R);
	emit(Oadd, Kl, r0, ap, c16);
	breg = split(fn, b);
	breg->jmp.type = Jjmp;
	breg->s1 = b0;

	lstk = newtmp("abi", Kl, fn);
	r0 = newtmp("abi", Kl, fn);
	r1 = newtmp("abi", Kl, fn);
	emit(Ostorel, Kw, R, r1, r0);
	emit(Oadd, Kl, r1, lstk, c8);
	emit(Oload, Kl, lstk, r0, R);
	emit(Oadd, Kl, r0, ap, c8);
	bstk = split(fn, b);
	bstk->jmp.type = Jjmp;
	bstk->s1 = b0;

	b0->phi = alloc(sizeof *b0->phi);
	*b0->phi = (Phi){
		.cls = Kl, .to = loc,
		.narg = 2,
		.blk = {bstk, breg},
		.arg = {lstk, lreg},
	};
	r0 = newtmp("abi", Kl, fn);
	r1 = newtmp("abi", Kw, fn);
	b->jmp.type = Jjnz;
	b->jmp.arg = r1;
	b->s1 = breg;
	b->s2 = bstk;
	c = getcon(isint ? 48 : 176, fn);
	emit(Ocmpw+Ciult, Kw, r1, nr, c);
	emit(Oloadsw, Kl, nr, r0, R);
	emit(Oadd, Kl, r0, ap, isint ? CON_Z : c4);
}

void
selvastart(Fn *fn, int fa, Ref ap)
{
	Ref r0, r1;
	int gp, fp, sp;

	gp = ((fa >> 4) & 15) * 8;
	fp = 48 + ((fa >> 8) & 15) * 16;
	sp = fa >> 12;
	r0 = newtmp("abi", Kl, fn);
	r1 = newtmp("abi", Kl, fn);
	emit(Ostorel, Kw, R, r1, r0);
	emit(Oadd, Kl, r1, TMP(RBP), getcon(-176, fn));
	emit(Oadd, Kl, r0, ap, getcon(16, fn));
	r0 = newtmp("abi", Kl, fn);
	r1 = newtmp("abi", Kl, fn);
	emit(Ostorel, Kw, R, r1, r0);
	emit(Oadd, Kl, r1, TMP(RBP), getcon(sp, fn));
	emit(Oadd, Kl, r0, ap, getcon(8, fn));
	r0 = newtmp("abi", Kl, fn);
	emit(Ostorew, Kw, R, getcon(fp, fn), r0);
	emit(Oadd, Kl, r0, ap, getcon(4, fn));
	emit(Ostorew, Kw, R, getcon(gp, fn), ap);
}

void
xv_abi(Fn *fn)
{
	Blk *b;
	Ins *i, *i0, *ip;
	RAlloc *ral;
	int n, fa;

	for (b=fn->start; b; b=b->link)
		b->visit = 0;

	/* lower parameters */
	for (b=fn->start, i=b->ins; i-b->ins<b->nins; i++)
		if (!ispar(i->op))
			break;
	fa = selpar(fn, b->ins, i);
	n = b->nins - (i - b->ins) + (&insb[NIns] - curi);
	i0 = alloc(n * sizeof(Ins));
	ip = icpy(ip = i0, curi, &insb[NIns] - curi);
	ip = icpy(ip, i, &b->ins[b->nins] - i);
	b->nins = n;
	b->ins = i0;

	/* lower calls, returns, and vararg instructions */
	ral = 0;
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
				selcall(fn, i0, i, &ral);
				i = i0;
				break;
			case Ovastart:
				selvastart(fn, fa, i->arg[0]);
				break;
			case Ovaarg:
				selvaarg(fn, b, i);
				break;
			case Oarg:
			case Oargc:
				die("unreachable");
			}
		if (b == fn->start)
			for (; ral; ral=ral->link)
				emiti(ral->i);
		b->nins = &insb[NIns] - curi;
		idup(&b->ins, curi, b->nins);
	} while (b != fn->start);

	if (debug['A']) {
		fprintf(stderr, "\n> After ABI lowering:\n");
		printfn(fn, stderr);
	}
}

#endif
