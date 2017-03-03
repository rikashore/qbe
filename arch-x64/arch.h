#ifndef PORTING

enum X64Reg {
	RAX = RXX+1, /* caller-save */
	RCX,
	RDX,
	RSI,
	RDI,
	R8,
	R9,
	R10,
	R11,

	RBX, /* callee-save */
	R12,
	R13,
	R14,
	R15,

	RBP, /* globally live */
	RSP,
#define RGLOB (BIT(RBP)|BIT(RSP))

	XMM0, /* sse */
	XMM1,
	XMM2,
	XMM3,
	XMM4,
	XMM5,
	XMM6,
	XMM7,
	XMM8,
	XMM9,
	XMM10,
	XMM11,
	XMM12,
	XMM13,
	XMM14,
	XMM15,
};

MAKESURE(X64Reg_not_tmp, XMM15 < (int)Tmp0);

enum {
	NRGlob = 2, // ok
	NIReg = R15 - RAX + 1 + NRGlob, // ok
	NFReg = XMM14 - XMM0 + 1, /* XMM15 is reserved */ // ok
	NISave = R11 - RAX + 1,
	NFSave = NFReg,
	NRSave = NISave + NFSave,
	NRClob = R15 - RBX + 1,
};

#endif

extern Target Tx64;
