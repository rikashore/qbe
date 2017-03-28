#include "all.h"

#define CMP(X) \
	X(Cieq,       "eq") \
	X(Cine,       "ne") \
	X(Cisge,      "ge") \
	X(Cisgt,      "gt") \
	X(Cisle,      "le") \
	X(Cislt,      "lt") \
	X(Ciuge,      "cs") \
	X(Ciugt,      "hi") \
	X(Ciule,      "ls") \
	X(Ciult,      "cc") \
	X(NCmpI+Cfeq, "eq") \
	X(NCmpI+Cfge, "ge") \
	X(NCmpI+Cfgt, "gt") \
	X(NCmpI+Cfle, "ls") \
	X(NCmpI+Cflt, "mi") \
	X(NCmpI+Cfne, "ne") \
	X(NCmpI+Cfo,  "vc") \
	X(NCmpI+Cfuo, "vs")

enum {
	Ki = -1, /* matches Kw and Kl */
	Ka = -2, /* matches all classes */
};

static struct {
	short op;
	short cls;
	char *asm;
} omap[] = {
	{ Oadd,    Ki, "add %=, %0, %1" },
	{ Osub,    Ki, "sub %=, %0, %1" },
	{ Oand,    Ki, "and %=, %0, %1" },
	{ Oor,     Ki, "orr %=, %0, %1" },
	{ Oxor,    Ki, "eor %=, %0, %1" },
	{ Osar,    Ki, "asr %=, %0, %1" },
	{ Oshr,    Ki, "lsr %=, %0, %1" },
	{ Oshl,    Ki, "lsl %=, %0, %1" },
	{ Omul,    Ki, "mul %=, %0, %1" },
	{ Odiv,    Ki, "sdiv %=, %0, %1" },
	{ Oudiv,   Ki, "udiv %=, %0, %1" },
	{ Ocopy,   Ka, "mov %=, %0" },
	{ Oloadsb, Ki, "ldrsb %=, %M0" },
	{ Oloadub, Ki, "ldrb %=, %M0" },
	{ Oloadsh, Ki, "ldrsh %=, %M0" },
	{ Oloaduh, Ki, "ldrh %=, %M0" },
	{ Oloadsw, Kw, "ldr %=, %M0" },
	{ Oloadsw, Kl, "ldrsw %=, %M0" },
	{ Oloaduw, Ki, "ldr %W=, %M0" },
	{ Oload,   Ki, "ldr %=, %M0" },
	{ Oextsb,  Ki, "sxtb %=, %W0" },
	{ Oextub,  Ki, "uxtb %W=, %W0" },
	{ Oextsh,  Ki, "sxth %=, %W0" },
	{ Oextuh,  Ki, "uxth %W=, %W0" },
	{ Oextsw,  Ki, "sxtw %L=, %W0" },
	{ Oextuw,  Ki, "mov %W=, %W0" },
	{ Ostoreb, Kw, "strb %W0, %M1" },
	{ Ostoreh, Kw, "strh %W0, %M1" },
	{ Ostorew, Kw, "str %W0, %M1" },
	{ Ostorel, Kw, "str %L0, %M1" },
	{ Ocall,   Kw, "blr %L0" },

	{ Oacmp,   Ki, "cmp %0, %1" },
	{ Oacmn,   Ki, "cmn %0, %1" },

#define X(c, str) \
	{ Oflag+c, Ki, "cset %=, " str },
	CMP(X)
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

static uint
slot(int s, Fn *fn)
{
	struct { int i:29; } x;

	x.i = s;
	assert(x.i <= fn->slot);
	assert(x.i >= 0);
	return 16 + 4 * (fn->slot - x.i);
}

static void
emitf(char *s, Ins *i, Fn *fn, FILE *f)
{
	Ref r;
	int k, c;
	Con *pc;
	unsigned n, sp;

	fputc('\t', f);

	sp = 0;
	for (;;) {
		k = i->cls;
		while ((c = *s++) != '%')
			if (c == ' ' && !sp) {
				fputc('\t', f);
				sp = 1;
			} else if ( !c) {
				fputc('\n', f);
				return;
			} else
				fputc(c, f);
	Switch:
		switch ((c = *s++)) {
		default:
			die("invalid escape");
		case 'W':
			k = Kw;
			goto Switch;
		case 'L':
			k = Kl;
			goto Switch;
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
				n = pc->bits.i;
				assert(pc->type == CBits);
				if (n & 0xfff000)
					fprintf(f, "#%u, lsl #12", n>>12);
				else
					fprintf(f, "#%u", n);
				break;
			}
			break;
		case 'M':
			c = *s++;
			assert(c == '0' || c == '1');
			r = i->arg[c - '0'];
			assert(isreg(r) && "TODO emit non reg addresses");
			fprintf(f, "[%s]", rname(r.val, Kl));
			break;
		}
	}
}

static void
loadcon(Con *c, int r, int k, FILE *f)
{
	char *rn, off[32];
	int64_t n;
	int w, sh;

	w = KWIDE(k);
	rn = rname(r, k);
	n = c->bits.i;
	if (c->type == CAddr) {
		rn = rname(r, Kl);
		if (n)
			sprintf(off, "+%"PRIi64, n);
		else
			off[0] = 0;
		fprintf(f, "\tadrp\t%s, %s%s\n",
			rn, c->label, off);
		fprintf(f, "\tadd\t%s, %s, #:lo12:%s%s\n",
			rn, rn, c->label, off);
		return;
	}
	assert(c->type == CBits);
	if (!w)
		n = (int32_t)n;
	if ((n | 0xffff) == -1 || arm64_logimm(n, k)) {
		fprintf(f, "\tmov\t%s, #%"PRIi64"\n", rn, n);
	} else {
		fprintf(f, "\tmov\t%s, #%d\n",
			rn, (int)(n & 0xffff));
		for (sh=16; n>>=16; sh+=16) {
			if ((!w && sh == 32) || sh == 64)
				break;
			fprintf(f, "\tmovk\t%s, #0x%x, lsl #%d\n",
				rn, (unsigned)(n & 0xffff), sh);
		}
	}
}

static void
emitins(Ins *i, Fn *fn, FILE *f)
{
	int o;

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
				die("no match for %s(%c)",
					opdesc[i->op].name, "wlsd"[i->cls]);
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
		if (req(i->to, i->arg[0]))
			break;
		if (rtype(i->arg[0]) != RCon)
			goto Table;
		loadcon(&fn->con[i->arg[0].val], i->to.val, i->cls, f);
		break;
	case Oaddr:
		assert(rtype(i->arg[0]) == RSlot);
		fprintf(f, "\tadd\t%s, sp, #%u\n",
			rname(i->to.val, Kl), slot(i->arg[0].val, fn)
		);
		break;
	}
}

static uint64_t
framesz(Fn *fn)
{
	int *r;
	uint o;
	uint64_t f;

	for (o=0, r=arm64_rclob; *r>=0; r++)
		o += 1 & (fn->reg >> *r);
	f = fn->slot;
	f = (f + 3) & -4;
	o += o & 1;
	return 4*f + 8*o;
}

/*

  Stack-frame layout:

  +=============+  ^
  | callee-save |  |
  |  registers  |  |
  +-------------+  |
  |    ...      |  |
  |   locals    |  | framesz(fn)
  |    ...      |  |
  +-------------+  |
  |    ...      |  |
  | spill slots |  |
  |    ...      |  |
  +-------------+  v
  |  saved x29  |
  |  saved x30  |
  +=============+ <- x29, sp

*/

void
arm64_emitfn(Fn *fn, FILE *f)
{
	static char *ctoa[] = {
	#define X(c, s) [c] = s,
		CMP(X)
	#undef X
	};
	static int id0;
	int c, lbl, *r;
	uint64_t fs, o;
	Blk *b, *s;
	Ins *i;

	fprintf(f, ".text\n");
	if (fn->export)
		fprintf(f, ".globl %s\n", fn->name);
	fprintf(f, "%s:\n", fn->name);

	fs = framesz(fn);
	if (fs + 16 > 512)
		fprintf(f,
			"\tsub\tsp, sp, #%" PRIu64 "\n"
			"\tstp\tx29, x30, [sp, -16]!\n",
			fs
		);
	else
		fprintf(f,
			"\tstp\tx29, x30, [sp, -%" PRIu64 "]!\n",
			fs + 16
		);
	fputs("\tadd\tx29, sp, 0\n", f);
	for (o=fs+16, r=arm64_rclob; *r>=0; r++)
		if (fn->reg & BIT(*r))
			fprintf(f,
				"\tstr\t%s, [sp, %" PRIu64 "]\n",
				rname(*r, Kl), o -= 8
			);

	for (lbl=0, b=fn->start; b; b=b->link) {
		if (lbl || b->npred > 1)
			fprintf(f, ".L%d:\n", id0+b->id);
		for (i=b->ins; i!=&b->ins[b->nins]; i++)
			emitins(i, fn, f);
		lbl = 1;
		switch (b->jmp.type) {
		case Jret0:
			for (o=fs+16, r=arm64_rclob; *r>=0; r++)
				if (fn->reg & BIT(*r))
					fprintf(f,
						"\tldr\t%s, [sp, %" PRIu64 "]\n",
						rname(*r, Kl), o -= 8
					);
			if (fs + 16 > 504)
				fprintf(f,
					"\tldp\tx29, x30, [sp], 16\n"
					"\tadd\tsp, sp, #%" PRIu64 "\n",
					fs
				);
			else
				fprintf(f,
					"\tldp\tx29, x30, [sp], %" PRIu64 "\n",
					fs + 16
				);
			fprintf(f, "\tret\n");
			break;
		case Jjmp:
		Jmp:
			if (b->s1 != b->link)
				fprintf(f, "\tb\t.L%d\n", id0+b->s1->id);
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
			fprintf(f, "\tb%s\t.L%d\n", ctoa[c], id0+b->s2->id);
			goto Jmp;
		}
	}
	id0 += fn->nblk;
}
