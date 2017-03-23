#include "all.h"

Target T_arm64 = {
	.gpr0 = R0,
	.ngpr = NGPR,
	.fpr0 = V0,
	.nfpr = NFPR,
	.rglob = BIT(FP) | BIT(SP),
	.nrglob = 2,
	.rsave = arm64_rsave,
	.nrsave = {NGPS, NFPS},
	.retregs = arm64_retregs,
	.argregs = arm64_argregs,
};
