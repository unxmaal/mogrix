/*
 * IRIX findUnwindSections implementation for LLVM libunwind.
 *
 * Uses dladdr() (via _rld_new_interface(_RLD_DLADDR)) to find which
 * loaded object contains a given PC address, then reads the ELF section
 * headers from the file to locate .eh_frame.
 *
 * Results are cached per-DSO base address.
 */

#include <sys/types.h>
#include <elf.h>
#include <rld_interface.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

/* Cache entry */
struct EhFrameCache {
    void       *dso_base;        /* dli_fbase — unique ID per loaded object */
    unsigned int eh_frame_va;    /* in-memory VA of .eh_frame */
    unsigned int eh_frame_size;  /* size */
    int          valid;          /* 1=found, 0=not found (negative cache) */
};

static EhFrameCache *cache = NULL;
static int cache_count = 0;
static int cache_cap = 0;

/* Read .eh_frame section info from an ELF file on disk.
 * Returns sh_addr (file VA) and sh_size. */
static int read_eh_frame_from_file(const char *path,
                                   unsigned int *out_addr,
                                   unsigned int *out_size) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) return -1;

    Elf32_Ehdr ehdr;
    if (read(fd, &ehdr, sizeof(ehdr)) != (int)sizeof(ehdr)) { close(fd); return -1; }
    if (ehdr.e_shoff == 0 || ehdr.e_shnum == 0) { close(fd); return -1; }

    /* Read section string table */
    Elf32_Shdr strhdr;
    lseek(fd, ehdr.e_shoff + ehdr.e_shstrndx * ehdr.e_shentsize, SEEK_SET);
    if (read(fd, &strhdr, sizeof(strhdr)) != (int)sizeof(strhdr)) { close(fd); return -1; }

    char *strtab = (char *)malloc(strhdr.sh_size);
    if (!strtab) { close(fd); return -1; }
    lseek(fd, strhdr.sh_offset, SEEK_SET);
    read(fd, strtab, strhdr.sh_size);

    /* Scan sections for .eh_frame */
    Elf32_Shdr shdr;
    int found = -1;
    for (int i = 0; i < ehdr.e_shnum; i++) {
        lseek(fd, ehdr.e_shoff + i * ehdr.e_shentsize, SEEK_SET);
        if (read(fd, &shdr, sizeof(shdr)) != (int)sizeof(shdr)) break;
        if (shdr.sh_name < strhdr.sh_size &&
            strcmp(strtab + shdr.sh_name, ".eh_frame") == 0 &&
            shdr.sh_size > 4) {  /* skip empty/terminator-only */
            *out_addr = shdr.sh_addr;
            *out_size = shdr.sh_size;
            found = 0;
            break;
        }
    }

    free(strtab);
    close(fd);
    return found;
}

/* Get the base VA of the first PT_LOAD from file (preferred load address) */
static unsigned int get_preferred_base(const char *path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) return 0;

    Elf32_Ehdr ehdr;
    if (read(fd, &ehdr, sizeof(ehdr)) != (int)sizeof(ehdr)) { close(fd); return 0; }

    Elf32_Phdr phdr;
    for (int i = 0; i < ehdr.e_phnum; i++) {
        lseek(fd, ehdr.e_phoff + i * ehdr.e_phentsize, SEEK_SET);
        if (read(fd, &phdr, sizeof(phdr)) != (int)sizeof(phdr)) break;
        if (phdr.p_type == PT_LOAD) {
            close(fd);
            return phdr.p_vaddr;
        }
    }
    close(fd);
    return 0;
}

static void dbg(const char *msg) {
    write(2, msg, strlen(msg));
}
static void dbg_hex(const char *label, unsigned int val) {
    char buf[64];
    /* manual hex format since snprintf might not be available */
    static const char hex[] = "0123456789abcdef";
    char *p = buf;
    const char *s = label;
    while (*s) *p++ = *s++;
    *p++ = '0'; *p++ = 'x';
    for (int i = 28; i >= 0; i -= 4)
        *p++ = hex[(val >> i) & 0xf];
    *p++ = '\n';
    write(2, buf, p - buf);
}

extern "C"
int irix_find_eh_frame(unsigned int targetAddr,
                       unsigned int *out_eh_frame,
                       unsigned int *out_eh_frame_size,
                       unsigned int *out_dso_base) {

    static int dbg_count = 0;
    int do_dbg = (dbg_count < 10);
    if (do_dbg) {
        dbg_hex("[find_eh] target=", targetAddr);
        dbg_count++;
    }

    /* Use dladdr to find which DSO contains targetAddr */
    Dl_info dl;
    if (!(long)_rld_new_interface(_RLD_DLADDR, (void*)(unsigned long)targetAddr, &dl)) {
        if (do_dbg) dbg("[find_eh] dladdr FAILED\n");
        return 0;
    }

    if (!dl.dli_fbase || !dl.dli_fname) {
        if (do_dbg) dbg("[find_eh] no fbase/fname\n");
        return 0;
    }

    if (do_dbg) {
        dbg("[find_eh] dladdr OK: ");
        dbg(dl.dli_fname);
        dbg("\n");
        dbg_hex("[find_eh] fbase=", (unsigned int)(unsigned long)dl.dli_fbase);
    }

    /* Check cache */
    for (int i = 0; i < cache_count; i++) {
        if (cache[i].dso_base == dl.dli_fbase) {
            if (!cache[i].valid)
                return 0;
            *out_eh_frame = cache[i].eh_frame_va;
            *out_eh_frame_size = cache[i].eh_frame_size;
            *out_dso_base = (unsigned int)(unsigned long)dl.dli_fbase;
            return 1;
        }
    }

    /* Not cached — read from file */
    unsigned int file_va = 0, file_size = 0;
    int found = read_eh_frame_from_file(dl.dli_fname, &file_va, &file_size);

    /* Grow cache */
    if (cache_count >= cache_cap) {
        int new_cap = cache_cap ? cache_cap * 2 : 16;
        EhFrameCache *nb = (EhFrameCache *)realloc(cache, new_cap * sizeof(EhFrameCache));
        if (!nb) return 0;
        cache = nb;
        cache_cap = new_cap;
    }

    EhFrameCache *e = &cache[cache_count++];
    e->dso_base = dl.dli_fbase;

    if (found == 0 && file_size > 0) {
        /* Compute slide: actual base - preferred base */
        unsigned int preferred = get_preferred_base(dl.dli_fname);
        unsigned int actual = (unsigned int)(unsigned long)dl.dli_fbase;
        unsigned int slide = actual - preferred;

        e->eh_frame_va = file_va + slide;
        e->eh_frame_size = file_size;
        e->valid = 1;

        *out_eh_frame = e->eh_frame_va;
        *out_eh_frame_size = e->eh_frame_size;
        *out_dso_base = actual;
        if (do_dbg) {
            dbg_hex("[find_eh] FOUND eh_frame=", e->eh_frame_va);
            dbg_hex("[find_eh] size=", e->eh_frame_size);
        }
        return 1;
    } else {
        if (do_dbg) dbg("[find_eh] .eh_frame NOT found in file\n");
        e->eh_frame_va = 0;
        e->eh_frame_size = 0;
        e->valid = 0;
        return 0;
    }
}
