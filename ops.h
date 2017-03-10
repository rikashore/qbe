#ifndef X64
	#define X64(NMemArgs, SetsZeroFlag, LeavesFlags)
#endif

#define T(a,b,c,d,e,f,g,h) {                          \
	{[Kw]=K##a, [Kl]=K##b, [Ks]=K##c, [Kd]=K##d}, \
	{[Kw]=K##e, [Kl]=K##f, [Ks]=K##g, [Kd]=K##h}  \
}


/*********************/
/* PUBLIC OPERATIONS */
/*********************/

/* Arithmetic and Bits */
OP(add,     T(w,l,s,d, w,l,s,d), 1) X64(2, 1, 0)
OP(sub,     T(w,l,s,d, w,l,s,d), 1) X64(2, 1, 0)
OP(div,     T(w,l,s,d, w,l,s,d), 1) X64(0, 0, 0)
OP(rem,     T(w,l,e,e, w,l,e,e), 1) X64(0, 0, 0)
OP(udiv,    T(w,l,e,e, w,l,e,e), 1) X64(0, 0, 0)
OP(urem,    T(w,l,e,e, w,l,e,e), 1) X64(0, 0, 0)
OP(mul,     T(w,l,s,d, w,l,s,d), 1) X64(2, 0, 0)
OP(and,     T(w,l,e,e, w,l,e,e), 1) X64(2, 1, 0)
OP(or,      T(w,l,e,e, w,l,e,e), 1) X64(2, 1, 0)
OP(xor,     T(w,l,e,e, w,l,e,e), 1) X64(2, 1, 0)
OP(sar,     T(w,l,e,e, w,w,e,e), 1) X64(1, 1, 0)
OP(shr,     T(w,l,e,e, w,w,e,e), 1) X64(1, 1, 0)
OP(shl,     T(w,l,e,e, w,w,e,e), 1) X64(1, 1, 0)

/* Comparisons */
OP(ceqw,    T(w,w,e,e, w,w,e,e), 1) X64(0, 1, 0)
OP(cnew,    T(w,w,e,e, w,w,e,e), 1) X64(0, 1, 0)
OP(csgew,   T(w,w,e,e, w,w,e,e), 1) X64(0, 1, 0)
OP(csgtw,   T(w,w,e,e, w,w,e,e), 1) X64(0, 1, 0)
OP(cslew,   T(w,w,e,e, w,w,e,e), 1) X64(0, 1, 0)
OP(csltw,   T(w,w,e,e, w,w,e,e), 1) X64(0, 1, 0)
OP(cugew,   T(w,w,e,e, w,w,e,e), 1) X64(0, 1, 0)
OP(cugtw,   T(w,w,e,e, w,w,e,e), 1) X64(0, 1, 0)
OP(culew,   T(w,w,e,e, w,w,e,e), 1) X64(0, 1, 0)
OP(cultw,   T(w,w,e,e, w,w,e,e), 1) X64(0, 1, 0)

OP(ceql,    T(l,l,e,e, l,l,e,e), 1) X64(0, 1, 0)
OP(cnel,    T(l,l,e,e, l,l,e,e), 1) X64(0, 1, 0)
OP(csgel,   T(l,l,e,e, l,l,e,e), 1) X64(0, 1, 0)
OP(csgtl,   T(l,l,e,e, l,l,e,e), 1) X64(0, 1, 0)
OP(cslel,   T(l,l,e,e, l,l,e,e), 1) X64(0, 1, 0)
OP(csltl,   T(l,l,e,e, l,l,e,e), 1) X64(0, 1, 0)
OP(cugel,   T(l,l,e,e, l,l,e,e), 1) X64(0, 1, 0)
OP(cugtl,   T(l,l,e,e, l,l,e,e), 1) X64(0, 1, 0)
OP(culel,   T(l,l,e,e, l,l,e,e), 1) X64(0, 1, 0)
OP(cultl,   T(l,l,e,e, l,l,e,e), 1) X64(0, 1, 0)

OP(ceqs,    T(s,s,e,e, s,s,e,e), 1) X64(0, 1, 0)
OP(cges,    T(s,s,e,e, s,s,e,e), 1) X64(0, 1, 0)
OP(cgts,    T(s,s,e,e, s,s,e,e), 1) X64(0, 1, 0)
OP(cles,    T(s,s,e,e, s,s,e,e), 1) X64(0, 1, 0)
OP(clts,    T(s,s,e,e, s,s,e,e), 1) X64(0, 1, 0)
OP(cnes,    T(s,s,e,e, s,s,e,e), 1) X64(0, 1, 0)
OP(cos,     T(s,s,e,e, s,s,e,e), 1) X64(0, 1, 0)
OP(cuos,    T(s,s,e,e, s,s,e,e), 1) X64(0, 1, 0)

OP(ceqd,    T(d,d,e,e, d,d,e,e), 1) X64(0, 1, 0)
OP(cged,    T(d,d,e,e, d,d,e,e), 1) X64(0, 1, 0)
OP(cgtd,    T(d,d,e,e, d,d,e,e), 1) X64(0, 1, 0)
OP(cled,    T(d,d,e,e, d,d,e,e), 1) X64(0, 1, 0)
OP(cltd,    T(d,d,e,e, d,d,e,e), 1) X64(0, 1, 0)
OP(cned,    T(d,d,e,e, d,d,e,e), 1) X64(0, 1, 0)
OP(cod,     T(d,d,e,e, d,d,e,e), 1) X64(0, 1, 0)
OP(cuod,    T(d,d,e,e, d,d,e,e), 1) X64(0, 1, 0)

/* Memory */
OP(storeb,  T(w,e,e,e, m,e,e,e), 0) X64(0, 0, 1)
OP(storeh,  T(w,e,e,e, m,e,e,e), 0) X64(0, 0, 1)
OP(storew,  T(w,e,e,e, m,e,e,e), 0) X64(0, 0, 1)
OP(storel,  T(l,e,e,e, m,e,e,e), 0) X64(0, 0, 1)
OP(stores,  T(s,e,e,e, m,e,e,e), 0) X64(0, 0, 1)
OP(stored,  T(d,e,e,e, m,e,e,e), 0) X64(0, 0, 1)

OP(loadsb,  T(m,m,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(loadub,  T(m,m,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(loadsh,  T(m,m,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(loaduh,  T(m,m,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(loadsw,  T(m,m,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(loaduw,  T(m,m,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(load,    T(m,m,m,m, x,x,x,x), 0) X64(0, 0, 1)

/* Extensions and Truncations */
OP(extsb,   T(w,w,e,e, x,x,e,e), 1) X64(0, 0, 1)
OP(extub,   T(w,w,e,e, x,x,e,e), 1) X64(0, 0, 1)
OP(extsh,   T(w,w,e,e, x,x,e,e), 1) X64(0, 0, 1)
OP(extuh,   T(w,w,e,e, x,x,e,e), 1) X64(0, 0, 1)
OP(extsw,   T(e,w,e,e, e,x,e,e), 1) X64(0, 0, 1)
OP(extuw,   T(e,w,e,e, e,x,e,e), 1) X64(0, 0, 1)

OP(exts,    T(e,e,e,s, e,e,e,x), 1) X64(0, 0, 1)
OP(truncd,  T(e,e,d,e, e,e,x,e), 1) X64(0, 0, 1)
OP(stosi,   T(s,s,e,e, x,x,e,e), 1) X64(0, 0, 1)
OP(dtosi,   T(d,d,e,e, x,x,e,e), 1) X64(0, 0, 1)
OP(swtof,   T(e,e,w,w, e,e,x,x), 1) X64(0, 0, 1)
OP(sltof,   T(e,e,l,l, e,e,x,x), 1) X64(0, 0, 1)
OP(cast,    T(s,d,w,l, x,x,x,x), 1) X64(0, 0, 1)

/* Stack Allocation */
OP(alloc4,  T(e,l,e,e, e,x,e,e), 0) X64(0, 0, 0)
OP(alloc8,  T(e,l,e,e, e,x,e,e), 0) X64(0, 0, 0)
OP(alloc16, T(e,l,e,e, e,x,e,e), 0) X64(0, 0, 0)

/* Variadic Function Helpers */
OP(vaarg,   T(m,m,m,m, x,x,x,x), 0) X64(0, 0, 0)
OP(vastart, T(m,e,e,e, x,e,e,e), 0) X64(0, 0, 0)

OP(copy,    T(w,l,s,d, x,x,x,x), 0) X64(0, 0, 1)


/****************************************/
/* INTERNAL OPERATIONS (keep nop first) */
/****************************************/

/* Miscellaneous and Architecture-Specific Operations */
OP(nop,     T(x,x,x,x, x,x,x,x), 0) X64(0, 0, 1)
OP(addr,    T(m,m,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(swap,    T(w,l,s,d, w,l,s,d), 0) X64(1, 0, 0)
OP(sign,    T(w,l,e,e, x,x,e,e), 0) X64(0, 0, 0)
OP(salloc,  T(e,l,e,e, e,x,e,e), 0) X64(0, 0, 0)
OP(xidiv,   T(w,l,e,e, x,x,e,e), 0) X64(1, 0, 0)
OP(xdiv,    T(w,l,e,e, x,x,e,e), 0) X64(1, 0, 0)
OP(xcmp,    T(w,l,s,d, w,l,s,d), 0) X64(1, 1, 0)
OP(xtest,   T(w,l,e,e, w,l,e,e), 0) X64(1, 1, 0)
OP(acmp,    T(w,l,e,e, w,l,e,e), 0) X64(0, 0, 0)
OP(acmn,    T(w,l,e,e, w,l,e,e), 0) X64(0, 0, 0)

/* Arguments, Parameters, and Calls */
OP(par,     T(x,x,x,x, x,x,x,x), 0) X64(0, 0, 0)
OP(parc,    T(e,x,e,e, e,x,e,e), 0) X64(0, 0, 0)
OP(pare,    T(x,x,x,x, x,x,x,x), 0) X64(0, 0, 0)
OP(arg,     T(w,l,s,d, x,x,x,x), 0) X64(0, 0, 0)
OP(argc,    T(e,x,e,e, e,l,e,e), 0) X64(0, 0, 0)
OP(arge,    T(w,l,s,d, x,x,x,x), 0) X64(0, 0, 0)
OP(call,    T(m,m,m,m, x,x,x,x), 0) X64(0, 0, 0)
OP(vacall,  T(m,m,m,m, x,x,x,x), 0) X64(0, 0, 0)

/* Flags Setting */
OP(flagieq,  T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(flagine,  T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(flagisge, T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(flagisgt, T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(flagisle, T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(flagislt, T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(flagiuge, T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(flagiugt, T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(flagiule, T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(flagiult, T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(flagfeq,  T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(flagfge,  T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(flagfgt,  T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(flagfle,  T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(flagflt,  T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(flagfne,  T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(flagfo,   T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(flagfuo,  T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)


#undef T
#undef X64
#undef OP

/*
| column -t -o ' '
*/
