#ifndef X64
	#define X64(NMemArgs, SetsFlags, LeavesFlags)
#endif

#define T(a,b,c,d,e,f,g,h) { \
  {[Kw]=K##a, [Kl]=K##b, [Ks]=K##c, [Kd]=K##d}, \
  {[Kw]=K##e, [Kl]=K##f, [Ks]=K##g, [Kd]=K##h} }

/* Arithmetic and Bits */
OP(Oadd,         "add",     T(w,l,s,d, w,l,s,d), 1) X64(2, 1, 0)
OP(Osub,         "sub",     T(w,l,s,d, w,l,s,d), 1) X64(2, 1, 0)
OP(Odiv,         "div",     T(w,l,s,d, w,l,s,d), 1) X64(0, 0, 0)
OP(Orem,         "rem",     T(w,l,e,e, w,l,e,e), 1) X64(0, 0, 0)
OP(Oudiv,        "udiv",    T(w,l,e,e, w,l,e,e), 1) X64(0, 0, 0)
OP(Ourem,        "urem",    T(w,l,e,e, w,l,e,e), 1) X64(0, 0, 0)
OP(Omul,         "mul",     T(w,l,s,d, w,l,s,d), 1) X64(2, 0, 0)
OP(Oand,         "and",     T(w,l,e,e, w,l,e,e), 1) X64(2, 1, 0)
OP(Oor,          "or",      T(w,l,e,e, w,l,e,e), 1) X64(2, 1, 0)
OP(Oxor,         "xor",     T(w,l,e,e, w,l,e,e), 1) X64(2, 1, 0)
OP(Osar,         "sar",     T(w,l,e,e, w,w,e,e), 1) X64(1, 1, 0)
OP(Oshr,         "shr",     T(w,l,e,e, w,w,e,e), 1) X64(1, 1, 0)
OP(Oshl,         "shl",     T(w,l,e,e, w,w,e,e), 1) X64(1, 1, 0)

/* Memory */
OP(Ostored,      "stored",  T(d,e,e,e, m,e,e,e), 0) X64(0, 0, 1)
OP(Ostores,      "stores",  T(s,e,e,e, m,e,e,e), 0) X64(0, 0, 1)
OP(Ostorel,      "storel",  T(l,e,e,e, m,e,e,e), 0) X64(0, 0, 1)
OP(Ostorew,      "storew",  T(w,e,e,e, m,e,e,e), 0) X64(0, 0, 1)
OP(Ostoreh,      "storeh",  T(w,e,e,e, m,e,e,e), 0) X64(0, 0, 1)
OP(Ostoreb,      "storeb",  T(w,e,e,e, m,e,e,e), 0) X64(0, 0, 1)
OP(Oload,        "load",    T(m,m,m,m, x,x,x,x), 0) X64(0, 0, 1)
OP(Oloadsw,      "loadsw",  T(m,m,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oloaduw,      "loaduw",  T(m,m,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oloadsh,      "loadsh",  T(m,m,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oloaduh,      "loaduh",  T(m,m,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oloadsb,      "loadsb",  T(m,m,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oloadub,      "loadub",  T(m,m,e,e, x,x,e,e), 0) X64(0, 0, 1)

/* Extensions and Truncations */
OP(Oextsw,       "extsw",   T(e,w,e,e, e,x,e,e), 1) X64(0, 0, 1)
OP(Oextuw,       "extuw",   T(e,w,e,e, e,x,e,e), 1) X64(0, 0, 1)
OP(Oextsh,       "extsh",   T(w,w,e,e, x,x,e,e), 1) X64(0, 0, 1)
OP(Oextuh,       "extuh",   T(w,w,e,e, x,x,e,e), 1) X64(0, 0, 1)
OP(Oextsb,       "extsb",   T(w,w,e,e, x,x,e,e), 1) X64(0, 0, 1)
OP(Oextub,       "extub",   T(w,w,e,e, x,x,e,e), 1) X64(0, 0, 1)
OP(Oexts,        "exts",    T(e,e,e,s, e,e,e,x), 1) X64(0, 0, 1)
OP(Otruncd,      "truncd",  T(e,e,d,e, e,e,x,e), 1) X64(0, 0, 1)
OP(Ostosi,       "stosi",   T(s,s,e,e, x,x,e,e), 1) X64(0, 0, 1)
OP(Odtosi,       "dtosi",   T(d,d,e,e, x,x,e,e), 1) X64(0, 0, 1)
OP(Oswtof,       "swtof",   T(e,e,w,w, e,e,x,x), 1) X64(0, 0, 1)
OP(Osltof,       "sltof",   T(e,e,l,l, e,e,x,x), 1) X64(0, 0, 1)
OP(Ocast,        "cast",    T(s,d,w,l, x,x,x,x), 1) X64(0, 0, 1)

/* Arguments, Parameters, and Calls */
OP(Opar,         "par",     T(x,x,x,x, x,x,x,x), 0) X64(0, 0, 0)
OP(Opare,        "pare",    T(x,x,x,x, x,x,x,x), 0) X64(0, 0, 0)
OP(Oparc,        "parc",    T(e,x,e,e, e,x,e,e), 0) X64(0, 0, 0)
OP(Oarg,         "arg",     T(w,l,s,d, x,x,x,x), 0) X64(0, 0, 0)
OP(Oarge,        "arge",    T(w,l,s,d, x,x,x,x), 0) X64(0, 0, 0)
OP(Oargc,        "argc",    T(e,x,e,e, e,l,e,e), 0) X64(0, 0, 0)
OP(Ocall,        "call",    T(m,m,m,m, x,x,x,x), 0) X64(0, 0, 0)
OP(Ovacall,      "vacall",  T(m,m,m,m, x,x,x,x), 0) X64(0, 0, 0)
OP(Ovaarg,       "vaarg",   T(m,m,m,m, x,x,x,x), 0) X64(0, 0, 0)
OP(Ovastart,     "vastart", T(m,e,e,e, x,e,e,e), 0) X64(0, 0, 0)

/* Stack Allocation */
OP(Oalloc,       "alloc4",  T(e,l,e,e, e,x,e,e), 0) X64(0, 0, 0)
OP(Oalloc+1,     "alloc8",  T(e,l,e,e, e,x,e,e), 0) X64(0, 0, 0)
OP(Oalloc+2,     "alloc16", T(e,l,e,e, e,x,e,e), 0) X64(0, 0, 0)
OP(Osalloc,      "salloc",  T(e,l,e,e, e,x,e,e), 0) X64(0, 0, 0)

/* Miscellaneous and Architecture-Specific Operations */
OP(Ocopy,        "copy",    T(w,l,s,d, x,x,x,x), 0) X64(0, 0, 1)
OP(Onop,         "nop",     T(x,x,x,x, x,x,x,x), 0) X64(0, 0, 1)
OP(Oswap,        "swap",    T(w,l,s,d, w,l,s,d), 0) X64(1, 0, 0)
OP(Osign,        "sign",    T(w,l,e,e, x,x,e,e), 0) X64(0, 0, 0)
OP(Oaddr,        "addr",    T(m,m,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oxidiv,       "xidiv",   T(w,l,e,e, x,x,e,e), 0) X64(1, 0, 0)
OP(Oxdiv,        "xdiv",    T(w,l,e,e, x,x,e,e), 0) X64(1, 0, 0)
OP(Oxcmp,        "xcmp",    T(w,l,s,d, w,l,s,d), 0) X64(1, 1, 0)
OP(Oxtest,       "xtest",   T(w,l,e,e, w,l,e,e), 0) X64(1, 1, 0)

/* Comparisons */
OP(Ocmpw+Cieq,   "ceqw",    T(w,w,e,e, w,w,e,e), 1) X64(0, 1, 0)
OP(Ocmpw+Cine,   "cnew",    T(w,w,e,e, w,w,e,e), 1) X64(0, 1, 0)
OP(Ocmpw+Cisge,  "csgew",   T(w,w,e,e, w,w,e,e), 1) X64(0, 1, 0)
OP(Ocmpw+Cisgt,  "csgtw",   T(w,w,e,e, w,w,e,e), 1) X64(0, 1, 0)
OP(Ocmpw+Cisle,  "cslew",   T(w,w,e,e, w,w,e,e), 1) X64(0, 1, 0)
OP(Ocmpw+Cislt,  "csltw",   T(w,w,e,e, w,w,e,e), 1) X64(0, 1, 0)
OP(Ocmpw+Ciuge,  "cugew",   T(w,w,e,e, w,w,e,e), 1) X64(0, 1, 0)
OP(Ocmpw+Ciugt,  "cugtw",   T(w,w,e,e, w,w,e,e), 1) X64(0, 1, 0)
OP(Ocmpw+Ciule,  "culew",   T(w,w,e,e, w,w,e,e), 1) X64(0, 1, 0)
OP(Ocmpw+Ciult,  "cultw",   T(w,w,e,e, w,w,e,e), 1) X64(0, 1, 0)

OP(Ocmpl+Cieq,   "ceql",    T(l,l,e,e, l,l,e,e), 1) X64(0, 1, 0)
OP(Ocmpl+Cine,   "cnel",    T(l,l,e,e, l,l,e,e), 1) X64(0, 1, 0)
OP(Ocmpl+Cisge,  "csgel",   T(l,l,e,e, l,l,e,e), 1) X64(0, 1, 0)
OP(Ocmpl+Cisgt,  "csgtl",   T(l,l,e,e, l,l,e,e), 1) X64(0, 1, 0)
OP(Ocmpl+Cisle,  "cslel",   T(l,l,e,e, l,l,e,e), 1) X64(0, 1, 0)
OP(Ocmpl+Cislt,  "csltl",   T(l,l,e,e, l,l,e,e), 1) X64(0, 1, 0)
OP(Ocmpl+Ciuge,  "cugel",   T(l,l,e,e, l,l,e,e), 1) X64(0, 1, 0)
OP(Ocmpl+Ciugt,  "cugtl",   T(l,l,e,e, l,l,e,e), 1) X64(0, 1, 0)
OP(Ocmpl+Ciule,  "culel",   T(l,l,e,e, l,l,e,e), 1) X64(0, 1, 0)
OP(Ocmpl+Ciult,  "cultl",   T(l,l,e,e, l,l,e,e), 1) X64(0, 1, 0)

OP(Ocmps+Cfeq,   "ceqs",    T(s,s,e,e, s,s,e,e), 1) X64(0, 1, 0)
OP(Ocmps+Cfge,   "cges",    T(s,s,e,e, s,s,e,e), 1) X64(0, 1, 0)
OP(Ocmps+Cfgt,   "cgts",    T(s,s,e,e, s,s,e,e), 1) X64(0, 1, 0)
OP(Ocmps+Cfle,   "cles",    T(s,s,e,e, s,s,e,e), 1) X64(0, 1, 0)
OP(Ocmps+Cflt,   "clts",    T(s,s,e,e, s,s,e,e), 1) X64(0, 1, 0)
OP(Ocmps+Cfne,   "cnes",    T(s,s,e,e, s,s,e,e), 1) X64(0, 1, 0)
OP(Ocmps+Cfo,    "cos",     T(s,s,e,e, s,s,e,e), 1) X64(0, 1, 0)
OP(Ocmps+Cfuo,   "cuos",    T(s,s,e,e, s,s,e,e), 1) X64(0, 1, 0)

OP(Ocmpd+Cfeq,   "ceqd",    T(d,d,e,e, d,d,e,e), 1) X64(0, 1, 0)
OP(Ocmpd+Cfge,   "cged",    T(d,d,e,e, d,d,e,e), 1) X64(0, 1, 0)
OP(Ocmpd+Cfgt,   "cgtd",    T(d,d,e,e, d,d,e,e), 1) X64(0, 1, 0)
OP(Ocmpd+Cfle,   "cled",    T(d,d,e,e, d,d,e,e), 1) X64(0, 1, 0)
OP(Ocmpd+Cflt,   "cltd",    T(d,d,e,e, d,d,e,e), 1) X64(0, 1, 0)
OP(Ocmpd+Cfne,   "cned",    T(d,d,e,e, d,d,e,e), 1) X64(0, 1, 0)
OP(Ocmpd+Cfo,    "cod",     T(d,d,e,e, d,d,e,e), 1) X64(0, 1, 0)
OP(Ocmpd+Cfuo,   "cuod",    T(d,d,e,e, d,d,e,e), 1) X64(0, 1, 0)

/* Flags Setting */
OP(Oflag+Cieq,       "flagieq",  T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oflag+Cine,       "flagine",  T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oflag+Cisge,      "flagisge", T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oflag+Cisgt,      "flagisgt", T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oflag+Cisle,      "flagisle", T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oflag+Cislt,      "flagislt", T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oflag+Ciuge,      "flagiuge", T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oflag+Ciugt,      "flagiugt", T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oflag+Ciule,      "flagiule", T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oflag+Ciult,      "flagiult", T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oflag+NCmpI+Cfeq, "flagfeq",  T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oflag+NCmpI+Cfge, "flagfge",  T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oflag+NCmpI+Cfgt, "flagfgt",  T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oflag+NCmpI+Cfle, "flagfle",  T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oflag+NCmpI+Cflt, "flagflt",  T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oflag+NCmpI+Cfne, "flagfne",  T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oflag+NCmpI+Cfo,  "flagfo",   T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)
OP(Oflag+NCmpI+Cfuo, "flagfuo",  T(x,x,e,e, x,x,e,e), 0) X64(0, 0, 1)

#undef T

/*
| column -t -o ' '
*/
