#include "../all.h"

int rsave[] = {
	RDI, RSI, RDX, RCX, R8, R9, R10, R11, RAX,
	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7,
	XMM8, XMM9, XMM10, XMM11, XMM12, XMM13, XMM14, -1
};
int rclob[] = {RBX, R12, R13, R14, R15, -1};

int x64_sysv_rsave[] = {
	RDI, RSI, RDX, RCX, R8, R9, R10, R11, RAX,
	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7,
	XMM8, XMM9, XMM10, XMM11, XMM12, XMM13, XMM14, -1
};
int x64_sysv_rclob[] = {RBX, R12, R13, R14, R15, -1};

MAKESURE(xvrsave_has_correct_size, sizeof x64_sysv_rsave == (NRSave+1) * sizeof(int));
MAKESURE(xvrclob_has_correct_size, sizeof x64_sysv_rclob == (NRClob+1) * sizeof(int));


Target Tx64 = {
	.gpr0 = RAX,
	.ngpr = RSP - RAX + 1,
	.fpr0 = XMM0,
	.nfpr = XMM14 - XMM0 + 1, /* reserve XMM15 */
	.rglob = BIT(RBP) | BIT(RSP),
	.nrglob = 2,
	.rsave = x64_sysv_rsave,
	.rclob = x64_sysv_rclob,
	.nrsave = {NISave, NFSave},
	.retregs = retregs,
	.argregs = argregs,
};
