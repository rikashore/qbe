#include "all.h"

Target T_amd64_sysv = {
	.gpr0 = RAX,
	.ngpr = NGPR,
	.fpr0 = XMM0,
	.nfpr = NFPR,
	.rglob = BIT(RBP) | BIT(RSP),
	.nrglob = 2,
	.rsave = amd64_sysv_rsave,
	.nrsave = {NGPS, NFPS},
	.retregs = amd64_sysv_retregs,
	.argregs = amd64_sysv_argregs,
};
