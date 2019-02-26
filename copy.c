#include "all.h"

static int
iscopy(Ins *i, Ref r, Tmp *tmp)
{
	static bits extcpy[] = {
		[WFull] = 0,
		[Wsb] = BIT(Wsb) | BIT(Wsh) | BIT(Wsw),
		[Wub] = BIT(Wub) | BIT(Wuh) | BIT(Wuw),
		[Wsh] = BIT(Wsh) | BIT(Wsw),
		[Wuh] = BIT(Wuh) | BIT(Wuw),
		[Wsw] = BIT(Wsw),
		[Wuw] = BIT(Wuw),
	};
	bits b;
	Tmp *t;

	if (i->op == Ocopy)
		return 1;
	if (!isext(i->op) || rtype(r) != RTmp)
		return 0;
	if (i->op == Oextsw || i->op == Oextuw)
	if (i->cls == Kw)
		return 1;

	t = &tmp[r.val];
	assert(KBASE(t->cls) == 0);
	if (i->cls == Kl && t->cls == Kw)
		return 0;
	b = extcpy[t->width];
	return (BIT(Wsb + (i->op-Oextsb)) & b) != 0;
}

static Ref
copyof(Ref r, Ref *cpy)
{
	if (rtype(r) == RTmp && !req(cpy[r.val], R))
		return cpy[r.val];
	return r;
}

static void
phisimpl(Phi *p, Ref r, Ref *cpy, Use ***pstk, BSet *ts, BSet *as, Tmp *tmp)
{
	Use **stk, *u, *u1;
	uint nstk, a;
	int t;
	Ref r1;

	bszero(ts);
	bszero(as);
	stk = *pstk;
	nstk = 1;
	stk[0] = &(Use){.type = UPhi, .u.phi = p};
	while (nstk) {
		u = stk[--nstk];
		if (u->type == UIns && iscopy(u->u.ins, r, tmp)) {
			p = &(Phi){.narg = 0};
			t = u->u.ins->to.val;
		}
		else if (u->type == UPhi) {
			p = u->u.phi;
			t = p->to.val;
		}
		else
			continue;
		if (bshas(ts, t))
			continue;
		bsset(ts, t);
		for (a=0; a<p->narg; a++) {
			r1 = copyof(p->arg[a], cpy);
			if (req(r1, r))
				continue;
			if (rtype(r1) != RTmp)
				return;
			bsset(as, r1.val);
		}
		u = tmp[t].use;
		u1 = &u[tmp[t].nuse];
		vgrow(pstk, nstk+(u1-u));
		stk = *pstk;
		for (; u<u1; u++)
			stk[nstk++] = u;
	}
	bsdiff(as, ts);
	if (!bscount(as))
		for (t=0; bsiter(ts, &t); t++)
			cpy[t] = r;
}

static void
subst(Ref *pr, Ref *cpy)
{
	assert(rtype(*pr) != RTmp || !req(cpy[pr->val], R));
	*pr = copyof(*pr, cpy);
}

void
copy(Fn *fn)
{
	BSet ts[1], as[1];
	Use **stk;
	Phi *p, **pp;
	Ins *i;
	Blk *b;
	uint n, a;
	Ref *cpy, r;
	int t;

	bsinit(ts, fn->ntmp);
	bsinit(as, fn->ntmp);
	cpy = emalloc(fn->ntmp * sizeof cpy[0]);
	stk = vnew(10, sizeof stk[0], Pheap);
	for (n=0; n<fn->nblk; n++) {
		b = fn->rpo[n];
		for (p=b->phi; p; p=p->link) {
			assert(rtype(p->to) == RTmp);
			if (!req(cpy[p->to.val], R))
				continue;
			r = R;
			for (a=0; a<p->narg; a++)
				if (p->blk[a]->id < n)
					r = copyof(p->arg[a], cpy);
			assert(!req(r, R));
			cpy[p->to.val] = p->to;
			phisimpl(p, r, cpy, &stk, ts, as, fn->tmp);
		}
		for (i=b->ins; i<&b->ins[b->nins]; i++) {
			assert(rtype(i->to) <= RTmp);
			if (!req(cpy[i->to.val], R))
				continue;
			r = copyof(i->arg[0], cpy);
			if (iscopy(i, r, fn->tmp))
				cpy[i->to.val] = r;
			else
				cpy[i->to.val] = i->to;
		}
	}
	for (b=fn->start; b; b=b->link) {
		for (pp=&b->phi; (p=*pp);) {
			r = cpy[p->to.val];
			if (!req(r, p->to)) {
				*pp = p->link;
				continue;
			}
			for (a=0; a<p->narg; a++)
				subst(&p->arg[a], cpy);
			pp=&p->link;
		}
		for (i=b->ins; i<&b->ins[b->nins]; i++) {
			r = cpy[i->to.val];
			if (!req(r, i->to)) {
				*i = (Ins){.op = Onop};
				continue;
			}
			subst(&i->arg[0], cpy);
			subst(&i->arg[1], cpy);
		}
		subst(&b->jmp.arg, cpy);
	}
	if (debug['C']) {
		fprintf(stderr, "\n> Copy information:");
		for (t=Tmp0; t<fn->ntmp; t++) {
			if (req(cpy[t], R)) {
				fprintf(stderr, "\n%10s not seen!",
					fn->tmp[t].name);
			}
			else if (!req(cpy[t], TMP(t))) {
				fprintf(stderr, "\n%10s copy of ",
					fn->tmp[t].name);
				printref(cpy[t], fn, stderr);
			}
		}
		fprintf(stderr, "\n\n> After copy elimination:\n");
		printfn(fn, stderr);
	}
	vfree(stk);
	free(cpy);
}
