/* IRIX-patched: replaced function pointer array with dispatch function.
 * IRIX rld intermittently fails R_MIPS_REL32 relocations for function
 * pointers in static data arrays. */
#include <ctype.h>
static struct cclass {
	const char *name;
} cclasses[] = {
	{ "alnum" },
	{ "alpha" },
	{ "blank" },
	{ "cntrl" },
	{ "digit" },
	{ "graph" },
	{ "lower" },
	{ "print" },
	{ "punct" },
	{ "space" },
	{ "upper" },
	{ "xdigit" },
	{ NULL }
};
static int
cclass_isctype(const char *name, int c)
{
	if (strcmp(name, "alnum") == 0) return isalnum(c);
	if (strcmp(name, "alpha") == 0) return isalpha(c);
	if (strcmp(name, "blank") == 0) return isblank(c);
	if (strcmp(name, "cntrl") == 0) return iscntrl(c);
	if (strcmp(name, "digit") == 0) return isdigit(c);
	if (strcmp(name, "graph") == 0) return isgraph(c);
	if (strcmp(name, "lower") == 0) return islower(c);
	if (strcmp(name, "print") == 0) return isprint(c);
	if (strcmp(name, "punct") == 0) return ispunct(c);
	if (strcmp(name, "space") == 0) return isspace(c);
	if (strcmp(name, "upper") == 0) return isupper(c);
	if (strcmp(name, "xdigit") == 0) return isxdigit(c);
	return 0;
}
#define NCCLASSES (sizeof(cclasses)/sizeof(cclasses[0])-1)
