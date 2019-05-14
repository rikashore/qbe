/* reassociation and simplification */
#include "all.h"

static int
matcharg(Ins *i, int64_t bits, int sel, Con *con)
{
	uint64_t mask;
	Ref r;
	int n;

	mask = KWIDE(i->cls) ? (uint64_t)-1 : (uint32_t)-1;
	for (n=0; n<2; n++) {
		r = i->arg[n];
		if ((1<<n) & sel)
		if (rtype(r) == RCon)
		if (con[r.val].type == CBits)
		if (((con[r.val].bits.i ^ bits) & mask) == 0)
			return 1 - n;
	}
	return -1;
}

/* preserves use */
void
simpl(Fn *fn)
{
	Blk *b;
	Ins *i;
	int n, sel;
	int64_t con;

	for (b=fn->start; b; b=b->link)
		for (i=b->ins; i<&b->ins[b->nins]; i++) {
			/* simplify instructions where one
			 * constant arg can make the result
			 * be the other arg */
			switch (i->op) {
			default: continue;
			case Oadd: con =  0; sel = 3; break;
			case Omul: con =  1; sel = 3; break;
			case Osub: con =  0; sel = 2; break;
			case Oudiv:
			case Odiv: con =  1; sel = 2; break;
			case Oor:
			case Oxor: con =  0; sel = 3; break;
			case Oand: con = -1; sel = 3; break;
			case Oshl:
			case Oshr:
			case Osar: con =  0; sel = 2; break;
			}
			n = matcharg(i, con, sel, fn->con);
			if (n >= 0) {
				i->op = Ocopy;
				i->arg[0] = i->arg[n];
				i->arg[1] = R;
			}
		}
				
}
