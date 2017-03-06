#include "../all.h"

Target Tx64 = {
	.gpr0 = RAX,
	.ngpr = RSP - RAX + 1,
	.fpr0 = XMM0,
	.nfpr = XMM14 - XMM0 + 1, /* XMM15 is reserved */
	.rglob = BIT(RBP) | BIT(RSP),
	.nrglob = 2, /* RBP and RSP */
	.nrsave = {NISave, NFSave},
	.retregs = retregs,
	.argregs = argregs,
};
