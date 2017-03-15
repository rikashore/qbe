#include "../all.h"
#include "arm64.h"

enum Imm {
	Iother,
	Iplo12,
	Iphi12,
	Iplo24,
	Inlo12,
	Inhi12,
	Inlo24
};

static enum Imm
imm(Con *c, int w, int64_t *pn)
{
	int64_t n;
	int i;

	if (c->type != CBits)
		return Iother;
	n = c->bits.i;
	if (!w)
		n = (int32_t)n;
	i = Iplo12;
	if (n < 0) {
		i = Inlo12;
		n = -n;
	}
	*pn = n;
	if ((n & 0x000fff) == n)
		return i;
	if ((n & 0xfff000) == n)
		return i + 1;
	if ((n & 0xffffff) == n)
		return i + 2;
	return Iother;
}

int
a_logimm(uint64_t x, int k)
{
	uint64_t n;

	if (KWIDE(k))
		x = (x & 0xffffffff) | x << 32;
	if (x & 1)
		x = ~x;
	if (x == 0)
		return 0;
	if (x == 0xaaaaaaaaaaaaaaaa)
		return 1;
	n = x & 0xf;
	if (0x1111111111111111 * n == x)
		goto Check;
	n = x & 0xff;
	if (0x0101010101010101 * n == x)
		goto Check;
	n = x & 0xffff;
	if (0x0001000100010001 * n == x)
		goto Check;
	n = x & 0xffffffff;
	if (0x0000000100000001 * n == x)
		goto Check;
	n = x;
Check:
	return (n & (n + (n & -n))) == 0;
}

static void
fixarg(Ref *pr, int k, Fn *fn)
{
	Ref r0, r1;
	int rt;

	r0 = *pr;
	rt = rtype(r0);
	if (rt == -1 || rt != RCon)
		return;
	assert(KBASE(k) == 0 && "TODO 1");
	r1 = newtmp("isel", k, fn);
	emit(Ocopy, k, r1, r0, R);
	*pr = r1;
}

static int
selcmp(Ref arg[2], int k, Fn *fn)
{
	Ref r, *iarg;
	Con *c;
	int swap, cmp, fix;
	int64_t n;

	assert(KBASE(k) == 0 && "TODO 3");
	swap = rtype(arg[0]) == RCon;
	if (swap) {
		r = arg[1];
		arg[1] = arg[0];
		arg[0] = r;
	}
	fix = 1;
	cmp = Oacmp;
	r = arg[1];
	if (rtype(r) == RCon) {
		c = &fn->con[r.val];
		switch (imm(c, k == Kl, &n)) {
		default:
			break;
		case Iplo12:
		case Iphi12:
			fix = 0;
			break;
		case Inlo12:
		case Inhi12:
			cmp = Oacmn;
			r = getcon(n, fn);
			fix = 0;
			break;
		}
	}
	emit(cmp, k, R, arg[0], r);
	iarg = curi->arg;
	fixarg(&iarg[0], k, fn);
	if (fix)
		fixarg(&iarg[1], k, fn);
	return swap;
}

static void
sel(Ins i, Fn *fn)
{
	Ref *iarg;
	Ins *i0;
	int ck, cc;

	if (iscmp(i.op, &ck, &cc)) {
		emit(Oflag, i.cls, i.to, R, R);
		i0 = curi;
		if (selcmp(i.arg, ck, fn))
			i0->op += cmpop(cc);
		else
			i0->op += cc;
	} else {
		emiti(i);
		iarg = curi->arg; /* fixarg() can change curi */
		fixarg(&iarg[0], argcls(&i, 0), fn);
		fixarg(&iarg[1], argcls(&i, 1), fn);
	}
}

static void
seljmp(Blk *b, Fn *fn)
{
	Ref r;
	Ins *i, *ir;
	int ck, cc, use;

	switch (b->jmp.type) {
	default:
		assert(0 && "TODO 2");
		break;
	case Jret0:
	case Jjmp:
		return;
	case Jjnz:
		break;
	}
	r = b->jmp.arg;
	use = -1;
	b->jmp.arg = R;
	ir = 0;
	i = &b->ins[b->nins];
	while (i > b->ins)
		if (req((--i)->to, r)) {
			use = fn->tmp[r.val].nuse;
			ir = i;
			break;
		}
	if (ir && use == 1
	&& iscmp(ir->op, &ck, &cc)) {
		if (selcmp(ir->arg, ck, fn))
			cc = cmpop(cc);
		b->jmp.type = Jjf + cc;
		*ir = (Ins){.op = Onop};
	}
	else {
		selcmp((Ref[]){r, CON_Z}, Kw, fn);
		b->jmp.type = Jjfine;
	}
}

void
a_isel(Fn *fn)
{
	Blk *b;
	Ins *i;

	for (b=fn->start; b; b=b->link) {
		curi = &insb[NIns];
		seljmp(b, fn);
		for (i=&b->ins[b->nins]; i!=b->ins;)
			sel(*--i, fn);
		b->nins = &insb[NIns] - curi;
		idup(&b->ins, curi, b->nins);
	}

	if (debug['I']) {
		fprintf(stderr, "\n> After instruction selection:\n");
		printfn(fn, stderr);
	}
}
