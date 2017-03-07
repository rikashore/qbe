#include "../all.h"
#include "x64.h"

Target T_x64_sysv = {
	.gpr0 = RAX,
	.ngpr = NGPR,
	.fpr0 = XMM0,
	.nfpr = NFPR,
	.rglob = BIT(RBP) | BIT(RSP),
	.nrglob = 2,
	.rsave = xv_rsave,
	.nrsave = {NGPS, NFPS},
	.retregs = xv_retregs,
	.argregs = xv_argregs,
};
