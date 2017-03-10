#ifndef X64
	#define X64(NMemArgs, SetsFlags, LeavesFlags)
#endif

#define T(a,b,c,d,e,f,g,h) {                          \
	{[Kw]=K##a, [Kl]=K##b, [Ks]=K##c, [Kd]=K##d}, \
	{[Kw]=K##e, [Kl]=K##f, [Ks]=K##g, [Kd]=K##h}  \
}


/*********************/
/* PUBLIC OPERATIONS */
/*********************/

/* Arithmetic and Bits */
OP(Oadd,     "add",     T(w,l,s,d, w,l,s,d), 1) X64(2, 1, 0)
OP(Osub,     "sub",     T(w,l,s,d, w,l,s,d), 1) X64(2, 1, 0)
OP(Odiv,     "div",     T(w,l,s,d, w,l,s,d), 1) X64(0, 0, 0)
OP(Orem,     "rem",     T(w,l,e,e, w,l,e,e), 1) X64(0, 0, 0)
OP(Oudiv,    "udiv",    T(w,l,e,e, w,l,e,e), 1) X64(0, 0, 0)
OP(Ourem,    "urem",    T(w,l,e,e, w,l,e,e), 1) X64(0, 0, 0)
OP(Omul,     "mul",     T(w,l,s,d, w,l,s,d), 1) X64(2, 0, 0)
OP(Oand,     "and",     T(w,l,e,e, w,l,e,e), 1) X64(2, 1, 0)
OP(Oor,      "or",      T(w,l,e,e, w,l,e,e), 1) X64(2, 1, 0)
OP(Oxor,     "xor",     T(w,l,e,e, w,l,e,e), 1) X64(2, 1, 0)
OP(Osar,     "sar",     T(w,l,e,e, w,w,e,e), 1) X64(1, 1, 0)
OP(Oshr,     "shr",     T(w,l,e,e, w,w,e,e), 1) X64(1, 1, 0)
OP(Oshl,     "shl",     T(w,l,e,e, w,w,e,e), 1) X64(1, 1, 0)

/* Comparisons */
OP(Oceqw,    "ceqw",    T(w,w,e,e, w,w,e,e), 1) X64(0, 1, 0)
OP(Ocnew,    "cnew",    T(w,w,e,e, w,w,e,e), 1) X64(0, 1, 0)
OP(Ocsgew,   "csgew",   T(w,w,e,e, w,w,e,e), 1) X64(0, 1, 0)
OP(Ocsgtw,   "csgtw",   T(w,w,e,e, w,w,e,e), 1) X64(0, 1, 0)
OP(Ocslew,   "cslew",   T(w,w,e,e, w,w,e,e), 1) X64(0, 1, 0)
OP(Ocsltw,   "csltw",   T(w,w,e,e, w,w,e,e), 1) X64(0, 1, 0)
OP(Ocugew,   "cugew",   T(w,w,e,e, w,w,e,e), 1) X64(0, 1, 0)
OP(Ocugtw,   "cugtw",   T(w,w,e,e, w,w,e,e), 1) X64(0, 1, 0)
OP(Oculew,   "culew",   T(w,w,e,e, w,w,e,e), 1) X64(0, 1, 0)
OP(Ocultw,   "cultw",   T(w,w,e,e, w,w,e,e), 1) X64(0, 1, 0)

OP(Oceql,    "ceql",    T(l,l,e,e, l,l,e,e), 1) X64(0, 1, 0)
OP(Ocnel,    "cnel",    T(l,l,e,e, l,l,e,e), 1) X64(0, 1, 0)
OP(Ocsgel,   "csgel",   T(l,l,e,e, l,l,e,e), 1) X64(0, 1, 0)
OP(Ocsgtl,   "csgtl",   T(l,l,e,e, l,l,e,e), 1) X64(0, 1, 0)
OP(Ocslel,   "cslel",   T(l,l,e,e, l,l,e,e), 1) X64(0, 1, 0)
OP(Ocsltl,   "csltl",   T(l,l,e,e, l,l,e,e), 1) X64(0, 1, 0)
OP(Ocugel,   "cugel",   T(l,l,e,e, l,l,e,e), 1) X64(0, 1, 0)
OP(Ocugtl,   "cugtl",   T(l,l,e,e, l,l,e,e), 1) X64(0, 1, 0)
OP(Oculel,   "culel",   T(l,l,e,e, l,l,e,e), 1) X64(0, 1, 0)
OP(Ocultl,   "cultl",   T(l,l,e,e, l,l,e,e), 1) X64(0, 1, 0)

OP(Oceqs,    "ceqs",    T(s,s,e,e, s,s,e,e), 1) X64(0, 1, 0)
OP(Ocges,    "cges",    T(s,s,e,e, s,s,e,e), 1) X64(0, 1, 0)
OP(Ocgts,    "cgts",    T(s,s,e,e, s,s,e,e), 1) X64(0, 1, 0)
OP(Ocles,    "cles",    T(s,s,e,e, s,s,e,e), 1) X64(0, 1, 0)
OP(Oclts,    "clts",    T(s,s,e,e, s,s,e,e), 1) X64(0, 1, 0)
OP(Ocnes,    "cnes",    T(s,s,e,e, s,s,e,e), 1) X64(0, 1, 0)
OP(Ocos,     "cos",     T(s,s,e,e, s,s,e,e), 1) X64(0, 1, 0)
OP(Ocuos,    "cuos",    T(s,s,e,e, s,s,e,e), 1) X64(0, 1, 0)

OP(Oceqd,    "ceqd",    T(d,d,e,e, d,d,e,e), 1) X64(0, 1, 0)
OP(Ocged,    "cged",    T(d,d,e,e, d,d,e,e), 1) X64(0, 1, 0)
OP(Ocgtd,    "cgtd",    T(d,d,e,e, d,d,e,e), 1) X64(0, 1, 0)
OP(Ocled,    "cled",    T(d,d,e,e, d,d,e,e), 1) X64(0, 1, 0)
OP(Ocltd,    "cltd",    T(d,d,e,e, d,d,e,e), 1) X64(0, 1, 0)
OP(Ocned,    "cned",    T(d,d,e,e, d,d,e,e), 1) X64(0, 1, 0)
OP(Ocod,     "cod",     T(d,d,e,e, d,d,e,e), 1) X64(0, 1, 0)
OP(Ocuod,    "cuod",    T(d,d,e,e, d,d,e,e), 1) X64(0, 1, 0)

/* Memory */
OP(Ostoreb,  "storeb",  T(w,e,e,e, m,e,e,e), 0) X64(0, 0, 1)
OP(Ostoreh,  "storeh",  T(w,e,e,e, m,e,e,e), 0) X64(0, 0, 1)
OP(Ostorew,  "storew",  T(w,e,e,e, m,e,e,e), 0) X64(0, 0, 1)
OP(Ostorel,  "storel",  T(l,e,e,e, m,e,e,e), 0) X64(0, 0, 1)
OP(Ostores,  "stores",  T(s,e,e,e, m,e,e,e), 0) X64(0, 0, 1)
OP(Ostored,  "stored",  T(d,e,e,e, m,e,e,e), 0) X64(0, 0, 1)

OP(Oloadsb,  "loadsb",  T(m,m,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oloadub,  "loadub",  T(m,m,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oloadsh,  "loadsh",  T(m,m,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oloaduh,  "loaduh",  T(m,m,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oloadsw,  "loadsw",  T(m,m,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oloaduw,  "loaduw",  T(m,m,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oload,    "load",    T(m,m,m,m, x,x,x,x), 0) X64(0, 0, 1)

/* Extensions and Truncations */
OP(Oextsb,   "extsb",   T(w,w,e,e, x,x,e,e), 1) X64(0, 0, 1)
OP(Oextub,   "extub",   T(w,w,e,e, x,x,e,e), 1) X64(0, 0, 1)
OP(Oextsh,   "extsh",   T(w,w,e,e, x,x,e,e), 1) X64(0, 0, 1)
OP(Oextuh,   "extuh",   T(w,w,e,e, x,x,e,e), 1) X64(0, 0, 1)
OP(Oextsw,   "extsw",   T(e,w,e,e, e,x,e,e), 1) X64(0, 0, 1)
OP(Oextuw,   "extuw",   T(e,w,e,e, e,x,e,e), 1) X64(0, 0, 1)

OP(Oexts,    "exts",    T(e,e,e,s, e,e,e,x), 1) X64(0, 0, 1)
OP(Otruncd,  "truncd",  T(e,e,d,e, e,e,x,e), 1) X64(0, 0, 1)
OP(Ostosi,   "stosi",   T(s,s,e,e, x,x,e,e), 1) X64(0, 0, 1)
OP(Odtosi,   "dtosi",   T(d,d,e,e, x,x,e,e), 1) X64(0, 0, 1)
OP(Oswtof,   "swtof",   T(e,e,w,w, e,e,x,x), 1) X64(0, 0, 1)
OP(Osltof,   "sltof",   T(e,e,l,l, e,e,x,x), 1) X64(0, 0, 1)
OP(Ocast,    "cast",    T(s,d,w,l, x,x,x,x), 1) X64(0, 0, 1)

/* Stack Allocation */
OP(Oalloc4,  "alloc4",  T(e,l,e,e, e,x,e,e), 0) X64(0, 0, 0)
OP(Oalloc8,  "alloc8",  T(e,l,e,e, e,x,e,e), 0) X64(0, 0, 0)
OP(Oalloc16, "alloc16", T(e,l,e,e, e,x,e,e), 0) X64(0, 0, 0)

/* Variadic Function Helpers */
OP(Ovaarg,   "vaarg",   T(m,m,m,m, x,x,x,x), 0) X64(0, 0, 0)
OP(Ovastart, "vastart", T(m,e,e,e, x,e,e,e), 0) X64(0, 0, 0)

OP(Ocopy,    "copy",    T(w,l,s,d, x,x,x,x), 0) X64(0, 0, 1)


/*****************************************/
/* INTERNAL OPERATIONS (keep Onop first) */
/*****************************************/

/* Miscellaneous and Architecture-Specific Operations */
OP(Onop,     "nop",     T(x,x,x,x, x,x,x,x), 0) X64(0, 0, 1)
OP(Oaddr,    "addr",    T(m,m,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oswap,    "swap",    T(w,l,s,d, w,l,s,d), 0) X64(1, 0, 0)
OP(Osign,    "sign",    T(w,l,e,e, x,x,e,e), 0) X64(0, 0, 0)
OP(Osalloc,  "salloc",  T(e,l,e,e, e,x,e,e), 0) X64(0, 0, 0)
OP(Oxidiv,   "xidiv",   T(w,l,e,e, x,x,e,e), 0) X64(1, 0, 0)
OP(Oxdiv,    "xdiv",    T(w,l,e,e, x,x,e,e), 0) X64(1, 0, 0)
OP(Oxcmp,    "xcmp",    T(w,l,s,d, w,l,s,d), 0) X64(1, 1, 0)
OP(Oxtest,   "xtest",   T(w,l,e,e, w,l,e,e), 0) X64(1, 1, 0)

/* Arguments, Parameters, and Calls */
OP(Opar,     "par",     T(x,x,x,x, x,x,x,x), 0) X64(0, 0, 0)
OP(Oparc,    "parc",    T(e,x,e,e, e,x,e,e), 0) X64(0, 0, 0)
OP(Opare,    "pare",    T(x,x,x,x, x,x,x,x), 0) X64(0, 0, 0)
OP(Oarg,     "arg",     T(w,l,s,d, x,x,x,x), 0) X64(0, 0, 0)
OP(Oargc,    "argc",    T(e,x,e,e, e,l,e,e), 0) X64(0, 0, 0)
OP(Oarge,    "arge",    T(w,l,s,d, x,x,x,x), 0) X64(0, 0, 0)
OP(Ocall,    "call",    T(m,m,m,m, x,x,x,x), 0) X64(0, 0, 0)
OP(Ovacall,  "vacall",  T(m,m,m,m, x,x,x,x), 0) X64(0, 0, 0)

/* Flags Setting */
OP(Oflagieq,  "flagieq",  T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oflagine,  "flagine",  T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oflagisge, "flagisge", T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oflagisgt, "flagisgt", T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oflagisle, "flagisle", T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oflagislt, "flagislt", T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oflagiuge, "flagiuge", T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oflagiugt, "flagiugt", T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oflagiule, "flagiule", T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oflagiult, "flagiult", T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oflagfeq,  "flagfeq",  T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oflagfge,  "flagfge",  T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oflagfgt,  "flagfgt",  T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oflagfle,  "flagfle",  T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oflagflt,  "flagflt",  T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oflagfne,  "flagfne",  T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oflagfo,   "flagfo",   T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oflagfuo,  "flagfuo",  T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)

#undef T
#undef X64
#undef OP

/*
| column -t -o ' '
*/
