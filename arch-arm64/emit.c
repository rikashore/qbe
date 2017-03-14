#include "../all.h"
#include "arm64.h"

#define CMP(X) \
	X(Cieq,"eq") \
	X(Cine,"ne") \
	X(Cisge,"ge") \
	X(Cisgt,"gt") \
	X(Cisle,"le") \
	X(Cislt,"lt") \
	X(Ciuge,"cs") \
	X(Ciugt,"hi") \
	X(Ciule,"ls") \
	X(Ciult,"cc") \
	X(NCmpI+Cfeq,"eq") \
	X(NCmpI+Cfge,"ge") \
	X(NCmpI+Cfgt,"gt") \
	X(NCmpI+Cfle,"ls") \
	X(NCmpI+Cflt,"mi") \
	X(NCmpI+Cfne,"nz") \
	X(NCmpI+Cfo,"vc") \
	X(NCmpI+Cfuo,"vs")

enum {
	Ki = -1, /* matches Kw and Kl */
	Ka = -2, /* matches all classes */
};

static struct {
	short op;
	short cls;
	char *asm;
} omap[] = {
	{ Oadd,  Ki, "add %=, %0, %1" },
	{ Osub,  Ki, "sub %=, %0, %1" },
	{ Oacmp, Ki, "cmp %0, %1" },
	{ Oacmn, Ki, "cmn %0, %1" },
	{ Ocopy, Ka, "mov %=, %1" },
#define X(c) \
	{ Oflag+c, Ki, "cset %=, " #c },
#undef X
	{ NOp, 0, 0 }
};

static char *
rname(int r, int k)
{
	static char buf[4];

	if (r == SP) {
		assert(k == Kl);
		sprintf(buf, "sp");
	}
	else if (R0 <= r && r <= LR)
		switch (k) {
		default: die("invalid class");
		case Kw: sprintf(buf, "w%d", r-R0); break;
		case Kl: sprintf(buf, "x%d", r-R0); break;
		}
	else if (V0 <= r && r <= V30)
		switch (k) {
		default: die("invalid class");
		case Ks: sprintf(buf, "s%d", r-V0); break;
		case Kd: sprintf(buf, "d%d", r-V0); break;
		}
	else
		die("invalid register");
	return buf;
}

static void
emitf(char *s, Ins *i, Fn *fn, FILE *f)
{
	Ref r;
	int k, c;
	Con *pc;
	unsigned n;

	k = i->cls;
	fputc('\t', f);
Next:
	while ((c = *s++) != '%')
		if ( !c) {
			fputc('\n', f);
			return;
		} else
			fputc(c, f);
	switch ((c = *s++)) {
	default:
		die("invalid escape");
	case '=':
	case '0':
		r = c == '=' ? i->to : i->arg[0];
		assert(isreg(r));
		fputs(rname(r.val, k), f);
		break;
	case '1':
		r = i->arg[1];
		switch (rtype(r)) {
		default:
			die("invalid second argument");
		case RTmp:
			assert(isreg(r));
			fputs(rname(r.val, k), f);
			break;
		case RCon:
			pc = &fn->con[r.val];
			assert(pc->type == CBits);
			n = pc->bits.i;
			if (n & 0xfff000)
				fprintf(f, "#%u, lsl 12", n>>12);
			else
				fprintf(f, "#%u", n);
			break;
		}
		break;
	}
	goto Next;
}

static void
emitins(Ins *i, Fn *fn, FILE *f)
{
	int o, sh;
	Con *pc;
	int64_t n;
	char *rn;

	switch (i->op) {
	default:
	Table:
		/* most instructions are just pulled out of
		 * the table omap[], some special cases are
		 * detailed below */
		for (o=0;; o++) {
			/* this linear search should really be a binary
			 * search */
			if (omap[o].op == NOp)
				die("no match for %s(%d)", opdesc[i->op].name, i->cls);
			if (omap[o].op == i->op)
			if (omap[o].cls == i->cls || omap[o].cls == Ka
			|| (omap[o].cls == Ki && KBASE(i->cls) == 0))
				break;
		}
		emitf(omap[o].asm, i, fn, f);
		break;
	case Onop:
		break;
	case Ocopy:
		if (rtype(i->arg[0]) != RCon)
			goto Table;
		pc = &fn->con[i->arg[0].val];
		switch (pc->type) {
		default:
			die("todo");
		case CBits:
			n = pc->bits.i;
			if (!KWIDE(i->cls))
				n = (int32_t)n;
			rn = rname(i->to.val, i->cls);
			fprintf(f, "\tmov %s, %d\n",
				rn, (int)(n & 0xffff));
			for (sh=16; n>>=16; sh+=16) {
				if ((!KWIDE(i->cls) && sh == 32)
				|| sh == 64)
					break;
				fprintf(f, "\tmovk %s, %d, lsl %d\n",
					rn, (int)(n & 0xffff), sh);
			}
			break;
		}
		break;
	}
}

void
a_emitfn(Fn *fn, FILE *f)
{
	static char *ctoa[] = {
	#define X(c, s) [c] = s,
		CMP(X)
	#undef X
	};
	static int id0;
	int c, lbl;
	Blk *b, *s;
	Ins *i;

	fprintf(f, ".text\n");
	if (fn->export)
		fprintf(f, ".global %s\n", fn->name);
	fprintf(f,
		"%s:\n",
		fn->name
	);
	for (lbl=0, b=fn->start; b; b=b->link) {
		if (lbl || b->npred > 1)
			fprintf(f, ".bb%d:\n", id0+b->id);
		for (i=b->ins; i!=&b->ins[b->nins]; i++)
			emitins(i, fn, f);
		lbl = 1;
		switch (b->jmp.type) {
		case Jret0:
			fprintf(f, "\tret\n");
			break;
		case Jjmp:
		Jmp:
			if (b->s1 != b->link)
				fprintf(f, "\tb .bb%d\n", id0+b->s1->id);
			else
				lbl = 0;
			break;
		default:
			c = b->jmp.type - Jjf;
			if (c < 0 || c > NCmp)
				die("unhandled jump %d", b->jmp.type);
			if (b->link == b->s2) {
				s = b->s1;
				b->s1 = b->s2;
				b->s2 = s;
			} else
				c = cmpneg(c);
			fprintf(f, "\tb%s .bb%d\n", ctoa[c], id0+b->s2->id);
			goto Jmp;
		}
	}
	id0 += fn->nblk;
}