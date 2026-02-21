/*
 * A04: GOT entry limit test - library
 *
 * Exports many global symbols to create a large global GOT.
 * We generate enough to approach but stay under the ~4370 limit.
 * The static check (verify_elf.py) confirms SYMTABNO-GOTSYM < 4370.
 */

/* Generate 200 exported globals — enough to test GOT accounting
   without hitting the limit in a test .so */

#define DEFN(n) int got_sym_##n = n;
#define FN(n)   int got_fn_##n(void) { return got_sym_##n; }

DEFN(0)   FN(0)   DEFN(1)   FN(1)   DEFN(2)   FN(2)   DEFN(3)   FN(3)
DEFN(4)   FN(4)   DEFN(5)   FN(5)   DEFN(6)   FN(6)   DEFN(7)   FN(7)
DEFN(8)   FN(8)   DEFN(9)   FN(9)   DEFN(10)  FN(10)  DEFN(11)  FN(11)
DEFN(12)  FN(12)  DEFN(13)  FN(13)  DEFN(14)  FN(14)  DEFN(15)  FN(15)
DEFN(16)  FN(16)  DEFN(17)  FN(17)  DEFN(18)  FN(18)  DEFN(19)  FN(19)
DEFN(20)  FN(20)  DEFN(21)  FN(21)  DEFN(22)  FN(22)  DEFN(23)  FN(23)
DEFN(24)  FN(24)  DEFN(25)  FN(25)  DEFN(26)  FN(26)  DEFN(27)  FN(27)
DEFN(28)  FN(28)  DEFN(29)  FN(29)  DEFN(30)  FN(30)  DEFN(31)  FN(31)
DEFN(32)  FN(32)  DEFN(33)  FN(33)  DEFN(34)  FN(34)  DEFN(35)  FN(35)
DEFN(36)  FN(36)  DEFN(37)  FN(37)  DEFN(38)  FN(38)  DEFN(39)  FN(39)
DEFN(40)  FN(40)  DEFN(41)  FN(41)  DEFN(42)  FN(42)  DEFN(43)  FN(43)
DEFN(44)  FN(44)  DEFN(45)  FN(45)  DEFN(46)  FN(46)  DEFN(47)  FN(47)
DEFN(48)  FN(48)  DEFN(49)  FN(49)

int got_total_count(void) { return 50; }
