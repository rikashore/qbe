#include "../all.h"
#include "arm64.h"

int a_rsave[] = {
	R0,  R1,  R2,  R3,  R4,  R5,  R6,  R7,
	R8,  R9,  R10, R11, R12, R13, R14, R15,
	IP0, IP1, R18,
	V0,  V1,  V2,  V3,  V4,  V5,  V6,  V7,
	V16, V17, V18, V19, V20, V21, V22, V23,
	V24, V25, V26, V27, V28, V29, V30,
	-1
};
MAKESURE(rsave_ok, sizeof a_rsave == (NGPS+NFPS+1)*sizeof(int));

Target T_arm64 = {
	.gpr0 = R0,
	.ngpr = NGPR,
	.fpr0 = V0,
	.nfpr = NFPR,
	.rglob = BIT(FP) | BIT(SP),
	.nrglob = 2,
	.rsave = a_rsave,
	.nrsave = {NGPS, NFPS},
	.retregs = 0,
	.argregs = 0,
};
