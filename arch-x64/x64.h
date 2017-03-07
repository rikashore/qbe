enum Reg {
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

	NFPR = XMM14 - XMM0 + 1, /* reserve XMM15 */
	NGPR = RSP - RAX + 1,
	NGPS = R11 - RAX + 1,
	NFPS = NFPR,
	NCLR = R15 - RBX + 1,
};

MAKESURE(Reg_not_tmp, XMM15 < (int)Tmp0);

/* sysv.c (abi) */
extern int xv_rsave[];
extern int xv_rclob[];
bits xv_retregs(Ref, int[2]);
bits xv_argregs(Ref, int[2]);
void xv_abi(Fn *);

/* isel.c */
void x_isel(Fn *);

/* emit.c */
extern char *x_locprefix;
extern char *x_symprefix;
void x_emitfn(Fn *, FILE *);
void x_emitdat(Dat *, FILE *);
int x_stashfp(int64_t, int);
void x_emitfin(FILE *);
