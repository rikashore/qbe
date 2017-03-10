#include "../all.h"
#include "arm64.h"

static void
fixarg(Ref *pr, int k, Fn *fn)
{
	Ref r0, r1;
	int rt;

	r0 = *pr;
	rt = rtype(r0);
	if (rt == -1 || rt != RCon)
		return;
	if (KBASE(k) != 0)
		die("TODO 1");
	r1 = newtmp("isel", k, fn);
	emit(Ocopy, k, r1, r0, R);
	*pr = r1;
}

static void
sel(Ins i, Fn *fn)
{
	Ref *iarg;

	emiti(i);
	iarg = curi->arg; /* fixarg() can change curi */
	fixarg(&iarg[0], argcls(&i, 0), fn);
	fixarg(&iarg[1], argcls(&i, 1), fn);
}

static void
seljmp(Blk *b, Fn *fn)
{
	Ref r;
	Ins *i, *ir, ic;
	int ck, cc;

	switch (b->jmp.type) {
	default:
		die("TODO 2");
	case Jret0:
	case Jjmp:
		return;
	case Jjnz:
		break;
	}
	ic.op = Oacmp;
	ic.to = R;
	r = b->jmp.arg;
	b->jmp.arg = R;
	ir = 0;
	i = &b->ins[b->nins];
	while (i > b->ins)
		if (req((--i)->to, r)) {
			ir = i;
			break;
		}
	if (ir && iscmp(ir->op, &ck, &cc)) {
		ic.cls = ck;
		memcpy(ic.arg, ir->arg, sizeof ic.arg);
		sel(ic, fn);
		b->jmp.type = Jjf + cc;
		if (fn->tmp[r.val].nuse == 1)
			*ir = (Ins){.op = Onop};
	}
	else {
		ic.cls = Kw;
		ic.arg[0] = r;
		ic.arg[0] = CON_Z;
		sel(ic, fn);
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
