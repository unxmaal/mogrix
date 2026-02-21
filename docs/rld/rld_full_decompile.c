// === newmain @ 0fb60150 (252 bytes) ===

void newmain(void)

{
  undefined8 uVar1;
  int iVar2;
  code *UNRECOVERED_JUMPTABLE_00;
  undefined8 *puVar3;
  int iStack00000000;
  undefined1 auStack_10 [8];
  undefined8 uStack_8;
  undefined8 *puVar4;
  
  uStack_8 = 0;
  uVar1 = sgi_main();
  if (clearFlag != '\0') {
    uStack_8 = uVar1;
    iVar2 = endofstack(auStack_10);
    puVar3 = (undefined8 *)&stack0x00000000;
    do {
      puVar4 = puVar3 + -2;
      puVar3[-2] = 0;
      puVar3[-1] = 0;
      uVar1 = uStack_8;
      puVar3 = puVar3 + -2;
    } while (iVar2 < (int)puVar4);
  }
  UNRECOVERED_JUMPTABLE_00 = (code *)uVar1;
  if (__sgi_fixup_args == '\0') {
    if (has_pixified_init != '\0') {
      restore_pixie_regs(iStack00000000,(int)&stack0x00000000 + 4,
                         &stack0x00000008 + iStack00000000 * 4);
    }
                    /* WARNING: Could not recover jumptable at 0x0fb60244. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*UNRECOVERED_JUMPTABLE_00)();
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x0fb6020c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE_00)
            (iStack00000000 + -1,(int)&stack0x00000000 + 4,
             (int)&stack0x00000000 + iStack00000000 * 4 + 4);
  return;
}



// === getreuid @ 0fb6024c (48 bytes) ===

void getreuid(undefined4 *param_1,undefined4 *param_2,undefined8 param_3,longlong param_4)

{
  undefined4 in_v1_lo;
  
  syscall(0);
  if (param_4 == 0) {
    *param_1 = 0x400;
    *param_2 = in_v1_lo;
    return;
  }
  _cerror();
  return;
}



// === getregid @ 0fb6027c (48 bytes) ===

void getregid(undefined4 *param_1,undefined4 *param_2,undefined8 param_3,longlong param_4)

{
  undefined4 in_v1_lo;
  
  syscall(0);
  if (param_4 == 0) {
    *param_1 = 0x417;
    *param_2 = in_v1_lo;
    return;
  }
  _cerror();
  return;
}



// === _rld_text_resolve @ 0fb602ac (256 bytes) ===

void _rld_text_resolve(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,
                      undefined8 param_5,undefined8 param_6,undefined8 param_7,undefined8 param_8,
                      undefined8 param_9,undefined8 param_10,undefined8 param_11,undefined8 param_12
                      ,undefined8 param_13,undefined8 param_14,undefined8 param_15,
                      undefined8 param_16)

{
  code *UNRECOVERED_JUMPTABLE;
  undefined8 in_t7;
  undefined8 in_t8;
  undefined8 in_ra;
  
  if (has_pixified_init != '\0') {
    save_pixie_regs(in_ra,in_t8,in_t7);
  }
  UNRECOVERED_JUMPTABLE = (code *)lazy_text_resolve();
  if (has_pixified_init != '\0') {
    restore_pixie_regs();
  }
                    /* WARNING: Could not recover jumptable at 0x0fb603a4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)
            (param_1,param_2,param_3,param_4,param_5,param_6,param_7,param_8,param_9,param_10,
             param_11,param_12,param_13,param_14,param_15,param_16);
  return;
}



// === fini_bridge @ 0fb603b4 (160 bytes) ===

void fini_bridge(code *param_1,longlong param_2)

{
  if (param_2 != 0) {
    restore_pixie_regs();
  }
  (*param_1)();
  if (param_2 != 0) {
    save_pixie_regs();
  }
  return;
}



// === init_bridge @ 0fb60454 (160 bytes) ===

void init_bridge(code *param_1,longlong param_2)

{
  if (param_2 != 0) {
    restore_pixie_regs();
  }
  (*param_1)();
  if (param_2 != 0) {
    save_pixie_regs();
  }
  return;
}



// === restore_pixie_regs @ 0fb604f4 (52 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void restore_pixie_regs(void)

{
  return;
}



// === save_pixie_regs @ 0fb60528 (52 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void save_pixie_regs(void)

{
  undefined8 unaff_s0;
  undefined8 unaff_s1;
  undefined8 unaff_s2;
  undefined8 unaff_s3;
  undefined8 unaff_s4;
  undefined8 unaff_s5;
  undefined8 unaff_s6;
  undefined8 unaff_s7;
  undefined8 unaff_s8;
  
  _DAT_00200780 = unaff_s0;
  _DAT_00200788 = unaff_s1;
  _DAT_00200790 = unaff_s2;
  _DAT_00200798 = unaff_s3;
  _DAT_002007a0 = unaff_s4;
  _DAT_002007a8 = unaff_s5;
  _DAT_002007b0 = unaff_s6;
  _DAT_002007b8 = unaff_s7;
  _DAT_002007c0 = unaff_s8;
  return;
}



// === _rld_new_interface @ 0fb6055c (104 bytes) ===

void _rld_new_interface(void)

{
  if (has_pixified_init != '\0') {
    save_pixie_regs();
  }
  _rld_new_interface_body();
  if (has_pixified_init != '\0') {
    restore_pixie_regs();
  }
  return;
}



// === rld_app_bridge @ 0fb605c4 (196 bytes) ===

undefined8
rld_app_bridge(code *param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,
              longlong param_5)

{
  undefined8 uVar1;
  
  if (param_5 != 0) {
    restore_pixie_regs();
  }
  uVar1 = (*param_1)(param_2,param_3,param_4,param_5);
  if (param_5 != 0) {
    save_pixie_regs();
  }
  return uVar1;
}



// === elfunmap @ 0fb606a0 (376 bytes) ===

undefined1  [16] elfunmap(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  int iVar5;
  void *__addr;
  ulonglong *puVar6;
  longlong lVar7;
  undefined8 unaff_s8;
  code *UNRECOVERED_JUMPTABLE;
  undefined1 auVar8 [16];
  ulonglong uStack_80;
  
  iVar3 = *(int *)(param_1 + 0x10);
  lVar7 = (longlong)iVar3;
  if (*(int *)(param_1 + 0x124) != 0) {
    __rld_pagemap_return_to_available(*(int *)(param_1 + 0x124));
LAB_0fb606d4:
    auVar8._0_8_ = lVar7;
    auVar8._8_8_ = unaff_s8;
    return auVar8;
  }
  uStack_80 = (ulonglong)*(ushort *)(iVar3 + 0x2c);
  uVar4 = (uint)*(ushort *)(iVar3 + 0x2c);
  iVar3 = *(int *)(param_1 + 0x3c) - iVar3;
  puVar6 = &uStack_80 + uVar4 * -4;
  memmove(&uStack_80 + uVar4 * -4,*(void **)(param_1 + 0x20),uVar4 * 0x20);
  if (0 < (longlong)uStack_80) {
    do {
      if (*(int *)puVar6 == 1) {
        iVar1 = *(int *)((int)puVar6 + 8);
        uVar4 = iVar1 - iVar3;
        __addr = (void *)(uVar4 & ~(pagesize - 1U));
        iVar5 = uVar4 - (int)__addr;
        iVar2 = munmap(__addr,*(int *)((int)puVar6 + 0x14) + iVar5);
        if (iVar2 < 0) {
          error("rld: munmap at address 0x%lx, size %ld  p_vaddr  0x%lx  map_diff 0x%lx diff 0x%lx\n"
                ,__addr,*(int *)((int)puVar6 + 0x14) + iVar5,iVar1,iVar3);
          perror("rld: munmap failed in elfunmap() ");
          lVar7 = -1;
          goto LAB_0fb606d4;
        }
      }
      puVar6 = (ulonglong *)((int)puVar6 + 0x20);
    } while (puVar6 != &uStack_80);
  }
                    /* WARNING: Could not recover jumptable at 0x0fb6081c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  auVar8 = (*UNRECOVERED_JUMPTABLE)();
  return auVar8;
}



// === elfmap @ 0fb60830 (1432 bytes) ===

/* WARNING: Instruction at (ram,0x0fb60acc) overlaps instruction at (ram,0x0fb60ac8)
    */

int elfmap(char *param_1,int param_2,ulonglong param_3,int param_4)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar5;
  int *piVar6;
  longlong lVar4;
  char *pcVar7;
  undefined1 *puVar8;
  uint uVar9;
  undefined1 *puVar10;
  uint uVar11;
  uint uVar12;
  undefined1 *puVar13;
  int iVar14;
  int *__src;
  undefined1 auStack_2d0 [8];
  int aiStack_2c8 [4];
  uint auStack_2b8 [2];
  undefined1 auStack_2b0 [488];
  int iStack_40;
  int iStack_3c;
  undefined4 uStack_34;
  undefined4 uStack_2c;
  
  if (param_2 == 0) {
    if (param_1 == (char *)0x0) {
      error("No file specified to elfmap.");
      return -1;
    }
    iStack_40 = 0;
    iStack_3c = 0xffffffff;
    uStack_2c = 0x200;
    if (DAT_0fbd3150 != 0) {
      rld_fdprintf(1,"elfmap: opening file %s, opt = 0x%lx\n",param_1,param_3);
    }
    iVar5 = open64(param_1,0);
    if (iVar5 == -1) {
      if ((param_3 & 4) == 0) {
        error("rld: open of %s O_RDONLY failed in elfmap() \n",param_1);
        perror("rld: open failed in elfmap() ");
      }
      FUN_0fb61200(&iStack_40,0,0,param_1);
      return -1;
    }
  }
  else {
    uStack_2c = 0x200;
    iVar5 = param_2;
  }
  iStack_3c = iVar5;
  if (param_1 == (char *)0x0) {
    param_1 = "<no path available>";
  }
  uStack_34 = (undefined4)param_3;
  iStack_40 = 2;
  if (DAT_0fbd3150 != 0) {
    rld_fdprintf(1,"elfmap: reading ELF header.\n");
  }
  iVar5 = FUN_0fb60dd0(&iStack_40,auStack_2d0,param_1);
  if (iStack_40 != 4) {
    FUN_0fb61200(&iStack_40,iVar5,0,param_1);
    return -1;
  }
  if (DAT_0fbd3150 != 0) {
    rld_fdprintf(1,"elfmap: reading program header\n");
  }
  piVar6 = (int *)FUN_0fb610d0(iVar5,&iStack_40,auStack_2d0,param_1);
  if (iStack_40 != 6) {
    error("Couldn\'t read Program header in %s.",param_1);
    FUN_0fb61200(&iStack_40,iVar5,piVar6,param_1);
    return -1;
  }
  uVar11 = (uint)*(ushort *)(iVar5 + 0x2c);
  uVar12 = (uint)*(ushort *)(iVar5 + 0x2c);
  iVar1 = uVar12 * -0x20;
  puVar10 = auStack_2d0 + iVar1;
  puVar8 = auStack_2d0 + iVar1;
  if (uVar11 == 0) {
LAB_0fb60d3c:
    error("elfmap: no PT_LOAD segment found in %s.",param_1);
    FUN_0fb61200(&iStack_40,iVar5,piVar6,param_1);
    return -1;
  }
  iVar3 = 0;
  iVar14 = 0;
  uVar9 = 0;
  iVar2 = -1;
  __src = piVar6;
  do {
    while (*__src == 1) {
      memmove(puVar10,__src,0x20);
      if (__src[1] == 0) {
        iVar3 = __src[2];
      }
      if (__src[2] == 0) {
        pzero_mapped = 1;
      }
      __src = __src + 8;
      uVar11 = (uint)*(ushort *)(iVar5 + 0x2c);
      iVar14 = iVar14 + 1;
      uVar9 = uVar9 + 1;
      puVar10 = puVar10 + 0x20;
      if ((int)uVar11 <= iVar14) goto LAB_0fb60a50;
    }
    if (*__src == 6) {
      iVar2 = iVar14;
    }
    __src = __src + 8;
    iVar14 = iVar14 + 1;
  } while (iVar14 < (int)uVar11);
LAB_0fb60a50:
  if (uVar9 == 0) goto LAB_0fb60d3c;
  if (rld_big_page_load != '\0') {
    errno = 0;
    if (verbose != '\0') {
      trace("big page load attempted, %d segments %s \n",uVar9,param_1);
    }
    lVar4 = __rld_page_load_object(iStack_3c,auStack_2d0 + iVar1,uVar9,param_4);
    if (lVar4 == -1) {
      if (verbose != '\0') {
        trace("big page load failed  %s, next try normal IRIX load.\n",param_1);
      }
    }
    else if (verbose != '\0') {
      trace("big page load succeeded at 0x%lx %s\n",lVar4,param_1);
    }
    if (lVar4 != -1) goto code_r0x0fb60ad0;
  }
  errno = 0;
  lVar4 = syssgi(0x44,iStack_3c,auStack_2d0 + iVar1,uVar9);
code_r0x0fb60ad0:
  if (lVar4 < 0) {
    if (param_1 == (char *)0x0) {
      pcVar7 = strerror(errno);
      error("elfmap: couldn\'t map file: %s",pcVar7);
    }
    else {
      pcVar7 = strerror(errno);
      error("elfmap: couldn\'t map %s: %s",param_1,pcVar7);
    }
    FUN_0fb61200(&iStack_40,iVar5,piVar6,param_1);
    return -1;
  }
  iVar14 = aiStack_2c8[uVar12 * -8] - (int)lVar4;
  if (iVar2 < 0) {
    *(int *)(param_4 + 0x20) = (*(int *)(iVar5 + 0x1c) + iVar3) - iVar14;
  }
  else {
    *(int *)(param_4 + 0x20) = piVar6[iVar2 * 8 + 2] - iVar14;
  }
  uVar11 = 0;
  if ((int)uVar9 < 1) {
    puVar10 = (undefined1 *)0x0;
  }
  else {
    puVar10 = (undefined1 *)0x0;
    if ((uVar9 & 1) != 0) {
      puVar8 = auStack_2b0 + iVar1;
      uVar11 = 1;
      if ((auStack_2b8[uVar12 * -8] & 1) != 0) {
        puVar10 = auStack_2d0 + iVar1;
      }
    }
    if ((int)uVar9 >> 1 != 0) {
      do {
        uVar11 = uVar11 + 2;
        if (((puVar10 == (undefined1 *)0x0) ||
            (puVar13 = puVar10, *(uint *)(puVar8 + 4) < *(uint *)(puVar10 + 4))) &&
           (puVar13 = puVar8, (*(uint *)(puVar8 + 0x18) & 1) == 0)) {
          puVar13 = puVar10;
        }
        puVar10 = puVar13;
        if (((puVar13 == (undefined1 *)0x0) || (*(uint *)(puVar8 + 0x24) < *(uint *)(puVar13 + 4)))
           && ((*(uint *)(puVar8 + 0x38) & 1) != 0)) {
          puVar10 = puVar8 + 0x20;
        }
        puVar8 = puVar8 + 0x40;
      } while (uVar9 != uVar11);
    }
  }
  iVar1 = *(int *)(puVar10 + 8);
  iVar2 = *(int *)(puVar10 + 4);
  FUN_0fb61200(&iStack_40,iVar5,piVar6,param_1);
  return (iVar1 - iVar14) - iVar2;
}



// === FUN_0fb60dd0 @ 0fb60dd0 (336 bytes) ===

void * FUN_0fb60dd0(undefined4 *param_1,void *param_2,undefined8 param_3)

{
  ssize_t sVar3;
  __off64_t _Var1;
  void *pvVar4;
  longlong lVar2;
  code *UNRECOVERED_JUMPTABLE_00;
  
  DAT_0fbd3158 = 0;
  sVar3 = read(param_1[1],param_2,0x200);
  if (sVar3 != 0x200) {
    param_1[5] = 0x34;
    if (param_2 == (void *)0x0) {
      error("elfmap: out of memory. allocating Ehdr for %s",param_3);
      if (DAT_0fbd3150 != 0) {
        rld_fdprintf(1,"Couldn\'t allocate pEhdr");
      }
                    /* WARNING: Could not recover jumptable at 0x0fb60ee8. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      pvVar4 = (void *)(*UNRECOVERED_JUMPTABLE_00)();
      return pvVar4;
    }
    *param_1 = 3;
    _Var1 = lseek64(param_1[1],0,0);
    if (_Var1 != 0) {
      error("Seek back to 0 on %s to re-read elf header failed! (errno %d)\n",param_3,errno);
      return (void *)0x0;
    }
    sVar3 = read(param_1[1],param_2,0x34);
    if (sVar3 != 0x34) {
      error("elfmap failed on %s : read %ld bytes too short to be elf header\n",param_3,sVar3);
      error("elfmap: Unexpected EOF in %s",param_3);
      memset(param_2,0,0x34);
                    /* WARNING: Could not recover jumptable at 0x0fb60eac. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      pvVar4 = (void *)(*UNRECOVERED_JUMPTABLE_00)();
      return pvVar4;
    }
  }
  lVar2 = FUN_0fb60f20(param_2,param_3);
  if (lVar2 != 0) {
    *param_1 = 4;
  }
  param_1[4] = *(uint *)((int)param_2 + 0x24) & 2;
  return param_2;
}



// === FUN_0fb60f20 @ 0fb60f20 (424 bytes) ===

undefined8 FUN_0fb60f20(char *param_1,undefined8 param_2)

{
  undefined8 uVar1;
  char cVar2;
  char cVar3;
  
  cVar2 = param_1[2];
  if (*param_1 == '\x7f') {
    if (param_1[1] == 'E') {
      cVar3 = param_1[3];
      if ((cVar2 == 'L') && (cVar3 == 'F')) {
        if (param_1[5] == '\x02') {
          if ((*(short *)(param_1 + 0x10) == 2) || (*(short *)(param_1 + 0x10) == 3)) {
            if (param_1[4] == '\x01') {
              if ((*(short *)(param_1 + 0x12) == 0) || (*(short *)(param_1 + 0x12) == 8)) {
                if ((*(uint *)(param_1 + 0x24) & 0x20) == 0x20) {
                  if ((*(uint *)(param_1 + 0x24) & 6) == 0) {
                    error("elfmap: object %s is a non-shared object(no EF_MIPS_*PIC flag, e_flags 0x%lx). "
                          ,param_2);
                    uVar1 = 0;
                  }
                  else {
                    uVar1 = 1;
                  }
                }
                else {
                  warning("elfmap: running new 32-bit executable but finding old 32-bit shared object %s in the search path.  You may not have set the environment variables correctly, please set LD_LIBRARY_PATH for old 32-bit objects, LD_LIBRARYN32_PATH for new 32-bit objects and LD_LIBRARY64_PATH for 64-bit objects (e_flags 0x%x) -- continue searching ..."
                          ,param_2);
                  uVar1 = 0;
                }
              }
              else {
                error("elfmap: executable/DSO cannot be run on this machine since program header %s not marked as MIPS (e_machine %d)\n"
                      ,param_2);
                uVar1 = 0;
              }
            }
            else {
              warning("elfmap: running 32-bit executable but finding 64-bit shared objects %s in the search path.  You may not have set the environment variables correctly, please set LD_LIBRARY_PATH for old 32-bit objects, LD_LIBRARYN32_PATH for new 32-bit objects and LD_LIBRARY64_PATH for 64-bit objects (e_ident[EI_CLASS] %d)  -- continue searching ..."
                      ,param_2);
              uVar1 = 0;
            }
          }
          else {
            error("elfmap: object %s is not an executable or dynamic object(e_type is %d). ",param_2
                 );
            uVar1 = 0;
          }
          return uVar1;
        }
        error("elfmap: executable or dynamic object %s is opposite Endian-ness from rld.",param_2);
        return 0;
      }
    }
    else {
      cVar3 = param_1[3];
    }
  }
  else {
    cVar3 = param_1[3];
    cVar2 = param_1[2];
  }
  error("elfmap: executable or DSO file is not an ELF file. first 4 bytes are 0x%02x %02x %02x %02x not the required 0x%02x %02x %02x %02x (file name %s)\n"
        ,*param_1,param_1[1],cVar2,cVar3,0x7f,0x45,0x4c,0x46,(int)param_2);
  return 0;
}



// === FUN_0fb610d0 @ 0fb610d0 (304 bytes) ===

void * FUN_0fb610d0(int param_1,undefined4 *param_2,int param_3,undefined8 param_4)

{
  size_t __nbytes;
  ulonglong uVar1;
  size_t sVar2;
  void *__buf;
  
  __nbytes = (uint)*(ushort *)(param_1 + 0x2a) * (uint)*(ushort *)(param_1 + 0x2c);
  __buf = (void *)(*(int *)(param_1 + 0x1c) + param_3);
  if ((uint)param_2[5] < *(int *)(param_1 + 0x1c) + __nbytes) {
    __buf = (void *)malloc(__nbytes);
    if (__buf == (void *)0x0) {
      error("elfmap: cant allocate space for program header for %s.",param_4);
      return (void *)0x0;
    }
    DAT_0fbd3154 = 1;
    *param_2 = 5;
    uVar1 = lseek64(param_2[1],(ulonglong)*(uint *)(param_1 + 0x1c),0);
    if (uVar1 != *(uint *)(param_1 + 0x1c)) {
      error("elfmap: lseek to 0x%lx in %s failed\n",*(uint *)(param_1 + 0x1c),param_4);
      error("elfmap: lseek failed.");
      return (void *)0x0;
    }
    sVar2 = read(param_2[1],__buf,__nbytes);
    if (sVar2 != __nbytes) {
      error("elfmap: read of program header of %s failed.",param_4);
      return (void *)0x0;
    }
  }
  else {
    DAT_0fbd3154 = 0;
  }
  *param_2 = 6;
  return __buf;
}



// === FUN_0fb61200 @ 0fb61200 (84 bytes) ===

int FUN_0fb61200(undefined4 *param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4)

{
  int in_v0_lo;
  int iVar1;
  
  switch(*param_1) {
  case 0:
  case 8:
    return in_v0_lo;
  default:
    force_error("Bogus state in elfmap:cleanup: state %ld path %s\n",*param_1,param_4);
                    /* WARNING: Subroutine does not return */
    exit(10);
  case 5:
  case 6:
  case 7:
  case 9:
    if (DAT_0fbd3154 != 0) {
      free(param_3);
    }
    DAT_0fbd3154 = 0;
  case 3:
  case 4:
    if (DAT_0fbd3158 != 0) {
      free(param_2);
    }
    DAT_0fbd3158 = 0;
  case 2:
    iVar1 = close(param_1[1]);
    param_1[1] = 0xffffffff;
    return iVar1;
  }
}



// === atexit @ 0fb612c0 (16 bytes) ===

int atexit(__func *__func)

{
                    /* WARNING: Subroutine does not return */
  abort();
}



// === _exithandle @ 0fb612e0 (8 bytes) ===

void _exithandle(void)

{
  return;
}



// === __eachexithandle @ 0fb612f0 (8 bytes) ===

void __eachexithandle(void)

{
  return;
}



// === objlist_add_beginning @ 0fb61300 (44 bytes) ===

void objlist_add_beginning(int *param_1,int param_2)

{
  int iVar1;
  
  iVar1 = *param_1;
  *(int *)(param_2 + 8) = iVar1;
  if (iVar1 == 0) {
    *(int *)(param_2 + 0xc) = param_2;
    *param_1 = param_2;
    return;
  }
  *(undefined4 *)(param_2 + 0xc) = *(undefined4 *)(iVar1 + 0xc);
  *(int *)(iVar1 + 0xc) = param_2;
  *param_1 = param_2;
  return;
}



// === objlist_add_end @ 0fb61330 (56 bytes) ===

void objlist_add_end(int *param_1,int param_2)

{
  int iVar1;
  
  iVar1 = *param_1;
  if (iVar1 == 0) {
    *(int *)(param_2 + 0xc) = param_2;
    *(undefined4 *)(param_2 + 8) = 0;
    *param_1 = param_2;
    return;
  }
  *(int *)(*(int *)(iVar1 + 0xc) + 8) = param_2;
  *(undefined4 *)(param_2 + 0xc) = *(undefined4 *)(iVar1 + 0xc);
  *(int *)(iVar1 + 0xc) = param_2;
  *(undefined4 *)(param_2 + 8) = 0;
  return;
}



// === objlist_add_before @ 0fb61370 (64 bytes) ===

void objlist_add_before(int *param_1,int param_2,int param_3)

{
  if (*param_1 == param_2) {
    *param_1 = param_3;
    *(undefined4 *)(param_3 + 0xc) = *(undefined4 *)(param_2 + 0xc);
    *(int *)(param_2 + 0xc) = param_3;
    *(int *)(param_3 + 8) = param_2;
    return;
  }
  *(int *)(*(int *)(param_2 + 0xc) + 8) = param_3;
  *(undefined4 *)(param_3 + 0xc) = *(undefined4 *)(param_2 + 0xc);
  *(int *)(param_2 + 0xc) = param_3;
  *(int *)(param_3 + 8) = param_2;
  return;
}



// === objlist_add_after @ 0fb613b0 (68 bytes) ===

void objlist_add_after(int *param_1,int param_2,int param_3)

{
  if (*(int *)(*param_1 + 0xc) == param_2) {
    *(int *)(*param_1 + 0xc) = param_3;
    *(undefined4 *)(param_3 + 8) = *(undefined4 *)(param_2 + 8);
    *(int *)(param_2 + 8) = param_3;
    *(int *)(param_3 + 0xc) = param_2;
    return;
  }
  *(int *)(*(int *)(param_2 + 8) + 0xc) = param_3;
  *(undefined4 *)(param_3 + 8) = *(undefined4 *)(param_2 + 8);
  *(int *)(param_2 + 8) = param_3;
  *(int *)(param_3 + 0xc) = param_2;
  return;
}



// === objlist_delete_element @ 0fb61400 (68 bytes) ===

int objlist_delete_element(int *param_1,int param_2)

{
  int iVar1;
  int iVar2;
  
  iVar2 = *(int *)(param_2 + 0xc);
  iVar1 = *(int *)(param_2 + 8);
  if (*param_1 == param_2) {
    *param_1 = iVar1;
    iVar2 = iVar1;
  }
  else {
    *(int *)(iVar2 + 8) = iVar1;
  }
  if (*(int *)(param_2 + 8) != 0) {
    *(undefined4 *)(*(int *)(param_2 + 8) + 0xc) = *(undefined4 *)(param_2 + 0xc);
    return iVar2;
  }
  *(undefined4 *)(*param_1 + 0xc) = *(undefined4 *)(param_2 + 0xc);
  return iVar2;
}



// === objlist_replace_element @ 0fb61450 (92 bytes) ===

void objlist_replace_element(int *param_1,int param_2,int param_3)

{
  if (*param_1 == param_2) {
    *param_1 = param_3;
  }
  else {
    *(int *)(*(int *)(param_2 + 0xc) + 8) = param_3;
  }
  if (*(int *)(param_2 + 8) != 0) {
    *(int *)(*(int *)(param_2 + 8) + 0xc) = param_3;
    *(undefined4 *)(param_3 + 0xc) = *(undefined4 *)(param_2 + 0xc);
    *(undefined4 *)(param_3 + 8) = *(undefined4 *)(param_2 + 8);
    return;
  }
  *(int *)(*param_1 + 0xc) = param_3;
  *(undefined4 *)(param_3 + 0xc) = *(undefined4 *)(param_2 + 0xc);
  *(undefined4 *)(param_3 + 8) = *(undefined4 *)(param_2 + 8);
  return;
}



// === foreach_obj @ 0fb614b0 (124 bytes) ===

void foreach_obj(int param_1,code *param_2,undefined8 param_3)

{
  int iVar1;
  longlong lVar2;
  code *UNRECOVERED_JUMPTABLE_00;
  
  do {
    if (param_1 == 0) {
                    /* WARNING: Could not recover jumptable at 0x0fb61524. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (*UNRECOVERED_JUMPTABLE_00)();
      return;
    }
    iVar1 = *(int *)(param_1 + 8);
    lVar2 = (*param_2)(param_1,param_3);
    param_1 = iVar1;
  } while (lVar2 == -1);
                    /* WARNING: Could not recover jumptable at 0x0fb614fc. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE_00)(lVar2);
  return;
}



// === forall_previous_objs @ 0fb61530 (244 bytes) ===

undefined8 forall_previous_objs(int param_1,int param_2,code *param_3)

{
  int iVar1;
  longlong lVar2;
  undefined8 uVar3;
  int iVar4;
  code *UNRECOVERED_JUMPTABLE_00;
  
  iVar4 = param_1;
  if ((param_1 != 0) && (param_1 != param_2)) {
    for (iVar1 = *(int *)(param_1 + 8); iVar1 != 0; iVar1 = *(int *)(iVar1 + 8)) {
      if (iVar1 == 0) {
        return 0xffffffffffffffff;
      }
      iVar4 = iVar1;
      if (param_2 == iVar1) break;
    }
  }
  if ((iVar4 != 0) && (param_2 == iVar4)) {
    iVar4 = *(int *)(iVar4 + 0xc);
    if (*(int *)(param_1 + 0xc) != iVar4) {
      do {
        lVar2 = (*param_3)(iVar4,param_2);
        if (lVar2 != -1) {
                    /* WARNING: Could not recover jumptable at 0x0fb615ec. Too many branches */
                    /* WARNING: Treating indirect jump as call */
          uVar3 = (*UNRECOVERED_JUMPTABLE_00)(0xffffffffffffffff,lVar2);
          return uVar3;
        }
        iVar4 = *(int *)(iVar4 + 0xc);
      } while (*(int *)(param_1 + 0xc) != iVar4);
    }
                    /* WARNING: Could not recover jumptable at 0x0fb6161c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    uVar3 = (*UNRECOVERED_JUMPTABLE_00)();
    return uVar3;
  }
  return 0xffffffffffffffff;
}



// === list_last @ 0fb61630 (36 bytes) ===

int list_last(int param_1)

{
  if ((param_1 != 0) && (*(int *)(param_1 + 0xc) != 0)) {
    return *(int *)(param_1 + 0xc);
  }
  return -1;
}



// === foreach_sublist @ 0fb61660 (132 bytes) ===

void foreach_sublist(int param_1,code *param_2,undefined8 param_3)

{
  int iVar1;
  int iVar2;
  longlong lVar3;
  code *UNRECOVERED_JUMPTABLE_00;
  
  iVar2 = param_1;
  while( true ) {
    if (iVar2 == 0) {
                    /* WARNING: Could not recover jumptable at 0x0fb616dc. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (*UNRECOVERED_JUMPTABLE_00)(param_1);
      return;
    }
    iVar1 = *(int *)(iVar2 + 8);
    lVar3 = (*param_2)(iVar2,param_3);
    if (lVar3 != -1) break;
    param_1 = -1;
    iVar2 = iVar1;
  }
                    /* WARNING: Could not recover jumptable at 0x0fb616b4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE_00)();
  return;
}



// === _malloc @ 0fb61820 (92 bytes) ===

undefined8 _malloc(ulonglong param_1)

{
  undefined8 uVar1;
  
  if (param_1 < 0x400) {
    param_1 = 0x400;
  }
  if (multi_threaded != '\0') {
    enter_malloc();
    uVar1 = FUN_0fb61880((int)param_1);
    exit_malloc();
    return uVar1;
  }
  uVar1 = FUN_0fb61880(param_1);
  return uVar1;
}



// === FUN_0fb61880 @ 0fb61880 (916 bytes) ===

uint * FUN_0fb61880(uint param_1)

{
  uint *puVar1;
  uint *puVar2;
  uint *puVar3;
  uint uVar4;
  uint uVar5;
  
  puVar2 = DAT_0fbd318c;
  if ((param_1 & 7) != 0) {
    param_1 = (param_1 - (param_1 & 7)) + 8;
  }
  if (DAT_0fbd318c != (uint *)0x0) {
    uVar4 = DAT_0fbd318c[-2] & 0xfffffffc;
    puVar3 = DAT_0fbd318c + -2;
    if (uVar4 == param_1) {
      DAT_0fbd318c = (uint *)0x0;
      DAT_0fbd3198 = DAT_0fbd3198 + 1 & 1;
      (&DAT_0fbd3190)[DAT_0fbd3198] = 0;
      return puVar2;
    }
    if ((0x27 < param_1) && (param_1 < uVar4)) {
      DAT_0fbd3198 = DAT_0fbd3198 + 1 & 1;
      DAT_0fbd318c = (uint *)0x0;
      (&DAT_0fbd3190)[DAT_0fbd3198] = 0;
      uVar5 = puVar2[-2];
      puVar2[-2] = uVar4;
      uVar5 = uVar5 & 2;
      goto LAB_0fb6193c;
    }
  }
  FUN_0fb626c0(0);
  if (param_1 < 0x28) {
    if (param_1 == 0) {
      param_1 = 8;
    }
    if (DAT_0fbd319c != (uint *)0x0) {
      uVar4 = *DAT_0fbd319c;
      puVar3 = (uint *)0x0;
      puVar2 = DAT_0fbd319c;
      while( true ) {
        puVar1 = (uint *)puVar2[2];
        if (param_1 <= uVar4) {
          puVar1 = (uint *)((int)puVar2 + param_1 + 8);
          if (uVar4 - param_1 < 0x11) {
            puVar1 = (uint *)puVar2[2];
          }
          else {
            *puVar2 = param_1;
            *(uint *)((int)puVar2 + param_1 + 8) = (uVar4 - param_1) - 8;
            *(uint *)((int)puVar2 + param_1 + 0x10) = puVar2[2];
          }
          if (puVar3 != (uint *)0x0) {
            puVar3[2] = (uint)puVar1;
            puVar1 = DAT_0fbd319c;
          }
          DAT_0fbd319c = puVar1;
          uVar4 = *puVar2;
          if (0x27 < uVar4) {
            *puVar2 = 0x20;
            uVar4 = 0x20;
          }
          *puVar2 = uVar4 | 1;
          return puVar2 + 2;
        }
        if (puVar1 == (uint *)0x0) break;
        uVar4 = *puVar1;
        puVar3 = puVar2;
        puVar2 = puVar1;
      }
    }
    puVar2 = (uint *)FUN_0fb61880(0xa0);
    if (puVar2 == (uint *)0x0) {
      return (uint *)0x0;
    }
    *puVar2 = param_1;
    *(uint *)((int)puVar2 + param_1 + 8) = 0x90 - param_1;
    *(uint **)((int)puVar2 + param_1 + 0x10) = DAT_0fbd319c;
    DAT_0fbd319c = (uint *)((int)puVar2 + param_1 + 8);
    *puVar2 = *puVar2 | 1;
    return puVar2 + 2;
  }
  if (DAT_0fbd3180 == (uint *)0x0) {
LAB_0fb61a88:
    if ((DAT_0fbd3184 == (uint *)0x0) || (*DAT_0fbd3184 < param_1)) {
      puVar3 = (uint *)FUN_0fb620f0(param_1);
      if (puVar3 == (uint *)0x0) {
        return (uint *)0x0;
      }
      goto LAB_0fb6191c;
    }
    uVar4 = *DAT_0fbd3184;
    puVar3 = DAT_0fbd3184;
  }
  else {
    uVar4 = 0;
    puVar3 = (uint *)0x0;
    puVar2 = DAT_0fbd3180;
    do {
      while (puVar1 = puVar2, uVar5 = *puVar1, param_1 <= uVar5) {
        if ((uVar4 == 0) || (uVar5 <= uVar4)) {
          uVar4 = uVar5;
          puVar3 = puVar1;
        }
        puVar2 = (uint *)puVar1[4];
        if ((uint *)puVar1[4] == (uint *)0x0) goto LAB_0fb61904;
      }
      puVar2 = (uint *)puVar1[6];
    } while ((uint *)puVar1[6] != (uint *)0x0);
LAB_0fb61904:
    if (puVar3 == (uint *)0x0) {
      if (puVar1 != DAT_0fbd3180) {
        FUN_0fb62450(puVar1);
        DAT_0fbd3180 = puVar1;
      }
    }
    else {
      FUN_0fb622c0(puVar3);
    }
    if (puVar3 == (uint *)0x0) goto LAB_0fb61a88;
LAB_0fb6191c:
    uVar4 = *puVar3;
  }
  *(uint *)((int)puVar3 + uVar4 + 8) = *(uint *)((int)puVar3 + uVar4 + 8) & 0xfffffffd;
  uVar5 = 0;
  uVar4 = *puVar3;
LAB_0fb6193c:
  if ((int)(uVar4 - param_1) < 0x30) {
    if (DAT_0fbd3188 == (int)puVar3 + uVar4 + 0x10) {
      DAT_0fbd3184 = (uint *)0x0;
    }
  }
  else {
    *puVar3 = param_1;
    *(uint *)((int)puVar3 + param_1 + 8) = (uVar4 - param_1) - 8 | 1;
    FUN_0fb61ee0((int)puVar3 + param_1 + 0x10);
    uVar4 = *puVar3;
  }
  *puVar3 = uVar4 | uVar5 | 1;
  return puVar3 + 2;
}



// === _realloc @ 0fb61af0 (1008 bytes) ===

uint * _realloc(uint *param_1,uint param_2)

{
  bool bVar1;
  uint uVar2;
  bool bVar3;
  bool bVar4;
  uint uVar5;
  uint *puVar6;
  uint uVar7;
  uint uVar8;
  uint *puVar9;
  int *piVar10;
  uint __n;
  uint uVar11;
  
  if (param_1 == (uint *)0x0) {
    puVar9 = (uint *)malloc(param_2);
    return puVar9;
  }
  if (param_2 < 0x400) {
    param_2 = 0x400;
  }
  bVar1 = multi_threaded != '\0';
  if (bVar1) {
    enter_malloc();
  }
  FUN_0fb626c0(param_1);
  if ((param_2 & 7) != 0) {
    param_2 = (param_2 - (param_2 & 7)) + 8;
  }
  uVar2 = param_1[-2];
  puVar9 = param_1 + -2;
  if ((uVar2 & 1) == 0) {
    if (bVar1) {
      exit_malloc();
    }
    return (uint *)0x0;
  }
  __n = uVar2 & 0xfffffffc;
  param_1[-2] = __n;
  if (__n == param_2) {
    param_1[-2] = uVar2;
    if (bVar1) {
      exit_malloc();
    }
  }
  else {
    uVar7 = __n;
    if ((0x27 < param_2) && (0x27 < __n)) {
      if (__n < param_2) {
        uVar11 = *(uint *)((int)puVar9 + __n + 8);
        piVar10 = (int *)((int)puVar9 + __n + 8);
        if ((uVar11 & 1) == 0) {
          bVar3 = DAT_0fbd3184 == piVar10;
          param_1[-2] = __n + uVar11 + 8;
          if (bVar3) {
            DAT_0fbd3184 = (int *)0x0;
          }
          else {
            FUN_0fb622c0(piVar10);
          }
          *(uint *)((int)piVar10 + *piVar10 + 8) =
               *(uint *)((int)piVar10 + *piVar10 + 8) & 0xfffffffd;
          uVar7 = *puVar9;
        }
      }
      uVar11 = uVar2 & 3;
      if (param_2 <= uVar7) {
LAB_0fb61cf8:
        if ((int)(uVar7 - param_2) < 0x30) {
          if (DAT_0fbd3188 == (int)puVar9 + uVar7 + 0x10) {
            DAT_0fbd3184 = (int *)0x0;
          }
        }
        else {
          *puVar9 = param_2;
          *(uint *)((int)puVar9 + param_2 + 8) = (uVar7 - param_2) - 8 | 1;
          FUN_0fb61ee0((int)puVar9 + param_2 + 0x10);
          uVar7 = *puVar9;
        }
        *puVar9 = uVar11 | uVar7;
        if (!bVar1) {
          return param_1;
        }
        exit_malloc();
        return param_1;
      }
      uVar7 = param_1[-2];
    }
    uVar11 = uVar2 & 3;
    uVar5 = uVar2 & 3;
    do {
      param_1[-2] = uVar7 | uVar5;
      puVar6 = (uint *)FUN_0fb61880(param_2);
      if (puVar6 != (uint *)0x0) {
        if (param_2 < __n) {
          __n = param_2;
        }
        memmove(puVar6,param_1,__n);
        __free(param_1);
        if (bVar1) {
          exit_malloc();
        }
        return puVar6;
      }
      uVar7 = param_1[-2] & 0xfffffffc;
      param_1[-2] = uVar7;
      if (0x27 < uVar7) {
        if (param_2 < 0x28) {
          param_2 = 0x28;
          uVar7 = *puVar9;
          goto LAB_0fb61cf8;
        }
        if (((uVar2 & 2) != 0) && (puVar6 = (uint *)param_1[-4], param_2 <= *puVar6 + uVar7 + 8)) {
          FUN_0fb622c0(puVar6);
          *puVar6 = param_1[-2] + *puVar6 + 8;
          memmove(puVar6 + 2,param_1,param_1[-2]);
          uVar11 = uVar2 & 1;
          uVar7 = *puVar6;
          puVar9 = puVar6;
          param_1 = puVar6 + 2;
          goto LAB_0fb61cf8;
        }
        uVar8 = *puVar9;
        break;
      }
      uVar8 = *puVar9;
      bVar4 = param_2 < uVar8;
      bVar3 = param_2 < 0x28;
      param_2 = 0x28;
      if (bVar4) {
        *puVar9 = uVar8 | uVar5;
        if (!bVar1) {
          return param_1;
        }
        exit_malloc();
        return param_1;
      }
    } while (bVar3);
    *puVar9 = uVar8 | uVar5;
    if (bVar1) {
      exit_malloc();
    }
    param_1 = (uint *)0x0;
  }
  return param_1;
}



// === FUN_0fb61ee0 @ 0fb61ee0 (524 bytes) ===

void FUN_0fb61ee0(undefined4 *param_1)

{
  uint uVar1;
  uint *puVar2;
  int iVar3;
  uint uVar4;
  uint *puVar5;
  uint *puVar6;
  
  uVar1 = param_1[-2];
  puVar6 = param_1 + -2;
  uVar4 = uVar1 & 0xfffffffc;
  if ((uVar1 & 1) == 0) {
    return;
  }
  param_1[-2] = uVar4;
  if (uVar4 < 0x28) {
    *param_1 = DAT_0fbd319c;
    DAT_0fbd319c = puVar6;
    return;
  }
  puVar5 = (uint *)(uVar4 + (int)param_1);
  uVar4 = *puVar5;
  if ((uVar4 & 1) == 0) {
    if (DAT_0fbd3184 != puVar5) {
      FUN_0fb622c0(puVar5);
      uVar4 = *puVar5;
    }
    *puVar6 = *puVar6 + uVar4 + 8;
  }
  puVar5 = puVar6;
  if ((uVar1 & 2) != 0) {
    puVar5 = (uint *)param_1[-4];
    FUN_0fb622c0(puVar5);
    *puVar5 = *puVar6 + *puVar5 + 8;
  }
  puVar5[2] = 0;
  puVar5[4] = 0;
  puVar5[6] = 0;
  puVar5[8] = 0;
  *(uint **)(*puVar5 + (int)puVar5) = puVar5;
  uVar1 = *puVar5;
  iVar3 = uVar1 + (int)puVar5;
  puVar6 = DAT_0fbd3180;
  puVar2 = puVar5;
  if ((DAT_0fbd3188 != iVar3 + 0x10) &&
     (puVar6 = puVar5, puVar2 = DAT_0fbd3184, DAT_0fbd3180 != (uint *)0x0)) {
    uVar4 = *DAT_0fbd3180;
    puVar6 = DAT_0fbd3180;
    do {
      if (uVar1 < uVar4) {
        puVar2 = (uint *)puVar6[4];
        if ((uint *)puVar6[4] == (uint *)0x0) goto LAB_0fb620a4;
      }
      else {
        if (uVar1 <= uVar4) {
          uVar1 = puVar6[2];
          puVar2 = puVar5;
          if (uVar1 != 0) {
            if (*(uint **)(uVar1 + 0x10) == puVar6) {
              *(uint **)(uVar1 + 0x10) = puVar5;
            }
            else {
              *(uint **)(uVar1 + 0x18) = puVar5;
            }
            puVar5[2] = uVar1;
            puVar2 = DAT_0fbd3180;
          }
          DAT_0fbd3180 = puVar2;
          uVar1 = puVar6[4];
          if (uVar1 != 0) {
            *(uint **)(uVar1 + 8) = puVar5;
          }
          puVar5[4] = uVar1;
          uVar1 = puVar6[6];
          if (uVar1 != 0) {
            *(uint **)(uVar1 + 8) = puVar5;
          }
          puVar5[8] = (uint)puVar6;
          puVar5[6] = uVar1;
          puVar6[4] = 0xffffffff;
          puVar6[2] = (uint)puVar5;
          goto LAB_0fb62020;
        }
        puVar2 = (uint *)puVar6[6];
        if ((uint *)puVar6[6] == (uint *)0x0) {
          puVar6[6] = (uint)puVar5;
          puVar5[2] = (uint)puVar6;
          goto LAB_0fb62020;
        }
      }
      puVar6 = puVar2;
      uVar4 = *puVar6;
    } while( true );
  }
LAB_0fb62028:
  DAT_0fbd3184 = puVar2;
  DAT_0fbd3180 = puVar6;
  *(uint *)(iVar3 + 8) = *(uint *)(iVar3 + 8) | 2;
  return;
LAB_0fb620a4:
  puVar6[4] = (uint)puVar5;
  puVar5[2] = (uint)puVar6;
LAB_0fb62020:
  iVar3 = *puVar5 + (int)puVar5;
  puVar6 = DAT_0fbd3180;
  puVar2 = DAT_0fbd3184;
  goto LAB_0fb62028;
}



// === FUN_0fb620f0 @ 0fb620f0 (456 bytes) ===

/* WARNING: Instruction at (ram,0x0fb621bc) overlaps instruction at (ram,0x0fb621b8)
    */

void FUN_0fb620f0(int param_1)

{
  bool bVar1;
  uint *puVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  uint *puVar6;
  code *UNRECOVERED_JUMPTABLE_00;
  
  puVar2 = DAT_0fbd3184;
  uVar3 = _sbrk(0);
  if ((uVar3 & 7) == 0) {
    iVar5 = 0;
    if (DAT_0fbd3188 == 0) {
      uVar3 = pagesize - 1 & pagesize - (pagesize - 1 & uVar3);
      iVar5 = 0;
      if (param_1 + 0x10U <= uVar3) goto LAB_0fb6215c;
      iVar5 = 0;
      goto code_r0x0fb62254;
    }
  }
  else {
    iVar5 = 8 - (uVar3 & 7);
    if (DAT_0fbd3188 == 0) {
      uVar3 = pagesize - 1 & pagesize - (pagesize - 1 & uVar3 + iVar5);
      if (param_1 + 0x10U <= uVar3) goto LAB_0fb6215c;
code_r0x0fb62254:
      if (pagesize == 0) {
        trap(7);
      }
      uVar3 = uVar3 + pagesize * ((param_1 + 0xfU) / pagesize + 1);
      goto LAB_0fb6215c;
    }
  }
  uVar3 = pagesize * ((param_1 + 0xfU) / pagesize + 1);
  if (pagesize == 0) {
    trap(7);
  }
LAB_0fb6215c:
  iVar4 = _sbrk(uVar3 + iVar5);
  if (iVar4 == -1) {
                    /* WARNING: Could not recover jumptable at 0x0fb621f4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*UNRECOVERED_JUMPTABLE_00)();
    return;
  }
  puVar6 = (uint *)(iVar4 + iVar5);
  if (iVar4 == DAT_0fbd3188) {
    if (puVar2 == (uint *)0x0) {
      uVar3 = uVar3 + 8;
      puVar6 = (uint *)(DAT_0fbd3188 + -8);
    }
    else {
      uVar3 = *puVar2 + uVar3 + 0x10;
      puVar6 = puVar2;
    }
  }
  *puVar6 = uVar3 - 0x10;
  puVar2 = DAT_0fbd3184;
  DAT_0fbd3188 = uVar3 + (int)puVar6;
  bVar1 = DAT_0fbd3184 != (uint *)0x0;
  *(uint *)(DAT_0fbd3188 + -8) = *(uint *)(DAT_0fbd3188 + -8) | 1;
  if ((bVar1) && (puVar6 != puVar2)) {
    *puVar2 = *puVar2 | 1;
    FUN_0fb61ee0(puVar2 + 2);
  }
                    /* WARNING: Could not recover jumptable at 0x0fb621d4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE_00)();
  return;
}



// === FUN_0fb622c0 @ 0fb622c0 (396 bytes) ===

void FUN_0fb622c0(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  
  iVar1 = *(int *)(param_1 + 8);
  if (*(int *)(param_1 + 0x10) == -1) {
    iVar2 = *(int *)(param_1 + 0x20);
    if (iVar2 != 0) {
      *(int *)(iVar2 + 8) = iVar1;
    }
    *(int *)(iVar1 + 0x20) = iVar2;
    return;
  }
  if (iVar1 != 0) {
    FUN_0fb62450();
  }
  DAT_0fbd3180 = *(int *)(param_1 + 0x20);
  if (DAT_0fbd3180 != 0) {
    *(undefined4 *)(DAT_0fbd3180 + 8) = 0;
    iVar1 = *(int *)(param_1 + 0x10);
    if (iVar1 != 0) {
      *(int *)(iVar1 + 8) = DAT_0fbd3180;
    }
    *(int *)(DAT_0fbd3180 + 0x10) = iVar1;
    iVar1 = *(int *)(param_1 + 0x18);
    if (iVar1 != 0) {
      *(int *)(iVar1 + 8) = DAT_0fbd3180;
    }
    *(int *)(DAT_0fbd3180 + 0x18) = iVar1;
    return;
  }
  DAT_0fbd3180 = *(int *)(param_1 + 0x10);
  if (DAT_0fbd3180 == 0) {
    DAT_0fbd3180 = *(int *)(param_1 + 0x18);
    if (DAT_0fbd3180 != 0) {
      *(undefined4 *)(DAT_0fbd3180 + 8) = 0;
    }
  }
  else {
    *(undefined4 *)(DAT_0fbd3180 + 8) = 0;
    iVar1 = *(int *)(param_1 + 0x18);
    if (iVar1 != 0) {
      iVar2 = *(int *)(DAT_0fbd3180 + 0x18);
      if (iVar2 != 0) {
        iVar1 = *(int *)(iVar2 + 0x18);
        iVar3 = DAT_0fbd3180;
        DAT_0fbd3180 = iVar2;
        while( true ) {
          iVar2 = *(int *)(DAT_0fbd3180 + 0x10);
          if (iVar1 == 0) {
            *(int *)(iVar3 + 0x18) = iVar2;
            if (iVar2 != 0) {
              *(int *)(iVar2 + 8) = iVar3;
            }
            iVar1 = *(int *)(iVar3 + 8);
            *(int *)(DAT_0fbd3180 + 8) = iVar1;
            if (iVar1 != 0) {
              if (*(int *)(*(int *)(iVar3 + 8) + 0x10) == iVar3) {
                *(int *)(iVar1 + 0x10) = DAT_0fbd3180;
              }
              else {
                *(int *)(iVar1 + 0x18) = DAT_0fbd3180;
              }
            }
            *(int *)(DAT_0fbd3180 + 0x10) = iVar3;
            *(int *)(iVar3 + 8) = DAT_0fbd3180;
          }
          else {
            iVar2 = *(int *)(iVar1 + 0x10);
            *(int *)(DAT_0fbd3180 + 0x18) = iVar2;
            if (iVar2 != 0) {
              *(int *)(iVar2 + 8) = DAT_0fbd3180;
            }
            iVar2 = *(int *)(iVar3 + 8);
            *(int *)(iVar1 + 8) = iVar2;
            if (iVar2 != 0) {
              if (*(int *)(*(int *)(iVar3 + 8) + 0x10) == iVar3) {
                *(int *)(iVar2 + 0x10) = iVar1;
              }
              else {
                *(int *)(iVar2 + 0x18) = iVar1;
              }
            }
            *(int *)(iVar3 + 8) = iVar1;
            *(int *)(iVar1 + 0x10) = iVar3;
            DAT_0fbd3180 = iVar1;
          }
          iVar2 = *(int *)(DAT_0fbd3180 + 0x18);
          if (iVar2 == 0) break;
          iVar1 = *(int *)(iVar2 + 0x18);
          iVar3 = DAT_0fbd3180;
          DAT_0fbd3180 = iVar2;
        }
        iVar1 = *(int *)(param_1 + 0x18);
      }
      *(int *)(DAT_0fbd3180 + 0x18) = iVar1;
      *(int *)(iVar1 + 8) = DAT_0fbd3180;
    }
  }
  return;
}



// === FUN_0fb62450 @ 0fb62450 (408 bytes) ===

void FUN_0fb62450(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  
  iVar2 = *(int *)(param_1 + 8);
  if (iVar2 != 0) {
    iVar1 = *(int *)(iVar2 + 0x10);
    while( true ) {
      iVar3 = *(int *)(iVar2 + 8);
      if (iVar1 == param_1) {
        if ((iVar3 == 0) || (*(int *)(iVar3 + 0x10) != iVar2)) {
          iVar1 = *(int *)(param_1 + 0x18);
          *(int *)(iVar2 + 0x10) = iVar1;
          if (iVar1 != 0) {
            *(int *)(iVar1 + 8) = iVar2;
            iVar3 = *(int *)(iVar2 + 8);
          }
          *(int *)(param_1 + 8) = iVar3;
          if (iVar3 != 0) {
            if (*(int *)(*(int *)(iVar2 + 8) + 0x10) == iVar2) {
              *(int *)(iVar3 + 0x10) = param_1;
            }
            else {
              *(int *)(iVar3 + 0x18) = param_1;
            }
          }
          *(int *)(param_1 + 0x18) = iVar2;
          *(int *)(iVar2 + 8) = param_1;
        }
        else {
          iVar1 = *(int *)(iVar2 + 0x18);
          *(int *)(iVar3 + 0x10) = iVar1;
          if (iVar1 != 0) {
            *(int *)(iVar1 + 8) = iVar3;
          }
          iVar1 = *(int *)(param_1 + 0x18);
          *(int *)(iVar2 + 0x10) = iVar1;
          if (iVar1 != 0) {
            *(int *)(iVar1 + 8) = iVar2;
          }
          iVar1 = *(int *)(iVar3 + 8);
          *(int *)(param_1 + 8) = iVar1;
          if (iVar1 != 0) {
            if (*(int *)(*(int *)(iVar3 + 8) + 0x10) == iVar3) {
              *(int *)(iVar1 + 0x10) = param_1;
            }
            else {
              *(int *)(iVar1 + 0x18) = param_1;
            }
          }
          *(int *)(param_1 + 0x18) = iVar2;
          *(int *)(iVar2 + 0x18) = iVar3;
          *(int *)(iVar2 + 8) = param_1;
          *(int *)(iVar3 + 8) = iVar2;
        }
      }
      else if ((iVar3 == 0) || (*(int *)(iVar3 + 0x18) != iVar2)) {
        iVar1 = *(int *)(param_1 + 0x10);
        *(int *)(iVar2 + 0x18) = iVar1;
        if (iVar1 != 0) {
          *(int *)(iVar1 + 8) = iVar2;
          iVar3 = *(int *)(iVar2 + 8);
        }
        *(int *)(param_1 + 8) = iVar3;
        if (iVar3 != 0) {
          if (*(int *)(*(int *)(iVar2 + 8) + 0x10) == iVar2) {
            *(int *)(iVar3 + 0x10) = param_1;
          }
          else {
            *(int *)(iVar3 + 0x18) = param_1;
          }
        }
        *(int *)(param_1 + 0x10) = iVar2;
        *(int *)(iVar2 + 8) = param_1;
      }
      else {
        *(int *)(iVar3 + 0x18) = iVar1;
        if (iVar1 != 0) {
          *(int *)(iVar1 + 8) = iVar3;
        }
        iVar1 = *(int *)(param_1 + 0x10);
        *(int *)(iVar2 + 0x18) = iVar1;
        if (iVar1 != 0) {
          *(int *)(iVar1 + 8) = iVar2;
        }
        iVar1 = *(int *)(iVar3 + 8);
        *(int *)(param_1 + 8) = iVar1;
        if (iVar1 != 0) {
          if (*(int *)(*(int *)(iVar3 + 8) + 0x10) == iVar3) {
            *(int *)(iVar1 + 0x10) = param_1;
          }
          else {
            *(int *)(iVar1 + 0x18) = param_1;
          }
        }
        *(int *)(param_1 + 0x10) = iVar2;
        *(int *)(iVar2 + 0x10) = iVar3;
        *(int *)(iVar2 + 8) = param_1;
        *(int *)(iVar3 + 8) = iVar2;
      }
      iVar2 = *(int *)(param_1 + 8);
      if (iVar2 == 0) break;
      iVar1 = *(int *)(iVar2 + 0x10);
    }
  }
  return;
}



// === _free @ 0fb62600 (80 bytes) ===

void _free(undefined8 param_1)

{
  if (multi_threaded == '\0') {
    __free();
  }
  else {
    enter_malloc();
    __free(param_1);
    exit_malloc();
  }
  return;
}



// === __free @ 0fb62650 (112 bytes) ===

void __free(longlong param_1)

{
  int *piVar1;
  
  if (param_1 != 0) {
    piVar1 = &DAT_0fbd3190 + DAT_0fbd3198;
    if (*piVar1 != 0) {
      FUN_0fb61ee0();
      piVar1 = &DAT_0fbd3190 + DAT_0fbd3198;
    }
    *piVar1 = (int)param_1;
    DAT_0fbd318c = (int)param_1;
    DAT_0fbd3198 = DAT_0fbd3198 + 1 & 1;
    return;
  }
  return;
}



// === FUN_0fb626c0 @ 0fb626c0 (144 bytes) ===

void FUN_0fb626c0(int param_1)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  code *UNRECOVERED_JUMPTABLE;
  
  puVar1 = &DAT_0fbd3190 + DAT_0fbd3198;
  while( true ) {
    puVar2 = puVar1;
    if (puVar1 == &DAT_0fbd3190) {
      puVar2 = &DAT_0fbd3198;
    }
    puVar1 = puVar2 + -1;
    if (puVar2[-1] == 0) break;
    if (puVar2[-1] == param_1) {
      *puVar1 = 0;
    }
    else {
      FUN_0fb61ee0();
      *puVar1 = 0;
    }
  }
  DAT_0fbd318c = 0;
  DAT_0fbd3198 = 0;
                    /* WARNING: Could not recover jumptable at 0x0fb62754. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
}



// === malloc @ 0fb62760 (8 bytes) ===

undefined8 malloc(ulonglong param_1)

{
  undefined8 uVar1;
  
  if (param_1 < 0x400) {
    param_1 = 0x400;
  }
  if (multi_threaded != '\0') {
    enter_malloc();
    uVar1 = FUN_0fb61880((int)param_1);
    exit_malloc();
    return uVar1;
  }
  uVar1 = FUN_0fb61880(param_1);
  return uVar1;
}



// === realloc @ 0fb62770 (8 bytes) ===

uint * realloc(uint *param_1,uint param_2)

{
  bool bVar1;
  uint uVar2;
  bool bVar3;
  bool bVar4;
  uint uVar5;
  uint *puVar6;
  uint uVar7;
  uint uVar8;
  uint *puVar9;
  int *piVar10;
  uint __n;
  uint uVar11;
  
  if (param_1 == (uint *)0x0) {
    puVar9 = (uint *)malloc(param_2);
    return puVar9;
  }
  if (param_2 < 0x400) {
    param_2 = 0x400;
  }
  bVar1 = multi_threaded != '\0';
  if (bVar1) {
    enter_malloc();
  }
  FUN_0fb626c0(param_1);
  if ((param_2 & 7) != 0) {
    param_2 = (param_2 - (param_2 & 7)) + 8;
  }
  uVar2 = param_1[-2];
  puVar9 = param_1 + -2;
  if ((uVar2 & 1) == 0) {
    if (bVar1) {
      exit_malloc();
    }
    return (uint *)0x0;
  }
  __n = uVar2 & 0xfffffffc;
  param_1[-2] = __n;
  if (__n == param_2) {
    param_1[-2] = uVar2;
    if (bVar1) {
      exit_malloc();
    }
  }
  else {
    uVar7 = __n;
    if ((0x27 < param_2) && (0x27 < __n)) {
      if (__n < param_2) {
        uVar11 = *(uint *)((int)puVar9 + __n + 8);
        piVar10 = (int *)((int)puVar9 + __n + 8);
        if ((uVar11 & 1) == 0) {
          bVar3 = DAT_0fbd3184 == piVar10;
          param_1[-2] = __n + uVar11 + 8;
          if (bVar3) {
            DAT_0fbd3184 = (int *)0x0;
          }
          else {
            FUN_0fb622c0(piVar10);
          }
          *(uint *)((int)piVar10 + *piVar10 + 8) =
               *(uint *)((int)piVar10 + *piVar10 + 8) & 0xfffffffd;
          uVar7 = *puVar9;
        }
      }
      uVar11 = uVar2 & 3;
      if (param_2 <= uVar7) {
LAB_0fb61cf8:
        if ((int)(uVar7 - param_2) < 0x30) {
          if (DAT_0fbd3188 == (int)puVar9 + uVar7 + 0x10) {
            DAT_0fbd3184 = (int *)0x0;
          }
        }
        else {
          *puVar9 = param_2;
          *(uint *)((int)puVar9 + param_2 + 8) = (uVar7 - param_2) - 8 | 1;
          FUN_0fb61ee0((int)puVar9 + param_2 + 0x10);
          uVar7 = *puVar9;
        }
        *puVar9 = uVar11 | uVar7;
        if (!bVar1) {
          return param_1;
        }
        exit_malloc();
        return param_1;
      }
      uVar7 = param_1[-2];
    }
    uVar11 = uVar2 & 3;
    uVar5 = uVar2 & 3;
    do {
      param_1[-2] = uVar7 | uVar5;
      puVar6 = (uint *)FUN_0fb61880(param_2);
      if (puVar6 != (uint *)0x0) {
        if (param_2 < __n) {
          __n = param_2;
        }
        memmove(puVar6,param_1,__n);
        __free(param_1);
        if (bVar1) {
          exit_malloc();
        }
        return puVar6;
      }
      uVar7 = param_1[-2] & 0xfffffffc;
      param_1[-2] = uVar7;
      if (0x27 < uVar7) {
        if (param_2 < 0x28) {
          param_2 = 0x28;
          uVar7 = *puVar9;
          goto LAB_0fb61cf8;
        }
        if (((uVar2 & 2) != 0) && (puVar6 = (uint *)param_1[-4], param_2 <= *puVar6 + uVar7 + 8)) {
          FUN_0fb622c0(puVar6);
          *puVar6 = param_1[-2] + *puVar6 + 8;
          memmove(puVar6 + 2,param_1,param_1[-2]);
          uVar11 = uVar2 & 1;
          uVar7 = *puVar6;
          puVar9 = puVar6;
          param_1 = puVar6 + 2;
          goto LAB_0fb61cf8;
        }
        uVar8 = *puVar9;
        break;
      }
      uVar8 = *puVar9;
      bVar4 = param_2 < uVar8;
      bVar3 = param_2 < 0x28;
      param_2 = 0x28;
      if (bVar4) {
        *puVar9 = uVar8 | uVar5;
        if (!bVar1) {
          return param_1;
        }
        exit_malloc();
        return param_1;
      }
    } while (bVar3);
    *puVar9 = uVar8 | uVar5;
    if (bVar1) {
      exit_malloc();
    }
    param_1 = (uint *)0x0;
  }
  return param_1;
}



// === free @ 0fb62780 (8 bytes) ===

void free(undefined8 param_1)

{
  if (multi_threaded == '\0') {
    __free();
  }
  else {
    enter_malloc();
    __free(param_1);
    exit_malloc();
  }
  return;
}



// === obj_init @ 0fb627a0 (1408 bytes) ===

void * obj_init(undefined4 *param_1)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  int *piVar5;
  void *pvVar6;
  longlong lVar4;
  uint uVar7;
  int *piVar8;
  int iVar9;
  int iVar10;
  int unaff_gp_lo;
  
  if (*(char *)(unaff_gp_lo + -0x7eec) != '\0') {
    debug(unaff_gp_lo + -28000,param_1[6]);
  }
  *param_1 = 0xffffffff;
  param_1[1] = 0x20;
  iVar9 = 0;
  piVar8 = (int *)param_1[8];
  iVar1 = param_1[4];
  iVar10 = 0;
  param_1[0xf] = iVar1;
  if (piVar8 == (int *)0x0) {
    piVar8 = (int *)(*(int *)(iVar1 + 0x1c) + iVar1);
    param_1[8] = piVar8;
  }
  param_1[0x15] = *(undefined4 *)(iVar1 + 0x18);
  uVar7 = (uint)*(ushort *)(iVar1 + 0x2c);
  if (uVar7 == 0) {
    iVar9 = 0;
    *(byte *)(param_1 + 0x30) = (byte)((uint)*(undefined4 *)(iVar1 + 0x24) >> 0x1c);
  }
  else {
    uVar3 = 0xffffffff;
    do {
      if (*piVar8 == 1) {
        if ((piVar8[6] & 6U) == 4) {
          uVar7 = piVar8[1];
          if ((uVar3 == 0xffffffff) || (uVar7 <= uVar3)) {
            iVar2 = piVar8[2];
            param_1[5] = iVar2 - uVar7;
            param_1[0xf] = iVar2 - uVar7;
            uVar3 = uVar7;
          }
          if (param_1[0x10] == -1) {
            param_1[0x10] = piVar8[2];
            param_1[0x11] = piVar8[4];
          }
          else {
            piVar5 = (int *)malloc(0xc);
            if (piVar5 == (int *)0x0) {
              fatal(unaff_gp_lo + -0x6d20);
            }
            *piVar5 = piVar8[2];
            piVar5[1] = piVar8[4];
            piVar5[2] = param_1[0x42];
            param_1[0x42] = piVar5;
          }
          uVar7 = (uint)*(ushort *)(iVar1 + 0x2c);
        }
        else if ((piVar8[6] & 6U) == 6) {
          if (param_1[0x12] == -1) {
            iVar2 = piVar8[2];
            param_1[0x12] = iVar2;
            param_1[0x13] = piVar8[4] + iVar2;
            param_1[0x14] = piVar8[5] - piVar8[4];
          }
          else {
            piVar5 = (int *)malloc(0x10);
            if (piVar5 == (int *)0x0) {
              fatal(unaff_gp_lo + -0x6d48);
            }
            iVar2 = piVar8[2];
            *piVar5 = iVar2;
            piVar5[1] = piVar8[4] + iVar2;
            piVar5[2] = piVar8[5] - piVar8[4];
            piVar5[3] = param_1[0x43];
            param_1[0x43] = piVar5;
          }
          uVar7 = (uint)*(ushort *)(iVar1 + 0x2c);
        }
      }
      else if (*piVar8 == 2) {
        iVar10 = piVar8[2];
      }
      if ((int)uVar7 <= iVar9 + 1) break;
      if (piVar8[8] == 1) {
        if ((piVar8[0xe] & 6U) == 4) {
          uVar7 = piVar8[9];
          if ((uVar3 == 0xffffffff) || (uVar7 <= uVar3)) {
            iVar2 = piVar8[10];
            param_1[5] = iVar2 - uVar7;
            param_1[0xf] = iVar2 - uVar7;
            uVar3 = uVar7;
          }
          if (param_1[0x10] == -1) {
            param_1[0x10] = piVar8[10];
            param_1[0x11] = piVar8[0xc];
          }
          else {
            piVar5 = (int *)malloc(0xc);
            if (piVar5 == (int *)0x0) {
              fatal(unaff_gp_lo + -0x6d20);
            }
            *piVar5 = piVar8[10];
            piVar5[1] = piVar8[0xc];
            piVar5[2] = param_1[0x42];
            param_1[0x42] = piVar5;
          }
          uVar7 = (uint)*(ushort *)(iVar1 + 0x2c);
        }
        else if ((piVar8[0xe] & 6U) == 6) {
          if (param_1[0x12] == -1) {
            iVar2 = piVar8[10];
            param_1[0x12] = iVar2;
            param_1[0x13] = piVar8[0xc] + iVar2;
            param_1[0x14] = piVar8[0xd] - piVar8[0xc];
          }
          else {
            piVar5 = (int *)malloc(0x10);
            if (piVar5 == (int *)0x0) {
              fatal(unaff_gp_lo + -0x6d48);
            }
            iVar2 = piVar8[10];
            *piVar5 = iVar2;
            piVar5[1] = piVar8[0xc] + iVar2;
            piVar5[2] = piVar8[0xd] - piVar8[0xc];
            piVar5[3] = param_1[0x43];
            param_1[0x43] = piVar5;
          }
          uVar7 = (uint)*(ushort *)(iVar1 + 0x2c);
        }
      }
      else if (piVar8[8] == 2) {
        iVar10 = piVar8[10];
      }
      iVar9 = iVar9 + 2;
      piVar8 = piVar8 + 0x10;
    } while (iVar9 < (int)uVar7);
    iVar9 = param_1[0xf] - param_1[4];
    *(byte *)(param_1 + 0x30) = (byte)((uint)*(undefined4 *)(iVar1 + 0x24) >> 0x1c);
    if (iVar10 != 0) {
      FUN_0fb62e70(iVar10 - iVar9,param_1);
      if (param_1[0xf] - param_1[4] == iVar9) {
        piVar8 = (int *)param_1[0x42];
      }
      else {
        warning(unaff_gp_lo + -0x6cf8,param_1[0xb],iVar9);
        piVar8 = (int *)param_1[0x42];
      }
      goto LAB_0fb62b5c;
    }
  }
  fatal(unaff_gp_lo + -0x6c88,param_1[0xb]);
  piVar8 = (int *)param_1[0x42];
LAB_0fb62b5c:
  param_1[0x10] = param_1[0x10] - iVar9;
  for (; piVar8 != (int *)0x0; piVar8 = (int *)piVar8[2]) {
    *piVar8 = *piVar8 - iVar9;
  }
  piVar8 = (int *)param_1[0x43];
  param_1[0x13] = param_1[0x13] - iVar9;
  param_1[0x12] = param_1[0x12] - iVar9;
  for (; piVar8 != (int *)0x0; piVar8 = (int *)piVar8[3]) {
    piVar8[1] = piVar8[1] - iVar9;
    *piVar8 = *piVar8 - iVar9;
  }
  param_1[0x15] = param_1[0x15] - iVar9;
  pvVar6 = (void *)FUN_0fb640a0(param_1);
  if (param_1[0x1a] == 0) {
    if (*(char *)(unaff_gp_lo + -0x7ee9) != '\0') {
      trace(unaff_gp_lo + -0x6c18);
    }
    iVar1 = param_1[0x27];
    lVar4 = malloc();
    if (lVar4 != 0) {
      pvVar6 = memset((void *)lVar4,0,iVar1 << 3);
      param_1[0x1a] = (void *)lVar4;
      *(undefined1 *)((int)param_1 + 0xc1) = 1;
      return pvVar6;
    }
    pvVar6 = (void *)fatal(unaff_gp_lo + -0x6bf8,param_1[0xb]);
  }
  return pvVar6;
}



// === address_to_obj @ 0fb62e30 (60 bytes) ===

longlong address_to_obj(undefined8 param_1,undefined8 param_2)

{
  longlong lVar1;
  
  lVar1 = foreach_obj(param_1,&LAB_0fb62d30,param_2);
  if (lVar1 != -1) {
    return lVar1;
  }
  return 0;
}



// === FUN_0fb62e70 @ 0fb62e70 (3636 bytes) ===

/* WARNING: Instruction at (ram,0x0fb63220) overlaps instruction at (ram,0x0fb6321c)
    */

void * FUN_0fb62e70(int *param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  void *__s;
  void *pvVar3;
  int *piVar4;
  int iVar5;
  char *pcVar6;
  int iVar7;
  int iVar8;
  int *piVar9;
  int *piVar10;
  int iVar11;
  int iVar12;
  int iVar13;
  int aiStack_140 [42];
  int aiStack_28 [2];
  int iStack_20;
  int iStack_1c;
  
  if (debug_trace != '\0') {
    trace("get_the_dynamic_section_info: dynaddr 0x%lx, obj 0x%lx\n",param_1,param_2);
  }
  if (debug_map != '\0') {
    debug("get_the_dynamic_section_info: obj 0x%lx, name %s, dyn 0x%lx\n",param_2,
          *(undefined4 *)(param_2 + 0x2c),param_1);
  }
  piVar1 = aiStack_140;
  piVar4 = aiStack_140;
  *(undefined4 *)(param_2 + 0x120) = 1;
  *(undefined1 *)(param_2 + 0xc9) = 0;
  *(undefined4 *)(param_2 + 0x68) = 0;
  *(undefined4 *)(param_2 + 0xb0) = 0xffffffff;
  *(undefined4 *)(param_2 + 0xbc) = 0xffffffff;
  iStack_20 = 0;
  iVar8 = *param_1;
  if (iVar8 == 0) {
    iVar12 = 0x28;
    iVar7 = 0;
    iVar11 = 0;
  }
  else {
    iVar11 = 0;
    iVar12 = 0x28;
    iVar2 = 0;
    iVar13 = 0xa0;
    iVar7 = 0;
    do {
      iVar5 = iVar2;
      if (0x70000007 < iVar8) {
        if (iVar8 < 0x70000009) {
          *(int *)(param_2 + 0x78) = param_1[1];
          if (debug_map != '\0') {
            debug("DT_MIPS_CONFLICT: 0x%lx\n");
            iVar8 = param_1[2];
            goto LAB_0fb62f5c;
          }
        }
        else if (iVar8 < 0x70000027) {
          if (iVar8 < 0x70000012) {
            if (iVar8 < 0x7000000b) {
              if (iVar8 < 0x7000000a) {
                if (iVar8 != 0x70000009) goto LAB_0fb63540;
                *(int *)(param_2 + 0x74) = param_1[1];
                if (debug_map == '\0') goto LAB_0fb62f58;
                debug("DT_MIPS_LIBLIST: 0x%lx\n");
                iVar8 = param_1[2];
              }
              else {
                if (0x7000000a < iVar8) goto LAB_0fb63540;
                iStack_1c = param_1[1];
                if (debug_map == '\0') goto LAB_0fb62f58;
                debug("DT_MIPS_LOCAL_GOTNO: # %ld\n");
                iVar8 = param_1[2];
              }
            }
            else if (iVar8 < 0x7000000c) {
              *(int *)(param_2 + 0xbc) = param_1[1];
              if (debug_map == '\0') goto LAB_0fb62f58;
              debug("DT_MIPS_CONFLICTNO: # %ld\n");
              iVar8 = param_1[2];
            }
            else if (iVar8 < 0x70000011) {
              if (iVar8 != 0x70000010) goto LAB_0fb63540;
              *(int *)(param_2 + 0xb0) = param_1[1];
              if (debug_map == '\0') goto LAB_0fb62f58;
              debug("DT_MIPS_LIBLISTNO: # %ld\n");
              iVar8 = param_1[2];
            }
            else {
              if (0x70000011 < iVar8) goto LAB_0fb63540;
              *(int *)(param_2 + 0x9c) = param_1[1];
              if (debug_map == '\0') goto LAB_0fb62f58;
              debug("DT_MIPS_SYMTABNO: # %ld\n");
              iVar8 = param_1[2];
            }
            goto LAB_0fb62f5c;
          }
          if (iVar8 < 0x70000013) {
            *(int *)(param_2 + 0xf0) = param_1[1];
            if (debug_map != '\0') {
              debug("DT_MIPS_UNREFEXTNO: index of first unreferenced external symbol # %ld\n");
              iVar8 = param_1[2];
              goto LAB_0fb62f5c;
            }
          }
          else {
            if (iVar8 < 0x70000023) {
              if (iVar8 < 0x70000016) {
                if (iVar8 != 0x70000013) goto LAB_0fb63540;
                *(int *)(param_2 + 0xf4) = param_1[1];
                if (debug_map == '\0') goto LAB_0fb62f58;
                debug("DT_MIPS_GOTSYM: first symbol that has a got # %ld\n");
                iVar8 = param_1[2];
              }
              else {
                if (0x70000016 < iVar8) goto LAB_0fb63540;
                *(int *)(param_2 + 0xf8) = param_1[1];
                if (debug_map == '\0') goto LAB_0fb62f58;
                debug("DT_MIPS_RLD_MAP: 0x%lx\n");
                iVar8 = param_1[2];
              }
              goto LAB_0fb62f5c;
            }
            if (iVar8 < 0x70000024) {
              if (param_1[1] != 0) {
                *(int *)(param_2 + 0xfc) = param_1[1];
              }
              if (debug_map != '\0') {
                debug("DT_MIPS_PIXIE_INIT: 0x%lx\n",*(undefined4 *)(param_2 + 0xfc));
                iVar8 = param_1[2];
                goto LAB_0fb62f5c;
              }
            }
            else {
              if (iVar8 != 0x70000024) goto LAB_0fb63540;
              *(int *)(param_2 + 0x110) = param_1[1];
            }
          }
        }
        else if (iVar8 < 0x70000028) {
          iStack_20 = param_1[1];
          if (debug_map != '\0') {
            debug("DT_MIPS_HIDDEN_GOTIDX: = %ld\n");
            iVar8 = param_1[2];
            goto LAB_0fb62f5c;
          }
        }
        else if (iVar8 < 0x7000002e) {
          if (iVar8 < 0x7000002b) {
            if (iVar8 < 0x7000002a) {
              if (iVar8 != 0x70000029) goto LAB_0fb63540;
              *(int *)(param_2 + 0x7c) = param_1[1];
              if (debug_map != '\0') {
                debug("DT_MIPS_OPTIONS: 0x%lx\n");
                iVar8 = param_1[2];
                goto LAB_0fb62f5c;
              }
            }
            else {
              if (0x7000002a < iVar8) goto LAB_0fb63540;
              *(int *)(param_2 + 0x118) = param_1[1];
            }
          }
          else if (0x7000002b < iVar8) {
            if (iVar8 < 0x7000002d) {
              if (iVar8 == 0x7000002c) {
                *(int *)(param_2 + 0x11c) = param_1[1];
                goto LAB_0fb62f58;
              }
            }
            else if (iVar8 < 0x7000002e) {
              iVar8 = param_1[2];
              goto LAB_0fb62f5c;
            }
            goto LAB_0fb63540;
          }
        }
        else {
          if (0x7000002e < iVar8) {
            if (iVar8 < 0x70000032) {
              if (iVar8 < 0x70000031) {
                if (iVar8 != 0x70000030) goto LAB_0fb63540;
                iVar8 = param_1[2];
              }
              else {
                if (0x70000031 < iVar8) goto LAB_0fb63540;
                FUN_0fb64a20(param_2,param_1[1]);
                iVar8 = param_1[2];
              }
            }
            else if (iVar8 < 0x70000033) {
              *(byte *)(param_2 + 0xc9) = *(byte *)(param_2 + 0xc9) | 2;
              if (debug_map == '\0') goto LAB_0fb62f58;
              debug("DT_MIPS_DIRECT: is present\n");
              iVar8 = param_1[2];
            }
            else {
              if (iVar8 != 0x70000033) goto LAB_0fb63540;
              __rld_recent_update_callback = 1;
              iVar8 = param_1[1];
              *(int *)(param_2 + 0x128) = iVar8;
              if (debug_map == '\0') goto LAB_0fb62f58;
              debug("DT_MIPS_RLD_OBJ_UPDATE: dynsym index %ld (0x%lx)\n",iVar8);
              iVar8 = param_1[2];
            }
            goto LAB_0fb62f5c;
          }
          if (perf_suffix == 0) {
            perf_suffix = param_1[1];
            iVar7 = iVar7 + 1;
          }
        }
        goto LAB_0fb62f58;
      }
      if (0xf < iVar8) {
        if (iVar8 < 0x11) {
          *(byte *)(param_2 + 0xc9) = *(byte *)(param_2 + 0xc9) | 1;
          if (debug_map == '\0') goto LAB_0fb62f58;
          debug("DT_SYMBOLIC: is present\n");
          iVar8 = param_1[2];
        }
        else if (iVar8 < 0x70000003) {
          if (0x12 < iVar8) {
            if (iVar8 < 0x14) {
              *(int *)(param_2 + 0xac) = param_1[1];
              if (debug_map != '\0') {
                debug("DT_RELENT: 0x%lx\n");
                iVar8 = param_1[2];
                goto LAB_0fb62f5c;
              }
            }
            else if (iVar8 < 0x70000002) {
              if (iVar8 != 0x70000001) goto LAB_0fb63540;
              *(int *)(param_2 + 0x8c) = param_1[1];
            }
            else {
              if (0x70000002 < iVar8) goto LAB_0fb63540;
              *(int *)(param_2 + 0x90) = param_1[1];
              if (debug_map != '\0') {
                debug("DT_MIPS_TIME_STAMP: 0x%lx\n");
                iVar8 = param_1[2];
                goto LAB_0fb62f5c;
              }
            }
            goto LAB_0fb62f58;
          }
          if (iVar8 < 0x12) {
            if (iVar8 != 0x11) goto LAB_0fb63540;
            *(int *)(param_2 + 0x70) = param_1[1];
            if (debug_map == '\0') goto LAB_0fb62f58;
            debug("DT_REL: 0x%lx\n");
            iVar8 = param_1[2];
          }
          else {
            *(int *)(param_2 + 0xa8) = param_1[1];
            if (debug_map == '\0') goto LAB_0fb62f58;
            debug("DT_RELSZ: # %ld\n");
            iVar8 = param_1[2];
          }
        }
        else if (iVar8 < 0x70000004) {
          *(int *)(param_2 + 0x94) = param_1[1];
          if (debug_map == '\0') goto LAB_0fb62f58;
          debug("DT_MIPS_ICHECKSUM: 0x%lx\n");
          iVar8 = param_1[2];
        }
        else if (iVar8 < 0x70000006) {
          if (iVar8 < 0x70000005) {
            if (iVar8 != 0x70000004) goto LAB_0fb63540;
            *(int *)(param_2 + 0x98) = param_1[1];
            if (debug_map == '\0') goto LAB_0fb62f58;
            debug("DT_MIPS_IVERSION: 0x%lx\n");
            iVar8 = param_1[2];
          }
          else {
            if (0x70000005 < iVar8) goto LAB_0fb63540;
            *(int *)(param_2 + 0x84) = param_1[1];
            if (debug_map == '\0') goto LAB_0fb62f58;
            debug("DT_MIPS_FLAGS: 0x%lx\n");
            iVar8 = param_1[2];
          }
        }
        else if (iVar8 < 0x70000007) {
          iVar8 = param_1[1];
          *(int *)(param_2 + 0x14) = iVar8;
          *(int *)(param_2 + 0x3c) = iVar8;
          if (debug_map == '\0') goto LAB_0fb62f58;
          debug("DT_MIPS_BASE_ADDRESS 0x%lx\n");
          iVar8 = param_1[2];
        }
        else {
          if (iVar8 != 0x70000007) goto LAB_0fb63540;
          *(int *)(param_2 + 0x68) = param_1[1];
          if (debug_map == '\0') goto LAB_0fb62f58;
          debug("DT_MIPS_MSYM: 0x%lx\n");
          iVar8 = param_1[2];
        }
        goto LAB_0fb62f5c;
      }
      if (iVar8 < 10) {
        if (iVar8 < 4) {
          if (iVar8 < 3) {
            if (iVar8 != 1) goto LAB_0fb63540;
            if (iVar12 <= iVar11) {
              if (debug_malloc != '\0') {
                debug("DT_NEEDED");
              }
              iVar12 = iVar12 + 0x28;
              iVar13 = iVar13 + 0xa0;
              if (iVar12 == 0x50) {
                piVar4 = (int *)malloc(iVar13);
                if (piVar4 == (int *)0x0) {
                  fatal("Unable to malloc %ld bytes for DT_NEEDED list\n",iVar13);
                }
                memmove(piVar4,piVar1,0xa0);
                piVar1 = piVar4;
              }
              else {
                piVar1 = (int *)realloc(piVar1,iVar13);
                if (piVar1 == (int *)0x0) {
                  fatal("Unable to realloc %ld bytes for DT_NEEDED list\n",iVar13);
                }
              }
            }
            iVar5 = iVar2 + 4;
            iVar11 = iVar11 + 1;
            *(int *)((int)piVar1 + iVar2) = param_1[1];
          }
          else {
            aiStack_28[0] = param_1[1];
            if (debug_map != '\0') {
              debug("DT_PLTGOT 0:  0x%lx\n");
              iVar8 = param_1[2];
              goto LAB_0fb62f5c;
            }
          }
        }
        else {
          if (4 < iVar8) {
            if (iVar8 < 6) {
              if (iVar8 != 5) goto LAB_0fb63540;
              *(int *)(param_2 + 0x60) = param_1[1];
              if (debug_map == '\0') goto LAB_0fb62f58;
              debug("DT_STRTAB: 0x%lx\n");
              iVar8 = param_1[2];
            }
            else if (iVar8 < 7) {
              *(int *)(param_2 + 100) = param_1[1];
              if (debug_map == '\0') goto LAB_0fb62f58;
              debug("DT_SYMTAB: 0x%lx\n");
              iVar8 = param_1[2];
            }
            else {
LAB_0fb63540:
              if (debug_map == '\0') goto LAB_0fb62f58;
              debug("DT: 0x%lx ignored here\n");
              iVar8 = param_1[2];
            }
            goto LAB_0fb62f5c;
          }
          *(int *)(param_2 + 0x5c) = param_1[1];
          if (debug_map != '\0') {
            debug("DT_HASH: 0x%lx\n");
            iVar8 = param_1[2];
            goto LAB_0fb62f5c;
          }
        }
LAB_0fb62f58:
        iVar8 = param_1[2];
        iVar2 = iVar5;
      }
      else {
        if (iVar8 < 0xb) {
          *(int *)(param_2 + 0xa4) = param_1[1];
          if (debug_map != '\0') {
            debug("DT_STRSZ: # %ld\n");
            iVar8 = param_1[2];
            goto LAB_0fb62f5c;
          }
          goto LAB_0fb62f58;
        }
        if (0xc < iVar8) {
          if (iVar8 < 0xe) {
            if (param_1[1] != 0) {
              *(int *)(param_2 + 0xec) = param_1[1];
            }
            if (debug_map != '\0') {
              debug("DT_FINI: 0x%lx\n",*(undefined4 *)(param_2 + 0xec));
              iVar8 = param_1[2];
              goto LAB_0fb62f5c;
            }
          }
          else if (iVar8 < 0xf) {
            if (iVar8 != 0xe) goto LAB_0fb63540;
            *(int *)(param_2 + 0x30) = param_1[1];
          }
          else {
            *(int *)(param_2 + 0x88) = param_1[1];
          }
          goto LAB_0fb62f58;
        }
        if (iVar8 < 0xc) {
          if (iVar8 != 0xb) goto LAB_0fb63540;
          *(int *)(param_2 + 0xa0) = param_1[1];
          if (debug_map == '\0') goto LAB_0fb62f58;
          debug("DT_SYMENT: 0x%lx\n");
          iVar8 = param_1[2];
        }
        else {
          if (param_1[1] != 0) {
            *(int *)(param_2 + 0xe8) = param_1[1];
          }
          if (debug_map == '\0') goto LAB_0fb62f58;
          debug("DT_INIT: 0x%lx\n",*(undefined4 *)(param_2 + 0xe8));
          iVar8 = param_1[2];
        }
      }
LAB_0fb62f5c:
      param_1 = param_1 + 2;
      piVar4 = piVar1;
    } while (iVar8 != 0);
  }
  iVar8 = *(int *)(param_2 + 0x3c);
  if (iVar8 == 0) {
    fatal("DT_MIPS_BASE_ADDRESS missing or zero in %s. rld cannot continue\n",
          *(undefined4 *)(param_2 + 0x2c));
    iVar8 = *(int *)(param_2 + 0x3c);
  }
  iVar8 = iVar8 - *(int *)(param_2 + 0x10);
  if (iVar8 == 0) {
    if ((*(int **)(param_2 + 0x5c) != (int *)0x0) && (**(int **)(param_2 + 0x5c) == 0)) {
      fatal("Hash table not correctly formatted. number of buckets %lx %ld.\n",0);
    }
  }
  else {
    if (*(int *)(param_2 + 0xe8) != 0) {
      *(int *)(param_2 + 0xe8) = *(int *)(param_2 + 0xe8) - iVar8;
    }
    if (*(int *)(param_2 + 0xfc) != 0) {
      *(int *)(param_2 + 0xfc) = *(int *)(param_2 + 0xfc) - iVar8;
    }
    if (*(int *)(param_2 + 0xec) != 0) {
      *(int *)(param_2 + 0xec) = *(int *)(param_2 + 0xec) - iVar8;
    }
    piVar1 = (int *)(*(int *)(param_2 + 0x5c) - iVar8);
    if (((*(int *)(param_2 + 0x5c) == 0) ||
        (*(int **)(param_2 + 0x5c) = piVar1, piVar1 == (int *)0x0)) || (*piVar1 != 0)) {
      iVar2 = *(int *)(param_2 + 0x60);
    }
    else {
      fatal("Hash table not correctly formatted. number of buckets %lx %ld.\n",0);
      iVar2 = *(int *)(param_2 + 0x60);
    }
    if (iVar2 != 0) {
      *(int *)(param_2 + 0x60) = iVar2 - iVar8;
    }
    if (*(int *)(param_2 + 100) != 0) {
      *(int *)(param_2 + 100) = *(int *)(param_2 + 100) - iVar8;
    }
    if (*(int *)(param_2 + 0x70) != 0) {
      *(int *)(param_2 + 0x70) = *(int *)(param_2 + 0x70) - iVar8;
    }
    if (*(int *)(param_2 + 0x68) != 0) {
      *(int *)(param_2 + 0x68) = *(int *)(param_2 + 0x68) - iVar8;
    }
    if (aiStack_28[0] != 0) {
      aiStack_28[0] = aiStack_28[0] - iVar8;
    }
    if (*(int *)(param_2 + 0x74) != 0) {
      *(int *)(param_2 + 0x74) = *(int *)(param_2 + 0x74) - iVar8;
    }
    if (*(int *)(param_2 + 0x78) != 0) {
      *(int *)(param_2 + 0x78) = *(int *)(param_2 + 0x78) - iVar8;
    }
    if (*(int *)(param_2 + 0x7c) != 0) {
      *(int *)(param_2 + 0x7c) = *(int *)(param_2 + 0x7c) - iVar8;
    }
    if (*(int *)(param_2 + 0xf8) != 0) {
      *(int *)(param_2 + 0xf8) = *(int *)(param_2 + 0xf8) - iVar8;
    }
    if (*(int *)(param_2 + 0x110) != 0) {
      *(int *)(param_2 + 0x110) = *(int *)(param_2 + 0x110) - iVar8;
    }
    if (*(int *)(param_2 + 0x118) != 0) {
      *(int *)(param_2 + 0x118) = *(int *)(param_2 + 0x118) - iVar8;
    }
    FUN_0fb649b0(param_2,iVar8);
  }
  if (iStack_20 == 0) {
    iStack_20 = iStack_1c;
  }
  FUN_0fb64af0(param_2,aiStack_28,iVar8);
  *(int *)(param_2 + 0x88) = *(int *)(param_2 + 0x60) + *(int *)(param_2 + 0x88);
  if (debug_map != '\0') {
    debug(" rpath %s\n");
  }
  if (((perf_suffix != 0) && (iVar7 != 0)) &&
     (perf_suffix = *(int *)(param_2 + 0x60) + perf_suffix, debug_map != '\0')) {
    debug("perf_suffix %s\n");
  }
  pcVar6 = *(char **)(param_2 + 0x30);
  if (pcVar6 != (char *)0x0) {
    pcVar6 = pcVar6 + *(int *)(param_2 + 0x60);
    *(char **)(param_2 + 0x30) = pcVar6;
  }
  if (debug_map != '\0') {
    if (pcVar6 == (char *)0x0) {
      pcVar6 = "no DT_SONAME!";
    }
    debug("get_the_dynamic_section_info: soname %s\n",pcVar6);
  }
  if ((*(int *)(param_2 + 0x74) == 0) && (iVar11 != 0)) {
    if (debug_map != '\0') {
      debug("get_the_dynamic_section_info: object\'s liblist is null\n");
    }
    if (debug_malloc != '\0') {
      debug("in get_the_dynamic_section_info, for pliblist\n");
    }
    piVar1 = (int *)malloc(iVar11 * 0x14);
    memset(piVar1,0,iVar11 * 0x14);
    if (0 < iVar11) {
      iVar8 = 0;
      piVar10 = piVar4;
      piVar9 = piVar1;
      do {
        iVar7 = *piVar10;
        *piVar9 = iVar7;
        if (debug_map != '\0') {
          debug("get_the_dynamic_section_info: %ld library %s is added to liblist\n",iVar8,
                *(int *)(param_2 + 0x60) + iVar7);
        }
        iVar8 = iVar8 + 1;
        piVar10 = piVar10 + 1;
        piVar9 = piVar9 + 5;
      } while (iVar8 != iVar11);
    }
    *(int *)(param_2 + 0xb0) = iVar11;
    *(int **)(param_2 + 0x74) = piVar1;
  }
  if (iVar12 != 0x28) {
    free(piVar4);
  }
  pvVar3 = *(void **)(param_2 + 0xb0);
  if (0 < (int)pvVar3) {
    if (debug_malloc != '\0') {
      debug("in get_the_dynamic_section_info, malloc o_ollmap\n");
    }
    __s = (void *)_rld_alloc_obj_space(param_2,(int)pvVar3 << 2,4,param_2 + 0xe2);
    *(void **)(param_2 + 0xb4) = __s;
    pvVar3 = memset(__s,0,(int)pvVar3 << 2);
  }
  return pvVar3;
}



// === FUN_0fb63cb0 @ 0fb63cb0 (584 bytes) ===

int FUN_0fb63cb0(int param_1)

{
  size_t __nbytes;
  ulonglong uVar1;
  size_t sVar2;
  int iVar3;
  int __fd;
  uint uVar4;
  uint *puVar5;
  uint *puVar6;
  undefined1 *puVar7;
  int iVar8;
  
  iVar3 = *(int *)(param_1 + 0x10);
  __nbytes = (uint)*(ushort *)(iVar3 + 0x2e) * (uint)*(ushort *)(iVar3 + 0x30);
  iVar8 = -(__nbytes + 0xf & 0xfffffff0);
  puVar7 = &stack0xffffff90 + iVar8;
  __fd = *(int *)(param_1 + 0x34);
  if ((__fd == 0) && (__fd = open64(*(char **)(param_1 + 0x18),0), __fd == -1)) {
    iVar3 = fatal("Cannot open %s to fix up prefetches\n",*(undefined4 *)(param_1 + 0x18));
  }
  else {
    uVar1 = lseek64(__fd,(ulonglong)*(uint *)(iVar3 + 0x20),0);
    if (uVar1 == *(uint *)(iVar3 + 0x20)) {
      sVar2 = read(__fd,&stack0xffffff90 + iVar8,__nbytes);
      if (sVar2 == __nbytes) {
        uVar4 = (uint)*(ushort *)(iVar3 + 0x30);
        iVar8 = 0;
        if (uVar4 != 0) {
          do {
            if ((*(uint *)(puVar7 + 8) & 6) == 6) {
              puVar6 = (uint *)(*(int *)(puVar7 + 0xc) -
                               (*(int *)(param_1 + 0x3c) - *(int *)(param_1 + 0x10)));
              puVar5 = (uint *)(*(int *)(puVar7 + 0x14) + (int)puVar6);
              if (puVar6 < puVar5) {
                do {
                  while( true ) {
                    uVar4 = *puVar6 >> 0x1a;
                    if ((uVar4 == 0x33) || ((uVar4 == 0x13 && ((*puVar6 & 0x3f) == 0xf)))) break;
LAB_0fb63da0:
                    puVar6 = puVar6 + 1;
                    if (puVar5 <= puVar6) goto LAB_0fb63e1c;
                  }
                  *puVar6 = 0x21;
                  if (user_tracking == '\0') {
                    puVar5 = (uint *)(*(int *)(puVar7 + 0x14) +
                                     (*(int *)(puVar7 + 0xc) -
                                     (*(int *)(param_1 + 0x3c) - *(int *)(param_1 + 0x10))));
                    goto LAB_0fb63da0;
                  }
                  track("changing prefetch instruction at 0x%lx to NADA\n",puVar6);
                  puVar6 = puVar6 + 1;
                  puVar5 = (uint *)(*(int *)(puVar7 + 0x14) +
                                   (*(int *)(puVar7 + 0xc) -
                                   (*(int *)(param_1 + 0x3c) - *(int *)(param_1 + 0x10))));
                } while (puVar6 < puVar5);
LAB_0fb63e1c:
                uVar4 = (uint)*(ushort *)(iVar3 + 0x30);
              }
            }
            iVar8 = iVar8 + 1;
            puVar7 = puVar7 + 0x28;
          } while (iVar8 < (int)uVar4);
        }
        iVar3 = close(__fd);
      }
      else {
        iVar3 = fatal("Cannot read %s\'s shdr to fix up prefetches\n",
                      *(undefined4 *)(param_1 + 0x18));
      }
    }
    else {
      iVar3 = fatal("Cannot lseek %s to fix up prefetches\n",*(undefined4 *)(param_1 + 0x18));
    }
  }
  return iVar3;
}



// === FUN_0fb63f10 @ 0fb63f10 (400 bytes) ===

/* WARNING: Instruction at (ram,0x0fb64034) overlaps instruction at (ram,0x0fb64030)
    */

void FUN_0fb63f10(int param_1,undefined8 param_2)

{
  int iVar1;
  void *pvVar2;
  size_t __len;
  uint __len_00;
  uint *puVar3;
  void *pvVar4;
  uint uVar5;
  code *UNRECOVERED_JUMPTABLE;
  
  __len = *(size_t *)(param_1 + 0x44);
  uVar5 = pagesize - 1;
  if (0 < (int)__len) {
    pvVar4 = *(void **)(param_1 + 0x40);
    pvVar2 = (void *)(~uVar5 & (uint)pvVar4);
    if (pvVar2 != pvVar4) {
      __len = (int)pvVar4 + (__len - (int)pvVar2);
      pvVar4 = pvVar2;
    }
    if (verbose != '\0') {
      trace("change_text_permissions: %s to 0x%lx (start 0x%lx size 0x%lx)\n",
            *(undefined4 *)(param_1 + 0x18),param_2,pvVar4,__len);
    }
    iVar1 = mprotect(pvVar4,__len,(int)param_2);
    if (iVar1 == -1) {
      fatal("Cannot change %s\'s text protection to 0x%lx.. (address 0x%lx size 0x%lx) errno %d\n",
            *(undefined4 *)(param_1 + 0x18),param_2,*(undefined4 *)(param_1 + 0x40),
            *(undefined4 *)(param_1 + 0x44),errno);
      puVar3 = *(uint **)(param_1 + 0x108);
      goto LAB_0fb63fa8;
    }
  }
  puVar3 = *(uint **)(param_1 + 0x108);
LAB_0fb63fa8:
  if ((puVar3 != (uint *)0x0) && (puVar3 != (uint *)0x0)) {
    __len_00 = puVar3[1];
    while( true ) {
      if (__len_00 == 0) {
        puVar3 = (uint *)puVar3[2];
      }
      else {
        pvVar4 = (void *)*puVar3;
        pvVar2 = (void *)(~uVar5 & (uint)pvVar4);
        if (pvVar2 != pvVar4) {
          __len_00 = (int)pvVar4 + (__len_00 - (int)pvVar2);
          pvVar4 = pvVar2;
        }
        if (verbose != '\0') {
          trace("change_text_permissions: %s to 0x%lx (Start 0x%lx size 0x%lx)\n",
                *(undefined4 *)(param_1 + 0x18),param_2,pvVar4,__len_00);
        }
        iVar1 = mprotect(pvVar4,__len_00,(int)param_2);
        if (iVar1 == -1) {
          fatal("Cannot change %s\'s text protection to 0x%lx... (address 0x%lx size 0x%lx) errno %d\n"
                ,*(undefined4 *)(param_1 + 0x18),param_2,*puVar3,puVar3[1],errno);
          puVar3 = (uint *)puVar3[2];
        }
        else {
          puVar3 = (uint *)puVar3[2];
        }
      }
      if (puVar3 == (uint *)0x0) break;
      __len_00 = puVar3[1];
    }
  }
                    /* WARNING: Could not recover jumptable at 0x0fb64058. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
}



// === FUN_0fb640a0 @ 0fb640a0 (336 bytes) ===

void FUN_0fb640a0(int param_1)

{
  char cVar1;
  byte bVar2;
  int iVar3;
  int iVar4;
  char *pcVar5;
  int *piVar6;
  code *UNRECOVERED_JUMPTABLE;
  
  pcVar5 = *(char **)(param_1 + 0x7c);
  *(undefined4 *)(param_1 + 0x80) = 0x1f00;
  if (pcVar5 != (char *)0x0) {
    piVar6 = *(int **)(param_1 + 0x20);
    iVar3 = *piVar6;
    iVar4 = 0;
    while( true ) {
      if (iVar3 == 1) {
        warning("Object %s has a DT_MIPS_OPTIONS tag, but no corresponding\nprogram header.  This could be the result of file corruption\nor faulty compilation/linking."
                ,*(undefined4 *)(param_1 + 0x18));
        return;
      }
      if (iVar3 == 0x70000002) break;
      iVar3 = piVar6[8];
      piVar6 = piVar6 + 8;
    }
    iVar3 = piVar6[5];
    if (0 < iVar3) {
      cVar1 = *pcVar5;
      while( true ) {
        if (cVar1 == '\x02') {
          *(undefined4 *)(param_1 + 0x80) = *(undefined4 *)(pcVar5 + 4);
        }
        if (((R8000_arch == '\0') || (*pcVar5 != '\x04')) || ((*(uint *)(pcVar5 + 4) & 2) != 2)) {
          bVar2 = pcVar5[1];
        }
        else {
          FUN_0fb63f10(param_1,3);
          FUN_0fb63cb0(param_1);
          FUN_0fb63f10(param_1,5);
          bVar2 = pcVar5[1];
        }
        iVar4 = iVar4 + (uint)bVar2;
        if (iVar3 <= iVar4) break;
        pcVar5 = pcVar5 + bVar2;
        cVar1 = *pcVar5;
      }
    }
  }
                    /* WARNING: Could not recover jumptable at 0x0fb641c8. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
}



// === elfhash @ 0fb64200 (132 bytes) ===

uint elfhash(byte *param_1)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  byte *pbVar4;
  
  uVar1 = (uint)*param_1;
  if (uVar1 == 0) {
    return 0;
  }
  uVar2 = 0;
  pbVar4 = param_1 + 1;
  do {
    uVar1 = uVar1 + uVar2 * 0x10;
    uVar2 = uVar1 & 0xf0000000;
    if (uVar2 != 0) {
      uVar1 = ~uVar2 & (uVar1 ^ uVar2 >> 0x18);
    }
    if (*pbVar4 == 0) {
      return uVar1;
    }
    uVar2 = (uint)*pbVar4 + uVar1 * 0x10;
    uVar3 = uVar2 & 0xf0000000;
    uVar1 = (uint)pbVar4[1];
    if (uVar3 != 0) {
      uVar2 = ~uVar3 & (uVar2 ^ uVar3 >> 0x18);
    }
    pbVar4 = pbVar4 + 2;
  } while (uVar1 != 0);
  return uVar2;
}



// === rld_obj_newstr @ 0fb64290 (120 bytes) ===

longlong rld_obj_newstr(char *param_1)

{
  size_t sVar2;
  longlong lVar1;
  
  sVar2 = strlen(param_1);
  if (debug_malloc != '\0') {
    debug("in rld_obj_newstr, for new, ");
  }
  lVar1 = malloc(sVar2 + 1);
  if (lVar1 == 0) {
    fatal("obj_newstr: unable to rld_malloc for new %ld bytes \n",sVar2 + 1);
  }
  strcpy((char *)lVar1,param_1);
  return lVar1;
}



// === get_dynsym_hash_value @ 0fb64310 (128 bytes) ===

undefined8 get_dynsym_hash_value(int param_1,longlong param_2)

{
  undefined8 uVar1;
  
  if (param_2 != 0) {
    uVar1 = elfhash(*(int *)(*(int *)(param_1 + 100) + (int)param_2 * 0x10) +
                    *(int *)(param_1 + 0x60));
    if (*(char *)(param_1 + 0xc1) != '\0') {
      *(int *)(*(int *)(param_1 + 0x68) + (int)param_2 * 8) = (int)uVar1;
    }
    return uVar1;
  }
  return 0;
}



// === elf_get_pt_interp @ 0fb64390 (84 bytes) ===

int elf_get_pt_interp(int param_1)

{
  ushort uVar1;
  int *piVar2;
  int *piVar3;
  int iVar4;
  
  uVar1 = *(ushort *)(*(int *)(param_1 + 0x10) + 0x2c);
  piVar3 = *(int **)(param_1 + 0x20);
  if (uVar1 != 0) {
    piVar2 = piVar3 + (uint)uVar1 * 8;
    iVar4 = *piVar3;
    while( true ) {
      if (iVar4 == 3) {
        return piVar3[2];
      }
      piVar3 = piVar3 + 8;
      if (piVar3 == piVar2) break;
      iVar4 = *piVar3;
    }
  }
  return 0;
}



// === dumpobj @ 0fb643f0 (1244 bytes) ===

void dumpobj(int param_1)

{
  if (execrld != '\0') {
    debug("Obj_Info Struct\n\n");
    if (execrld != '\0') {
      debug("  oi_size = 0x%lx\n",*(undefined4 *)(param_1 + 4));
      if (execrld != '\0') {
        debug("  oi_next = 0x%lx\n",*(undefined4 *)(param_1 + 8));
        if (execrld != '\0') {
          debug("  oi_prev = 0x%lx\n",*(undefined4 *)(param_1 + 0xc));
          if (execrld != '\0') {
            debug("  oi_ehdr = 0x%lx\n",*(undefined4 *)(param_1 + 0x10));
            if (execrld != '\0') {
              debug("  oi_orig_ehdr = 0x%lx\n",*(undefined4 *)(param_1 + 0x14));
              if (execrld != '\0') {
                debug("  oi_pathame = %s\n",*(undefined4 *)(param_1 + 0x18));
                if (execrld != '\0') {
                  debug("  oi_pathname_len = %ld\n\n",*(undefined4 *)(param_1 + 0x1c));
                  if (execrld != '\0') {
                    debug("Rld private information\n\n");
                    if (execrld != '\0') {
                      debug("  o_arch = %ld\n",*(undefined1 *)(param_1 + 0xc0));
                      if (execrld != '\0') {
                        debug("  o_pproghdr = 0x%lx\n",*(undefined4 *)(param_1 + 0x20));
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  if ((*(int *)(param_1 + 0x2c) != 0) && (execrld != '\0')) {
    debug("  o_name = %s\n");
  }
  if ((*(int *)(param_1 + 0x30) != 0) && (execrld != '\0')) {
    debug("  o_soname = %s\n");
  }
  if (execrld != '\0') {
    debug("  o_fd = %ld\n",*(undefined4 *)(param_1 + 0x34));
    if (execrld != '\0') {
      debug("  o_base_address = 0x%lx\n",*(undefined4 *)(param_1 + 0x3c));
      if (execrld != '\0') {
        debug("  o_text_start   = 0x%lx\n",*(undefined4 *)(param_1 + 0x40));
        if (execrld != '\0') {
          debug("  o_text_size    = %ld\n",*(undefined4 *)(param_1 + 0x44));
          if (execrld != '\0') {
            debug("  o_data_start   = 0x%lx\n",*(undefined4 *)(param_1 + 0x48));
            if (execrld != '\0') {
              debug("  o_bss_start    = 0x%lx\n",*(undefined4 *)(param_1 + 0x4c));
              if (execrld != '\0') {
                debug("  o_bss_size = %ld\n",*(undefined4 *)(param_1 + 0x50));
                if (execrld != '\0') {
                  debug("  o_entry    = 0x%lx\n",*(undefined4 *)(param_1 + 0x54));
                  if (execrld != '\0') {
                    debug("  o_base     = 0x%lx\n",*(undefined4 *)(param_1 + 0x58));
                    if (execrld != '\0') {
                      debug("  o_hash_table_ptr     = 0x%lx\n",*(undefined4 *)(param_1 + 0x5c));
                      if (execrld != '\0') {
                        debug("  o_dynamic_string_table      = 0x%lx\n",
                              *(undefined4 *)(param_1 + 0x60));
                        if (execrld != '\0') {
                          debug("  o_dynamic_symbol_table     = 0x%lx\n",
                                *(undefined4 *)(param_1 + 100));
                          if (execrld != '\0') {
                            debug("  o_msym_table_ptr     = 0x%lx\n",*(undefined4 *)(param_1 + 0x68)
                                 );
                            if (execrld != '\0') {
                              debug("  o_gotinfo  = 0x%lx\n",*(undefined4 *)(param_1 + 0x6c));
                              if (execrld != '\0') {
                                debug("  o_relocation_table_ptr      = 0x%lx\n",
                                      *(undefined4 *)(param_1 + 0x70));
                                if (execrld != '\0') {
                                  debug("  o_liblist_table_ptr     = 0x%lx\n",
                                        *(undefined4 *)(param_1 + 0x74));
                                  if (execrld != '\0') {
                                    debug("  o_conflict_table_ptr    = 0x%lx\n",
                                          *(undefined4 *)(param_1 + 0x78));
                                    if (execrld != '\0') {
                                      debug("  o_dynamic_flags = 0x%lx\n",
                                            *(undefined4 *)(param_1 + 0x84));
                                    }
                                  }
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  if (*(int *)(param_1 + 0x88) == 0) {
    if (execrld != '\0') {
      debug("  o_directory_path_ptr = NULL\n");
    }
  }
  else if (execrld != '\0') {
    debug("  o_directory_path_ptr = 0x%lx\n");
  }
  if ((((execrld != '\0') &&
       (debug("  o_old_rld_version_number   = %ld\n",*(undefined4 *)(param_1 + 0x8c)),
       execrld != '\0')) &&
      (debug("  o_tstamp   = %ld\n",*(undefined4 *)(param_1 + 0x90)), execrld != '\0')) &&
     ((debug("  o_o_ichecksum  = 0x%lx\n",*(undefined4 *)(param_1 + 0x94)), execrld != '\0' &&
      (debug("  o_interface_version_index = %ld\n",*(undefined4 *)(param_1 + 0x98)), execrld != '\0'
      )))) {
    debug("  o_dynsym_count = %ld\n",*(undefined4 *)(param_1 + 0x9c));
    if (execrld != '\0') {
      debug("  o_syent    = %ld\n",*(undefined4 *)(param_1 + 0xa0));
      if (execrld != '\0') {
        debug("  o_stsize   = %ld\n",*(undefined4 *)(param_1 + 0xa4));
        if (execrld != '\0') {
          debug("  o_rlsize   = %ld\n",*(undefined4 *)(param_1 + 0xa8));
          if (execrld != '\0') {
            debug("  o_reloc_table_entry_size    = %ld\n",*(undefined4 *)(param_1 + 0xac));
            if (execrld != '\0') {
              debug("  o_count_liblist_table_entries  = %ld\n",*(undefined4 *)(param_1 + 0xb0));
              if ((((execrld != '\0') &&
                   (debug("  o_htsize   = %ld\n",*(undefined4 *)(param_1 + 0xb8)), execrld != '\0'))
                  && ((debug("  o_conflict_table_entry_count  = %ld\n",
                             *(undefined4 *)(param_1 + 0xbc)), execrld != '\0' &&
                      (((debug("  o_init_section_ptr     = 0x%lx\n",*(undefined4 *)(param_1 + 0xe8))
                        , execrld != '\0' &&
                        (debug("  o_fini_section_ptr     = 0x%lx\n",*(undefined4 *)(param_1 + 0xec))
                        , execrld != '\0')) &&
                       (debug("  o_first_external_not_ref_this_object = %ld\n",
                              *(undefined4 *)(param_1 + 0xf0)), execrld != '\0')))))) &&
                 (debug("  o_index_first_dynsym_with_got   = %ld\n",*(undefined4 *)(param_1 + 0xf4))
                 , execrld != '\0')) {
                debug("  o_dso_list_base_addr  = 0x%lx\n",*(undefined4 *)(param_1 + 0xf8));
              }
            }
          }
        }
      }
    }
  }
  return;
}



// === obj_create_liblist_entry @ 0fb648d0 (120 bytes) ===

void obj_create_liblist_entry
               (undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
               undefined4 param_5)

{
  undefined4 *puVar1;
  code *UNRECOVERED_JUMPTABLE_00;
  
  puVar1 = (undefined4 *)malloc(0x14);
  if (puVar1 == (undefined4 *)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0fb6490c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*UNRECOVERED_JUMPTABLE_00)();
    return;
  }
  puVar1[1] = param_2;
  puVar1[4] = param_5;
  puVar1[3] = param_4;
  puVar1[2] = param_3;
  *puVar1 = param_1;
                    /* WARNING: Could not recover jumptable at 0x0fb64940. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE_00)();
  return;
}



// === get_symlib_table_entry @ 0fb64950 (84 bytes) ===

ushort get_symlib_table_entry(int param_1,int param_2)

{
  int iVar1;
  ushort uVar2;
  
  iVar1 = *(int *)(param_1 + 0x110);
  if (iVar1 == 0) {
    return 0;
  }
  if (0 < *(int *)(param_1 + 0xb0)) {
    if (*(int *)(param_1 + 0xb0) < 0xff) {
      uVar2 = (ushort)*(byte *)(iVar1 + param_2);
    }
    else {
      uVar2 = *(ushort *)(iVar1 + param_2 * 2);
    }
    return uVar2;
  }
  return 0;
}



// === FUN_0fb649b0 @ 0fb649b0 (108 bytes) ===

void FUN_0fb649b0(int param_1,int param_2)

{
  int *piVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  
  uVar4 = *(uint *)(param_1 + 0x120);
  if (1 < uVar4) {
    piVar1 = (int *)(DAT_0fbd31a0 + 4);
    uVar2 = 2;
    do {
      if (*piVar1 != 0) {
        *piVar1 = *piVar1 - param_2;
        uVar4 = *(uint *)(param_1 + 0x120);
      }
      uVar3 = uVar2 + 1;
      if (uVar4 <= uVar2) {
        return;
      }
      if (piVar1[1] != 0) {
        piVar1[1] = piVar1[1] - param_2;
        uVar4 = *(uint *)(param_1 + 0x120);
      }
      uVar2 = uVar2 + 2;
      piVar1 = piVar1 + 2;
    } while (uVar3 < uVar4);
  }
  return;
}



// === FUN_0fb64a20 @ 0fb64a20 (200 bytes) ===

void FUN_0fb64a20(int param_1,undefined4 param_2)

{
  int iVar1;
  
  if (DAT_0fbd31a4 <= *(uint *)(param_1 + 0x120)) {
    if (DAT_0fbd31a4 == 0) {
      DAT_0fbd31a4 = 2;
    }
    else {
      DAT_0fbd31a4 = DAT_0fbd31a4 * 2;
    }
    DAT_0fbd31a0 = realloc(DAT_0fbd31a0,DAT_0fbd31a4 << 2);
    if (DAT_0fbd31a0 == 0) {
      fatal("Cannot realloc aux_dyn_list with %ld bytes",DAT_0fbd31a4 << 2);
      iVar1 = *(int *)(param_1 + 0x120);
    }
    else {
      iVar1 = *(int *)(param_1 + 0x120);
    }
    *(undefined4 *)(DAT_0fbd31a0 + iVar1 * 4) = param_2;
    *(int *)(param_1 + 0x120) = *(int *)(param_1 + 0x120) + 1;
    return;
  }
  *(undefined4 *)(DAT_0fbd31a0 + *(uint *)(param_1 + 0x120) * 4) = param_2;
  *(int *)(param_1 + 0x120) = *(int *)(param_1 + 0x120) + 1;
  return;
}



// === FUN_0fb64af0 @ 0fb64af0 (540 bytes) ===

void FUN_0fb64af0(int param_1,int *param_2,int param_3)

{
  int *piVar1;
  int iVar2;
  int *piVar3;
  int *piVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int *piVar8;
  int iVar9;
  uint uVar10;
  code *UNRECOVERED_JUMPTABLE;
  
  piVar3 = (int *)_rld_alloc_obj_space(param_1,*(int *)(param_1 + 0x120) << 4,4,param_1 + 0xe3);
  *(int **)(param_1 + 0x6c) = piVar3;
  if (piVar3 == (int *)0x0) {
    fatal("Cannot rld_malloc gotinfo");
    piVar3 = *(int **)(param_1 + 0x6c);
  }
  *piVar3 = *param_2;
  piVar3[1] = *param_2 + param_2[3] * 4;
  piVar3[2] = param_2[2];
  piVar3[3] = param_2[3];
  if (1 < *(uint *)(param_1 + 0x120)) {
    uVar10 = 1;
    iVar9 = 4;
    piVar4 = *(int **)(DAT_0fbd31a0 + 4);
    do {
      piVar3[4] = 0;
      piVar3[7] = 0;
      piVar3[6] = 0;
      iVar7 = 0;
      iVar6 = 0;
      iVar5 = 0;
      piVar8 = piVar3 + 4;
      iVar2 = *piVar4;
      while (iVar2 != 0) {
        if (iVar2 == 3) {
          iVar5 = piVar4[1] - param_3;
          *piVar8 = iVar5;
        }
        else if (iVar2 == 0x7000000a) {
          iVar6 = piVar4[1];
          piVar3[7] = iVar6;
        }
        else if (iVar2 == 0x70000027) {
          iVar7 = piVar4[1];
          piVar3[6] = iVar7;
        }
        iVar2 = piVar4[2];
        if (iVar2 == 0) break;
        if (iVar2 == 3) {
          iVar5 = piVar4[3] - param_3;
          *piVar8 = iVar5;
        }
        else if (iVar2 == 0x7000000a) {
          iVar6 = piVar4[3];
          piVar3[7] = iVar6;
        }
        else if (iVar2 == 0x70000027) {
          iVar7 = piVar4[3];
          piVar3[6] = iVar7;
        }
        piVar1 = piVar4 + 4;
        piVar4 = piVar4 + 4;
        iVar2 = *piVar1;
      }
      if (iVar5 == 0) {
        fatal("No DT_PLTGOT in aux_got %ld obj %s\n",uVar10,*(undefined4 *)(param_1 + 0x18));
        iVar6 = piVar3[7];
        iVar7 = piVar3[6];
        iVar5 = *piVar8;
      }
      uVar10 = uVar10 + 1;
      piVar3[5] = iVar5 + iVar6 * 4;
      if (iVar7 == 0) {
        piVar3[6] = iVar6;
      }
      iVar9 = iVar9 + 4;
      if (*(uint *)(param_1 + 0x120) <= uVar10) break;
      piVar4 = *(int **)(DAT_0fbd31a0 + iVar9);
      piVar3 = piVar8;
    } while( true );
  }
                    /* WARNING: Could not recover jumptable at 0x0fb64cd0. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
}



// === FUN_0fb64d20 @ 0fb64d20 (228 bytes) ===

int FUN_0fb64d20(void)

{
  longlong lVar1;
  int iVar2;
  int iVar3;
  
  if (DAT_0fbde2d4 != -1) {
    return DAT_0fbde2d4;
  }
  DAT_0fbde2d4 = 4;
  lVar1 = setinvent();
  if (lVar1 == -1) {
    return DAT_0fbde2d4;
  }
  iVar2 = getinvent();
  if (iVar2 != 0) {
    iVar3 = *(int *)(iVar2 + 4);
    while( true ) {
      if ((iVar3 == 1) && (*(int *)(iVar2 + 8) == 2)) {
        iVar3 = (int)(*(uint *)(iVar2 + 0x14) & 0xff00) >> 8;
        goto joined_r0x0fb64db8;
      }
      iVar2 = getinvent();
      if (iVar2 == 0) break;
      iVar3 = *(int *)(iVar2 + 4);
    }
  }
  iVar2 = 0;
  iVar3 = 0;
joined_r0x0fb64db8:
  if (iVar2 != 0) {
    iVar2 = 4;
    switch(iVar3) {
    case 0:
    case 1:
    case 2:
      iVar2 = 0;
      DAT_0fbde2d4 = 0;
      break;
    default:
      DAT_0fbde2d4 = 4;
      break;
    case 4:
      iVar2 = 2;
      DAT_0fbde2d4 = 2;
      break;
    case 9:
      iVar2 = 3;
      DAT_0fbde2d4 = 3;
      break;
    case 0x10:
      iVar2 = 3;
      DAT_0fbde2d4 = 3;
      break;
    case 0x20:
    case 0x21:
    case 0x23:
      iVar2 = 3;
      DAT_0fbde2d4 = 3;
    }
    return iVar2;
  }
  return DAT_0fbde2d4;
}



// === verify_o_arch_is_ok @ 0fb64e40 (144 bytes) ===

void verify_o_arch_is_ok(int param_1,int param_2)

{
  byte bVar1;
  longlong lVar2;
  
  if ((param_1 != 0) && (param_2 != 0)) {
    bVar1 = *(byte *)(param_2 + 0xc0);
    if ((*(byte *)(param_1 + 0xc0) != bVar1) &&
       ((*(byte *)(param_1 + 0xc0) <= bVar1 && (2 < bVar1)))) {
      lVar2 = FUN_0fb64d20();
      if (lVar2 < (longlong)(ulonglong)*(byte *)(param_2 + 0xc0)) {
        force_warning("The base program is MIPS%d while %s is MIPS%d on a cpu supporting MIPS%d. This may not work\n"
                      ,*(byte *)(param_1 + 0xc0) + 1,*(undefined4 *)(param_2 + 0x18),
                      *(byte *)(param_2 + 0xc0) + 1,(int)lVar2 + 1);
        return;
      }
    }
  }
  return;
}



// === split @ 0fb64ed0 (232 bytes) ===

uint split(undefined8 param_1,undefined4 *param_2,undefined8 param_3,undefined8 param_4,
          char *param_5,longlong param_6)

{
  char *pcVar1;
  undefined4 *puVar2;
  uint uVar3;
  
  if (debug_trace != '\0') {
    trace("split: %s\n",param_1);
  }
  __rld_strlcpy(param_3,param_1,param_4);
  puVar2 = param_2;
  if (param_6 == 1) {
    puVar2 = param_2 + 1;
    *param_2 = 0;
  }
  uVar3 = (uint)(param_6 == 1);
  pcVar1 = strtok((char *)param_3,param_5);
  *puVar2 = pcVar1;
  if (pcVar1 != (char *)0x0) {
    pcVar1 = strtok((char *)0x0,param_5);
    puVar2[1] = pcVar1;
    puVar2 = puVar2 + 2;
    while (uVar3 = uVar3 + 1, pcVar1 != (char *)0x0) {
      pcVar1 = strtok((char *)0x0,param_5);
      *puVar2 = pcVar1;
      puVar2 = puVar2 + 1;
    }
  }
  return uVar3;
}



// === found_matching_string_ignore_comment_and_whitespace @ 0fb64fc0 (1156 bytes) ===

/* WARNING: Instruction at (ram,0x0fb65184) overlaps instruction at (ram,0x0fb65180)
    */

void found_matching_string_ignore_comment_and_whitespace
               (undefined8 param_1,char *param_2,longlong param_3)

{
  bool bVar1;
  int iVar2;
  size_t sVar4;
  char *pcVar5;
  size_t sVar6;
  char *pcVar7;
  longlong lVar3;
  int iVar8;
  char *pcVar9;
  char cVar10;
  int iVar11;
  char *pcVar12;
  undefined4 *puVar13;
  char *pcVar14;
  undefined1 *puVar15;
  code *UNRECOVERED_JUMPTABLE_00;
  char acStack_81 [9];
  
  pcVar14 = acStack_81 + 1;
  sVar4 = strlen((char *)param_1);
  if (debug_map != '\0') {
    debug("checking versions: liblist %s check against dynamic sect %s\n",param_2,param_1);
  }
  pcVar5 = strchr(param_2,0x23);
  if (pcVar5 != (char *)0x0) {
    param_2 = pcVar5 + 1;
  }
  cVar10 = *param_2;
  pcVar5 = param_2;
  if (cVar10 != '\0') {
    while ((cVar10 == ' ' || (pcVar5 = param_2, cVar10 == '\t'))) {
      cVar10 = param_2[1];
      pcVar5 = param_2 + 1;
      if ((cVar10 == '\0') || ((cVar10 != ' ' && (cVar10 != '\t')))) break;
      cVar10 = param_2[2];
      pcVar5 = param_2 + 2;
      param_2 = pcVar5;
      if (cVar10 == '\0') break;
    }
  }
  sVar6 = strlen(pcVar5);
  if (sVar6 == 0) {
    if (debug_map != '\0') {
      debug("PASS version match empty matches all\n");
    }
                    /* WARNING: Could not recover jumptable at 0x0fb65348. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*UNRECOVERED_JUMPTABLE_00)();
    return;
  }
  if ((pcVar5[sVar6 - 1] == ' ') || (bVar1 = false, pcVar12 = pcVar5, pcVar5[sVar6 - 1] == '\t')) {
    iVar11 = -(sVar6 + 0x10 & 0xfffffff0);
    pcVar12 = acStack_81 + 1 + iVar11;
    pcVar14 = acStack_81 + iVar11 + 1;
    strcpy(acStack_81 + iVar11 + 1,pcVar5);
    pcVar5 = acStack_81 + sVar6 + iVar11;
    if (acStack_81 + 1 + iVar11 <= pcVar5) {
      while( true ) {
        if ((*pcVar5 != ' ') && (*pcVar5 != '\t')) break;
        cVar10 = pcVar5[-1];
        *pcVar5 = '\0';
        if ((pcVar5 + -1 < acStack_81 + 1 + iVar11) ||
           (((pcVar7 = pcVar5 + -2, cVar10 != ' ' && (cVar10 != '\t')) ||
            (pcVar5[-1] = '\0', pcVar5 = pcVar7, pcVar7 < acStack_81 + 1 + iVar11)))) break;
      }
    }
    bVar1 = true;
  }
  puVar15 = pcVar14;
  pcVar5 = pcVar12;
  if (param_3 != 0) {
    pcVar7 = strchr(pcVar12,0x2e);
    iVar11 = (int)pcVar7 - (int)pcVar12;
    if (pcVar7 != (char *)0x0) {
      if (iVar11 == 0) {
        if (debug_map != '\0') {
          debug("version %s is bad version format\n",pcVar12);
        }
                    /* WARNING: Could not recover jumptable at 0x0fb65390. Too many branches */
                    /* WARNING: Treating indirect jump as call */
        (*UNRECOVERED_JUMPTABLE_00)();
        return;
      }
      if (bVar1) {
        *pcVar7 = '\0';
      }
      else {
        iVar2 = -(iVar11 + 0x10U & 0xfffffff0);
        puVar15 = pcVar14 + iVar2;
        pcVar5 = pcVar14 + iVar2;
        __rld_strlcpy((int)pcVar14 + iVar2,pcVar12,iVar11 + 1);
      }
    }
  }
  iVar11 = -(sVar4 + 0x10 & 0xfffffff0);
  iVar2 = -((sVar4 + 1) * 4 + 0xf & 0xfffffff0);
  puVar13 = (undefined4 *)(puVar15 + iVar2 + iVar11);
  lVar3 = split(param_1,puVar15 + iVar2 + iVar11,(int)puVar15 + iVar11,sVar4 + 1,&DAT_0fbd56c8,0);
  if (0 < lVar3) {
    do {
      pcVar12 = (char *)*puVar13;
      pcVar7 = strchr(pcVar12,0x23);
      if (pcVar7 != (char *)0x0) {
        pcVar12 = pcVar7 + 1;
      }
      cVar10 = *pcVar12;
      pcVar7 = pcVar12;
      if (cVar10 != '\0') {
        while ((cVar10 == ' ' || (pcVar12 = pcVar7, cVar10 == '\t'))) {
          cVar10 = pcVar7[1];
          pcVar12 = pcVar7 + 1;
          if ((cVar10 == '\0') || ((cVar10 != ' ' && (cVar10 != '\t')))) break;
          cVar10 = pcVar7[2];
          pcVar12 = pcVar7 + 2;
          pcVar7 = pcVar12;
          if (cVar10 == '\0') break;
        }
      }
      if (cVar10 != '\0') {
        sVar4 = strlen(pcVar12);
        pcVar7 = pcVar12 + (sVar4 - 1);
        if (pcVar12 <= pcVar7) {
          cVar10 = *pcVar7;
          while( true ) {
            if ((cVar10 != ' ') && (cVar10 != '\t')) break;
            cVar10 = pcVar7[-1];
            *pcVar7 = '\0';
            if (pcVar7 + -1 < pcVar12) break;
            pcVar9 = pcVar7 + -2;
            if (((cVar10 != ' ') && (cVar10 != '\t')) || (pcVar7[-1] = '\0', pcVar9 < pcVar12))
            break;
            cVar10 = *pcVar9;
            pcVar7 = pcVar9;
          }
        }
        if ((param_3 != 0) && (pcVar7 = strchr(pcVar12,0x2e), pcVar7 != (char *)0x0)) {
          if (pcVar7 == pcVar12) goto code_r0x0fb65188;
          *pcVar7 = '\0';
        }
        iVar8 = strcmp(pcVar12,pcVar5);
        if (iVar8 == 0) {
          if (debug_map != '\0') {
            debug("PASS version match %s\n",pcVar12);
          }
                    /* WARNING: Could not recover jumptable at 0x0fb652fc. Too many branches */
                    /* WARNING: Treating indirect jump as call */
          (*UNRECOVERED_JUMPTABLE_00)();
          return;
        }
      }
code_r0x0fb65188:
      puVar13 = puVar13 + 1;
    } while (puVar13 < puVar15 + (int)lVar3 * 4 + iVar2 + iVar11);
  }
  if (debug_map != '\0') {
    debug("Failed version match\n");
  }
                    /* WARNING: Could not recover jumptable at 0x0fb6527c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE_00)();
  return;
}



// === found_matching_string @ 0fb65450 (252 bytes) ===

void found_matching_string(undefined8 param_1,char *param_2)

{
  int iVar1;
  size_t sVar3;
  longlong lVar2;
  int iVar4;
  char *__s1;
  undefined4 *puVar5;
  undefined8 unaff_s8;
  undefined8 in_ra;
  undefined8 uStack_50;
  
  uStack_50 = in_ra;
  sVar3 = strlen((char *)param_1);
  iVar1 = -(sVar3 * 4 + 0x13 & 0xfffffff0);
  puVar5 = (undefined4 *)((int)&uStack_50 + iVar1);
  lVar2 = split(param_1,(int)&uStack_50 + iVar1,
                (int)&uStack_50 + (iVar1 - (sVar3 + 0x10 & 0xfffffff0)),sVar3 + 1,&DAT_0fbd56c8,0);
  if (0 < lVar2) {
    __s1 = *(char **)((int)&uStack_50 + iVar1);
    while( true ) {
      iVar4 = strcmp(__s1,param_2);
      puVar5 = puVar5 + 1;
      if (iVar4 == 0) {
                    /* WARNING: Could not recover jumptable at 0x0fb65544. Too many branches */
                    /* WARNING: Treating indirect jump as call */
        (*(code *)uStack_50)(uStack_50,unaff_s8);
        return;
      }
      if ((undefined4 *)((int)&uStack_50 + (int)lVar2 * 4 + iVar1) <= puVar5) break;
      __s1 = (char *)*puVar5;
    }
  }
                    /* WARNING: Could not recover jumptable at 0x0fb6551c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)uStack_50)();
  return;
}



// === obj_dynsym_got @ 0fb65550 (304 bytes) ===

/* WARNING: Instruction at (ram,0x0fb65644) overlaps instruction at (ram,0x0fb65640)
    */

undefined4 obj_dynsym_got(int param_1,uint param_2)

{
  uint uVar1;
  uint uVar2;
  undefined4 *puVar3;
  int *piVar4;
  uint uVar5;
  
  piVar4 = *(int **)(param_1 + 0x6c);
  if (*(uint *)(param_1 + 0x120) < 2) {
    puVar3 = (undefined4 *)(piVar4[1] + param_2 * 4 + *(int *)(param_1 + 0xf4) * -4);
    if (puVar3 < (undefined4 *)(*piVar4 + piVar4[2] * 4)) {
      return 0;
    }
    return *puVar3;
  }
  uVar2 = find_reloc(param_1,param_2);
  if (uVar2 != 0) {
    piVar4 = (int *)(*(int *)(param_1 + 0x70) + uVar2 * 8);
    uVar1 = piVar4[1];
    if ((param_2 == uVar1 >> 8) && (uVar5 = *(uint *)(param_1 + 0xa8) >> 3, uVar2 < uVar5)) {
      do {
        if ((uVar1 & 0xff) == 0x24) {
          return *(undefined4 *)(*piVar4 - (*(int *)(param_1 + 0x3c) - *(int *)(param_1 + 0x10)));
        }
        uVar1 = piVar4[3];
        piVar4 = piVar4 + 2;
      } while ((param_2 == uVar1 >> 8) && (piVar4 < (int *)(*(int *)(param_1 + 0x70) + uVar5 * 8)));
    }
    return 0;
  }
  return *(undefined4 *)(*(int *)(param_1 + 100) + param_2 * 0x10 + 4);
}



// === obj_set_dynsym_got @ 0fb65680 (48 bytes) ===

void obj_set_dynsym_got(int param_1,uint param_2,undefined4 param_3)

{
  if (*(uint *)(param_1 + 0xf4) <= param_2) {
    *(undefined4 *)
     (*(int *)(*(int *)(param_1 + 0x6c) + 4) + (param_2 - *(uint *)(param_1 + 0xf4)) * 4) = param_3;
    return;
  }
  return;
}



// === FUN_0fb656b0 @ 0fb656b0 (144 bytes) ===

int FUN_0fb656b0(uint param_1)

{
  int iVar1;
  
  if (DAT_0fbd31ac < param_1) {
    if (param_1 < 0x400) {
      param_1 = 0x400;
    }
    if (DAT_0fbd31a8 != 0) {
      free(DAT_0fbd31a8);
    }
    iVar1 = malloc(param_1);
    if (iVar1 == 0) {
      fatal("Out of memory allocating buffer %ld bytes for debug tracing\n",param_1);
    }
    DAT_0fbd31a8 = iVar1;
    DAT_0fbd31ac = param_1;
    return iVar1;
  }
  return DAT_0fbd31a8;
}



// === add_a_tried_path @ 0fb65740 (180 bytes) ===

char * add_a_tried_path(int *param_1,char *param_2)

{
  undefined1 uVar1;
  undefined1 uVar2;
  char *in_v0_lo;
  size_t sVar3;
  char *pcVar4;
  undefined1 *puVar5;
  int iVar6;
  
  if (param_1 != (int *)0x0) {
    sVar3 = strlen(param_2);
    iVar6 = param_1[2];
    if ((uint)param_1[3] <= sVar3 + iVar6 + 3) {
      tried_path_grow(param_1,sVar3 + 3);
      iVar6 = param_1[2];
    }
    pcVar4 = strcpy((char *)(*param_1 + iVar6),param_2);
    uVar2 = DAT_0fbd5761;
    uVar1 = DAT_0fbd5760;
    iVar6 = param_1[2];
    param_1[2] = iVar6 + sVar3;
    puVar5 = (undefined1 *)(*param_1 + iVar6 + sVar3);
    puVar5[1] = uVar2;
    *puVar5 = uVar1;
    param_1[2] = param_1[2] + 1;
    return pcVar4;
  }
  return in_v0_lo;
}



// === tried_path_grow @ 0fb65800 (256 bytes) ===

void * tried_path_grow(int *param_1,ulonglong param_2)

{
  int iVar1;
  void *pvVar2;
  void *pvVar3;
  
  if (param_2 < 0x400) {
    iVar1 = param_1[3] + 0x400;
    param_1[3] = iVar1;
  }
  else {
    iVar1 = param_1[3] + (int)param_2;
    param_1[3] = iVar1;
  }
  if (*(char *)(param_1 + 1) == '\0') {
    *(undefined1 *)(param_1 + 1) = 1;
    pvVar2 = (void *)malloc(iVar1);
    if (pvVar2 == (void *)0x0) {
      fatal("Cannot malloc tried_path maxlen=%ld\n",param_1[3]);
    }
    if (debug_malloc != '\0') {
      debug("tried_path malloc\'ed, addr = 0x%lx\n",*param_1);
    }
    pvVar3 = memmove(pvVar2,(void *)*param_1,param_1[2]);
    *(undefined1 *)(param_1[2] + (int)pvVar2) = 0;
    *param_1 = (int)pvVar2;
    return pvVar3;
  }
  pvVar2 = (void *)realloc(*param_1,iVar1);
  *param_1 = (int)pvVar2;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)fatal("Cannot realloc tried_path maxlen=%ld\n",param_1[3]);
  }
  if (debug_malloc != '\0') {
    pvVar2 = (void *)debug("tried_path remalloc\'ed, addr = 0x%lx\n",*param_1);
  }
  return pvVar2;
}



// === FUN_0fb65900 @ 0fb65900 (172 bytes) ===

int FUN_0fb65900(longlong param_1)

{
  int iVar3;
  undefined8 uVar1;
  longlong lVar2;
  undefined1 *puStack_20;
  undefined1 uStack_1c;
  undefined4 uStack_18;
  undefined4 uStack_14;
  
  if (DAT_0fbde374 == -1) {
    if ((param_1 == 0) || (iVar3 = strcmp("__missing_function_",(char *)param_1), iVar3 != 0)) {
      uStack_1c = 0;
      puStack_20 = &DAT_0fbd5820;
      uStack_14 = 0;
      uStack_18 = 0;
      uVar1 = elfhash("__missing_function_");
      lVar2 = resolve_symbol("__missing_function_","rld missing func check",uVar1,0,0,0,&puStack_20)
      ;
      if (lVar2 == -1) {
        DAT_0fbde374 = 0;
      }
      else {
        DAT_0fbde374 = (int)lVar2;
      }
      free_things_tried(&puStack_20);
    }
    else {
      DAT_0fbde374 = 0;
    }
    return DAT_0fbde374;
  }
  return DAT_0fbde374;
}



// === lazy_text_resolve @ 0fb659b0 (1628 bytes) ===

/* WARNING: Instruction at (ram,0x0fb65bb0) overlaps instruction at (ram,0x0fb65bac)
    */

longlong lazy_text_resolve(undefined8 param_1,undefined8 param_2,undefined8 param_3)

{
  uint uVar1;
  int iVar4;
  longlong lVar2;
  longlong lVar3;
  char cVar5;
  undefined8 uVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  undefined1 *puStack_60;
  undefined1 uStack_5c;
  undefined4 uStack_58;
  undefined4 uStack_54;
  longlong lStack_50;
  longlong lStack_48;
  
  uStack_5c = 0;
  puStack_60 = &DAT_0fbd5820;
  uStack_54 = 0;
  uStack_58 = 0;
  if (ltr_extra != '\0') {
    if (user_tracking != '\0') {
      track("Entering RLD through lazy_text_resolve\n");
    }
    if (debug_trace != '\0') {
      trace("lazy_text_resolve: pc 0x%lx, index %ld, ra 0x%lx\n",param_1,param_2,param_3);
    }
  }
  if (multi_threaded != '\0') {
    enter_LTR(pObj_Head);
  }
  has_unresolvable_symbol = '\0';
  iVar4 = address_to_obj(pObj_Head,param_1);
  if (iVar4 == 0) {
    free_things_tried(&puStack_60);
    fatal("pc not in objlist: pc 0x%lx symbol_index %ld\n",param_1,param_2);
    iVar8 = iRam00000064;
  }
  else {
    iVar8 = *(int *)(iVar4 + 100);
  }
  iVar10 = (int)param_2 * 0x10;
  iVar8 = *(int *)(iVar4 + 0x60) + *(int *)(iVar8 + iVar10);
  if (ltr_extra != '\0') {
    if (user_tracking != '\0') {
      track("lazy text of %s in %s, ra == 0x%lx, stub == 0x%lx\n",iVar8,
            *(undefined4 *)(iVar4 + 0x18),param_3,param_1);
    }
    if (debug_symbol != '\0') {
      debug("lazy_text_resolve: looking for %s\n",
            *(int *)(*(int *)(iVar4 + 100) + iVar10) + *(int *)(iVar4 + 0x60));
    }
    if ((rld_has_listed_symbols != '\0') &&
       (lVar2 = rld_is_listed_symbol
                          (*(int *)(*(int *)(iVar4 + 100) + iVar10) + *(int *)(iVar4 + 0x60)),
       lVar2 != 0)) {
      debug("lazy_text_resolve: looking for %s\n",iVar8);
    }
  }
  iVar9 = (int)param_2 * 8;
  iVar10 = *(int *)(*(int *)(iVar4 + 0x68) + iVar9);
  if (iVar10 == 0) {
    iVar10 = get_dynsym_hash_value(iVar4,param_2);
  }
  lVar2 = resolve_symbol(iVar8,*(undefined4 *)(iVar4 + 0x2c),iVar10,1,iVar4,param_2,&puStack_60);
  if (lVar2 == -1) {
    lVar3 = get_symlib_table_entry(iVar4,param_2);
    if (lVar3 != 0) {
      iVar10 = *(int *)(iVar4 + 0x74) + (int)lVar3 * 0x14;
      if ((*(uint *)(iVar10 + -4) & 0x10) != 0) {
        iVar7 = *(int *)(iVar10 + -0x14) + *(int *)(iVar4 + 0x60);
        lStack_48 = (longlong)(iVar10 + -0x14);
        if (multi_threaded != '\0') {
          exit_LTR();
        }
        if (save_error_message_for_dlerror == '\0') {
          if (in_delay_load == '\0') {
            in_delay_load = '\x01';
            lStack_50 = 1;
          }
          else {
            lStack_50 = 0;
          }
        }
        else {
          lStack_50 = 0;
        }
        uVar6 = 7;
        if (*(char *)(iVar4 + 0xca) == '\0') {
          uVar6 = 6;
        }
        if (_rld_delay_load_versions_checking_on == '\0') {
          iVar10 = _rld_new_interface(6,uVar6,iVar7,1,0,0);
        }
        else {
          iVar10 = _rld_new_interface(6,uVar6,iVar7,1,
                                      *(int *)((int)lStack_48 + 0xc) + *(int *)(iVar4 + 0x60),
                                      *(undefined4 *)((int)lStack_48 + 0x10));
        }
        if (lStack_50 != 0) {
          in_delay_load = '\0';
        }
        if (iVar10 == 0) {
          if ((rld_has_listed_symbols != '\0') && (lVar2 = rld_is_listed_symbol(iVar8), lVar2 != 0))
          {
            debug("tried\n %s\n",puStack_60);
          }
          free_things_tried(&puStack_60);
          fatal("could not delay load the object %s to resolve the symbol %s\n",iVar7,iVar8);
        }
        else if (*(char *)(iVar4 + 0xce) != '\0') {
          if (*(char *)(*(int *)(iVar10 + 0xc) + 0xce) == '\0') {
            *(char *)(*(int *)(iVar10 + 0xc) + 0xce) = *(char *)(iVar4 + 0xce);
          }
          *(undefined1 *)(iVar10 + 5) = 1;
        }
        if (multi_threaded != '\0') {
          enter_LTR();
        }
        iVar10 = *(int *)(*(int *)(iVar4 + 0x68) + iVar9);
        if (iVar10 == 0) {
          iVar10 = get_dynsym_hash_value(iVar4,param_2);
        }
        lVar2 = resolve_symbol(iVar8,*(undefined4 *)(iVar4 + 0x2c),iVar10,1,iVar4,param_2,
                               &puStack_60);
      }
    }
    if (lVar2 == -1) {
      lVar3 = FUN_0fb6c0a0(iVar4,param_2);
      if (lVar3 != 0) {
        if ((rld_has_listed_symbols != '\0') && (lVar2 = rld_is_listed_symbol(iVar8), lVar2 != 0)) {
          debug("resolve to missing symbol: \n %s\n",puStack_60);
        }
        lVar2 = FUN_0fb65900(iVar8);
      }
      if (lVar2 == -1) {
        if ((rld_has_listed_symbols != '\0') && (lVar3 = rld_is_listed_symbol(iVar8), lVar3 != 0)) {
          debug("looked at: \n %s\n",puStack_60);
        }
        free_things_tried(&puStack_60);
        fatal("attempted access to unresolvable symbol in %s: %s\n",*(undefined4 *)(iVar4 + 0x18),
              iVar8);
      }
    }
  }
  if (has_unresolvable_symbol != '\0') {
    if (debug_symbol != '\0') {
      debug("fatal: this executable has unresolvable symbols\n");
    }
    free_things_tried(&puStack_60);
    fatal("this executable has unresolvable symbols\n");
  }
  if ((rld_has_listed_symbols != '\0') && (lVar3 = rld_is_listed_symbol(iVar8), lVar3 != 0)) {
    debug("lazy_text_resolve: resolved addr 0x%lx\n",lVar2);
  }
  if (debug_symbol != '\0') {
    debug("lazy_text_resolve: resolved addr 0x%lx\n",lVar2);
  }
  uVar1 = *(uint *)(*(int *)(iVar4 + 0x68) + iVar9 + 4);
  if ((*(char *)(iVar4 + 0xc1) != '\0') && ((uVar1 & 0xff) == 0)) {
    iVar10 = find_reloc(iVar4,param_2);
    *(int *)(*(int *)(iVar4 + 0x68) + iVar9 + 4) = iVar10 * 0x100 + 1;
    uVar1 = *(uint *)(*(int *)(iVar4 + 0x68) + iVar9 + 4);
  }
  if ((uVar1 >> 8 != 0) && (lVar3 = FUN_0fb6b990(iVar4,param_2), lVar3 != 0)) {
    free_things_tried(&puStack_60);
    fatal("lazy_text_resolve: symbol %s should not have any relocation entry\n",iVar8);
  }
  if (*(uint *)(iVar4 + 0x120) < 2) {
    obj_set_dynsym_got(iVar4,param_2,lVar2);
  }
  else {
    resolve_relocations(iVar4,param_2,lVar2);
  }
  if (ltr_extra == '\0') goto LAB_0fb65c9c;
  if ((rld_has_listed_symbols != '\0') && (lVar3 = rld_is_listed_symbol(iVar8), lVar3 != 0)) {
    debug("lazy_text_resolve: resolved addr 0x%lx\n",lVar2);
  }
  if (debug_symbol != '\0') {
    debug("lazy_text_resolve: resolved addr 0x%lx\n",lVar2);
  }
  if (((guarantee_start_init == '\0') || (iVar8 = address_to_obj(pObj_Head,lVar2), iVar8 == iVar4))
     || (iVar8 == 0)) goto LAB_0fb65c9c;
  cVar5 = *(char *)(iVar8 + 0xcf);
  if (guarantee_init == '\0') {
LAB_0fb65c94:
    if (cVar5 != '\0') goto LAB_0fb65c9c;
  }
  else if (cVar5 != '\0') {
    if (*(char *)(iVar8 + 0xd0) == '\0') {
      free_things_tried(&puStack_60);
      fatal("At least one of the objects in this link has specified the -guarantee_init flag.\nObjects %s and %s have init code which call each other.\n"
            ,*(undefined4 *)(iVar4 + 0x2c),*(undefined4 *)(iVar8 + 0x2c));
      cVar5 = *(char *)(iVar8 + 0xcf);
    }
    goto LAB_0fb65c94;
  }
  iVar4 = *(int *)(iVar8 + 0xe8);
  *(undefined1 *)(iVar8 + 0xcf) = 1;
  if (iVar4 != 0) {
    if (debug_trace != '\0') {
      trace("lazy_text_resolve: calling .init section of \"%s\" -- 0x%lx\n",
            *(undefined4 *)(iVar8 + 0x18),iVar4);
    }
    init_bridge(iVar4,has_pixified_init);
  }
  *(undefined1 *)(iVar8 + 0xd0) = 1;
LAB_0fb65c9c:
  if (multi_threaded != '\0') {
    exit_LTR();
  }
  if ((ltr_extra != '\0') && (user_tracking != '\0')) {
    track("Exiting RLD through lazy_text_resolve\n");
  }
  free_things_tried(&puStack_60);
  return lVar2;
}



// === FUN_0fb66010 @ 0fb66010 (228 bytes) ===

longlong FUN_0fb66010(undefined8 param_1,undefined8 param_2)

{
  short sVar1;
  longlong lVar2;
  int iVar3;
  undefined1 *puStack_40;
  undefined1 uStack_3c;
  undefined4 uStack_38;
  undefined4 uStack_34;
  int aiStack_30 [2];
  undefined8 uStack_28;
  undefined8 uStack_20;
  longlong lStack_10;
  
  uStack_3c = 0;
  puStack_40 = &DAT_0fbd5820;
  uStack_34 = 0;
  uStack_38 = 0;
  uStack_28 = param_2;
  uStack_20 = param_1;
  lVar2 = FUN_0fb67290(param_1,aiStack_30,0,pObj_Head,&puStack_40);
  if (lVar2 == -1) {
    lStack_10 = find_rld_allocated_common_symbol(uStack_20,uStack_28);
  }
  else {
    iVar3 = *(int *)(aiStack_30[0] + 100) + (int)lVar2 * 0x10;
    sVar1 = *(short *)(iVar3 + 0xe);
    if (sVar1 == -0x100) {
      lStack_10 = common_handling(aiStack_30[0],lVar2,&puStack_40);
    }
    else {
      iVar3 = *(int *)(iVar3 + 4);
      lStack_10 = (longlong)iVar3;
      if ((sVar1 != -0xf) &&
         (lStack_10 = (longlong)
                      (iVar3 - (*(int *)(aiStack_30[0] + 0x3c) - *(int *)(aiStack_30[0] + 0x10))),
         debug_conflict != '\0')) {
        debug("Global definition of conflict symbol %s taken from %s, address 0x%lx\n",uStack_20,
              *(undefined4 *)(aiStack_30[0] + 0x2c),lStack_10);
      }
    }
  }
  free_things_tried(&puStack_40);
  return lStack_10;
}



// === FUN_0fb66100 @ 0fb66100 (648 bytes) ===

void FUN_0fb66100(undefined8 param_1)

{
  int iVar1;
  longlong lVar2;
  longlong lVar3;
  int iVar4;
  int iVar5;
  longlong lVar6;
  int iVar7;
  int iVar8;
  undefined1 *puStack_80;
  undefined1 uStack_7c;
  int iStack_78;
  undefined4 uStack_74;
  ulonglong uStack_70;
  longlong lStack_68;
  undefined8 uStack_60;
  longlong lStack_58;
  
  uStack_7c = 0;
  puStack_80 = &DAT_0fbd5820;
  uStack_74 = 0;
  iStack_78 = 0;
  uStack_60 = param_1;
  if (debug_trace != '\0') {
    trace("search_for_conflicts: obj 0x%lx, name %s\n",param_1,*(undefined4 *)((int)param_1 + 0x18))
    ;
  }
  if (*(int *)((int)uStack_60 + 0x78) != 0) {
    if (*(int *)((int)uStack_60 + 0xbc) != 0) {
      lStack_68 = 0;
      uStack_70 = 0;
      do {
        iVar5 = *(int *)(*(int *)((int)uStack_60 + 0x78) + (int)lStack_68);
        iVar4 = *(int *)(*(int *)((int)uStack_60 + 0x68) + iVar5 * 8);
        if (iVar4 == 0) {
          iVar4 = get_dynsym_hash_value(uStack_60,iVar5);
        }
        iVar5 = *(int *)(*(int *)((int)uStack_60 + 100) + iVar5 * 0x10) +
                *(int *)((int)uStack_60 + 0x60);
        if (iStack_78 != 0) {
          iStack_78 = 0;
          *puStack_80 = 0;
        }
        lStack_58 = FUN_0fb66010(iVar5,iVar4);
        iVar1 = pObj_Head;
        if (lStack_58 != -1) {
          for (; iVar1 != 0; iVar1 = *(int *)(iVar1 + 8)) {
            lVar2 = find_symbol_in_object(iVar1,iVar5,iVar4,&LAB_0fb67880,&puStack_80);
            if (lVar2 != -1) {
              iVar8 = *(int *)(iVar1 + 100) + (int)lVar2 * 0x10;
              if (((*(byte *)(iVar1 + 0xc9) & 1) != 0) ||
                 (lVar6 = lStack_58, *(char *)(iVar8 + 0xd) == '\x03')) {
                if (debug_conflict != '\0') {
                  debug("Using local definition of conflict symbol %s in %s\n",iVar5,
                        *(undefined4 *)(iVar1 + 0x2c));
                }
                iVar7 = *(int *)(iVar8 + 4);
                if (*(short *)(iVar8 + 0xe) != -0xf) {
                  lVar3 = is_weak_or_global_symbol(iVar1,lVar2);
                  lVar6 = lStack_58;
                  if (lVar3 != 1) goto LAB_0fb66250;
                  iVar7 = *(int *)(iVar8 + 4) - (*(int *)(iVar1 + 0x3c) - *(int *)(iVar1 + 0x10));
                }
                lVar6 = (longlong)iVar7;
              }
LAB_0fb66250:
              resolve_relocations(iVar1,lVar2,lVar6);
              if (*(int *)(iVar1 + 0x120) == 1) {
                obj_set_dynsym_got(iVar1,lVar2,lVar6);
              }
              if (debug_conflict != '\0') {
                debug("resolved conflict symbol %s in %s to 0x%lx\n",iVar5,
                      *(undefined4 *)(iVar1 + 0x2c),lVar6);
              }
            }
          }
        }
        uStack_70 = (ulonglong)((int)uStack_70 + 1);
        lStack_68 = (longlong)((int)lStack_68 + 4);
      } while (uStack_70 < (ulonglong)(longlong)*(int *)((int)uStack_60 + 0xbc));
    }
    free_things_tried(&puStack_80);
  }
  return;
}



// === FUN_0fb663a0 @ 0fb663a0 (2016 bytes) ===

/* WARNING: Instruction at (ram,0x0fb664e8) overlaps instruction at (ram,0x0fb664e4)
    */

void FUN_0fb663a0(int param_1,undefined8 param_2,int *param_3,undefined4 *param_4)

{
  bool bVar1;
  longlong lVar2;
  longlong lVar3;
  char cVar4;
  char cVar6;
  int iVar5;
  int iVar8;
  undefined8 uVar7;
  byte bVar9;
  int iVar10;
  int unaff_gp_lo;
  
  if (*(char *)(unaff_gp_lo + -0x77b0) == '\0') {
LAB_0fb663f8:
    cVar6 = *(char *)(unaff_gp_lo + -0x7eeb);
  }
  else {
    lVar2 = rld_is_listed_symbol(*param_3 + *(int *)(param_1 + 0x60));
    cVar6 = *(char *)(unaff_gp_lo + -0x7eeb);
    if (lVar2 != 0) {
      debug(unaff_gp_lo + -0x5498,*param_3 + *(int *)(param_1 + 0x60));
      goto LAB_0fb663f8;
    }
  }
  if (cVar6 != '\0') {
    debug(unaff_gp_lo + -0x5498,*param_3 + *(int *)(param_1 + 0x60));
  }
  if ((*(short *)((int)param_3 + 0xe) == -0xe) || (*(short *)((int)param_3 + 0xe) == -0x100)) {
    lVar2 = common_handling(param_1,param_2,param_4);
    *(int *)(unaff_gp_lo + -0x7e4c) = *(int *)(unaff_gp_lo + -0x7e4c) + 1;
  }
  else {
    iVar5 = (int)param_2 * 8;
    iVar8 = *(int *)(*(int *)(param_1 + 0x68) + iVar5);
    if (iVar8 == 0) {
      iVar8 = get_dynsym_hash_value(param_1,param_2);
    }
    lVar2 = resolve_symbol(*param_3 + *(int *)(param_1 + 0x60),*(undefined4 *)(param_1 + 0x2c),iVar8
                           ,0,param_1,param_2,param_4);
    if (*(char *)(unaff_gp_lo + -0x77b0) == '\0') {
LAB_0fb664ac:
      cVar6 = *(char *)(unaff_gp_lo + -0x7eeb);
    }
    else {
      lVar3 = rld_is_listed_symbol(*param_3 + *(int *)(param_1 + 0x60));
      cVar6 = *(char *)(unaff_gp_lo + -0x7eeb);
      if (lVar3 != 0) {
        debug(unaff_gp_lo + -0x5478,*param_3 + *(int *)(param_1 + 0x60),lVar2);
        goto LAB_0fb664ac;
      }
    }
    if (cVar6 != '\0') {
      debug(unaff_gp_lo + -0x5478,*param_3 + *(int *)(param_1 + 0x60),lVar2);
    }
    if ((((lVar2 == -1) && (lVar3 = get_symlib_table_entry(param_1,param_2), lVar3 != 0)) &&
        (iVar8 = *(int *)(param_1 + 0x74) + (int)lVar3 * 0x14, (*(uint *)(iVar8 + -4) & 0x10) != 0))
       && (*(char *)(unaff_gp_lo + -0x7ee0) == '\0')) {
      iVar10 = *(int *)(iVar8 + -0x14) + *(int *)(param_1 + 0x60);
      if (*(char *)(unaff_gp_lo + -0x7f48) == '\0') {
        bVar1 = true;
        if (*(char *)(unaff_gp_lo + -0x7f88) == '\0') {
          *(undefined1 *)(unaff_gp_lo + -0x7f88) = 1;
        }
        else {
          bVar1 = false;
        }
      }
      else {
        bVar1 = false;
      }
      uVar7 = 4;
      if (*(char *)(param_1 + 0xca) == '\0') {
        uVar7 = 3;
      }
      if (*(char *)(unaff_gp_lo + 0x3208) == '\0') {
        iVar8 = _rld_dlopen(iVar10,5,0,0,uVar7);
      }
      else {
        iVar8 = _rld_dlopen(iVar10,5,*(int *)(iVar8 + -8) + *(int *)(param_1 + 0x60),
                            *(undefined4 *)(iVar8 + -4),uVar7);
      }
      if (bVar1) {
        *(undefined1 *)(unaff_gp_lo + -0x7f88) = 0;
      }
      if (iVar8 == 0) {
        fatal(unaff_gp_lo + -0x5800,iVar10,*param_3 + *(int *)(param_1 + 0x60));
      }
      else if (*(char *)(param_1 + 0xce) != '\0') {
        if (*(char *)(*(int *)(iVar8 + 0xc) + 0xce) == '\0') {
          *(char *)(*(int *)(iVar8 + 0xc) + 0xce) = *(char *)(param_1 + 0xce);
        }
        *(undefined1 *)(iVar8 + 5) = 1;
      }
      iVar8 = *(int *)(*(int *)(param_1 + 0x68) + iVar5);
      if (iVar8 == 0) {
        iVar8 = get_dynsym_hash_value(param_1,param_2);
      }
      lVar2 = resolve_symbol(*param_3 + *(int *)(param_1 + 0x60),*(undefined4 *)(param_1 + 0x2c),
                             iVar8,0,param_1,param_2,param_4);
    }
    cVar6 = *(char *)(unaff_gp_lo + -0x7ec0);
    if (lVar2 == -1) {
      if ((*(byte *)(param_3 + 3) & 0xf) != 2) {
        lVar2 = common_handling(param_1,param_2,param_4);
        *(int *)(unaff_gp_lo + -0x7e4c) = *(int *)(unaff_gp_lo + -0x7e4c) + 1;
      }
      cVar6 = *(char *)(unaff_gp_lo + -0x7ec0);
      if (lVar2 == -1) {
        lVar3 = FUN_0fb6c0a0(param_1,param_2);
        if (lVar3 != 0) {
          lVar2 = FUN_0fb65900(*param_3 + *(int *)(param_1 + 0x60));
        }
        cVar6 = *(char *)(unaff_gp_lo + -0x7ec0);
      }
    }
    iVar8 = *(int *)(unaff_gp_lo + -0x7e54);
    if ((cVar6 == '\0') && (iVar8 = *(int *)(unaff_gp_lo + -0x7e54), lVar2 == -1)) {
      if ((int)(uint)*(byte *)(param_3 + 3) >> 4 == 2) {
        cVar6 = *(char *)(unaff_gp_lo + -0x7edc);
        if (*(char *)(unaff_gp_lo + -0x77b0) != '\0') {
          lVar2 = rld_is_listed_symbol(*param_3 + *(int *)(param_1 + 0x60));
          if (lVar2 != 0) {
            debug(unaff_gp_lo + -0x5450,*param_4);
          }
          cVar6 = *(char *)(unaff_gp_lo + -0x7edc);
        }
        cVar4 = *(char *)(unaff_gp_lo + -0x77b0);
        if (cVar6 != '\0') {
          trace(unaff_gp_lo + -0x5430,*(undefined4 *)(param_1 + 0x18),
                *param_3 + *(int *)(param_1 + 0x60));
          cVar4 = *(char *)(unaff_gp_lo + -0x77b0);
        }
        cVar6 = *(char *)(unaff_gp_lo + -0x7eeb);
        if (cVar4 != '\0') {
          lVar2 = rld_is_listed_symbol(*param_3 + *(int *)(param_1 + 0x60));
          cVar6 = *(char *)(unaff_gp_lo + -0x7eeb);
          if (lVar2 != 0) {
            debug(unaff_gp_lo + -0x5408,*(undefined1 *)(unaff_gp_lo + -0x7f60),
                  *param_3 + *(int *)(param_1 + 0x60));
            cVar6 = *(char *)(unaff_gp_lo + -0x7eeb);
          }
        }
        if (cVar6 != '\0') {
          debug(unaff_gp_lo + -0x53b8,*(undefined1 *)(unaff_gp_lo + -0x7f60),
                *param_3 + *(int *)(param_1 + 0x60));
        }
        *(undefined1 *)(param_1 + 0xc5) = 1;
        lVar2 = 0;
        *(int *)(unaff_gp_lo + -0x7e54) = *(int *)(unaff_gp_lo + -0x7e54) + 1;
        goto code_r0x0fb664fc;
      }
      bVar9 = *(byte *)(param_3 + 3) & 0xf;
      if ((*(char *)(unaff_gp_lo + -0x77b0) != '\0') &&
         (lVar3 = rld_is_listed_symbol(*param_3 + *(int *)(param_1 + 0x60)), lVar3 != 0)) {
        debug(unaff_gp_lo + -0x5368,*(undefined1 *)(unaff_gp_lo + -0x7f60),
              *param_3 + *(int *)(param_1 + 0x60),bVar9);
      }
      cVar6 = *(char *)(unaff_gp_lo + -0x7f60);
      if (*(char *)(unaff_gp_lo + -0x7eeb) != '\0') {
        debug(unaff_gp_lo + -0x5368,*(undefined1 *)(unaff_gp_lo + -0x7f60),
              *param_3 + *(int *)(param_1 + 0x60),bVar9);
        cVar6 = *(char *)(unaff_gp_lo + -0x7f60);
      }
      cVar4 = *(char *)(unaff_gp_lo + -0x77b0);
      if ((cVar6 == '\0') ||
         (cVar4 = *(char *)(unaff_gp_lo + -0x77b0), (*(byte *)(param_3 + 3) & 0xf) != 2)) {
        cVar6 = *(char *)(unaff_gp_lo + -0x7eeb);
        if (cVar4 != '\0') {
          lVar3 = rld_is_listed_symbol(*param_3 + *(int *)(param_1 + 0x60));
          cVar6 = *(char *)(unaff_gp_lo + -0x7eeb);
          if (lVar3 != 0) {
            debug(unaff_gp_lo + -0x5320,*(undefined4 *)(param_1 + 0x18),
                  *param_3 + *(int *)(param_1 + 0x60));
            cVar6 = *(char *)(unaff_gp_lo + -0x7eeb);
          }
        }
        if (cVar6 != '\0') {
          debug(unaff_gp_lo + -0x5320,*(undefined4 *)(param_1 + 0x18),
                *param_3 + *(int *)(param_1 + 0x60));
        }
        *(undefined1 *)(param_1 + 0xc5) = 1;
        if ((*(char *)(unaff_gp_lo + -0x77b0) != '\0') &&
           (lVar3 = rld_is_listed_symbol(*param_3 + *(int *)(param_1 + 0x60)), lVar3 != 0)) {
          debug(unaff_gp_lo + -0x52f0,*param_4);
        }
        if (*(char *)(unaff_gp_lo + -0x7f48) == '\0') {
          error(unaff_gp_lo + -0x52d0,*(undefined4 *)(param_1 + 0x18),
                *param_3 + *(int *)(param_1 + 0x60));
          if (*(char *)(unaff_gp_lo + -0x7edc) != '\0') {
            trace(unaff_gp_lo + -0x52d0,*(undefined4 *)(param_1 + 0x18),
                  *param_3 + *(int *)(param_1 + 0x60));
          }
          *(char *)(unaff_gp_lo + -0x7ea9) = *(char *)(unaff_gp_lo + -0x7ea9) + '\x01';
          if ((*(char *)(unaff_gp_lo + -0x77b0) != '\0') &&
             (lVar3 = rld_is_listed_symbol(*param_3 + *(int *)(param_1 + 0x60)), lVar3 != 0)) {
            debug(unaff_gp_lo + -0x52b0,*(undefined1 *)(unaff_gp_lo + -0x7ea9));
          }
          if (*(char *)(unaff_gp_lo + -0x7eeb) != '\0') {
            debug(unaff_gp_lo + -0x52b0,*(undefined1 *)(unaff_gp_lo + -0x7ea9));
          }
        }
        else {
          fatal(unaff_gp_lo + -0x52d0);
        }
      }
      iVar8 = *(int *)(unaff_gp_lo + -0x7e54);
    }
    *(int *)(unaff_gp_lo + -0x7e54) = iVar8 + 1;
  }
  if (lVar2 == -1) {
    cVar6 = *(char *)(unaff_gp_lo + -0x7eeb);
    if (*(char *)(unaff_gp_lo + -0x77b0) != '\0') {
      lVar2 = rld_is_listed_symbol(*param_3 + *(int *)(param_1 + 0x60));
      cVar6 = *(char *)(unaff_gp_lo + -0x7eeb);
      if (lVar2 != 0) {
        debug(unaff_gp_lo + -0x5238,*param_3 + *(int *)(param_1 + 0x60),
              *(undefined4 *)(param_1 + 0x2c));
        cVar6 = *(char *)(unaff_gp_lo + -0x7eeb);
      }
    }
    if (cVar6 != '\0') {
      debug(unaff_gp_lo + -0x5238,*param_3 + *(int *)(param_1 + 0x60),
            *(undefined4 *)(param_1 + 0x2c));
    }
    *(undefined1 *)(param_1 + 0xc5) = 1;
    return;
  }
code_r0x0fb664fc:
  resolve_relocations(param_1,param_2,lVar2);
  cVar6 = *(char *)(unaff_gp_lo + -0x77b0);
  if (*(int *)(param_1 + 0x120) == 1) {
    obj_set_dynsym_got(param_1,param_2,lVar2);
    cVar6 = *(char *)(unaff_gp_lo + -0x77b0);
  }
  cVar4 = '\0';
  if (cVar6 != '\0') {
    lVar3 = rld_is_listed_symbol(*param_3 + *(int *)(param_1 + 0x60));
    cVar4 = *(char *)(unaff_gp_lo + -0x77b0);
    if (lVar3 != 0) {
      debug(unaff_gp_lo + -0x5280,*param_3 + *(int *)(param_1 + 0x60),*param_4);
      cVar4 = *(char *)(unaff_gp_lo + -0x77b0);
    }
  }
  cVar6 = *(char *)(unaff_gp_lo + -0x7eeb);
  if (cVar4 != '\0') {
    lVar3 = rld_is_listed_symbol(*param_3 + *(int *)(param_1 + 0x60));
    cVar6 = *(char *)(unaff_gp_lo + -0x7eeb);
    if (lVar3 != 0) {
      debug(unaff_gp_lo + -0x5268,*param_3 + *(int *)(param_1 + 0x60),
            *(undefined4 *)(param_1 + 0x2c),lVar2);
      cVar6 = *(char *)(unaff_gp_lo + -0x7eeb);
    }
  }
  if (cVar6 != '\0') {
    debug(unaff_gp_lo + -0x5268,*param_3 + *(int *)(param_1 + 0x60),*(undefined4 *)(param_1 + 0x2c),
          lVar2);
  }
  return;
}



// === resolve_symbol @ 0fb66b90 (1788 bytes) ===

/* WARNING: Instruction at (ram,0x0fb66fcc) overlaps instruction at (ram,0x0fb66fc8)
    */

undefined *
resolve_symbol(undefined8 param_1,undefined8 param_2,undefined *param_3,longlong param_4,
              longlong param_5,longlong param_6,undefined4 *param_7)

{
  short sVar1;
  longlong lVar2;
  longlong lVar3;
  int iVar4;
  char *pcVar5;
  int iVar6;
  undefined *puVar7;
  undefined **ppuVar8;
  char *__s;
  int aiStack_80 [2];
  longlong lStack_78;
  longlong lStack_70;
  longlong lStack_68;
  
  aiStack_80[0] = 0;
  if (ltr_extra != '\0') {
    if (debug_trace != '\0') {
      trace("resolve_symbol:  %s, from %s hash value 0x%lx \n",param_1,param_2,param_3);
    }
    if ((rld_has_listed_symbols != '\0') && (lVar2 = rld_is_listed_symbol(param_1), lVar2 != 0)) {
      debug("resolve_symbol:  resolving %s from %s, with hash value 0x%lx\n",param_1,param_2,param_3
           );
    }
    if (debug_symbol != '\0') {
      debug("resolve_symbol:  resolving %s from %s, with hash value 0x%lx\n",param_1,param_2,param_3
           );
    }
  }
  if (rld_new_interface_hash_val == param_3) {
    iVar4 = strcmp((char *)param_1,_rld_special_funcs);
    if (iVar4 == 0) {
      return PTR__rld_new_interface_0fbde2e0;
    }
  }
  lVar2 = -1;
  if (param_5 == 0) {
LAB_0fb66c98:
    iVar4 = 8;
    if (old_interface == '\0') {
      iVar4 = 1;
    }
    ppuVar8 = &_rld_special_funcs;
    do {
      if ((ppuVar8[1] == param_3) && (iVar6 = strcmp((char *)param_1,*ppuVar8), iVar6 == 0)) {
        if (ltr_extra != '\0') {
          if ((rld_has_listed_symbols != '\0') &&
             (lVar2 = rld_is_listed_symbol(param_1), lVar2 != 0)) {
            debug("resolve symbol: resolved %s from rld, this is rld interface function \"%s\"\n",
                  param_1,*ppuVar8);
          }
          if (debug_symbol != '\0') {
            debug("resolve symbol: resolved %s from rld, this is rld interface function \"%s\"\n",
                  param_1,*ppuVar8);
          }
          if (interface != '\0') {
            trace("resolve symbol: resolved %s , this is rld interface function \"%s\" with addr 0x%lx\n"
                  ,param_1,*ppuVar8,ppuVar8[2]);
          }
        }
        return ppuVar8[2];
      }
      ppuVar8 = ppuVar8 + 3;
    } while (ppuVar8 != &_rld_special_funcs + iVar4 * 3);
    if (lVar2 != -1) goto LAB_0fb66d8c;
    lVar2 = FUN_0fb67290(param_1,aiStack_80,param_4,pObj_Head,param_7);
  }
  else {
    iVar4 = (int)param_5;
    if ((*(byte *)(iVar4 + 0xc9) == 0) &&
       (*(char *)(*(int *)(iVar4 + 100) + (int)param_6 * 0x10 + 0xd) != '\x03')) {
      lVar2 = -1;
      goto LAB_0fb66c98;
    }
    if ((*(byte *)(iVar4 + 0xc9) & 2) != 0) {
      lStack_68 = get_symlib_table_entry(param_5,param_6);
      if ((rld_has_listed_symbols != '\0') && (lVar2 = rld_is_listed_symbol(param_1), lVar2 != 0)) {
        debug("resolve_symbol: shared object %s is defined with DT_IRIX_DIRECT, look for symbol %s\n"
              ,param_2,param_1);
      }
      if (lStack_68 == 0) {
        lVar2 = param_5;
        if ((rld_has_listed_symbols != '\0') && (lVar3 = rld_is_listed_symbol(param_1), lVar3 != 0))
        {
          debug("resolve_symbol: so %s ref %s Bdirect,  resolve to liblist entry from myself.\n",
                param_2,param_1);
        }
      }
      else {
        lVar2 = (longlong)*(int *)(*(int *)(iVar4 + 0xb4) + (int)lStack_68 * 4 + -4);
        if ((rld_has_listed_symbols != '\0') && (lVar3 = rld_is_listed_symbol(param_1), lVar3 != 0))
        {
          debug("resolve_symbol: so %s ref %s Bdirect,  resolve to liblist entry o_llmap[%d] 0x%lx\n"
                ,param_2,param_1,(int)lStack_68 + -1,lVar2);
        }
      }
      if (lVar2 == 0) {
        __s = (char *)(*(int *)(*(int *)(iVar4 + 0x74) + (int)lStack_68 * 0x14 + -0x14) +
                      *(int *)(iVar4 + 0x60));
        pcVar5 = strchr(__s,0x2f);
        lVar2 = locate_obj_by_name(__s,pcVar5 != (char *)0x0);
        if (lVar2 != -1) {
          if (rld_has_listed_symbols == '\0') {
            iVar6 = (int)lStack_68 << 2;
          }
          else {
            lVar3 = rld_is_listed_symbol(param_1);
            iVar6 = (int)lStack_68 * 4;
            if (lVar3 != 0) {
              lStack_78 = (longlong)iVar6;
              debug("resolve_symbol: so %s ref %s Bdirect,  update llmap entry o_llval[%d] from 0x%lx to 0x%lx\n"
                    ,param_2,param_1,(int)lStack_68 + -1,
                    *(undefined4 *)(*(int *)(iVar4 + 0xb4) + iVar6 + -4),lVar2);
              iVar6 = (int)lStack_78;
            }
          }
          *(int *)(*(int *)(iVar4 + 0xb4) + iVar6 + -4) = (int)lVar2;
          goto LAB_0fb66eec;
        }
      }
      else {
LAB_0fb66eec:
        lStack_70 = lVar2;
        if (lVar2 != 0) {
          lVar2 = find_symbol_in_object(lVar2,param_1,param_3,is_global_strong_symbol,param_7);
          if (lVar2 == -1) {
            if ((rld_has_listed_symbols != '\0') &&
               (lVar3 = rld_is_listed_symbol(param_1), lVar3 != 0)) {
              debug("resolve_symbol: so %s ref %s not found Bdirect,  not resolved Bdirect.\n",
                    param_2,param_1);
            }
          }
          else {
            aiStack_80[0] = (int)lStack_70;
            if ((rld_has_listed_symbols != '\0') &&
               (lVar3 = rld_is_listed_symbol(param_1), lVar3 != 0)) {
              debug("resolve_symbol: so %s ref %s FOUND Bdirect,  resolved \n",param_2,param_1);
            }
          }
          goto LAB_0fb66fc8;
        }
      }
      lVar2 = -1;
      goto LAB_0fb66c98;
    }
    if ((rld_has_listed_symbols != '\0') && (lVar2 = rld_is_listed_symbol(param_1), lVar2 != 0)) {
      debug("resolve_symbol: shared object %s is defined with DT_SYMBOLIC or symbol %s has STO_PROTECTED on\n"
            ,param_2,param_1);
    }
    if (debug_symbol != '\0') {
      debug("resolve_symbol: shared object %s is defined with DT_SYMBOLIC or symbol %s has STO_PROTECTED on\n"
            ,param_2,param_1);
    }
    if (param_4 == 0) {
      lVar2 = find_symbol_in_object(param_5,param_1,param_3,is_global_strong_symbol,param_7);
    }
    else {
      lVar2 = find_symbol_in_object(param_5,param_1,param_3,&LAB_0fb677b0,param_7);
    }
    aiStack_80[0] = iVar4;
    if ((lVar2 == -1) && (*(char *)(*(int *)(iVar4 + 100) + (int)param_6 * 0x10 + 0xd) == '\x03')) {
      if ((rld_has_listed_symbols != '\0') &&
         (lVar3 = rld_is_listed_symbol(param_1,param_1), lVar3 != 0)) {
        debug("sto protected undef!: %s\n",*param_7);
      }
      fatal("%s has STO_PROTECTED on but is not defined in its shared obj %s\n",param_1,
            *(undefined4 *)(iVar4 + 0x18));
    }
LAB_0fb66fc8:
    if (lVar2 == -1) goto LAB_0fb66c98;
  }
  if (lVar2 == -1) {
    if (debug_trace != '\0') {
      trace("resolve_symbol:  %s, BAD INDEX gets BAD_ADDR\n",param_1);
    }
    return (undefined *)0xffffffff;
  }
LAB_0fb66d8c:
  iVar4 = aiStack_80[0];
  iVar6 = *(int *)(aiStack_80[0] + 100) + (int)lVar2 * 0x10;
  sVar1 = *(short *)(iVar6 + 0xe);
  if ((int)(uint)*(byte *)(iVar6 + 0xc) >> 4 == 2) {
    puVar7 = *(undefined **)(iVar6 + 4);
    if (sVar1 != -0xf) {
      puVar7 = puVar7 + -(*(int *)(aiStack_80[0] + 0x3c) - *(int *)(aiStack_80[0] + 0x10));
    }
  }
  else if (((sVar1 == -0x100) && (param_5 != 0)) && (param_6 != 0)) {
    puVar7 = (undefined *)common_handling(param_5,param_6,param_7);
  }
  else {
    puVar7 = *(undefined **)(iVar6 + 4);
    if (sVar1 != -0xf) {
      puVar7 = puVar7 + -(*(int *)(aiStack_80[0] + 0x3c) - *(int *)(aiStack_80[0] + 0x10));
    }
  }
  if (ltr_extra != '\0') {
    if (rld_has_listed_symbols != '\0') {
      lVar2 = rld_is_listed_symbol(param_1);
      if (lVar2 != 0) {
        debug("resolve symbol: resolved %s from %s\n",param_1,*(undefined4 *)(iVar4 + 0x18));
      }
      if ((rld_has_listed_symbols != '\0') && (lVar2 = rld_is_listed_symbol(param_1), lVar2 != 0)) {
        debug("resolved reasons: %s\n",*param_7);
      }
    }
    if (debug_symbol != '\0') {
      debug("resolve symbol: resolved %s from %s\n",param_1,*(undefined4 *)(iVar4 + 0x18));
    }
    if (user_tracking != '\0') {
      track("found %s in %s for %s at 0x%lx.\n",param_1,*(undefined4 *)(iVar4 + 0x18),param_2,puVar7
           );
    }
  }
  return puVar7;
}



// === FUN_0fb67290 @ 0fb67290 (1300 bytes) ===

/* WARNING: Instruction at (ram,0x0fb67330) overlaps instruction at (ram,0x0fb6732c)
    */

void FUN_0fb67290(undefined8 param_1,int *param_2,longlong param_3,int param_4,undefined8 param_5)

{
  byte bVar1;
  char cVar2;
  short sVar3;
  char *pcVar4;
  uint *puVar5;
  uint uVar6;
  uint uVar7;
  uint uVar9;
  size_t sVar10;
  undefined8 uVar8;
  int iVar11;
  uint uVar12;
  size_t sVar13;
  int *piVar14;
  uint uVar15;
  char *__s;
  code *UNRECOVERED_JUMPTABLE;
  int iStack_80;
  
  uVar9 = elfhash();
  if (param_4 != 0) {
    uVar15 = 0xffffffff;
    do {
      if (rld_has_listed_symbols != '\0') {
        pcVar4 = *(char **)(param_4 + 0x18);
        sVar10 = strlen(pcVar4);
        uVar8 = FUN_0fb656b0(sVar10 + 300);
        rld_snprintf(uVar8,sVar10 + 300,"now search %s\n",pcVar4);
        add_a_tried_path(param_5,uVar8);
      }
      cVar2 = *(char *)(param_4 + 0xca);
      if (((cVar2 == '\0') || (override_dso_hidden_flag != '\0')) ||
         (*(char *)(param_4 + 0xd8) != '\0')) {
        if (debug_hash != '\0') {
          debug("general_find_symbol: searching for %s in object %s hidden? %d override_hidden? %d temp_unhidden %d\n"
                ,param_1,*(undefined4 *)(param_4 + 0x2c),cVar2,override_dso_hidden_flag,
                *(undefined1 *)(param_4 + 0xd8));
        }
        puVar5 = *(uint **)(param_4 + 0x5c);
        uVar6 = *puVar5;
        if ((*(uint *)(param_4 + 0x84) & 2) == 0) {
          uVar7 = puVar5[(uVar9 & uVar6 - 1) + 2];
        }
        else {
          if (uVar6 == 0) {
            trap(7);
          }
          uVar7 = puVar5[uVar9 % uVar6 + 2];
        }
        if (uVar7 != 0) {
          iVar11 = *(int *)(param_4 + 0x68);
          do {
            uVar12 = *(uint *)(iVar11 + uVar7 * 8);
            if (uVar12 == 0) {
              uVar12 = get_dynsym_hash_value(param_4,uVar7);
            }
            if (uVar12 == uVar9) {
              piVar14 = (int *)(*(int *)(param_4 + 100) + uVar7 * 0x10);
              iVar11 = strcmp((char *)param_1,(char *)(*piVar14 + *(int *)(param_4 + 0x60)));
              if (iVar11 == 0) {
                sVar3 = *(short *)((int)piVar14 + 0xe);
                bVar1 = *(byte *)(piVar14 + 3);
                if (((sVar3 == 0) || (sVar3 == -0xe)) ||
                   ((param_3 != 0 && ((sVar3 != -0xff && ((bVar1 & 0xf) != 2)))))) {
                  uVar8 = 1;
                  if (debug_hash != '\0') {
                    if ((sVar3 == -0xff) || ((bVar1 & 0xf) == 2)) {
                      uVar8 = 0;
                    }
                    debug("general_find_symbol ignores this symbol in this object: symbol UNDEF? %d COMMON? %d lazy_text? %ld  not func? %d\n"
                          ,sVar3 == 0,sVar3 == -0xe,param_3,uVar8);
                  }
                }
                else {
                  if ((int)(uint)bVar1 >> 4 != 2) {
                    if (debug_hash != '\0') {
                      debug("general_find_symbol: found symbol: index = 0x%lx\n",uVar7);
                    }
                    *param_2 = param_4;
                    /* WARNING: Could not recover jumptable at 0x0fb67794. Too many branches */
                    /* WARNING: Treating indirect jump as call */
                    (*UNRECOVERED_JUMPTABLE)();
                    return;
                  }
                  if (debug_hash != '\0') {
                    debug("general_find_symbol: found weak symbol\n");
                  }
                  if ((uVar15 == 0xffffffff) &&
                     (uVar15 = uVar7, iStack_80 = param_4, rld_has_listed_symbols != '\0')) {
                    pcVar4 = *(char **)(param_4 + 0x18);
                    sVar10 = strlen(pcVar4);
                    __s = (char *)(*piVar14 + *(int *)(param_4 + 0x60));
                    sVar13 = strlen(__s);
                    iVar11 = sVar13 + sVar10 + 300;
                    uVar8 = FUN_0fb656b0(iVar11);
                    rld_snprintf(uVar8,iVar11,"found first weak instance of %s in %s\n",__s,pcVar4);
                    add_a_tried_path(param_5,uVar8);
                  }
                }
                break;
              }
              if (debug_hash != '\0') {
                debug("general_find_symbol: string match failed: %s : %s\n",param_1,
                      *piVar14 + *(int *)(param_4 + 0x60));
              }
              uVar7 = puVar5[uVar6 + uVar7 + 2];
            }
            else {
              if (debug_hash != '\0') {
                iVar11 = *(int *)(*(int *)(param_4 + 0x68) + uVar7 * 8);
                if (iVar11 == 0) {
                  iVar11 = get_dynsym_hash_value(param_4,uVar7);
                }
                debug("general_find_symbol: hash match failed: 0x%lx : 0x%lx\n",iVar11,uVar9);
              }
              uVar7 = puVar5[uVar6 + uVar7 + 2];
            }
            if (uVar7 == 0) break;
            iVar11 = *(int *)(param_4 + 0x68);
          } while( true );
        }
        param_4 = *(int *)(param_4 + 8);
        if (rld_has_listed_symbols != '\0') {
          if (param_4 == 0) {
            add_a_tried_path(param_5,"Next object to search returns null curobj\n");
          }
          else {
            pcVar4 = *(char **)(param_4 + 0x18);
            sVar10 = strlen(pcVar4);
            uVar8 = FUN_0fb656b0(sVar10 + 300);
            rld_snprintf(uVar8,sVar10 + 300,"Next object to search is %s\n",pcVar4);
            add_a_tried_path(param_5,uVar8);
          }
        }
      }
      else {
        if (debug_hash != '\0') {
          debug("ignoring object %s in search: o_is_hidden %d override_dso_hidden_flag %d temp_unhid %d\n"
                ,*(undefined4 *)(param_4 + 0x2c),cVar2);
        }
        param_4 = *(int *)(param_4 + 8);
        if (rld_has_listed_symbols != '\0') {
          if (param_4 == 0) {
            add_a_tried_path(param_5,"next object to search returns null curobj\n");
          }
          else {
            pcVar4 = *(char **)(param_4 + 0x18);
            sVar10 = strlen(pcVar4);
            uVar8 = FUN_0fb656b0(sVar10 + 300);
            rld_snprintf(uVar8,sVar10 + 300,"next object to search is %s\n",pcVar4);
            add_a_tried_path(param_5,uVar8);
          }
        }
      }
    } while (param_4 != 0);
    if (uVar15 != 0xffffffff) {
      *param_2 = iStack_80;
                    /* WARNING: Could not recover jumptable at 0x0fb67658. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (*UNRECOVERED_JUMPTABLE)();
      return;
    }
  }
  *param_2 = 0;
                    /* WARNING: Could not recover jumptable at 0x0fb67694. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
}



// === is_weak_or_global_symbol @ 0fb67800 (48 bytes) ===

undefined8 is_weak_or_global_symbol(int param_1,int param_2)

{
  short sVar1;
  
  sVar1 = *(short *)(*(int *)(param_1 + 100) + param_2 * 0x10 + 0xe);
  if ((sVar1 != 0) && (sVar1 != -0xe)) {
    return 1;
  }
  return 0;
}



// === is_global_strong_symbol @ 0fb67830 (72 bytes) ===

undefined8 is_global_strong_symbol(int param_1,int param_2)

{
  short sVar1;
  int iVar2;
  
  iVar2 = *(int *)(param_1 + 100) + param_2 * 0x10;
  if (*(byte *)(iVar2 + 0xc) >> 4 == 1) {
    sVar1 = *(short *)(iVar2 + 0xe);
    if ((sVar1 != 0) && (sVar1 != -0xe)) {
      return 1;
    }
  }
  return 0;
}



// === is_symbol_in_got @ 0fb67890 (64 bytes) ===

undefined8 is_symbol_in_got(int param_1,uint param_2)

{
  if ((*(byte *)(*(int *)(param_1 + 100) + param_2 * 0x10 + 0xc) >> 4 == 1) &&
     (*(uint *)(param_1 + 0xf4) <= param_2)) {
    return 1;
  }
  return 0;
}



// === find_symbol_in_object @ 0fb678d0 (756 bytes) ===

uint find_symbol_in_object
               (int param_1,undefined8 param_2,uint param_3,code *param_4,undefined8 param_5)

{
  uint *puVar1;
  uint uVar2;
  uint uVar3;
  char *__s;
  int iVar5;
  uint uVar6;
  size_t sVar7;
  longlong lVar4;
  undefined1 auStack_90 [8];
  
  if (debug_hash != '\0') {
    debug("find_symbol_in_object: searching for %s in %s\n",param_2,*(undefined4 *)(param_1 + 0x18))
    ;
  }
  if (((*(char *)(param_1 + 0xca) != '\0') && (override_dso_hidden_flag == '\0')) &&
     (*(char *)(param_1 + 0xd8) == '\0')) {
    if (rld_has_listed_symbols != '\0') {
      __s = *(char **)(param_1 + 0x18);
      sVar7 = strlen(__s);
      iVar5 = -(sVar7 + 0x13b & 0xfffffff0);
      rld_snprintf(auStack_90 + iVar5,sVar7 + 300,
                   "ignoring %s because hidden. hidden %d override flag %d temp_unhidden %d \n",__s,
                   *(undefined1 *)(param_1 + 0xca),override_dso_hidden_flag,
                   *(undefined1 *)(param_1 + 0xd8));
      add_a_tried_path(param_5,auStack_90 + iVar5);
    }
    return 0xffffffff;
  }
  puVar1 = *(uint **)(param_1 + 0x5c);
  uVar2 = *puVar1;
  if ((*(uint *)(param_1 + 0x84) & 2) == 0) {
    uVar3 = puVar1[(param_3 & uVar2 - 1) + 2];
  }
  else {
    if (uVar2 == 0) {
      trap(7);
    }
    uVar3 = puVar1[param_3 % uVar2 + 2];
  }
  if (uVar3 != 0) {
    iVar5 = *(int *)(param_1 + 0x68);
    while( true ) {
      uVar6 = *(uint *)(iVar5 + uVar3 * 8);
      if (uVar6 == 0) {
        uVar6 = get_dynsym_hash_value(param_1,uVar3);
      }
      if (uVar6 == param_3) {
        iVar5 = strcmp((char *)param_2,
                       (char *)(*(int *)(*(int *)(param_1 + 100) + uVar3 * 0x10) +
                               *(int *)(param_1 + 0x60)));
        if (iVar5 == 0) {
          if (debug_hash != '\0') {
            debug("find_symbol_in_object: string match succeeded: %s : %s\n",param_2,
                  *(int *)(*(int *)(param_1 + 100) + uVar3 * 0x10) + *(int *)(param_1 + 0x60));
          }
          lVar4 = (*param_4)(param_1,uVar3);
          if (lVar4 == 0) {
            if (debug_hash == '\0') {
              return 0xffffffff;
            }
            debug("find_symbol_in_object: symbol %ld was invalid\n",uVar3);
            return 0xffffffff;
          }
          if (debug_hash != '\0') {
            debug("find_symbol_in_object: returning index 0x%lx\n",uVar3);
          }
          return uVar3;
        }
        uVar3 = puVar1[uVar2 + uVar3 + 2];
      }
      else {
        if (debug_hash != '\0') {
          iVar5 = *(int *)(*(int *)(param_1 + 0x68) + uVar3 * 8);
          if (iVar5 == 0) {
            iVar5 = get_dynsym_hash_value(param_1,uVar3);
          }
          debug("find_symbol_in_object: hash match failed: 0x%lx : 0x%lx\n",iVar5,param_3);
        }
        uVar3 = puVar1[uVar2 + uVar3 + 2];
      }
      if (uVar3 == 0) break;
      iVar5 = *(int *)(param_1 + 0x68);
    }
  }
  if (debug_hash != '\0') {
    debug("find_symbol_in_object: symbol not found: %s\n",param_2);
  }
  return 0xffffffff;
}



// === handle_undefineds @ 0fb67bd0 (444 bytes) ===

undefined8 handle_undefineds(int param_1,ulonglong param_2)

{
  if (debug_trace != '\0') {
    trace("handle_undefineds: o 0x%lx, bind %ld name %s already done? %d\n",param_1,param_2,
          *(undefined4 *)(param_1 + 0x18),*(undefined1 *)(param_1 + 0xc4));
  }
  if (*(char *)(param_1 + 0xc4) != '\0') {
    return 0xffffffffffffffff;
  }
  *(undefined1 *)(param_1 + 0xc5) = 0;
  *(undefined1 *)(param_1 + 0xc4) = 1;
  if (*(code **)**(undefined4 **)(param_1 + 0x6c) != _rld_text_resolve) {
    *(code **)**(undefined4 **)(param_1 + 0x6c) = _rld_text_resolve;
  }
  param_2 = *(byte *)(param_1 + 0xe1) | param_2;
  if (debug_symbol != '\0') {
    debug("handle_undefineds: %s being resolved\n",*(undefined4 *)(param_1 + 0x18));
  }
  if ((*(char *)(param_1 + 0xc2) == '\0') || (*(char *)(param_1 + 200) != '\0')) {
    if ((force_moved == '\0') || (pObj_Head == param_1)) goto LAB_0fb67cb4;
LAB_0fb67c98:
    if (verbose != '\0') {
      trace("obj %s has been forced to be MOVED\n",*(undefined4 *)(param_1 + 0x18));
    }
  }
  else if (force_moved != '\0') goto LAB_0fb67c98;
  FUN_0fb68ef0(param_1);
LAB_0fb67cb4:
  if (((*(char *)(param_1 + 0xc6) == '\0') && (*(char *)(param_1 + 199) == '\0')) &&
     (force_checksum == '\0')) {
    if (((all_fully_quickstarted == '\0') || ((*(uint *)(param_1 + 0x84) & 1) == 0)) ||
       ((force_moved != '\0' || (force_timestamp != '\0')))) {
      FUN_0fb69760(param_1,0,param_2);
    }
    else if (param_2 == 0) {
      if ((past_startup != 0) || ((*(uint *)(param_1 + 0x84) & 0x2000) == 0)) {
        FUN_0fb69560(param_1);
      }
    }
    else {
      FUN_0fb69760(param_1,1,param_2);
    }
  }
  else {
    FUN_0fb69a50(param_1,param_2);
  }
  return 0xffffffffffffffff;
}



// === handle_conflicts @ 0fb67d90 (72 bytes) ===

/* WARNING: Instruction at (ram,0x0fb67db0) overlaps instruction at (ram,0x0fb67dac)
    */

undefined8 handle_conflicts(int param_1)

{
  if (((*(char *)(param_1 + 0xc6) == '\0') && (*(char *)(param_1 + 199) == '\0')) &&
     (force_checksum == '\0')) {
    FUN_0fb66100();
    return 0xffffffffffffffff;
  }
  return 0xffffffffffffffff;
}



// === resolve_relocations @ 0fb67de0 (1188 bytes) ===

/* WARNING: Instruction at (ram,0x0fb6805c) overlaps instruction at (ram,0x0fb68058)
    */

void resolve_relocations(int param_1,uint param_2,int param_3)

{
  char cVar1;
  longlong lVar2;
  int *piVar3;
  uint uVar4;
  int iVar5;
  int *piVar6;
  int iVar7;
  uint uVar8;
  int iVar9;
  int iVar10;
  
  if (*(int *)(param_1 + 0x70) != 0) {
    if (debug_trace != '\0') {
      trace("resolve_relocations: o 0x%lx, name %s, sym index %ld, addr 0x%lx\n",param_1,
            *(undefined4 *)(param_1 + 0x18),param_2,param_3);
    }
    piVar6 = (int *)(*(int *)(param_1 + 100) + param_2 * 0x10);
    iVar7 = param_2 * 8;
    if (*(short *)((int)piVar6 + 0xe) == -0xf) {
      iVar10 = 0;
    }
    else {
      iVar10 = *(int *)(param_1 + 0x3c) - *(int *)(param_1 + 0x10);
    }
    uVar8 = *(uint *)(*(int *)(param_1 + 0x68) + iVar7 + 4);
    if ((*(char *)(param_1 + 0xc1) != '\0') && ((uVar8 & 0xff) == 0)) {
      iVar9 = find_reloc(param_1,param_2);
      *(int *)(*(int *)(param_1 + 0x68) + iVar7 + 4) = iVar9 * 0x100 + 1;
      uVar8 = *(uint *)(*(int *)(param_1 + 0x68) + iVar7 + 4);
    }
    uVar8 = uVar8 >> 8;
    if (debug_symbol != '\0') {
      debug("obj_rel_sym(%s, %ld) = %ld (sym = %ld)\n",*(undefined4 *)(param_1 + 0x18),uVar8,
            *(uint *)(*(int *)(param_1 + 0x70) + uVar8 * 8 + 4) >> 8,param_2);
    }
    iVar7 = obj_dynsym_got(param_1,param_2);
    if ((iVar7 == 0) ||
       ((*(char *)((int)piVar6 + 0xd) != '\0' && (*(char *)((int)piVar6 + 0xd) != '\x04')))) {
      iVar7 = piVar6[1];
    }
    if (debug_symbol != '\0') {
      debug("resolve_relocations: sym_value = 0x%lx\n",iVar7);
    }
    if ((rld_has_listed_symbols != '\0') &&
       (lVar2 = rld_is_listed_symbol(*piVar6 + *(int *)(param_1 + 0x60)), lVar2 != 0)) {
      debug("resolve_relocations: sym_value = 0x%lx\n",iVar7);
    }
    iVar9 = uVar8 * 8;
    if (uVar8 < *(uint *)(param_1 + 0xa8) >> 3) {
      uVar4 = *(uint *)(*(int *)(param_1 + 0x70) + iVar9 + 4);
      while (param_2 == uVar4 >> 8) {
        if ((rld_has_listed_symbols != '\0') &&
           (lVar2 = rld_is_listed_symbol(*piVar6 + *(int *)(param_1 + 0x60)), lVar2 != 0)) {
          debug("resolve_relocations: sym %s, relocation index %ld, o_index_first_dynsym_with_got %ld\n"
                ,*piVar6 + *(int *)(param_1 + 0x60),uVar8,*(undefined4 *)(param_1 + 0xf4));
        }
        if (debug_symbol != '\0') {
          debug("resolve_relocations: sym %s, relocation index %ld, o_index_first_dynsym_with_got %ld\n"
                ,*piVar6 + *(int *)(param_1 + 0x60),uVar8,*(undefined4 *)(param_1 + 0xf4));
        }
        piVar3 = (int *)(*(int *)(param_1 + 0x70) + iVar9);
        cVar1 = *(char *)((int)piVar3 + 7);
        piVar3 = (int *)(*piVar3 - iVar10);
        if (cVar1 == '\x03') {
          if (param_3 == iVar7) {
            if ((rld_has_listed_symbols != '\0') &&
               (lVar2 = rld_is_listed_symbol(*piVar6 + *(int *)(param_1 + 0x60)), lVar2 != 0)) {
              debug("Resolve relocations: R_MIPS_REL32 skipped because raddr == sym_value\n");
            }
            if (debug_symbol == '\0') goto LAB_0fb67f60;
            debug("Resolve relocations: R_MIPS_REL32 skipped because raddr == sym_value\n");
            uVar4 = *(uint *)(param_1 + 0xa8);
          }
          else {
            iVar5 = *piVar3;
            if ((rld_has_listed_symbols != '\0') &&
               (lVar2 = rld_is_listed_symbol(*piVar6 + *(int *)(param_1 + 0x60)), lVar2 != 0)) {
              debug("Resolve relocations: R_MIPS_REL32 moved_loc (0x%lx) contains 0x%lx\n",piVar3,
                    iVar5);
            }
            if (debug_symbol != '\0') {
              debug("Resolve relocations: R_MIPS_REL32 moved_loc (0x%lx) contains 0x%lx\n",piVar3,
                    iVar5);
            }
            iVar5 = iVar5 - iVar7;
            if ((rld_has_listed_symbols != '\0') &&
               (lVar2 = rld_is_listed_symbol(*piVar6 + *(int *)(param_1 + 0x60)), lVar2 != 0)) {
              debug("resolve_relocations: tmp (moved_loc - 0x%lx) = 0x%lx\n",iVar7,iVar5);
            }
            if (debug_symbol != '\0') {
              debug("resolve_relocations: tmp (moved_loc - 0x%lx) = 0x%lx\n",iVar7,iVar5);
            }
            iVar5 = param_3 + iVar5;
            *piVar3 = iVar5;
            if ((rld_has_listed_symbols != '\0') &&
               (lVar2 = rld_is_listed_symbol(*piVar6 + *(int *)(param_1 + 0x60)), lVar2 != 0)) {
              debug("resolve_relocations: *moved_loc now = 0x%lx\n",iVar5);
            }
            if (debug_symbol == '\0') goto LAB_0fb67f60;
            debug("resolve_relocations: *moved_loc now = 0x%lx\n",iVar5);
            uVar4 = *(uint *)(param_1 + 0xa8);
          }
        }
        else {
          if (cVar1 == '$') {
            if ((*(char *)((int)piVar6 + 0xd) == '\0') || (*(char *)((int)piVar6 + 0xd) == '\x04'))
            {
              if ((rld_has_listed_symbols != '\0') &&
                 (lVar2 = rld_is_listed_symbol(*piVar6 + *(int *)(param_1 + 0x60)), lVar2 != 0)) {
                debug("resolve_relocations: R_MIPS_RELGOT moved_loc 0x%lx oldval 0x%lx new 0x%lx\n",
                      piVar3,*piVar3,param_3);
              }
              if (*piVar3 == param_3) {
                if ((rld_has_listed_symbols != '\0') &&
                   (lVar2 = rld_is_listed_symbol(*piVar6 + *(int *)(param_1 + 0x60)), lVar2 != 0)) {
                  debug(
                       "resolve_relocations: R_MIPS_RELGOT relocation skipped because value at moved_loc is unchanged\n"
                       );
                  uVar4 = *(uint *)(param_1 + 0xa8);
                  goto LAB_0fb67f64;
                }
              }
              else {
                *piVar3 = param_3;
              }
            }
          }
          else {
            fatal("Encountered unsupported relocation in .rel.dyn of %s\n",
                  *(undefined4 *)(param_1 + 0x30));
          }
LAB_0fb67f60:
          uVar4 = *(uint *)(param_1 + 0xa8);
        }
LAB_0fb67f64:
        uVar8 = uVar8 + 1;
        iVar9 = iVar9 + 8;
        if (uVar4 >> 3 <= uVar8) {
          return;
        }
        uVar4 = *(uint *)(*(int *)(param_1 + 0x70) + iVar9 + 4);
      }
    }
  }
  return;
}



// === generate_global_lib_search_path_from_obj @ 0fb68290 (316 bytes) ===

char * generate_global_lib_search_path_from_obj
                 (undefined8 param_1,longlong param_2,undefined1 *param_3)

{
  char cVar1;
  size_t sVar2;
  char *pcVar3;
  size_t sVar4;
  char *__dest;
  char *pcVar5;
  
  pcVar3 = (char *)param_1;
  if (param_2 != 0) {
    sVar2 = strlen((char *)param_2);
    sVar4 = strlen(pcVar3);
    __dest = (char *)malloc();
    if (__dest == (char *)0x0) {
      fatal("cannot malloc for dirpath");
    }
    memset(__dest,0,sVar4 + sVar2 + 3);
    strcpy(__dest,(char *)param_2);
    cVar1 = DAT_0fbd5761;
    pcVar5 = __dest + sVar2;
    *pcVar5 = DAT_0fbd5760;
    pcVar5[1] = cVar1;
    strcpy(pcVar5 + 1,pcVar3);
    free(param_2);
    return __dest;
  }
  sVar2 = strlen(pcVar3);
  pcVar3 = (char *)malloc(sVar2 + 1);
  if (pcVar3 == (char *)0x0) {
    fatal("cannot malloc for dirpath");
  }
  __rld_strlcpy(pcVar3,param_1,sVar2 + 1);
  *param_3 = 1;
  return pcVar3;
}



// === apply_oex_flags @ 0fb683d0 (64 bytes) ===

undefined8 apply_oex_flags(int param_1)

{
  uint uVar1;
  
  uVar1 = *(uint *)(param_1 + 0x80) | oex_global;
  oex_global = uVar1 & 0x60000 |
               uVar1 & 0x90000 | uVar1 & 0x1f | *(uint *)(param_1 + 0x80) & oex_global & 0x1f00;
  return 0xffffffffffffffff;
}



// === update_oex_state @ 0fb68410 (1120 bytes) ===

/* WARNING: Instruction at (ram,0x0fb684c8) overlaps instruction at (ram,0x0fb684c4)
    */

void update_oex_state(undefined1 *param_1,ulonglong param_2)

{
  uint uVar1;
  uint uVar5;
  ulonglong uVar2;
  ulonglong uVar3;
  int iVar6;
  longlong lVar4;
  char *pcVar7;
  undefined1 *puVar8;
  ulonglong uVar9;
  ulonglong uVar10;
  undefined4 uStack_90;
  undefined4 uStack_8c;
  undefined4 uStack_88;
  undefined4 uStack_84;
  undefined4 uStack_80;
  size_t sStack_7c;
  undefined4 uStack_78;
  undefined4 uStack_74;
  longlong lStack_70;
  longlong lStack_68;
  ulonglong uStack_60;
  longlong lStack_58;
  longlong lStack_50;
  longlong lStack_48;
  ulonglong uStack_40;
  ulonglong uStack_38;
  
  uVar9 = (ulonglong)(int)oex_global;
  uVar10 = uVar9 & 0x1f;
  uVar5 = (oex_global & 0x1f00) >> 8;
  uStack_38 = (ulonglong)(int)uVar5;
  uVar1 = (uint)param_2;
  if (verbose != '\0') {
    puVar8 = &DAT_0fbd5820;
    if (param_1 != (undefined1 *)0x0) {
      puVar8 = param_1;
    }
    trace("update_oex_state on 0x%lx %s. user_touched %d oex_gl 0x%x  prev oex 0x%x have_set smm %d fp_prec %d spec-exec %d  oex_min,max 0x%x 0x%x prev_oex_min,max 0x%x 0x%x\n"
          ,param_1,puVar8,user_touched_fp_csr,uVar9,param_2,have_set_smm,have_set_fp_precise,
          have_set_speculative_exec,(int)uVar10,uVar5,uVar1 & 0x1f,(uVar1 & 0x1f00) >> 8);
  }
  if ((user_touched_fp_csr != '\0') ||
     ((uStack_38 == (longlong)(int)((uVar1 & 0x1f00) >> 8) && (uVar10 == (param_2 & 0x1f)))))
  goto LAB_0fb684b0;
  uVar2 = get_fpc_csr();
  if (uStack_38 == (uVar10 | uStack_38)) {
    if (param_1 == (undefined1 *)0x0) {
      lVar4 = 0;
      uStack_40 = uVar2;
      goto LAB_0fb68718;
    }
    lVar4 = 0;
    uVar10 = uVar2;
LAB_0fb68578:
    uVar3 = (ulonglong)(int)last_fpc_csr_value;
    uVar1 = (uint)uVar2;
    if (uVar3 != uVar2) {
      uVar5 = (uVar1 & 0xfff) >> 0xb;
      if ((((uVar5 != (last_fpc_csr_value & 0xfff) >> 0xb) ||
           (uVar5 != (last_fpc_csr_value & 0x7ff) >> 10)) ||
          ((last_fpc_csr_value & 0x3ff) >> 9 != (uVar1 & 0x3ff) >> 9)) ||
         (((last_fpc_csr_value & 0x1ff) >> 8 != (uVar1 & 0x1ff) >> 8 ||
          ((last_fpc_csr_value & 0xff) >> 7 != (uVar1 & 0xff) >> 7)))) {
        user_touched_fp_csr = '\x01';
        goto LAB_0fb684b0;
      }
    }
  }
  else {
    uStack_40 = uVar2;
    force_warning("The aggregate IEEE exceptions required (OEX_FPU_MIN) (0x%lx) not as complete as the aggregate IEEE exceptions permitted (OEX_FPU_MAX>>8)(0x%lx).\n"
                  ,uVar10,uStack_38);
    puVar8 = &DAT_0fbd5820;
    if (param_1 != (undefined1 *)0x0) {
      puVar8 = param_1;
    }
    force_warning("Use \"elfdump -op %s\" to see the floating-point exceptions (OEX) flags for the latest object.\n"
                  ,puVar8);
    force_warning("Using aggregate 0x%x as OEX_FPU_MIN to add floating-point exceptions flags with set_fpc_csr().\n"
                  ,uVar10);
    if (param_1 != (undefined1 *)0x0) {
      lVar4 = 1;
      uVar10 = uStack_40;
      goto LAB_0fb68578;
    }
    lVar4 = 1;
LAB_0fb68718:
    uVar3 = (ulonglong)(int)last_fpc_csr_value;
    uVar10 = uStack_40;
  }
  if ((uVar9 & 0x10) != 0) {
    uVar10 = uVar2 | 0x800;
  }
  if ((uVar9 & 8) != 0) {
    uVar10 = uVar10 | 0x400;
  }
  if ((uVar9 & 4) != 0) {
    uVar10 = uVar10 | 0x200;
  }
  if ((uVar9 & 2) != 0) {
    uVar10 = uVar10 | 0x100;
  }
  if ((uVar9 & 1) != 0) {
    uVar10 = uVar10 | 0x80;
  }
  uStack_60 = uVar10;
  lStack_58 = lVar4;
  uStack_40 = uVar10;
  if (uVar3 != uVar10) {
    set_fpc_csr(uVar10);
    last_fpc_csr_value = (uint)uStack_60;
    if (verbose != '\0') {
      trace("Setting csr to 0x%x\n",uStack_60);
    }
    if (lStack_58 != 0) {
      force_warning("Calling set_fpc_csr with (0x%x)\n",uStack_60);
    }
  }
LAB_0fb684b0:
  if ((pzero_mapped == '\0') && ((longlong)(int)oex_global << 0x2f < 0)) {
    uStack_74 = 0x1000;
    sStack_7c = 0x1000;
    uStack_80 = 0x1000;
    uStack_84 = 0;
    uStack_88 = 0;
    uStack_8c = 0;
    uStack_90 = 1;
    uStack_78 = 4;
    iVar6 = open64("/dev/zero",0);
    lStack_50 = (longlong)iVar6;
    if (lStack_50 == -1) {
      lStack_70 = (longlong)errno;
      pcVar7 = strerror(errno);
      fatal("handle_oex:Can\'t open /dev/zero. (errno %d (%s))",lStack_70,pcVar7);
    }
    if (verbose != '\0') {
      trace("syssgi(SGI_ELFMAP...) on page zero phdr \n");
    }
    lVar4 = syssgi(0x44,lStack_50,&uStack_90,1);
    if (lVar4 < 0) {
      lStack_68 = (longlong)errno;
      pcVar7 = strerror(errno);
      warning("couldn\'t map page zero even though some speculative operations may require it. (errno %d (%s))"
              ,lStack_68,pcVar7);
    }
    else if (lVar4 != 0) {
      lStack_48 = lVar4;
      if (verbose != '\0') {
        trace("syssgi SGI ELFMAP on page zero returned addr 0x%lx, so unmapping\n",lVar4);
      }
      munmap((void *)lStack_48,sStack_7c);
    }
    pzero_mapped = '\x01';
  }
  if ((have_set_smm == '\0') && ((longlong)(int)oex_global << 0x2e < 0)) {
    if (verbose != '\0') {
      trace("syssgi(SGI_SET_CONFIG_SMM, 1);\n",1);
    }
    syssgi(0x6e,1);
    have_set_smm = '\x01';
  }
  if ((have_set_fp_precise == '\0') && ((longlong)(int)oex_global << 0x2d < 0)) {
    if (verbose != '\0') {
      trace("syssgi(SGI_SET_FP_PRECISE, 1);\n",1);
    }
    syssgi(0x6b,1);
    have_set_fp_precise = '\x01';
  }
  if ((have_set_speculative_exec == '\0') && ((longlong)(int)oex_global << 0x2c < 0)) {
    if (verbose != '\0') {
      trace("syssgi(SGI_SPECULATIVE_EXEC, 1);\n",1);
    }
    syssgi(0x83,1);
    have_set_speculative_exec = 1;
    return;
  }
  return;
}



// === set_ltr_extra @ 0fb68870 (172 bytes) ===

void set_ltr_extra(void)

{
  ltr_extra = quickstart_info | warnconflict |
              user_tracking | interact |
              rld_has_listed_symbols | check_interface |
              execrld | var_stat |
              debug_hash | debug_olist |
              debug_malloc | debug_threads |
              debug_conflict | debug_trace |
              debug_map | debug_symbol |
              debug_general | guarantee_start_init | interface | verbose | guarantee_init;
  return;
}



// === ascending_init_order @ 0fb68920 (60 bytes) ===

undefined8 ascending_init_order(int *param_1,int *param_2)

{
  if (*(int *)(*param_1 + 0xd4) < *(int *)(*param_2 + 0xd4)) {
    return 0xffffffffffffffff;
  }
  if (*(int *)(*param_1 + 0xd4) == *(int *)(*param_2 + 0xd4)) {
    return 0;
  }
  return 1;
}



// === FUN_0fb68960 @ 0fb68960 (380 bytes) ===

void FUN_0fb68960(void)

{
  int *piVar1;
  int *__base;
  int iVar2;
  int *piVar3;
  int iVar4;
  uint __nmemb;
  
  __nmemb = 0;
  if (pObj_Head != 0) {
    __nmemb = 0;
    iVar2 = pObj_Head;
    do {
      iVar4 = *(int *)(iVar2 + 8);
      if (*(int *)(iVar2 + 0xd4) != 0) {
        __nmemb = __nmemb + 1;
      }
      if (iVar4 == 0) break;
      if (*(int *)(iVar4 + 0xd4) != 0) {
        __nmemb = __nmemb + 1;
      }
      iVar2 = *(int *)(iVar4 + 8);
    } while (iVar2 != 0);
  }
  __base = (int *)malloc(__nmemb << 2);
  iVar2 = pObj_Head;
  piVar3 = __base;
  if (__base == (int *)0x0) {
    fatal("reassign_init_order_values:  malloc failed for array length = %lu\n",__nmemb);
    iVar2 = pObj_Head;
  }
  for (; iVar2 != 0; iVar2 = *(int *)(iVar2 + 8)) {
    if (*(int *)(iVar2 + 0xd4) != 0) {
      *piVar3 = iVar2;
      piVar3 = piVar3 + 1;
    }
    iVar2 = *(int *)(iVar2 + 8);
    if (iVar2 == 0) break;
    if (*(int *)(iVar2 + 0xd4) != 0) {
      *piVar3 = iVar2;
      piVar3 = piVar3 + 1;
    }
  }
  qsort(__base,__nmemb,4,ascending_init_order);
  if (0 < (int)__nmemb) {
    iVar2 = 1;
    piVar3 = __base;
    if ((__nmemb & 1) != 0) {
      piVar3 = __base + 1;
      *(undefined4 *)(*__base + 0xd4) = 1;
      iVar2 = 2;
    }
    if ((int)__nmemb >> 1 != 0) {
      do {
        *(int *)(*piVar3 + 0xd4) = iVar2;
        iVar4 = iVar2 + 1;
        iVar2 = iVar2 + 2;
        piVar1 = piVar3 + 1;
        piVar3 = piVar3 + 2;
        *(int *)(*piVar1 + 0xd4) = iVar4;
      } while (iVar2 != __nmemb + 1);
    }
  }
  free(__base);
  next_init_order_value = __nmemb + 1;
  return;
}



// === FUN_0fb68ae0 @ 0fb68ae0 (292 bytes) ===

undefined8 FUN_0fb68ae0(int param_1)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0xd4) == 0) {
    if (next_init_order_value == 0x7fffffff) {
      FUN_0fb68960();
    }
    *(int *)(param_1 + 0xd4) = next_init_order_value;
    next_init_order_value = next_init_order_value + 1;
  }
  iVar1 = *(int *)(param_1 + 0xe8);
  if (iVar1 != 0) {
    if (((*(char *)(param_1 + 0xc4) != '\0') && (*(char *)(param_1 + 0xcf) == '\0')) &&
       (*(char *)(param_1 + 0xd0) == '\0')) {
      *(undefined1 *)(param_1 + 0xcf) = 1;
      if (debug_trace != '\0') {
        trace("call_init: calling initialization code of \"%s\" -- 0x%lx base aout marrk %d\n",
              *(undefined4 *)(param_1 + 0x18),iVar1,*(undefined1 *)(param_1 + 0xce));
      }
      if (verbose != '\0') {
        trace("call_init: calling initialization code of \"%s\" -- 0x%lx base aout marrk %d\n",
              *(undefined4 *)(param_1 + 0x18),iVar1,*(undefined1 *)(param_1 + 0xce));
      }
      init_bridge(iVar1,has_pixified_init);
      *(undefined1 *)(param_1 + 0xd0) = 1;
      return 0xffffffffffffffff;
    }
  }
  if (verbose != '\0') {
    trace("call_init: \"%s\" -- 0x%lx has no initialization code or init code has been run already (%d %d %d) base_aout %d \n"
          ,*(undefined4 *)(param_1 + 0x18),iVar1,*(undefined1 *)(param_1 + 0xc4),
          *(undefined1 *)(param_1 + 0xcf),*(undefined1 *)(param_1 + 0xd0),
          *(undefined1 *)(param_1 + 0xce));
  }
  return 0xffffffffffffffff;
}



// === FUN_0fb68c10 @ 0fb68c10 (216 bytes) ===

undefined8 FUN_0fb68c10(int param_1)

{
  char *pcVar2;
  longlong lVar1;
  char *__s;
  int iVar3;
  int iVar4;
  
  if (((*(char *)(param_1 + 0xc4) != '\0') && (*(char *)(param_1 + 0xcf) == '\0')) &&
     (*(char *)(param_1 + 0xd0) == '\0')) {
    if ((*(char *)(param_1 + 0xdb) == '\0') &&
       (*(undefined1 *)(param_1 + 0xdb) = 1, 0 < *(int *)(param_1 + 0xb0))) {
      iVar4 = 0;
      iVar3 = 0;
      do {
        __s = (char *)(*(int *)(*(int *)(param_1 + 0x74) + iVar4) + *(int *)(param_1 + 0x60));
        pcVar2 = strchr(__s,0x2f);
        lVar1 = locate_obj(__s,pcVar2 != (char *)0x0);
        if (lVar1 != -1) {
          FUN_0fb68c10(lVar1);
        }
        iVar3 = iVar3 + 1;
        iVar4 = iVar4 + 0x14;
      } while (iVar3 < *(int *)(param_1 + 0xb0));
    }
    FUN_0fb68ae0(param_1);
  }
  return 0xffffffffffffffff;
}



// === call_pixie_init @ 0fb68cf0 (156 bytes) ===

/* WARNING: Instruction at (ram,0x0fb68d0c) overlaps instruction at (ram,0x0fb68d08)
    */

undefined8 call_pixie_init(int param_1)

{
  undefined4 uVar1;
  
  if ((*(int *)(param_1 + 0xfc) != 0) && (*(char *)(param_1 + 0xda) == '\0')) {
    has_pixified_init = 1;
    uVar1 = *(undefined4 *)(param_1 + 0xfc);
    if (debug_trace != '\0') {
      trace("call_pixie_init: calling pixie initialization code of \"%s\" -- 0x%lx\n",
            *(undefined4 *)(param_1 + 0x18),uVar1);
    }
    if (verbose != '\0') {
      trace("call_pixie_init: calling pixie initialization code of \"%s\" -- 0x%lx\n",
            *(undefined4 *)(param_1 + 0x18),uVar1);
    }
    init_bridge(uVar1,has_pixified_init);
    *(undefined1 *)(param_1 + 0xda) = 1;
    return 0xffffffffffffffff;
  }
  return 0xffffffffffffffff;
}



// === execute_all_init_sections @ 0fb68d90 (244 bytes) ===

void execute_all_init_sections(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  
  if (param_1 != 0) {
    iVar2 = *(int *)(param_1 + 4) + -1;
    iVar1 = *(int *)(param_1 + 8);
    if (-1 < iVar2) {
      puVar3 = (undefined4 *)(iVar1 + iVar2 * 4);
      do {
        FUN_0fb68c10(*puVar3);
        puVar3 = puVar3 + -1;
        last_error = 0;
      } while (puVar3 != (undefined4 *)(iVar1 + -4));
      last_error = 0;
    }
    if (verbose != '\0') {
      trace("execute_all_init_sections, %lu records considered: finished running inits\n",
            *(undefined4 *)(param_1 + 4));
    }
    init_done = 1;
    return;
  }
  if (guarantee_start_init == '\0') {
    FUN_0fb68c10(*(undefined4 *)(pObj_Head + 0xc));
    forall_previous_objs(pObj_Head,*(undefined4 *)(pObj_Head + 0xc),FUN_0fb68c10);
  }
  else {
    foreach_obj(pObj_Head,FUN_0fb68ae0,0);
  }
  if (verbose != '\0') {
    trace("execute_all_init_sections: finished running inits\n");
  }
  init_done = 1;
  return;
}



// === __dbx_link @ 0fb68e90 (8 bytes) ===

void __dbx_link(void)

{
  return;
}



// === __rld_calls_on_rld_list_change @ 0fb68ea0 (68 bytes) ===

void __rld_calls_on_rld_list_change(void)

{
  __dbx_link();
  if (__rld_recent_update_callback != '\0') {
    __rld_recent_update_callback = '\0';
    __rld_refresh_callback_list();
  }
  if (0 < __rld_callback_list_count) {
    __rld_callback_list_execute();
  }
  return;
}



// === FUN_0fb68ef0 @ 0fb68ef0 (1132 bytes) ===

void FUN_0fb68ef0(int param_1)

{
  int *piVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  uint uVar8;
  int *piVar9;
  bool bVar10;
  uint uVar11;
  uint uVar12;
  uint uVar13;
  int iVar14;
  uint uVar15;
  code *UNRECOVERED_JUMPTABLE;
  undefined4 *puStack_70;
  
  uVar13 = *(uint *)(param_1 + 0xf4);
  uVar15 = *(uint *)(param_1 + 0xf0);
  if (debug_trace != '\0') {
    trace("fix_all_defineds: obj 0x%lx, name %s\n",param_1,*(undefined4 *)(param_1 + 0x18));
  }
  if (debug_symbol != '\0') {
    debug("fix_all_defineds: obj 0x%lx, name %s, o_first_external_not_ref_this_object %ld, gotsym %ld\n"
          ,param_1,*(undefined4 *)(param_1 + 0x18),*(undefined4 *)(param_1 + 0xf0),uVar13);
  }
  uVar12 = *(uint *)(param_1 + 0x120);
  iVar14 = *(int *)(param_1 + 0x3c) - *(int *)(param_1 + 0x10);
  if (uVar12 != 0) {
    iVar6 = 0;
    uVar8 = 1;
    iVar2 = *(int *)(param_1 + 0x6c);
    do {
      puStack_70 = (undefined4 *)(iVar2 + iVar6);
      piVar1 = (int *)*puStack_70;
      uVar3 = puStack_70[3];
      if (iVar6 == 0) {
        piVar1 = piVar1 + 1;
        uVar11 = 1;
        if (1 < uVar3) {
LAB_0fb68f9c:
          do {
            *piVar1 = *piVar1 - iVar14;
            uVar3 = puStack_70[3];
            if (uVar3 <= uVar11 + 1) break;
            piVar1[1] = piVar1[1] - iVar14;
            uVar3 = puStack_70[3];
            uVar11 = uVar11 + 2;
            piVar1 = piVar1 + 2;
          } while (uVar11 < uVar3);
          uVar12 = *(uint *)(param_1 + 0x120);
        }
      }
      else {
        uVar11 = 0;
        if (uVar3 != 0) goto LAB_0fb68f9c;
      }
      bVar10 = uVar12 <= uVar8;
      uVar8 = uVar8 + 1;
      iVar6 = iVar6 + 0x10;
      if (bVar10) goto LAB_0fb6902c;
      iVar2 = *(int *)(param_1 + 0x6c);
    } while( true );
  }
  uVar3 = puStack_70[3];
LAB_0fb6902c:
  if (uVar15 < uVar13) {
    uVar15 = uVar13;
  }
  piVar1 = *(int **)(param_1 + 100);
  iVar2 = puStack_70[2];
  if (1 < uVar15) {
    uVar12 = 1;
    iVar6 = 4;
    do {
      if (((*(char *)((int)piVar1 + 0x1d) != '\0') && (*(char *)((int)piVar1 + 0x1d) != '\x04')) ||
         ((int)(uint)*(byte *)(piVar1 + 7) >> 4 == 0)) {
        if (*(short *)((int)piVar1 + 0x1e) == -0xf) {
          if (*(int *)(param_1 + 0x120) == 1) {
            if ((iVar2 + uVar13) - uVar3 <= uVar12) {
              *(int *)(*(int *)(*(int *)(param_1 + 0x6c) + 4) + iVar6 + uVar13 * -4) = piVar1[5];
            }
          }
          else {
            uVar8 = find_reloc(param_1,uVar12);
            if (uVar8 != 0) {
              bVar10 = false;
              iVar7 = *(int *)(param_1 + 0x70);
              iVar5 = uVar8 * 8;
              piVar9 = (int *)(iVar7 + iVar5);
              uVar11 = piVar9[1];
              if ((uVar12 == uVar11 >> 8) && (uVar8 < *(uint *)(param_1 + 0xa8) >> 3)) {
                while( true ) {
                  uVar4 = uVar8 + 1;
                  if ((uVar11 & 0xff) == 0x24) {
                    *(int *)(*piVar9 - (*(int *)(param_1 + 0x3c) - *(int *)(param_1 + 0x10))) =
                         piVar1[5];
                    bVar10 = true;
                    iVar7 = *(int *)(param_1 + 0x70);
                  }
                  piVar9 = (int *)(iVar7 + iVar5 + 8);
                  uVar11 = piVar9[1];
                  if (uVar12 != uVar11 >> 8) break;
                  iVar5 = iVar5 + 0x10;
                  uVar8 = uVar8 + 2;
                  if (*(uint *)(param_1 + 0xa8) >> 3 <= uVar4) break;
                  if ((uVar11 & 0xff) == 0x24) {
                    *(int *)(*piVar9 - (*(int *)(param_1 + 0x3c) - *(int *)(param_1 + 0x10))) =
                         piVar1[5];
                    bVar10 = true;
                    iVar7 = *(int *)(param_1 + 0x70);
                  }
                  piVar9 = (int *)(iVar7 + iVar5);
                  uVar11 = piVar9[1];
                  if ((uVar12 != uVar11 >> 8) || (*(uint *)(param_1 + 0xa8) >> 3 <= uVar8)) break;
                }
                if (bVar10) goto LAB_0fb6920c;
              }
              if (verbose != '\0') {
                trace("Unable to find GOT entry for SHN_ABS symbol %s (symbol number %ld) in %s. App may not work correctly."
                      ,piVar1[4] + *(int *)(param_1 + 0x60),uVar12,*(undefined4 *)(param_1 + 0x18));
              }
            }
          }
LAB_0fb6920c:
          resolve_relocations(param_1,uVar12,piVar1[5]);
        }
        else {
          resolve_relocations(param_1,uVar12,piVar1[5] - iVar14);
        }
      }
      uVar12 = uVar12 + 1;
      iVar6 = iVar6 + 4;
      piVar1 = piVar1 + 4;
    } while (uVar15 != uVar12);
    piVar1 = *(int **)(param_1 + 100);
  }
  uVar15 = *(uint *)(param_1 + 0x9c);
  piVar1 = piVar1 + uVar13 * 4;
  if (uVar13 < uVar15) {
    do {
      while( true ) {
        if (((*(short *)((int)piVar1 + 0xe) == -0xf) || (piVar1[1] == 0)) ||
           ((*(short *)((int)piVar1 + 0xe) == -0xe && ((uint)piVar1[1] < 0x10001)))) break;
        iVar2 = obj_dynsym_got(param_1,uVar13);
        resolve_relocations(param_1,uVar13,iVar2 - iVar14);
        if (*(int *)(param_1 + 0x120) == 1) {
          obj_set_dynsym_got(param_1,uVar13,iVar2 - iVar14);
          uVar15 = *(uint *)(param_1 + 0x9c);
        }
        else {
          uVar15 = *(uint *)(param_1 + 0x9c);
        }
        uVar13 = uVar13 + 1;
        piVar1 = piVar1 + 4;
        if (uVar15 <= uVar13) goto LAB_0fb692c4;
      }
      uVar13 = uVar13 + 1;
      piVar1 = piVar1 + 4;
    } while (uVar13 < uVar15);
  }
LAB_0fb692c4:
  *(undefined1 *)(param_1 + 200) = 1;
                    /* WARNING: Could not recover jumptable at 0x0fb692f4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
}



// === FUN_0fb69380 @ 0fb69380 (472 bytes) ===

/* WARNING: Instruction at (ram,0x0fb69450) overlaps instruction at (ram,0x0fb6944c)
    */

void FUN_0fb69380(int param_1,undefined8 param_2)

{
  uint uVar1;
  longlong lVar2;
  undefined8 uVar3;
  code *pcVar4;
  int iVar5;
  int iVar6;
  int *piVar7;
  
  piVar7 = (int *)(*(int *)(param_1 + 100) + (int)param_2 * 0x10);
  if ((rld_has_listed_symbols != '\0') &&
     (lVar2 = rld_is_listed_symbol(*piVar7 + *(int *)(param_1 + 0x60)), lVar2 != 0)) {
    uVar3 = obj_dynsym_got(param_1,param_2);
    debug("do_relocations_and_check_reloc_on_stub: index %ld, name %s, value 0x%lx, shndx 0x%lx, got 0x%lx\n"
          ,param_2,*piVar7 + *(int *)(param_1 + 0x60),piVar7[1],*(undefined2 *)((int)piVar7 + 0xe),
          uVar3);
  }
  if (debug_symbol != '\0') {
    uVar3 = obj_dynsym_got(param_1,param_2);
    debug("do_relocations_and_check_reloc_on_stub: index %ld, name %s, value 0x%lx, shndx 0x%lx, got 0x%lx\n"
          ,param_2,*piVar7 + *(int *)(param_1 + 0x60),piVar7[1],*(undefined2 *)((int)piVar7 + 0xe),
          uVar3);
  }
  iVar6 = (int)param_2 * 8;
  uVar1 = *(uint *)(*(int *)(param_1 + 0x68) + iVar6 + 4);
  if ((*(char *)(param_1 + 0xc1) != '\0') && ((uVar1 & 0xff) == 0)) {
    iVar5 = find_reloc(param_1,param_2);
    *(int *)(*(int *)(param_1 + 0x68) + iVar6 + 4) = iVar5 * 0x100 + 1;
    uVar1 = *(uint *)(*(int *)(param_1 + 0x68) + iVar6 + 4);
  }
  if ((uVar1 >> 8 != 0) && (lVar2 = FUN_0fb6b990(param_1,param_2), lVar2 != 0)) {
    fatal("lazy_text_resolve: symbol %s should not have any relocation entry.\n",
          *piVar7 + *(int *)(param_1 + 0x60));
  }
  resolve_relocations(param_1,param_2,
                      piVar7[1] - (*(int *)(param_1 + 0x3c) - *(int *)(param_1 + 0x10)));
  if ((*(int *)(param_1 + 0x120) == 1) &&
     ((pcVar4 = (code *)obj_dynsym_got(param_1,param_2), pcVar4 != _rld_new_interface ||
      (iVar6 = strcmp((char *)(*piVar7 + *(int *)(param_1 + 0x60)),"_rld_new_interface"), iVar6 != 0
      )))) {
    obj_set_dynsym_got(param_1,param_2,
                       piVar7[1] - (*(int *)(param_1 + 0x3c) - *(int *)(param_1 + 0x10)));
  }
  return;
}



// === FUN_0fb69560 @ 0fb69560 (500 bytes) ===

void FUN_0fb69560(int param_1)

{
  longlong lVar1;
  int *piVar2;
  uint uVar3;
  undefined1 *puStack_40;
  undefined1 uStack_3c;
  int iStack_38;
  undefined4 uStack_34;
  
  uStack_3c = 0;
  puStack_40 = &DAT_0fbd5820;
  uStack_34 = 0;
  iStack_38 = 0;
  if (ltr_extra != '\0') {
    if (debug_trace != '\0') {
      trace("search_for_unresolved: obj 0x%lx, name %s \n",param_1,*(undefined4 *)(param_1 + 0x18));
    }
    if (debug_symbol != '\0') {
      debug("search_for_unresolved: obj 0x%lx, name %s, o_first_external_not_ref_this_object %ld, o_index_first_dynsym_with_got = %ld max symbol search count: %ld\n"
            ,param_1,*(undefined4 *)(param_1 + 0x18),*(undefined4 *)(param_1 + 0xf0),
            *(int *)(param_1 + 0xf4),*(int *)(param_1 + 0x9c) - *(int *)(param_1 + 0xf4));
    }
  }
  uVar3 = *(uint *)(param_1 + 0xf4);
  piVar2 = (int *)(*(int *)(param_1 + 100) + uVar3 * 0x10);
  if (uVar3 < *(uint *)(param_1 + 0x9c)) {
    do {
      if (iStack_38 != 0) {
        iStack_38 = 0;
        *puStack_40 = 0;
      }
      if (((*(short *)((int)piVar2 + 0xe) != -0xf) && ((int)(uint)*(byte *)(piVar2 + 3) >> 4 != 0))
         && (*(char *)((int)piVar2 + 0xd) != '\x02')) {
        if ((piVar2[1] != 0) &&
           ((*(short *)((int)piVar2 + 0xe) != -0xe || (0x10000 < (uint)piVar2[1])))) break;
        if (rld_has_listed_symbols != '\0') {
          lVar1 = rld_is_listed_symbol(*piVar2 + *(int *)(param_1 + 0x60));
          if (lVar1 != 0) {
            debug("search_for_unresolved: index %ld, name %s, value 0x%lx, shndx 0x%lx\n",uVar3,
                  *piVar2 + *(int *)(param_1 + 0x60),piVar2[1],*(undefined2 *)((int)piVar2 + 0xe));
          }
        }
        if (debug_symbol != '\0') {
          debug("search_for_unresolved: index %ld, name %s, value 0x%lx, shndx 0x%lx\n",uVar3,
                *piVar2 + *(int *)(param_1 + 0x60),piVar2[1],*(undefined2 *)((int)piVar2 + 0xe));
        }
        lVar1 = obj_dynsym_got(param_1,uVar3);
        if ((lVar1 == 0) || (all_fully_quickstarted == '\0')) {
          FUN_0fb663a0(param_1,uVar3,piVar2,&puStack_40);
        }
      }
      uVar3 = uVar3 + 1;
      piVar2 = piVar2 + 4;
    } while (uVar3 < *(uint *)(param_1 + 0x9c));
  }
  if (debug_symbol != '\0') {
    debug("search_for_unresolved actually checked %ld symbols\n",uVar3 - *(int *)(param_1 + 0xf4));
  }
  free_things_tried(&puStack_40);
  return;
}



// === FUN_0fb69760 @ 0fb69760 (752 bytes) ===

void FUN_0fb69760(int param_1,longlong param_2,longlong param_3)

{
  bool bVar1;
  longlong lVar2;
  uint uVar3;
  int iVar4;
  uint uVar5;
  int *piVar6;
  uint uVar7;
  undefined1 *puStack_50;
  undefined1 uStack_4c;
  int iStack_48;
  undefined4 uStack_44;
  longlong lStack_40;
  
  uStack_4c = 0;
  puStack_50 = &DAT_0fbd5820;
  uStack_44 = 0;
  iStack_48 = 0;
  lStack_40 = param_3;
  if (debug_trace != '\0') {
    trace("search_for_undefineds: obj 0x%lx, name %s\n",param_1,*(undefined4 *)(param_1 + 0x18));
  }
  if (debug_symbol != '\0') {
    debug("search_for_undefineds: obj 0x%lx, name %s, o_first_external_not_ref_this_object %ld, gotsym %ld sym count %ld. Max search %ld\n"
          ,param_1,*(undefined4 *)(param_1 + 0x18),*(undefined4 *)(param_1 + 0xf0),
          *(int *)(param_1 + 0xf4),*(int *)(param_1 + 0x9c),
          *(int *)(param_1 + 0x9c) - *(int *)(param_1 + 0xf4));
  }
  uVar7 = *(uint *)(param_1 + 0xf4);
  piVar6 = (int *)(*(int *)(param_1 + 100) + uVar7 * 0x10);
  if (uVar7 < *(uint *)(param_1 + 0x9c)) {
    do {
      if (iStack_48 != 0) {
        iStack_48 = 0;
        *puStack_50 = 0;
      }
      if (((*(short *)((int)piVar6 + 0xe) == -0xf) || ((int)(uint)*(byte *)(piVar6 + 3) >> 4 == 0))
         || (*(char *)((int)piVar6 + 0xd) == '\x02')) {
LAB_0fb69814:
        uVar5 = *(uint *)(param_1 + 0x9c);
      }
      else if (param_2 == 0) {
        uVar5 = piVar6[1];
LAB_0fb6986c:
        if (uVar5 == 0) {
LAB_0fb69a00:
          bVar1 = false;
        }
        else {
LAB_0fb69874:
          bVar1 = true;
          if ((*(short *)((int)piVar6 + 0xe) == -0xe) && (uVar5 < 0x10001)) goto LAB_0fb69a00;
        }
        if (bVar1) {
          if (lStack_40 != 0) {
            lVar2 = get_symlib_table_entry(param_1,uVar7);
            if ((lVar2 == 0) ||
               ((*(uint *)(*(int *)(param_1 + 0x74) + (int)lVar2 * 0x14 + -4) & 0x10) == 0))
            goto LAB_0fb69968;
          }
          if ((*(short *)((int)piVar6 + 0xe) != 0) ||
             (iVar4 = obj_dynsym_got(param_1,uVar7), piVar6[1] == iVar4)) {
            if (((*(char *)(param_1 + 0xdf) != '\0') || ((*(uint *)(param_1 + 0x84) & 0x4000) == 0))
               || (*(short *)((int)piVar6 + 0xe) == 0)) goto LAB_0fb69814;
            if (user_tracking != '\0') {
              track("Search for undefineds stopped at symbol %ld %s due to RHF_RLD_ORDER_SAFE\n",
                    uVar7,*piVar6 + *(int *)(param_1 + 0x60));
            }
            break;
          }
          FUN_0fb69380(param_1,uVar7);
          uVar5 = *(uint *)(param_1 + 0x9c);
        }
        else {
LAB_0fb69968:
          if ((rld_has_listed_symbols != '\0') &&
             (lVar2 = rld_is_listed_symbol(*piVar6 + *(int *)(param_1 + 0x60)), lVar2 != 0)) {
            debug("search_for_undefineds: index %ld, name %s, value 0x%lx, shndx 0x%lx\n",uVar7,
                  *piVar6 + *(int *)(param_1 + 0x60),piVar6[1],*(undefined2 *)((int)piVar6 + 0xe));
          }
          if (debug_symbol != '\0') {
            debug("search_for_undefineds: index %ld, name %s, value 0x%lx, shndx 0x%lx\n",uVar7,
                  *piVar6 + *(int *)(param_1 + 0x60),piVar6[1],*(undefined2 *)((int)piVar6 + 0xe));
          }
          FUN_0fb663a0(param_1,uVar7,piVar6,&puStack_50);
          uVar5 = *(uint *)(param_1 + 0x9c);
        }
      }
      else {
        if (*(short *)((int)piVar6 + 0xe) != 0) goto LAB_0fb69814;
        uVar3 = obj_dynsym_got(param_1,uVar7);
        uVar5 = piVar6[1];
        if (uVar3 == uVar5) goto LAB_0fb6986c;
        if (uVar5 != 0) goto LAB_0fb69874;
        uVar5 = *(uint *)(param_1 + 0x9c);
      }
      uVar7 = uVar7 + 1;
      piVar6 = piVar6 + 4;
    } while (uVar7 < uVar5);
  }
  if (debug_symbol != '\0') {
    debug("search_for_undefineds %s actually checked %ld syms.\n",*(undefined4 *)(param_1 + 0x18),
          uVar7 - *(int *)(param_1 + 0xf4));
  }
  free_things_tried(&puStack_50);
  return;
}



// === FUN_0fb69a50 @ 0fb69a50 (860 bytes) ===

/* WARNING: Instruction at (ram,0x0fb69ce8) overlaps instruction at (ram,0x0fb69ce4)
    */

void FUN_0fb69a50(int param_1,longlong param_2)

{
  uint uVar1;
  bool bVar2;
  longlong lVar3;
  undefined8 uVar4;
  int iVar5;
  undefined8 uVar6;
  int *piVar7;
  uint uVar8;
  undefined1 *puStack_50;
  undefined1 uStack_4c;
  int iStack_48;
  undefined4 uStack_44;
  
  uStack_4c = 0;
  puStack_50 = &DAT_0fbd5820;
  uStack_44 = 0;
  iStack_48 = 0;
  if (debug_trace != '\0') {
    trace("search_for_externals: obj 0x%lx, name %s\n",param_1,*(undefined4 *)(param_1 + 0x18));
  }
  if (debug_symbol != '\0') {
    debug("search_for_externals: obj 0x%lx, name %s, o_first_external_not_ref_this_object %ld, o_index_first_dynsym_with_got %ld last %ld  max syms to ck %ld\n"
          ,param_1,*(undefined4 *)(param_1 + 0x18),*(undefined4 *)(param_1 + 0xf0),
          *(int *)(param_1 + 0xf4),*(int *)(param_1 + 0x9c),
          *(int *)(param_1 + 0x9c) - *(int *)(param_1 + 0xf4));
  }
  uVar8 = *(uint *)(param_1 + 0xf4);
  piVar7 = (int *)(*(int *)(param_1 + 100) + uVar8 * 0x10);
  if (uVar8 < *(uint *)(param_1 + 0x9c)) {
    do {
      if (iStack_48 != 0) {
        iStack_48 = 0;
        *puStack_50 = 0;
      }
      if (((*(short *)((int)piVar7 + 0xe) == -0xf) || ((int)(uint)*(byte *)(piVar7 + 3) >> 4 == 0))
         || (*(char *)((int)piVar7 + 0xd) == '\x02')) {
LAB_0fb69b68:
        uVar1 = *(uint *)(param_1 + 0x9c);
      }
      else {
        if ((rld_has_listed_symbols != '\0') &&
           (lVar3 = rld_is_listed_symbol(*piVar7 + *(int *)(param_1 + 0x60)), lVar3 != 0)) {
          uVar4 = get_symlib_table_entry(param_1,uVar8);
          lVar3 = get_symlib_table_entry(param_1,uVar8);
          if ((lVar3 == 0) ||
             (uVar6 = 1, (*(uint *)(*(int *)(param_1 + 0x74) + (int)lVar3 * 0x14 + -4) & 0x10) == 0)
             ) {
            uVar6 = 0;
          }
          debug("search_for_externals dynsym val 0x%lx shndx 0x%lx ldbind now %d symlib 0x%lx delay_load? 0x%lx\n"
                ,piVar7[1],*(undefined2 *)((int)piVar7 + 0xe),param_2,uVar4,uVar6);
        }
        if (debug_symbol != '\0') {
          uVar4 = get_symlib_table_entry(param_1,uVar8);
          lVar3 = get_symlib_table_entry(param_1,uVar8);
          if ((lVar3 == 0) ||
             (uVar6 = 1, (*(uint *)(*(int *)(param_1 + 0x74) + (int)lVar3 * 0x14 + -4) & 0x10) == 0)
             ) {
            uVar6 = 0;
          }
          debug("search_for_externals dynsym val 0x%lx shndx 0x%lx ldbind now %d symlib 0x%lx delay_load? 0x%lx\n"
                ,piVar7[1],*(undefined2 *)((int)piVar7 + 0xe),param_2,uVar4,uVar6);
        }
        bVar2 = false;
        if ((piVar7[1] != 0) && (*(short *)((int)piVar7 + 0xe) == 0)) {
          bVar2 = true;
        }
        if ((!bVar2) ||
           ((param_2 != 0 &&
            ((lVar3 = get_symlib_table_entry(param_1,uVar8), lVar3 == 0 ||
             ((*(uint *)(*(int *)(param_1 + 0x74) + (int)lVar3 * 0x14 + -4) & 0x10) == 0)))))) {
          if ((rld_has_listed_symbols != '\0') &&
             (lVar3 = rld_is_listed_symbol(*piVar7 + *(int *)(param_1 + 0x60)), lVar3 != 0)) {
            uVar4 = obj_dynsym_got(param_1,uVar8);
            debug("search_for_externals: index %ld, name %s, value 0x%lx, shndx 0x%lx, got 0x%lx\n",
                  uVar8,*piVar7 + *(int *)(param_1 + 0x60),piVar7[1],
                  *(undefined2 *)((int)piVar7 + 0xe),uVar4);
          }
          if (debug_symbol != '\0') {
            uVar4 = obj_dynsym_got(param_1,uVar8);
            debug("search_for_externals: index %ld, name %s, value 0x%lx, shndx 0x%lx, got 0x%lx\n",
                  uVar8,*piVar7 + *(int *)(param_1 + 0x60),piVar7[1],
                  *(undefined2 *)((int)piVar7 + 0xe),uVar4);
          }
          FUN_0fb663a0(param_1,uVar8,piVar7,&puStack_50);
          goto LAB_0fb69b68;
        }
        iVar5 = obj_dynsym_got(param_1,uVar8);
        if (piVar7[1] == iVar5) goto LAB_0fb69b68;
        FUN_0fb69380(param_1,uVar8);
        uVar1 = *(uint *)(param_1 + 0x9c);
      }
      uVar8 = uVar8 + 1;
      piVar7 = piVar7 + 4;
    } while (uVar8 < uVar1);
  }
  if (debug_symbol != '\0') {
    debug("search_for_externals: obj name %s,  actually checked %ld syms\n",
          *(undefined4 *)(param_1 + 0x18),uVar8 - *(int *)(param_1 + 0xf4));
  }
  free_things_tried(&puStack_50);
  return;
}



// === obj_already_mapped @ 0fb69db0 (124 bytes) ===

int obj_already_mapped(int param_1,char *param_2)

{
  int iVar1;
  
  if (((*(char **)(param_1 + 0x30) == (char *)0x0) ||
      (iVar1 = strcmp(param_2,*(char **)(param_1 + 0x30)), iVar1 != 0)) &&
     ((*(char **)(param_1 + 0x2c) == (char *)0x0 ||
      (iVar1 = strcmp(param_2,*(char **)(param_1 + 0x2c)), iVar1 != 0)))) {
    return -1;
  }
  return param_1;
}



// === FUN_0fb69e30 @ 0fb69e30 (212 bytes) ===

undefined * FUN_0fb69e30(longlong param_1)

{
  undefined *puVar1;
  
  if (param_1 != 0) {
    DAT_0fbd3304 = pObj_Head;
    if (interface != '\0') {
      trace("_rld_first_pathname: path_obj = %s\n",*(undefined4 *)(pObj_Head + 0x18));
    }
    puVar1 = main_exec_name;
    if (main_exec_name == (undefined *)0x0) {
      puVar1 = &DAT_0fbd7778;
    }
    return puVar1;
  }
  if (DAT_0fbd3304 != 0) {
    if (*(int *)(pObj_Head + 0xc) == DAT_0fbd3304) {
      if (interface != '\0') {
        trace("_rld_next_pathname: path_obj = %s\n",*(undefined4 *)(DAT_0fbd3304 + 0x18));
      }
      return (undefined *)0x0;
    }
    DAT_0fbd3304 = *(int *)(DAT_0fbd3304 + 8);
    if (interface != '\0') {
      trace("_rld_next_pathname: path_obj = %s\n",*(undefined4 *)(DAT_0fbd3304 + 0x18));
    }
    return *(undefined **)(DAT_0fbd3304 + 0x18);
  }
  if (interface != '\0') {
    trace("_rld_next_pathname: path_obj = NULL\n");
  }
  return (undefined *)0x0;
}



// === _rld_first_pathname @ 0fb69f10 (8 bytes) ===

/* WARNING: Removing unreachable block (ram,0x0fb69e70) */
/* WARNING: Removing unreachable block (ram,0x0fb69e80) */
/* WARNING: Removing unreachable block (ram,0x0fb69e8c) */
/* WARNING: Removing unreachable block (ram,0x0fb69e94) */
/* WARNING: Removing unreachable block (ram,0x0fb69ea4) */
/* WARNING: Removing unreachable block (ram,0x0fb69eb8) */
/* WARNING: Removing unreachable block (ram,0x0fb69ec4) */
/* WARNING: Removing unreachable block (ram,0x0fb69ed4) */
/* WARNING: Removing unreachable block (ram,0x0fb69ee4) */
/* WARNING: Removing unreachable block (ram,0x0fb69eec) */
/* WARNING: Removing unreachable block (ram,0x0fb69ef8) */

undefined * _rld_first_pathname(void)

{
  undefined *puVar1;
  
  DAT_0fbd3304 = pObj_Head;
  if (interface != '\0') {
    trace("_rld_first_pathname: path_obj = %s\n",*(undefined4 *)(pObj_Head + 0x18));
  }
  puVar1 = main_exec_name;
  if (main_exec_name == (undefined *)0x0) {
    puVar1 = &DAT_0fbd7778;
  }
  return puVar1;
}



// === _rld_next_pathname @ 0fb69f20 (8 bytes) ===

/* WARNING: Removing unreachable block (ram,0x0fb69e40) */
/* WARNING: Removing unreachable block (ram,0x0fb69e4c) */
/* WARNING: Removing unreachable block (ram,0x0fb69e58) */
/* WARNING: Removing unreachable block (ram,0x0fb69eb0) */
/* WARNING: Removing unreachable block (ram,0x0fb69e64) */

undefined4 _rld_next_pathname(void)

{
  if (DAT_0fbd3304 == 0) {
    if (interface != '\0') {
      trace("_rld_next_pathname: path_obj = NULL\n");
    }
    return 0;
  }
  if (*(int *)(pObj_Head + 0xc) != DAT_0fbd3304) {
    DAT_0fbd3304 = *(int *)(DAT_0fbd3304 + 8);
    if (interface != '\0') {
      trace("_rld_next_pathname: path_obj = %s\n",*(undefined4 *)(DAT_0fbd3304 + 0x18));
    }
    return *(undefined4 *)(DAT_0fbd3304 + 0x18);
  }
  if (interface != '\0') {
    trace("_rld_next_pathname: path_obj = %s\n",*(undefined4 *)(DAT_0fbd3304 + 0x18));
  }
  return 0;
}



// === _rld_name_to_address @ 0fb69f30 (148 bytes) ===

void _rld_name_to_address(undefined8 param_1)

{
  undefined8 uVar1;
  longlong lVar2;
  code *UNRECOVERED_JUMPTABLE_00;
  undefined1 *puStack_20;
  undefined1 uStack_1c;
  undefined4 uStack_18;
  undefined4 uStack_14;
  
  uStack_1c = 0;
  puStack_20 = &DAT_0fbd5820;
  uStack_14 = 0;
  uStack_18 = 0;
  uVar1 = elfhash();
  lVar2 = resolve_symbol(param_1,"_rld_name_to_address",uVar1,0,0,0,&puStack_20);
  if (lVar2 != -1) {
    free_things_tried(&puStack_20);
                    /* WARNING: Could not recover jumptable at 0x0fb69f9c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*UNRECOVERED_JUMPTABLE_00)();
    return;
  }
  free_things_tried(&puStack_20);
                    /* WARNING: Could not recover jumptable at 0x0fb69fbc. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE_00)();
  return;
}



// === _rld_address_to_name @ 0fb69fd0 (376 bytes) ===

int _rld_address_to_name(uint param_1)

{
  bool bVar1;
  uint uVar2;
  int iVar3;
  uint uVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  int *piVar9;
  
  iVar3 = address_to_obj(pObj_Head,param_1);
  if (iVar3 == 0) {
    return 0;
  }
  uVar2 = *(uint *)(iVar3 + 0xf4);
  uVar4 = *(int *)(iVar3 + 0x9c) - 1;
  if (uVar2 < uVar4) {
    iVar5 = *(int *)(iVar3 + 0x3c) - *(int *)(iVar3 + 0x10);
    uVar6 = uVar2 + uVar4;
    uVar8 = uVar2;
    do {
      uVar7 = uVar6 >> 1;
      piVar9 = (int *)(*(int *)(iVar3 + 100) + uVar7 * 0x10);
      uVar6 = piVar9[1] - iVar5;
      if (uVar6 <= param_1) {
        if (param_1 <= uVar6) {
          return *piVar9 + *(int *)(iVar3 + 0x60);
        }
        bVar1 = uVar7 == uVar4 - 1;
        uVar8 = uVar7;
        uVar7 = uVar4;
        if (bVar1) {
          piVar9 = (int *)(*(int *)(iVar3 + 100) + uVar4 * 0x10);
          if (param_1 == piVar9[1] - iVar5) {
            return *piVar9 + *(int *)(iVar3 + 0x60);
          }
          break;
        }
      }
      uVar6 = uVar8 + uVar7;
      uVar4 = uVar7;
    } while (uVar8 < uVar7);
  }
  if (*(uint *)(iVar3 + 0xf0) < uVar2) {
    piVar9 = (int *)(*(uint *)(iVar3 + 0xf0) * 0x10 + *(int *)(iVar3 + 100));
    iVar5 = piVar9[1];
    while( true ) {
      if (param_1 == iVar5 - (*(int *)(iVar3 + 0x3c) - *(int *)(iVar3 + 0x10))) {
        return *piVar9 + *(int *)(iVar3 + 0x60);
      }
      if ((int *)(uVar2 * 0x10 + *(int *)(iVar3 + 100)) <= piVar9 + 4) break;
      iVar5 = piVar9[5];
      piVar9 = piVar9 + 4;
    }
  }
  return 0;
}



// === __rld_dladdr @ 0fb6a150 (356 bytes) ===

void __rld_dladdr(uint param_1,undefined4 *param_2)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  code *UNRECOVERED_JUMPTABLE_00;
  
  iVar2 = address_to_obj(pObj_Head,param_1);
  if (iVar2 == 0) {
                    /* WARNING: Could not recover jumptable at 0x0fb6a2ac. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*UNRECOVERED_JUMPTABLE_00)();
    return;
  }
  uVar8 = 0;
  uVar7 = 0;
  uVar6 = *(int *)(iVar2 + 0x9c) - 1;
  uVar4 = *(uint *)(iVar2 + 0xf4);
  if (*(uint *)(iVar2 + 0xf0) < *(uint *)(iVar2 + 0xf4)) {
    uVar4 = *(uint *)(iVar2 + 0xf0);
  }
  iVar3 = *(int *)(iVar2 + 100) + uVar4 * 0x10;
  if (uVar4 < uVar6) {
    do {
      if ((*(short *)(iVar3 + 0xe) != 0) && (*(short *)(iVar3 + 0xe) != -0xf)) {
        uVar5 = *(int *)(iVar3 + 4) - (*(int *)(iVar2 + 0x3c) - *(int *)(iVar2 + 0x10));
        uVar9 = uVar4;
        if (param_1 == uVar5) break;
        if ((uVar5 < param_1) && (uVar7 < uVar5)) {
          uVar8 = uVar4;
          uVar7 = uVar5;
        }
      }
      uVar4 = uVar4 + 1;
      iVar3 = iVar3 + 0x10;
      uVar9 = uVar8;
    } while (uVar4 != uVar6);
    *param_2 = *(undefined4 *)(iVar2 + 0x18);
    param_2[1] = *(undefined4 *)(iVar2 + 0x10);
    if (uVar9 != 0) {
      piVar1 = (int *)(*(int *)(iVar2 + 100) + uVar9 * 0x10);
      param_2[2] = *piVar1 + *(int *)(iVar2 + 0x60);
      param_2[3] = piVar1[1] - (*(int *)(iVar2 + 0x3c) - *(int *)(iVar2 + 0x10));
      goto LAB_0fb6a264;
    }
  }
  else {
    *param_2 = *(undefined4 *)(iVar2 + 0x18);
    param_2[1] = *(undefined4 *)(iVar2 + 0x10);
  }
  param_2[3] = 0;
  param_2[2] = 0;
LAB_0fb6a264:
  param_2[4] = 0;
                    /* WARNING: Could not recover jumptable at 0x0fb6a27c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE_00)();
  return;
}



// === call_fini @ 0fb6a2c0 (148 bytes) ===

undefined8 call_fini(int param_1)

{
  int iVar1;
  
  if (param_1 == 0) {
    return 0xffffffffffffffff;
  }
  iVar1 = *(int *)(param_1 + 0xec);
  if ((iVar1 != 0) && (*(char *)(param_1 + 0xc4) != '\0')) {
    if (verbose != '\0') {
      trace("call_fini: calling .fini section of \"%s\" -- 0x%lx\n",*(undefined4 *)(param_1 + 0x18),
            iVar1);
    }
    if (debug_trace != '\0') {
      trace("call_fini: calling .fini section of \"%s\" -- 0x%lx\n",*(undefined4 *)(param_1 + 0x18),
            iVar1);
    }
    fini_bridge(iVar1,has_pixified_init);
  }
  return 0xffffffffffffffff;
}



// === _rld_interface @ 0fb6a360 (56 bytes) ===

undefined8 _rld_interface(longlong param_1)

{
  if (param_1 == 0) {
    execute_all_fini_sections(0,0);
  }
  else {
    fatal("_rld_interface: bad operation: %ld\n");
  }
  return 0;
}



// === load_dependent_libs @ 0fb6a3a0 (740 bytes) ===

void load_dependent_libs(int param_1,undefined8 param_2)

{
  char cVar1;
  undefined1 uVar2;
  int iVar3;
  longlong lVar4;
  int iVar6;
  undefined8 uVar5;
  char *pcVar7;
  uint uVar8;
  int iVar9;
  int *piVar10;
  int *piVar11;
  int iVar13;
  int unaff_gp_lo;
  int *piVar14;
  int *piVar15;
  undefined1 *puVar16;
  int aiStack_4a0 [8];
  undefined1 auStack_420 [1028];
  int aiStack_1c [7];
  int iVar12;
  
  if (*(char *)(param_1 + 0xdc) == '\0') {
    piVar15 = aiStack_4a0;
    piVar10 = aiStack_4a0;
    piVar14 = aiStack_4a0;
    if (&stack0x00000000 == (undefined1 *)0x4a0) {
      fatal(unaff_gp_lo + -0x38f8,0);
      return;
    }
    aiStack_4a0[1] = 0;
    aiStack_4a0[0] = param_1;
    do {
      iVar3 = *piVar10;
      if (*(char *)(iVar3 + 0xdc) == '\0') {
        iVar6 = *(int *)(iVar3 + 0xb0);
        iVar9 = 0;
        iVar13 = 0;
        *(undefined1 *)(iVar3 + 0xdc) = 1;
        if (0 < iVar6) {
          cVar1 = *(char *)(unaff_gp_lo + -0x7ee0);
          iVar12 = *(int *)(iVar3 + 0x74);
          do {
            piVar11 = (int *)(iVar12 + iVar9);
            if (cVar1 == '\0') {
              uVar8 = 0;
              if ((piVar11[4] & 0x10U) == 0) {
                iVar6 = *piVar11;
                goto LAB_0fb6a4e0;
              }
            }
            else {
              uVar8 = piVar11[4] & 0x10;
              iVar6 = *piVar11;
LAB_0fb6a4e0:
              lVar4 = load_single_object(iVar6 + *(int *)(iVar3 + 0x60),
                                         piVar11[3] + *(int *)(iVar3 + 0x60),uVar8,1,param_2,
                                         aiStack_1c);
              if (lVar4 == 0) {
                objlist_add_end(unaff_gp_lo + -0x7f8c,aiStack_1c[0]);
                add_to_global_cleanup_record(aiStack_1c[0]);
                iVar6 = aiStack_1c[0];
                uVar2 = *(undefined1 *)(param_1 + 0xce);
                *(undefined1 *)(aiStack_1c[0] + 0xcd) = 1;
                *(undefined1 *)(aiStack_1c[0] + 0xce) = uVar2;
                if (*(char *)(unaff_gp_lo + -0x7ee6) != '\0') {
                  print_rld_object_list(unaff_gp_lo + -0x38c8,aiStack_1c[0]);
                }
                verify_o_arch_is_ok(*(undefined4 *)(unaff_gp_lo + -0x7f8c),aiStack_1c[0]);
                puVar16 = (undefined1 *)((int)piVar15 + -0x10);
                if (piVar15 == (int *)0x10) {
                  fatal(unaff_gp_lo + -0x38a0);
                }
                *(undefined4 *)((int)piVar15 + -0xc) = 0;
                *(int *)((int)piVar15 + -0x10) = iVar6;
                *(undefined1 **)((int)piVar14 + 4) = (undefined1 *)((int)piVar15 + -0x10);
                piVar14 = (int *)((int)piVar15 + -0x10);
LAB_0fb6a480:
                cVar1 = *(char *)(unaff_gp_lo + -0x7f48);
              }
              else {
                iVar6 = locate_obj_with_soname(*piVar11 + *(int *)(iVar3 + 0x60));
                if ((iVar6 == -1) && (piVar11[3] != 0)) {
                  uVar5 = add_version_to_dlopen_name
                                    (*piVar11 + *(int *)(iVar3 + 0x60),
                                     *(int *)(iVar3 + 0x60) + piVar11[3],auStack_420,0x400);
                  pcVar7 = strchr((char *)uVar5,0x2f);
                  iVar6 = locate_obj(uVar5,pcVar7 != (char *)0x0);
                }
                cVar1 = *(char *)(unaff_gp_lo + -0x7f48);
                puVar16 = (undefined1 *)piVar15;
                if (iVar6 != -1) {
                  if (*(char *)(iVar6 + 0xce) == '\0') {
                    *(undefined1 *)(iVar6 + 0xce) = *(undefined1 *)(param_1 + 0xce);
                  }
                  *(undefined1 *)(iVar6 + 0xcd) = 1;
                  goto LAB_0fb6a480;
                }
              }
              if ((cVar1 != '\0') && (iVar6 != -1)) {
                if (*(char *)(unaff_gp_lo + -0x7ee9) != '\0') {
                  trace(unaff_gp_lo + -0x3870,*(undefined4 *)(iVar6 + 0x18));
                }
                *(undefined1 *)(iVar6 + 0xd8) = 1;
              }
              iVar6 = *(int *)(iVar3 + 0xb0);
              piVar15 = (int *)puVar16;
            }
            iVar13 = iVar13 + 1;
            cVar1 = *(char *)(unaff_gp_lo + -0x7ee0);
            iVar9 = iVar9 + 0x14;
            if (iVar6 <= iVar13) break;
            iVar12 = *(int *)(iVar3 + 0x74);
          } while( true );
        }
      }
      piVar10 = (int *)piVar10[1];
    } while (piVar10 != (int *)0x0);
  }
  return;
}



// === free_up_msym_gotinfo_llmap @ 0fb6a690 (152 bytes) ===

void free_up_msym_gotinfo_llmap(int param_1)

{
  if (*(char *)(param_1 + 0xc1) != '\0') {
    free(*(undefined4 *)(param_1 + 0x68));
  }
  *(undefined4 *)(param_1 + 0x68) = 0;
  *(undefined1 *)(param_1 + 0xc1) = 0;
  if ((*(char *)(param_1 + 0xe3) != '\0') && (*(int *)(param_1 + 0x6c) != 0)) {
    free();
  }
  *(undefined4 *)(param_1 + 0x6c) = 0;
  if ((*(char *)(param_1 + 0xe2) != '\0') && (*(int *)(param_1 + 0xb4) != 0)) {
    free();
  }
  *(undefined4 *)(param_1 + 0xb4) = 0;
  if ((*(char *)(param_1 + 0xe0) != '\0') && (*(int *)(param_1 + 0x124) != 0)) {
    free();
  }
  *(undefined4 *)(param_1 + 0x124) = 0;
  return;
}



// === set_postcksum_then_handle_undefineds_and_conflicts @ 0fb6a730 (112 bytes) ===

void set_postcksum_then_handle_undefineds_and_conflicts(int param_1,undefined8 param_2)

{
  foreach_obj(*(undefined4 *)(param_1 + 8),set_postchksum_bit,param_2);
  if (user_tracking != '\0') {
    track("Quickstart disabled: set_postcksum_bit called due to dlopen or rld_modify_list_*\n");
  }
  all_fully_quickstarted = 0;
  if (user_tracking != '\0') {
    track("set_postchecksum_then* handle_undefineds when l_bind_now = %ld\n",param_2);
  }
  foreach_obj(pObj_Head,handle_undefineds,param_2);
  return;
}



// === unset_undefineds_and_reresolve @ 0fb6a7a0 (72 bytes) ===

void unset_undefineds_and_reresolve(void)

{
  int unaff_gp_lo;
  
  if (*(char *)(unaff_gp_lo + -0x7ee4) != '\0') {
    track(unaff_gp_lo + -0x37a8);
  }
  foreach_obj(*(undefined4 *)(unaff_gp_lo + -0x7f8c),unset_undef_are_resolved_for_dlopen_obj,0);
  foreach_obj(*(undefined4 *)(unaff_gp_lo + -0x7f8c),handle_undefineds,1);
  return;
}



// === do_oex_setting @ 0fb6a7f0 (116 bytes) ===

void do_oex_setting(undefined8 param_1,longlong param_2,undefined8 param_3)

{
  int iVar1;
  
  iVar1 = oex_global;
  init_done = 0;
  if (param_2 == 0) {
    oex_global = 0x1f00;
    foreach_obj(pObj_Head,apply_oex_flags,0);
  }
  else {
    apply_oex_flags();
  }
  if (oex_global != iVar1) {
    update_oex_state(param_3,iVar1);
  }
  return;
}



// === _rld_modify_list_with_version @ 0fb6a870 (2440 bytes) ===

undefined4
_rld_modify_list_with_version
          (longlong param_1,char *param_2,char *param_3,undefined *param_4,undefined8 param_5,
          undefined8 param_6,longlong param_7)

{
  int in_zero_lo;
  char *pcVar2;
  int iVar3;
  longlong lVar1;
  int *piVar4;
  char *pcVar5;
  undefined *puVar6;
  longlong lVar7;
  int iStack_80;
  undefined4 uStack_7c;
  longlong lStack_78;
  longlong lStack_70;
  longlong lStack_68;
  longlong lStack_60;
  longlong lStack_58;
  longlong lStack_30;
  
  if (debug_olist != '\0') {
    print_rld_object_list("enter modify_list_with_version");
  }
  if (param_1 == 0) {
    lStack_30 = 0;
    lVar7 = 0;
LAB_0fb6a928:
    *(undefined1 *)((int)lVar7 + 0xc6) = 1;
    if (save_error_message_for_dlerror == '\0') {
LAB_0fb6ad80:
      foreach_obj(pObj_Head,unset_undefineds_are_resolved,0);
    }
    else if (param_7 == 2) {
      if (lStack_30 == 0) {
LAB_0fb6ae1c:
        foreach_obj(pObj_Head,unset_undefineds_are_resolved,0);
      }
      else {
        foreach_obj(pObj_Head,unset_undefineds_are_resolved_if_has_unres_undefs,0);
      }
    }
    else if (param_7 == 3) {
      if (lStack_30 == 0) goto LAB_0fb6ae9c;
      foreach_obj(pObj_Head,unset_undefineds_are_resolved_if_has_unres_undefs,0);
    }
    else {
LAB_0fb6a948:
      if (param_1 == 3) {
        if (verbose != '\0') {
          trace("No short circuit delete, resetting undefs are res\n");
        }
        foreach_obj(pObj_Head,unset_undefineds_are_resolved,0);
      }
    }
  }
  else {
    if (param_1 == 1) {
      pcVar2 = strchr(param_2,0x2f);
      lVar7 = locate_obj(param_2,pcVar2 != (char *)0x0);
      if (lVar7 == -1) {
        fatal("locate_obj: Can\'t find obj with name %s\n",param_2);
        lVar7 = 0;
      }
      else if (pObj_Head == lVar7) {
        fatal("Inserting before MAIN is not allowed\n");
        lVar7 = 0;
      }
      else {
        lStack_68 = lVar7;
        pcVar2 = strchr(param_3,0x2f);
        lVar7 = locate_obj(param_3,pcVar2 != (char *)0x0);
        if ((lVar7 == -1) &&
           (lVar1 = load_single_object(param_3,param_4,param_5,0,param_6,&iStack_80), lVar1 == 0)) {
          objlist_add_before(&pObj_Head,lStack_68,iStack_80);
          lVar7 = (longlong)iStack_80;
          lVar1 = lVar7;
          if (debug_olist != '\0') {
            print_rld_object_list("list _RLD_OP_INSERT done");
            lVar1 = (longlong)iStack_80;
          }
          verify_o_arch_is_ok(pObj_Head,lVar1);
          if (0 < *(int *)(iStack_80 + 0xb0)) {
            lStack_70 = 0;
            lStack_78 = 0;
            do {
              piVar4 = (int *)(*(int *)(iStack_80 + 0x74) + (int)lStack_70);
              lVar1 = load_single_object(*piVar4 + *(int *)(iStack_80 + 0x60),
                                         piVar4[3] + *(int *)(iStack_80 + 0x60),0,1,param_6,
                                         &uStack_7c);
              if (lVar1 == 0) {
                objlist_add_end(&pObj_Head,uStack_7c);
                add_to_global_cleanup_record(uStack_7c);
                if (debug_olist != '\0') {
                  print_rld_object_list("list _RLD_OP_INSERT liblist entry done",uStack_7c);
                }
                verify_o_arch_is_ok(pObj_Head,uStack_7c);
              }
              lStack_78 = (longlong)((int)lStack_78 + 1);
              lStack_70 = (longlong)((int)lStack_70 + 0x14);
            } while (lStack_78 < *(int *)(iStack_80 + 0xb0));
          }
        }
      }
      lStack_30 = 0;
      goto LAB_0fb6a928;
    }
    if (param_1 == 2) {
      if (verbose != '\0') {
        if (param_7 == 2) {
          pcVar2 = "sgidladd";
          if (param_4 != (undefined *)0x0) goto LAB_0fb6ab3c;
          puVar6 = &DAT_0fbd7b18;
        }
        else {
          if (param_7 == 3) {
            pcVar2 = "dladd-from-dladd-liblist";
          }
          else if (param_7 == 4) {
            pcVar2 = "dladd-from-dlopen-liblist";
          }
          else {
            pcVar2 = "Internal rld Error";
            if (param_7 == 1) {
              pcVar2 = "dlopen";
            }
          }
          puVar6 = &DAT_0fbd7b18;
          if (param_4 != (undefined *)0x0) {
LAB_0fb6ab3c:
            puVar6 = param_4;
          }
        }
        trace("_RLD_OP_ADD (%s) is called with name = %s, version = %s l_flags=%x, libdl? %d\n",
              pcVar2,param_3,puVar6,param_5,save_error_message_for_dlerror);
      }
      if (interface != '\0') {
        trace("_rld_modify_list_with_version: _RLD_OP_ADD of %s\n",param_3);
      }
      if (param_2 == (char *)0x0) {
        pcVar2 = strchr(param_3,0x2f);
        lVar7 = locate_obj(param_3,pcVar2 != (char *)0x0);
        lStack_58 = lVar7;
        if (lVar7 == -1) {
          lVar1 = load_single_object(param_3,param_4,param_5,0,param_6,&iStack_80);
          if (lVar1 == 0) {
            objlist_add_end(&pObj_Head,iStack_80);
            add_to_global_cleanup_record(iStack_80);
            lVar7 = (longlong)iStack_80;
            lVar1 = lVar7;
            if (debug_olist != '\0') {
              print_rld_object_list("_RLD_OP_ADD name, not orig_pathname, done");
              lVar1 = (longlong)iStack_80;
            }
            verify_o_arch_is_ok(pObj_Head,lVar1);
            lStack_30 = 1;
          }
          else {
            lStack_30 = 0;
          }
        }
        else {
          lStack_30 = 0;
          iStack_80 = (int)lVar7;
        }
      }
      else {
        pcVar2 = strchr(param_2,0x2f);
        lStack_60 = locate_obj(param_2,pcVar2 != (char *)0x0);
        if (lStack_60 == -1) {
          fatal("locate_obj: Can\'t find obj with name %s\n",param_2);
          lStack_30 = 0;
          lVar7 = 0;
        }
        else {
          pcVar2 = strchr(param_3,0x2f);
          lVar7 = locate_obj(param_3,pcVar2 != (char *)0x0);
          if (lVar7 == -1) {
            lVar1 = load_single_object(param_3,param_4,param_5,0,param_6,&iStack_80);
            if (lVar1 == 0) {
              objlist_add_after(&pObj_Head,lStack_60,iStack_80);
              lVar7 = (longlong)iStack_80;
              lVar1 = lVar7;
              if (debug_olist != '\0') {
                print_rld_object_list("_RLD_OP_ADD B done");
                lVar1 = (longlong)iStack_80;
              }
              verify_o_arch_is_ok(pObj_Head,lVar1);
              lStack_30 = 1;
            }
            else {
              lStack_30 = 0;
            }
          }
          else {
            iStack_80 = (int)lVar7;
            lStack_30 = 0;
          }
        }
      }
      if (save_error_message_for_dlerror != '\0') {
        if (debug_trace != '\0') {
          trace("_RLD_OP_ADD set temp_unhidden on %s\n",*(undefined4 *)((int)lVar7 + 0x18));
        }
        *(undefined1 *)((int)lVar7 + 0xd8) = 1;
      }
      load_dependent_libs(lVar7,param_6);
      goto LAB_0fb6a928;
    }
    if (param_1 == 3) {
      if (verbose != '\0') {
        pcVar2 = "NULL";
        if (param_2 != (char *)0x0) {
          pcVar2 = param_2;
        }
        trace("_RLD_OP_DELETE to be doneon orig_pathname = %s, \n",pcVar2);
      }
      if (interface != '\0') {
        trace("_rld_modify_list_with_version: _RLD_OP_DELETE of %s\n",param_2);
      }
      pcVar2 = strchr(param_2,0x2f);
      lStack_58 = locate_obj(param_2,pcVar2 != (char *)0x0);
      if (lStack_58 == -1) {
        fatal("locate_obj: Can\'t find obj with name %s\n",param_2);
LAB_0fb6acfc:
        lVar7 = 0;
      }
      else {
        if (pObj_Head == lStack_58) {
          fatal("Deleting MAIN is not allowed\n");
          goto LAB_0fb6acfc;
        }
        free_up_msym_gotinfo_llmap(lStack_58);
        lVar7 = objlist_delete_element(&pObj_Head,lStack_58);
        mark_handles_on_obj_obsolete(lStack_58);
        if (verbose != '\0') {
          trace("_RLD_OP_DELETE done\n");
        }
        if (debug_olist != '\0') {
          print_rld_object_list("after _RLD_OP_DELETE done");
        }
      }
      if (verbose != '\0') {
        trace("_RLD_OP_DELETE (not unmap) complete on %s at 0x%lx\n",param_2,
              *(undefined4 *)((int)lStack_58 + 0x10));
      }
      *(undefined1 *)((int)lVar7 + 0xc6) = 1;
      if (save_error_message_for_dlerror != '\0') {
        if (param_7 != 2) {
          if (param_7 != 3) {
            lStack_30 = 0;
            goto LAB_0fb6a948;
          }
          lStack_30 = 0;
          goto LAB_0fb6ae9c;
        }
        lStack_30 = 0;
        goto LAB_0fb6ae1c;
      }
      lStack_30 = 0;
      goto LAB_0fb6ad80;
    }
    if (param_1 != 4) {
      fatal("_rld_modify_list_with_version: Unknown operation: %ld on %s and/or %s\n",param_1,
            param_2,param_3);
      *(undefined1 *)(in_zero_lo + 0xc6) = 1;
      if (save_error_message_for_dlerror == '\0') {
        lStack_30 = 0;
        lVar7 = 0;
        goto LAB_0fb6ad80;
      }
      if (param_7 == 2) {
        lStack_30 = 0;
        lVar7 = 0;
        goto LAB_0fb6ae1c;
      }
      if (param_7 == 3) {
        lStack_30 = 0;
        lVar7 = 0;
        goto LAB_0fb6ae9c;
      }
      lStack_30 = 0;
      lVar7 = 0;
      goto LAB_0fb6a948;
    }
    if (verbose != '\0') {
      pcVar2 = param_2;
      if (param_2 == (char *)0x0) {
        pcVar2 = "<junk>";
      }
      pcVar5 = param_3;
      if (param_3 == (char *)0x0) {
        pcVar5 = "<newname junk>";
      }
      trace("_RLD_OP_REPLACE %s with %s\n",pcVar2,pcVar5);
    }
    pcVar2 = strchr(param_2,0x2f);
    iVar3 = locate_obj(param_2,pcVar2 != (char *)0x0);
    if (iVar3 == -1) {
      fatal("locate_obj: Can\'t find obj with name %s\n",param_2);
      lVar7 = 0;
    }
    else if (pObj_Head == iVar3) {
      fatal("Replacing MAIN is not allowed\n");
      lVar7 = 0;
    }
    else {
      lVar7 = load_single_object(param_3,param_4,param_5,0,param_6,&iStack_80);
      if (lVar7 == 0) {
        objlist_replace_element(&pObj_Head,iVar3,iStack_80);
        mark_handles_on_obj_switched(iVar3,iStack_80);
        if (verbose != '\0') {
          trace("_RLD_OP_REPLACE done\n");
        }
        lVar7 = (longlong)iStack_80;
        lVar1 = lVar7;
        if (debug_olist != '\0') {
          print_rld_object_list("after _RLD_OP_REPLACE done");
          lVar1 = (longlong)iStack_80;
        }
        verify_o_arch_is_ok(pObj_Head,lVar1);
      }
      else {
        lVar7 = 0;
      }
    }
    *(undefined1 *)((int)lVar7 + 0xc6) = 1;
    if (save_error_message_for_dlerror == '\0') {
      lStack_30 = 0;
      goto LAB_0fb6ad80;
    }
    if (param_7 == 2) {
      lStack_30 = 0;
      goto LAB_0fb6ae1c;
    }
    if (param_7 != 3) {
      lStack_30 = 0;
      goto LAB_0fb6a948;
    }
    lStack_30 = 0;
LAB_0fb6ae9c:
    foreach_obj(pObj_Head,unset_undefineds_are_resolved,0);
  }
  set_postcksum_then_handle_undefineds_and_conflicts(lVar7,_rld_environment_ld_bind_now);
  if (finished_base_inits_so_must_call_init_individually != '\0') {
    foreach_obj(pObj_Head,call_pixie_init,0);
    __rld_calls_on_rld_list_change();
    if (param_1 != 3) {
      if ((param_2 == (char *)0x0) && (param_2 = "<unknown name>", param_3 != (char *)0x0)) {
        param_2 = param_3;
      }
      do_oex_setting(lVar7,lStack_30,param_2);
      execute_all_init_sections(0);
      cleanup_record_memory_free(&_rld_global_cleanup_record);
    }
  }
  iVar3 = (int)lVar7;
  if (_rld_environment_ld_bind_now == '\0') {
    if (save_error_message_for_dlerror == '\0') goto LAB_0fb6a9cc;
    unset_undefineds_and_reresolve();
  }
  if ((save_error_message_for_dlerror != '\0') && (param_1 == 2)) {
    if (param_7 == 4) {
      if (debug_trace != '\0') {
        trace("Failing to set o_is_hidden in %s to mirror old error\n",*(undefined4 *)(iVar3 + 0x18)
              ,0);
      }
    }
    else if ((param_7 != 2) && (param_7 != 3)) {
      foreach_obj(lVar7,set_hidden_bit,0);
    }
    foreach_obj(pObj_Head,unset_tmp_unhidden_bit,0);
  }
LAB_0fb6a9cc:
  __rld_calls_on_rld_list_change();
  if (interface != '\0') {
    trace("_rld_modify_list_with_version: current_obj = %s\n",*(undefined4 *)(iVar3 + 0x18));
  }
  return *(undefined4 *)(iVar3 + 0x18);
}



// === _rld_modify_list @ 0fb6b200 (84 bytes) ===

undefined8 _rld_modify_list(void)

{
  undefined8 uVar1;
  undefined1 *puStack_20;
  undefined1 uStack_1c;
  undefined4 uStack_18;
  undefined4 uStack_14;
  
  uStack_1c = 0;
  puStack_20 = &DAT_0fbd5820;
  uStack_14 = 0;
  uStack_18 = 0;
  uVar1 = _rld_modify_list_with_version();
  free_things_tried(&puStack_20);
  return uVar1;
}



// === unset_undefineds_are_resolved_if_has_unres_undefs @ 0fb6b260 (116 bytes) ===

undefined8 unset_undefineds_are_resolved_if_has_unres_undefs(int param_1)

{
  if (debug_trace != '\0') {
    trace("unset_undefineds_are_resolved_if_has_unres_undefs: on obj %s is hidden? %d  undefineds_are_resolved %d undef_data? %d\n"
          ,*(undefined4 *)(param_1 + 0x18),*(undefined1 *)(param_1 + 0xca),
          *(undefined1 *)(param_1 + 0xc4),*(undefined1 *)(param_1 + 0xc5));
  }
  if (((*(char *)(param_1 + 0xca) == '\0') && (*(char *)(param_1 + 0xc4) != '\0')) &&
     (*(char *)(param_1 + 0xc5) != '\0')) {
    *(undefined1 *)(param_1 + 0xc4) = 0;
    return 0xffffffffffffffff;
  }
  return 0xffffffffffffffff;
}



// === unset_undefineds_are_resolved @ 0fb6b2e0 (108 bytes) ===

undefined8 unset_undefineds_are_resolved(int param_1)

{
  char *pcVar1;
  
  if (debug_trace != '\0') {
    pcVar1 = "Unset";
    if (*(char *)(param_1 + 0xca) != '\0') {
      pcVar1 = "leave untouched";
    }
    trace("unset_undefineds_are_resolved: on obj %s is hidden? %d  undefineds_are_resolved %d: %s\n"
          ,*(undefined4 *)(param_1 + 0x18),*(char *)(param_1 + 0xca),*(undefined1 *)(param_1 + 0xc4)
          ,pcVar1);
  }
  if (*(char *)(param_1 + 0xca) != '\0') {
    return 0xffffffffffffffff;
  }
  *(undefined1 *)(param_1 + 0xc4) = 0;
  return 0xffffffffffffffff;
}



// === unset_undef_are_resolved_for_dlopen_obj @ 0fb6b350 (112 bytes) ===

undefined8 unset_undef_are_resolved_for_dlopen_obj(int param_1)

{
  char *pcVar1;
  
  if (debug_trace != '\0') {
    pcVar1 = "Unset";
    if (*(char *)(param_1 + 0xd8) == '\0') {
      pcVar1 = "leave untouched";
    }
    trace("unset_undef_are_resolved_for_dlopen_obj: on obj %s is hidden? %d  undefineds_are_resolved %d: %s\n"
          ,*(undefined4 *)(param_1 + 0x18),*(char *)(param_1 + 0xd8),*(undefined1 *)(param_1 + 0xc4)
          ,pcVar1);
  }
  if (*(char *)(param_1 + 0xd8) != '\0') {
    *(undefined1 *)(param_1 + 0xc4) = 0;
    return 0xffffffffffffffff;
  }
  return 0xffffffffffffffff;
}



// === set_postchksum_bit @ 0fb6b3c0 (64 bytes) ===

undefined8 set_postchksum_bit(int param_1)

{
  if (debug_trace != '\0') {
    trace("set_postchksum_bit: on obj %s set o_follows_checksum_has_changed 1\n",
          *(undefined4 *)(param_1 + 0x18));
  }
  *(undefined1 *)(param_1 + 199) = 1;
  return 0xffffffffffffffff;
}



// === set_hidden_bit @ 0fb6b400 (84 bytes) ===

undefined8 set_hidden_bit(int param_1)

{
  if (*(char *)(param_1 + 0xcb) != '\0') {
    return 0xffffffffffffffff;
  }
  if (debug_trace != '\0') {
    trace("set_hidden_bit: on obj %s\n",*(undefined4 *)(param_1 + 0x18));
  }
  *(undefined1 *)(param_1 + 0xca) = 1;
  return 0xffffffffffffffff;
}



// === unset_tmp_unhidden_bit @ 0fb6b460 (60 bytes) ===

undefined8 unset_tmp_unhidden_bit(int param_1)

{
  if (debug_trace != '\0') {
    trace("unset_tmp_unhidden_bit: on obj %s\n",*(undefined4 *)(param_1 + 0x18));
  }
  *(undefined1 *)(param_1 + 0xd8) = 0;
  return 0xffffffffffffffff;
}



// === set_obj_initial_bit @ 0fb6b4a0 (68 bytes) ===

undefined8 set_obj_initial_bit(int param_1)

{
  if (debug_trace != '\0') {
    trace("set_obj_initial_bit: on obj %s\n",*(undefined4 *)(param_1 + 0x18));
  }
  *(undefined1 *)(param_1 + 0xce) = 1;
  *(undefined1 *)(param_1 + 0xcb) = 1;
  return 0xffffffffffffffff;
}



// === commit_obj @ 0fb6b4f0 (64 bytes) ===

undefined8 commit_obj(int param_1)

{
  if (debug_trace != '\0') {
    trace("commit_obj: on obj %s\n",*(undefined4 *)(param_1 + 0x18));
  }
  *(undefined1 *)(param_1 + 0xd9) = 1;
  return 0xffffffffffffffff;
}



// === locate_obj_with_soname @ 0fb6b530 (20 bytes) ===

void locate_obj_with_soname(undefined8 param_1)

{
  foreach_sublist(pObj_Head,soname_matches,param_1);
  return;
}



// === locate_obj @ 0fb6b550 (52 bytes) ===

void locate_obj(undefined4 param_1,longlong param_2)

{
  undefined4 uStack_10;
  uint uStack_c;
  
  uStack_c = (uint)(param_2 == 0);
  uStack_10 = param_1;
  foreach_sublist(pObj_Head,pathname_matches,&uStack_10);
  return;
}



// === soname_matches @ 0fb6b590 (84 bytes) ===

int soname_matches(int param_1,longlong param_2)

{
  int iVar1;
  
  if (((param_2 != 0) && (*(char **)(param_1 + 0x30) != (char *)0x0)) &&
     (iVar1 = strcmp((char *)param_2,*(char **)(param_1 + 0x30)), iVar1 == 0)) {
    return param_1;
  }
  return -1;
}



// === pathname_matches @ 0fb6b5f0 (196 bytes) ===

int pathname_matches(int param_1,undefined4 *param_2)

{
  char *__s2;
  char *__s1;
  int iVar1;
  char *pcVar2;
  char *__s2_00;
  
  __s2 = *(char **)(param_1 + 0x18);
  __s1 = (char *)*param_2;
  if (param_2[1] == 0) {
    iVar1 = strcmp(__s1,__s2);
    if (iVar1 != 0) {
      return -1;
    }
    return param_1;
  }
  __s2_00 = *(char **)(param_1 + 0x38);
  if (__s2_00 == (char *)0x0) {
    pcVar2 = strrchr(__s2,0x2f);
    __s2_00 = __s2;
    if (pcVar2 != (char *)0x0) {
      __s2_00 = pcVar2 + 1;
    }
    *(char **)(param_1 + 0x38) = __s2_00;
  }
  iVar1 = strcmp(__s1,__s2_00);
  if (iVar1 != 0) {
    return -1;
  }
  return param_1;
}



// === find_reloc @ 0fb6b6c0 (708 bytes) ===

int find_reloc(int param_1,uint param_2)

{
  uint *puVar1;
  bool bVar2;
  longlong lVar3;
  uint uVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  
  if (rld_has_listed_symbols != '\0') {
    lVar3 = rld_is_listed_symbol
                      (*(int *)(*(int *)(param_1 + 100) + param_2 * 0x10) + *(int *)(param_1 + 0x60)
                      );
    if (lVar3 != 0) {
      debug("find_reloc: sym = %s, o = \"%s\" ignoring msym\n",
            *(int *)(*(int *)(param_1 + 100) + param_2 * 0x10) + *(int *)(param_1 + 0x60),
            *(undefined4 *)(param_1 + 0x18));
    }
  }
  if (debug_symbol != '\0') {
    debug("find_reloc: sym = %s, o = \"%s\" ignoring msym\n",
          *(int *)(*(int *)(param_1 + 100) + param_2 * 0x10) + *(int *)(param_1 + 0x60),
          *(undefined4 *)(param_1 + 0x18));
  }
  uVar4 = *(uint *)(param_1 + 0xa8) >> 3;
  iVar5 = uVar4 - 1;
  iVar9 = 0;
  if (0 < iVar5) {
    iVar7 = uVar4 - 2;
    iVar6 = iVar5;
    do {
      iVar5 = iVar5 >> 1;
      uVar4 = *(uint *)(*(int *)(param_1 + 0x70) + iVar5 * 8 + 4) >> 8;
      if (param_2 < uVar4) {
        bVar2 = iVar9 < iVar5;
        iVar6 = iVar5;
      }
      else {
        if (param_2 <= uVar4) goto LAB_0fb6b7ec;
        bVar2 = iVar5 < iVar6;
        iVar9 = iVar5;
        if (iVar5 == iVar7) {
          iVar5 = iVar6;
          if (param_2 != *(uint *)(*(int *)(param_1 + 0x70) + iVar6 * 8 + 4) >> 8) {
            iVar5 = 0;
          }
          goto LAB_0fb6b7ec;
        }
      }
      iVar7 = iVar6 + -1;
      iVar5 = iVar6 + iVar9;
    } while (bVar2);
  }
  iVar5 = 0;
LAB_0fb6b7ec:
  if (rld_has_listed_symbols != '\0') {
    lVar3 = rld_is_listed_symbol
                      (*(int *)(*(int *)(param_1 + 100) + param_2 * 0x10) + *(int *)(param_1 + 0x60)
                      );
    if (lVar3 != 0) {
      debug("find_reloc: sym = %s, o = \"%s\", reloc after first while = %ld\n",
            *(int *)(*(int *)(param_1 + 100) + param_2 * 0x10) + *(int *)(param_1 + 0x60),
            *(undefined4 *)(param_1 + 0x18),iVar5);
    }
  }
  if (debug_symbol != '\0') {
    debug("find_reloc: sym = %s, o = \"%s\", reloc after first while = %ld\n",
          *(int *)(*(int *)(param_1 + 100) + param_2 * 0x10) + *(int *)(param_1 + 0x60),
          *(undefined4 *)(param_1 + 0x18),iVar5);
  }
  iVar9 = iVar5;
  if (iVar5 != 0) {
    iVar6 = *(int *)(param_1 + 0x70);
    iVar7 = iVar6 + iVar5 * 8;
    while( true ) {
      iVar8 = iVar5 + -1;
      iVar9 = iVar5;
      if ((param_2 != *(uint *)(iVar7 + -4) >> 8) || (iVar6 == iVar7 + -8)) break;
      puVar1 = (uint *)(iVar7 + -0xc);
      iVar7 = iVar7 + -0x10;
      iVar5 = iVar5 + -2;
      iVar9 = iVar8;
      if ((param_2 != *puVar1 >> 8) || (iVar6 == iVar7)) break;
    }
  }
  if (rld_has_listed_symbols != '\0') {
    lVar3 = rld_is_listed_symbol
                      (*(int *)(*(int *)(param_1 + 100) + param_2 * 0x10) + *(int *)(param_1 + 0x60)
                      );
    if (lVar3 != 0) {
      debug("find_reloc: sym = %s, relindex = %ld\n",
            *(int *)(*(int *)(param_1 + 100) + param_2 * 0x10) + *(int *)(param_1 + 0x60),iVar9);
    }
  }
  if (debug_symbol != '\0') {
    debug("find_reloc: sym = %s, relindex = %ld\n",
          *(int *)(*(int *)(param_1 + 100) + param_2 * 0x10) + *(int *)(param_1 + 0x60),iVar9);
  }
  return iVar9;
}



// === FUN_0fb6b990 @ 0fb6b990 (172 bytes) ===

void FUN_0fb6b990(int param_1,uint param_2)

{
  uint uVar1;
  longlong lVar2;
  int iVar3;
  code *UNRECOVERED_JUMPTABLE;
  
  lVar2 = find_reloc();
  if (lVar2 == -1) {
                    /* WARNING: Could not recover jumptable at 0x0fb6ba34. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*UNRECOVERED_JUMPTABLE)();
    return;
  }
  iVar3 = *(int *)(param_1 + 0x70) + (int)lVar2 * 8;
  uVar1 = *(uint *)(iVar3 + 4);
  while( true ) {
    if (param_2 != uVar1 >> 8) {
                    /* WARNING: Could not recover jumptable at 0x0fb6ba1c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (*UNRECOVERED_JUMPTABLE)();
      return;
    }
    if ((uVar1 & 0xff) != 0x24) break;
    uVar1 = *(uint *)(iVar3 + 0xc);
    iVar3 = iVar3 + 8;
  }
                    /* WARNING: Could not recover jumptable at 0x0fb6b9f0. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
}



// === default_in_middle @ 0fb6ba40 (140 bytes) ===

void default_in_middle(undefined4 *param_1,longlong param_2)

{
  int iVar1;
  undefined4 *puVar2;
  code *UNRECOVERED_JUMPTABLE_00;
  
  if (0 < param_2) {
    puVar2 = param_1 + (int)param_2;
    do {
      iVar1 = strcmp((char *)*param_1,"DEFAULT");
      param_1 = param_1 + 1;
      if (iVar1 == 0) {
                    /* WARNING: Could not recover jumptable at 0x0fb6bac4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
        (*UNRECOVERED_JUMPTABLE_00)();
        return;
      }
    } while (param_1 != puVar2);
  }
                    /* WARNING: Could not recover jumptable at 0x0fb6baa4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE_00)();
  return;
}



// === print_obj @ 0fb6bad0 (52 bytes) ===

undefined8 print_obj(int param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4)

{
  if (debug_trace != '\0') {
    trace("print_obj: obj \"%s\" mapped at 0x%lx\n",*(undefined4 *)(param_1 + 0x18),
          *(undefined4 *)(param_1 + 0x10),param_4,param_1);
  }
  return 0xffffffffffffffff;
}



// === _rld_new_interface_body @ 0fb6bb10 (112 bytes) ===

longlong _rld_new_interface_body
                   (undefined4 param_1,int *param_2,undefined4 param_3,undefined4 param_4,
                   undefined4 param_5,undefined4 param_6)

{
  bool bVar1;
  int iVar2;
  longlong lVar3;
  int iVar4;
  char *pcVar5;
  int iVar6;
  int iVar7;
  int *piVar8;
  undefined1 *puStack_90;
  undefined1 uStack_8c;
  undefined4 uStack_88;
  undefined4 uStack_84;
  longlong lStack_80;
  longlong lStack_78;
  
  iVar2 = pObj_Head;
  switch(param_1) {
  case 0:
    execute_all_fini_sections();
    return 0;
  case 1:
    lVar3 = _rld_first_pathname();
    break;
  case 2:
    lVar3 = _rld_next_pathname();
    break;
  case 3:
    uStack_8c = 0;
    uStack_84 = 0;
    uStack_88 = 0;
    puStack_90 = &DAT_0fbd5820;
    lStack_80 = _rld_modify_list_with_version(param_2,param_3,param_4,0,0,&puStack_90,5);
    free_things_tried(&puStack_90);
    return lStack_80;
  case 4:
    lVar3 = _rld_address_to_name(param_2);
    return lVar3;
  case 5:
    lVar3 = _rld_name_to_address(param_2);
    break;
  case 6:
    lVar3 = __rld_libdl_interface(param_2,param_3,param_4,param_5,param_6);
    break;
  case 7:
    iVar6 = 0;
    iVar7 = *(int *)(pObj_Head + 0xb0);
    if (iVar7 == 0) {
LAB_0fb6beb8:
      lVar3 = 0;
    }
    else {
      iVar4 = *(int *)(pObj_Head + 0x74);
      while( true ) {
        piVar8 = (int *)(iVar4 + iVar6);
        iVar4 = strcmp((char *)(*piVar8 + *(int *)(iVar2 + 0x60)),(char *)param_2);
        iVar6 = iVar6 + 0x14;
        if (iVar4 == 0) break;
        if (iVar6 == iVar7 * 0x14) goto LAB_0fb6beb8;
        iVar4 = *(int *)(iVar2 + 0x74);
      }
      lVar3 = (longlong)(piVar8[3] + *(int *)(iVar2 + 0x60));
    }
    break;
  case 8:
    if (((init_done == '\0') && (pthreads_involved == '\0')) &&
       (finished_base_inits_so_must_call_init_individually == '\0')) {
      if (guarantee_start_init == '\0') {
        error(
             "sproc/nsproc/sprocsp() has been called prior to completion\nof init code, which is not supported.  Results may be unpredictable.\n"
             );
      }
      else {
        error(
             "sproc/nsproc/sprocsp() has been called prior to completion\nof init code.  This process\nhas requested RHF_GUARANTEE_INIT\nor RHF_GUARANTEE_START_INIT handling soresults may be unpredictable.\n"
             );
      }
    }
    if (multi_threaded == '\x01') {
      if (debug_threads != '\0') {
        pcVar5 = "pthreads active";
        if (pthreads_involved == '\0') {
          pcVar5 = "already multi_threaded!";
        }
        trace("_RLD_SPROC_NOTIFY ignored: %s\n",pcVar5);
      }
    }
    else {
      if (debug_threads != '\0') {
        trace("_RLD_SPROC_NOTIFY\n");
      }
      multi_threaded = '\x01';
      (*__test_and_set)(0xfbd3388,0);
      (*__test_and_set)(0xfbd3490,0);
      (*__test_and_set)(0xfbd3590,0);
      iVar2 = _rld_libdl_depth;
      mpinfo._132_4_ = 0;
      if ((inside_non_thread_libdl_critical_region != '\0') && (_rld_libdl_depth != 0)) {
        iVar7 = 0;
        do {
          enter_libdl(1);
          iVar7 = iVar7 + 1;
        } while (iVar2 != iVar7);
      }
    }
    return 0;
  case 9:
    lVar3 = locate_obj_with_soname(param_2);
    if (lVar3 != -1) {
      return (longlong)(*(int *)((int)lVar3 + 0x60) + *(int *)((int)lVar3 + 0x98));
    }
    lVar3 = 0;
    break;
  case 10:
    return 0;
  case 0xb:
    if ((multi_threaded != '\0') || (pthreads_involved != '\0')) {
      multi_threaded = '\0';
      pthreads_involved = '\0';
      (*__test_and_set)(0xfbd3388,0);
      (*__test_and_set)(0xfbd3490,0);
      (*__test_and_set)(0xfbd3590,0);
      mpinfo._132_4_ = 0;
    }
    if (debug_threads == '\0') {
      return 0;
    }
    trace("_RLD_SPROC_FINI encountered\n");
    return 0;
  case 0xc:
    if (debug_threads != '\0') {
      trace("_RLD_PTHREADS_START  interface: ptr 0x%lx\n",param_2);
    }
    if (pthreads_involved != '\0') {
      fatal("_RLD_PTHREADS_START invoked twice! Internal error in libpthread or in rld\n");
    }
    if (multi_threaded == '\0') {
      bVar1 = false;
    }
    else {
      bVar1 = true;
      if (pthreads_involved == '\0') {
        error("Since sproc(2) in use, pthreads will not work properly\n");
        bVar1 = true;
      }
    }
    if (*param_2 != 0) {
      error(
           "Starting pthreads, but pthreads interface version is %d not 0. Application may not work properly\n"
           );
    }
    pthreads_interface_data._4_4_ = param_2[1];
    pthreads_interface_data._8_4_ = param_2[2];
    pthreads_interface_data._12_4_ = param_2[3];
    (*__test_and_set)(0xfbd3388,0);
    (*__test_and_set)(0xfbd3490,0);
    (*__test_and_set)(0xfbd3590,0);
    mpinfo._132_4_ = 0;
    pthreads_involved = 1;
    multi_threaded = 1;
    if (bVar1) {
      mpinfo._132_4_ = 0;
      multi_threaded = 1;
      pthreads_involved = 1;
      return 0;
    }
    if (inside_non_thread_libdl_critical_region == '\0') {
      mpinfo._132_4_ = 0;
      multi_threaded = 1;
      pthreads_involved = 1;
      return 0;
    }
    lStack_78 = (longlong)_rld_libdl_depth;
    if (lStack_78 != 0) {
      lVar3 = 0;
      do {
        enter_libdl(1);
        lVar3 = (longlong)((int)lVar3 + 1);
      } while (lStack_78 != lVar3);
      return 0;
    }
    mpinfo._132_4_ = 0;
    multi_threaded = 1;
    pthreads_involved = 1;
    return 0;
  case 0xd:
    if (debug_threads == '\0') {
      return 0;
    }
    trace("_RLD_NOP encountered\n");
    return 0;
  case 0xe:
    lVar3 = __rld_dladdr(param_2,param_3);
    break;
  default:
    fatal("_rld_new_interface: bad operation: %ld\n",param_1);
    return 0;
  }
  return lVar3;
}



// === FUN_0fb6c0a0 @ 0fb6c0a0 (60 bytes) ===

undefined8 FUN_0fb6c0a0(int param_1,int param_2)

{
  if (param_1 == 0) {
    return 0;
  }
  if (*(char *)(*(int *)(param_1 + 100) + param_2 * 0x10 + 0xd) != '\x04') {
    return 0;
  }
  return 1;
}



// === free_things_tried @ 0fb6c0e0 (84 bytes) ===

void free_things_tried(undefined4 *param_1)

{
  if ((param_1 != (undefined4 *)0x0) && (*(char *)(param_1 + 1) != '\0')) {
    free(*param_1);
    *param_1 = 0;
    param_1[2] = 0;
    param_1[3] = 0;
    *(undefined1 *)(param_1 + 1) = 0;
    return;
  }
  return;
}



// === add_to_global_cleanup_record @ 0fb6c140 (188 bytes) ===

/* WARNING: Instruction at (ram,0x0fb6c188) overlaps instruction at (ram,0x0fb6c184)
    */

int add_to_global_cleanup_record(int param_1)

{
  uint uVar1;
  int iVar2;
  
  uVar1 = DAT_0fbd31d4;
  if (DAT_0fbd31d4 < _rld_global_cleanup_record) {
    *(int *)(DAT_0fbd31d8 + DAT_0fbd31d4 * 4) = param_1;
    DAT_0fbd31d4 = DAT_0fbd31d4 + 1;
    return param_1;
  }
  _rld_global_cleanup_record = _rld_global_cleanup_record * 2;
  if (DAT_0fbd31d8 == 0) {
    _rld_global_cleanup_record = 0x100;
  }
  iVar2 = realloc(DAT_0fbd31d8,_rld_global_cleanup_record << 2);
  if (iVar2 == 0) {
    fatal("rld out of memory allocating cleanup array of %lu entries\n",_rld_global_cleanup_record);
  }
  DAT_0fbd31d8 = iVar2;
  *(int *)(iVar2 + uVar1 * 4) = param_1;
  DAT_0fbd31d4 = DAT_0fbd31d4 + 1;
  return DAT_0fbd31d4;
}



// === cleanup_record_memory_free @ 0fb6c200 (64 bytes) ===

void cleanup_record_memory_free(undefined4 *param_1)

{
  if (param_1[2] != 0) {
    free(param_1[2]);
    param_1[1] = 0;
    *param_1 = 0;
    param_1[2] = 0;
    return;
  }
  return;
}



// === __rld_strlcpy @ 0fb6c240 (88 bytes) ===

void __rld_strlcpy(char *param_1,char *param_2,int param_3)

{
  char *pcVar1;
  char cVar2;
  
  cVar2 = *param_2;
  param_3 = param_3 + -1;
  if ((cVar2 != '\0') && (pcVar1 = param_1, param_3 != 0)) {
    while( true ) {
      *pcVar1 = cVar2;
      param_1 = pcVar1 + 1;
      if ((param_2[1] == '\0') || (param_3 == 1)) break;
      *param_1 = param_2[1];
      cVar2 = param_2[2];
      param_3 = param_3 + -2;
      param_1 = pcVar1 + 2;
      param_2 = param_2 + 2;
      if ((cVar2 == '\0') || (pcVar1 = param_1, param_3 == 0)) break;
    }
  }
  *param_1 = '\0';
  return;
}



// === __rld_strlcat @ 0fb6c2a0 (84 bytes) ===

void __rld_strlcat(char *param_1,undefined8 param_2,uint param_3)

{
  size_t sVar1;
  
  sVar1 = strlen(param_1);
  if (sVar1 + 1 < param_3) {
    __rld_strlcpy(param_1 + sVar1,param_2,param_3 - sVar1);
  }
  return;
}



// === _rld_alloc_obj_space @ 0fb6c310 (116 bytes) ===

int _rld_alloc_obj_space(int param_1,uint param_2,int param_3,undefined1 *param_4)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  
  uVar3 = (uint)*(ushort *)(param_1 + 0xe4);
  uVar2 = uVar3 & param_3 - 1U;
  if (uVar2 != 0) {
    uVar3 = (uint)*(ushort *)(param_1 + 0xe4) + (param_3 - uVar2);
  }
  if (param_2 <= 0x400 - uVar3) {
    *(short *)(param_1 + 0xe4) = (short)param_2 + (short)uVar3;
    *param_4 = 0;
    return param_1 + uVar3;
  }
  iVar1 = malloc(param_2);
  *param_4 = 1;
  return iVar1;
}



// === FUN_0fb6c390 @ 0fb6c390 (216 bytes) ===

void * FUN_0fb6c390(void *param_1)

{
  undefined1 uVar1;
  undefined1 uVar2;
  undefined2 uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  void *pvVar8;
  
  if (debug_trace != '\0') {
    trace("clean_obj_save_path: obj 0x%lx\n",param_1);
  }
  uVar1 = *(undefined1 *)((int)param_1 + 0xde);
  uVar4 = *(undefined4 *)((int)param_1 + 0x18);
  uVar5 = *(undefined4 *)((int)param_1 + 0x1c);
  uVar6 = *(undefined4 *)((int)param_1 + 0x38);
  uVar3 = *(undefined2 *)((int)param_1 + 0xe4);
  uVar7 = *(undefined4 *)((int)param_1 + 0x2c);
  uVar2 = *(undefined1 *)((int)param_1 + 0xdd);
  free_up_msym_gotinfo_llmap(param_1);
  FUN_0fb6c4d0(param_1);
  pvVar8 = memset(param_1,0,0x130);
  *(undefined4 *)((int)param_1 + 0x48) = 0xffffffff;
  *(undefined4 *)((int)param_1 + 0x40) = 0xffffffff;
  *(undefined4 *)((int)param_1 + 0x24) = 0xfffffffe;
  *(undefined4 *)((int)param_1 + 0x38) = uVar6;
  *(undefined1 *)((int)param_1 + 0xdd) = uVar2;
  *(undefined4 *)((int)param_1 + 0x2c) = uVar7;
  *(undefined1 *)((int)param_1 + 0xde) = uVar1;
  *(undefined4 *)((int)param_1 + 0x1c) = uVar5;
  *(undefined4 *)((int)param_1 + 0x18) = uVar4;
  *(undefined2 *)((int)param_1 + 0xe4) = uVar3;
  return pvVar8;
}



// === FUN_0fb6c470 @ 0fb6c470 (88 bytes) ===

void FUN_0fb6c470(void *param_1)

{
  if (debug_trace != '\0') {
    trace("clean_obj: obj 0x%lx\n",param_1);
  }
  memset(param_1,0,0x130);
  *(undefined4 *)((int)param_1 + 0x48) = 0xffffffff;
  *(undefined4 *)((int)param_1 + 0x40) = 0xffffffff;
  *(undefined4 *)((int)param_1 + 0x24) = 0xfffffffe;
  return;
}



// === FUN_0fb6c4d0 @ 0fb6c4d0 (100 bytes) ===

void FUN_0fb6c4d0(int param_1)

{
  int iVar1;
  int iVar2;
  
  iVar2 = *(int *)(param_1 + 0x108);
  while (iVar2 != 0) {
    iVar1 = *(int *)(iVar2 + 8);
    free(iVar2);
    iVar2 = iVar1;
  }
  iVar2 = *(int *)(param_1 + 0x10c);
  while (iVar2 != 0) {
    iVar2 = *(int *)(iVar2 + 0xc);
    free();
  }
  return;
}



// === delete_object @ 0fb6c540 (116 bytes) ===

undefined8 delete_object(int param_1)

{
  free_up_msym_gotinfo_llmap();
  if (*(char *)(param_1 + 0xdd) != '\0') {
    free(*(undefined4 *)(param_1 + 0x2c));
    *(undefined1 *)(param_1 + 0xdd) = 0;
    *(undefined4 *)(param_1 + 0x2c) = 0;
  }
  if (*(char *)(param_1 + 0xde) != '\0') {
    free(*(undefined4 *)(param_1 + 0x18));
    *(undefined1 *)(param_1 + 0xde) = 0;
    *(undefined4 *)(param_1 + 0x38) = 0;
    *(undefined4 *)(param_1 + 0x18) = 0;
  }
  FUN_0fb6c4d0(param_1);
  free(param_1);
  return 0;
}



// === create_object @ 0fb6c5c0 (268 bytes) ===

int create_object(undefined8 param_1,longlong param_2)

{
  int iVar2;
  size_t sVar3;
  undefined8 uVar1;
  
  if (debug_trace != '\0') {
    trace("create_object: name %s was malloced? %d\n",param_1,param_2);
  }
  iVar2 = malloc(0x400);
  if (iVar2 == 0) {
    fatal("Unable to malloc space for object record\n");
  }
  if (debug_map != '\0') {
    debug("create_object: obj at 0x%lx\n",iVar2);
  }
  FUN_0fb6c470(iVar2);
  *(undefined2 *)(iVar2 + 0xe4) = 0x130;
  dumpobj(iVar2);
  if (param_2 == 0) {
    *(undefined **)(iVar2 + 0x18) = &DAT_0fbd86b8;
    *(char **)(iVar2 + 0x2c) = (char *)param_1;
    *(undefined1 *)(iVar2 + 0xdd) = 0;
    return iVar2;
  }
  sVar3 = strlen((char *)param_1);
  uVar1 = _rld_alloc_obj_space(iVar2,sVar3 + 1,1,iVar2 + 0xdd);
  __rld_strlcpy(uVar1,param_1,sVar3 + 1);
  *(undefined **)(iVar2 + 0x18) = &DAT_0fbd86b8;
  *(int *)(iVar2 + 0x2c) = (int)uVar1;
  return iVar2;
}



// === FUN_0fb6c6d0 @ 0fb6c6d0 (376 bytes) ===

undefined8
FUN_0fb6c6d0(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,
            undefined4 *param_5,undefined8 param_6,undefined8 param_7,undefined4 *param_8)

{
  longlong lVar1;
  undefined8 uVar2;
  int iVar3;
  code *UNRECOVERED_JUMPTABLE;
  
  __rld_strlcpy(param_6,param_3,param_7);
  __rld_strlcat(param_6,param_2,param_7);
  __rld_strlcat(param_6,&DAT_0fbd86bc,param_7);
  __rld_strlcat(param_6,param_1,param_7);
  if ((*(char *)*param_5 != '\0') &&
     (lVar1 = found_matching_string((char *)*param_5,param_6), lVar1 != 0)) {
                    /* WARNING: Could not recover jumptable at 0x0fb6c76c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    uVar2 = (*UNRECOVERED_JUMPTABLE)();
    return uVar2;
  }
  add_a_tried_path(param_5,param_6);
  if (debug_map != '\0') {
    debug("found_dso_using_path: trying constructed path with LD_LIBRARY_PATH %s\n",param_6);
  }
  iVar3 = strcmp((char *)param_6,pt_interp_name);
  if ((iVar3 == 0) && (we_are_ldd == '\0')) {
    if (execrld != '\0') {
      debug("%s Has Been mapped in through EXEC, addr = 0x%lx\n",param_6,rld_mapped_addr);
    }
    *param_8 = rld_mapped_addr;
    return 1;
  }
  lVar1 = elfmap(param_6,0,5,param_4);
  if (lVar1 == -1) {
    if (debug_map != '\0') {
      debug("MIPS_EXECMAP -- can\'t map %s ",param_6);
    }
    uVar2 = 0;
  }
  else {
    uVar2 = 1;
    *param_8 = (int)lVar1;
  }
  return uVar2;
}



// === FUN_0fb6c850 @ 0fb6c850 (1488 bytes) ===

int FUN_0fb6c850(undefined8 param_1,char *param_2,undefined8 param_3,undefined8 param_4,int *param_5
                )

{
  char cVar1;
  undefined4 uVar2;
  int iVar3;
  char *pcVar6;
  size_t sVar7;
  longlong lVar4;
  longlong lVar5;
  undefined *puVar8;
  int iVar9;
  char *__s;
  undefined **ppuVar10;
  undefined4 *puVar11;
  undefined4 *puVar12;
  undefined4 *puVar13;
  undefined4 *puVar14;
  undefined1 *puVar15;
  undefined1 *puVar16;
  code *UNRECOVERED_JUMPTABLE;
  undefined4 auStack_e0 [2];
  longlong lStack_98;
  int iStack_44;
  int iStack_40;
  undefined1 auStack_3c [60];
  
  puVar13 = auStack_e0;
  if (debug_map != '\0') {
    pcVar6 = ld_path;
    if (ld_path == (char *)0x0) {
      pcVar6 = "";
    }
    debug("attempt_map_on_paths: name %s , global_path_prefix %s, ld_path %s\n",param_1,
          global_path_prefix,pcVar6);
    if (debug_map != '\0') {
      debug("attempt_map_on_paths: name %s , global_library_search_rpath %s\n",param_1,
            global_library_search_rpath);
    }
  }
  __s = (char *)param_1;
  pcVar6 = strchr(__s,0x2f);
  if (pcVar6 != (char *)0x0) {
    sVar7 = strlen(__s);
    if (param_5[2] + sVar7 + 3 < (uint)param_5[3]) {
      cVar1 = *param_2;
    }
    else {
      tried_path_grow(param_5,sVar7 + 3);
      cVar1 = *param_2;
    }
    if ((cVar1 == '\0') && (*(char *)*param_5 == '\0')) {
      strcpy((char *)*param_5,__s);
    }
    else {
      lVar4 = found_matching_string(param_2,param_1);
      if (lVar4 != 0) {
        return -1;
      }
      strcat((char *)*param_5,":");
      iVar9 = param_5[2];
      param_5[2] = iVar9 + 1;
      strcpy((char *)(*param_5 + iVar9 + 1),__s);
      param_5[2] = param_5[2] + sVar7;
    }
    iVar9 = strcmp(__s,pt_interp_name);
    if ((iVar9 == 0) && (we_are_ldd == '\0')) {
      if (execrld != '\0') {
        debug("%s has been mapped in through EXEC, addr = 0x%lx\n",param_1,rld_mapped_addr);
      }
      __rld_strlcpy(param_2,param_1,param_3);
      return rld_mapped_addr;
    }
    if (debug_map != '\0') {
      debug("attempt_map_on_paths: trying name as given (has slashes, so no env vars used to look for name) %s\n"
            ,param_1);
    }
    iVar9 = elfmap(param_1,0,5,param_4);
    if (iVar9 == -1) {
      if (debug_map != '\0') {
        debug("elfmap -- can\'t map %s ",param_1);
      }
      return -1;
    }
    __rld_strlcpy(param_2,param_1,param_3);
    return iVar9;
  }
  iVar9 = strcmp(global_path_prefix,"");
  if (iVar9 != 0) {
    sVar7 = strlen(global_path_prefix);
    iVar9 = -(sVar7 * 4 + 0x13 & 0xfffffff0);
    puVar14 = (undefined4 *)((int)auStack_e0 + iVar9);
    iVar3 = -(sVar7 + 0x10 & 0xfffffff0);
    puVar13 = (undefined4 *)((int)auStack_e0 + iVar3 + iVar9);
    puVar15 = (undefined1 *)((int)auStack_e0 + iVar3 + iVar9);
    lStack_98 = split(global_path_prefix,(int)auStack_e0 + iVar9,(int)auStack_e0 + iVar3 + iVar9,
                      sVar7 + 1,&DAT_0fbd87e8,0);
    if (lStack_98 != 0) goto LAB_0fb6ca04;
  }
  lStack_98 = 1;
  puVar14 = (undefined4 *)((int)puVar13 + -0x10);
  puVar15 = (undefined1 *)((int)puVar13 + -0x10);
  *(undefined **)((int)puVar13 + -0x10) = &DAT_0fbd86b8;
LAB_0fb6ca04:
  puVar16 = puVar15;
  if (global_library_search_rpath != (char *)0x0) {
    sVar7 = strlen(global_library_search_rpath);
    iVar9 = -(sVar7 * 4 + 0x13 & 0xfffffff0);
    iVar3 = -(sVar7 + 0x10 & 0xfffffff0);
    lVar4 = split(global_library_search_rpath,(int)puVar15 + iVar9,puVar15 + iVar3 + iVar9,sVar7 + 1
                  ,&DAT_0fbd87e8,0);
    puVar16 = puVar15 + iVar3 + iVar9;
    if (0 < lStack_98) {
      puVar12 = puVar14;
      do {
        uVar2 = *puVar12;
        puVar11 = (undefined4 *)(puVar15 + iVar9);
        if (0 < lVar4) {
          do {
            lVar5 = FUN_0fb6c6d0(param_1,*puVar11,uVar2,param_4,param_5,param_2,param_3,auStack_3c);
            puVar11 = puVar11 + 1;
            if (lVar5 != 0) {
                    /* WARNING: Could not recover jumptable at 0x0fb6cce4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
              iVar9 = (*UNRECOVERED_JUMPTABLE)();
              return iVar9;
            }
          } while (puVar11 != (undefined4 *)(puVar15 + (int)lVar4 * 4 + iVar9));
        }
        puVar12 = puVar12 + 1;
        puVar16 = puVar15 + iVar3 + iVar9;
      } while (puVar12 != puVar14 + (int)lStack_98);
    }
  }
  if (debug_map != '\0') {
    pcVar6 = ld_path;
    if (ld_path == (char *)0x0) {
      pcVar6 = "none";
    }
    debug("attempt_map_on_paths: LD_LIBRARY_PATH %s\n",pcVar6);
  }
  if (ld_path != (char *)0x0) {
    sVar7 = strlen(ld_path);
    iVar9 = -(sVar7 * 4 + 0x13 & 0xfffffff0);
    puVar12 = (undefined4 *)(puVar16 + iVar9);
    lVar4 = split(ld_path,(int)puVar16 + iVar9,puVar16 + (iVar9 - (sVar7 + 0x10 & 0xfffffff0)),
                  sVar7 + 1,&DAT_0fbd88e0,0);
    if (0 < lVar4) {
      do {
        lVar5 = FUN_0fb6c6d0(param_1,*puVar12,&DAT_0fbd86b8,param_4,param_5,param_2,param_3,
                             &iStack_40);
        puVar12 = puVar12 + 1;
        if (lVar5 != 0) {
          return iStack_40;
        }
      } while (puVar12 < puVar16 + (int)lVar4 * 4 + iVar9);
    }
  }
  if (0 < lStack_98) {
    puVar12 = puVar14 + (int)lStack_98;
    do {
      uVar2 = *puVar14;
      if (lib_path != (undefined *)0x0) {
        ppuVar10 = &lib_path;
        puVar8 = lib_path;
        do {
          lVar4 = FUN_0fb6c6d0(param_1,puVar8,uVar2,param_4,param_5,param_2,param_3,&iStack_44);
          if (lVar4 != 0) {
            return iStack_44;
          }
          ppuVar10 = ppuVar10 + 1;
        } while ((ppuVar10 != (undefined **)0x0) && (puVar8 = *ppuVar10, puVar8 != (undefined *)0x0)
                );
      }
      puVar14 = puVar14 + 1;
    } while (puVar14 != puVar12);
  }
  return -1;
}



// === FUN_0fb6ce20 @ 0fb6ce20 (1104 bytes) ===

undefined8 FUN_0fb6ce20(int param_1,int param_2,int param_3)

{
  undefined8 uVar1;
  char *pcVar3;
  size_t sVar4;
  int iVar5;
  size_t sVar6;
  longlong lVar2;
  int *piVar7;
  char cVar8;
  byte bVar9;
  byte bVar10;
  byte *pbVar11;
  char *__s;
  char *pcVar12;
  
  if (debug_map != '\0') {
    debug("trying to add version to name %s\n",
          *(int *)(*(int *)(param_2 + 0x74) + param_3 * 0x14) + *(int *)(param_2 + 0x60));
  }
  piVar7 = (int *)(*(int *)(param_2 + 0x74) + param_3 * 0x14);
  if (piVar7[3] == 0) {
    uVar1 = 0;
    if (debug_map != '\0') {
      debug("version search suppressed because object %s in liblist has no interface version...\n",
            *piVar7 + *(int *)(param_2 + 0x60));
      uVar1 = 0;
    }
  }
  else {
    pcVar12 = (char *)(*(int *)(param_2 + 0x60) + piVar7[3]);
    pcVar3 = strchr(pcVar12,0x23);
    if (pcVar3 != (char *)0x0) {
      pcVar12 = pcVar3 + 1;
    }
    cVar8 = *pcVar12;
    pcVar3 = pcVar12;
    if (cVar8 != '\0') {
      while ((cVar8 == ' ' || (pcVar3 = pcVar12, cVar8 == '\t'))) {
        cVar8 = pcVar12[1];
        pcVar3 = pcVar12 + 1;
        if ((cVar8 == '\0') || ((cVar8 != ' ' && (cVar8 != '\t')))) break;
        cVar8 = pcVar12[2];
        pcVar3 = pcVar12 + 2;
        pcVar12 = pcVar3;
        if (cVar8 == '\0') break;
      }
    }
    sVar4 = strlen(pcVar3);
    if (sVar4 == 0) {
      if (debug_map != '\0') {
        debug("version search suppressed because object %s in liblist has nothing in interface version...\n"
              ,*piVar7 + *(int *)(param_2 + 0x60));
      }
      return 0;
    }
    iVar5 = strncmp(pcVar3,"sgi",3);
    if (iVar5 != 0) {
      warning("Version Search Suppressed in %s Because Object %s in liblist has non-sgi interface version (%s)\n"
              ,*(undefined4 *)(param_2 + 0x18),*piVar7 + *(int *)(param_2 + 0x60),pcVar3);
      return 0;
    }
    iVar5 = -(sVar4 + 0x10 & 0xfffffff0);
    strcpy(&stack0xffffff80 + iVar5,pcVar3);
    __s = &stack0xffffff83 + iVar5;
    pcVar12 = strchr(__s,0x2e);
    if ((pcVar12 == (char *)0x0) || (pcVar12 == __s)) {
      warning("Version Search Suppressed in %s Because version (%s) of object %s in liblist is not an sgi interface version.\n"
              ,*(undefined4 *)(param_2 + 0x18),pcVar3,*piVar7 + *(int *)(param_2 + 0x60));
      uVar1 = 0;
    }
    else {
      bVar10 = pcVar12[1];
      pbVar11 = (byte *)(pcVar12 + 1);
      *pcVar12 = '\0';
      if ((0x2f < bVar10) && (bVar10 < 0x3a)) {
        bVar9 = pcVar12[1];
        if (bVar9 == 0) {
          bVar10 = 0;
        }
        else {
          do {
            if ((bVar9 < 0x30) || (0x39 < bVar9)) {
              bVar10 = *pbVar11;
              break;
            }
            bVar9 = pbVar11[1];
            pbVar11 = pbVar11 + 1;
            bVar10 = 0;
          } while (bVar9 != 0);
        }
        for (; (bVar10 != 0 &&
               ((((bVar10 == 0x20 || (bVar10 == 9)) && (bVar10 = pbVar11[1], bVar10 != 0)) &&
                ((bVar10 == 0x20 || (bVar10 == 9)))))); pbVar11 = pbVar11 + 2) {
          bVar10 = pbVar11[2];
        }
      }
      if (bVar10 == 0) {
        bVar10 = (&stack0xffffff83)[iVar5];
        if (bVar10 != 0) {
          while ((0x2f < bVar10 && (bVar10 < 0x3a))) {
            bVar10 = __s[1];
            if ((bVar10 == 0) || ((bVar10 < 0x30 || (0x39 < bVar10)))) break;
            bVar10 = __s[2];
            __s = __s + 2;
            if (bVar10 == 0) break;
          }
        }
        if (bVar10 != 0) {
          warning("Version Search suppressed in %s because version (%s) of object %s in liblist is not an sgi interface version.\n"
                  ,*(undefined4 *)(param_2 + 0x18),pcVar3,*piVar7 + *(int *)(param_2 + 0x60));
          return 0;
        }
        (&stack0xffffff82)[iVar5] = 0x2e;
        sVar4 = strlen(*(char **)(param_1 + 0x2c));
        sVar6 = strlen(&stack0xffffff82 + iVar5);
        lVar2 = malloc(sVar4 + sVar6 + 1);
        if (lVar2 == 0) {
          fatal("add_version_to_o_name:cannot malloc new obj name\n");
        }
        pcVar12 = (char *)lVar2;
        strcpy(pcVar12,*(char **)(param_1 + 0x2c));
        strcat(pcVar12,&stack0xffffff82 + iVar5);
        uVar1 = 1;
        *(undefined1 *)(param_1 + 0xdd) = 1;
        *(char **)(param_1 + 0x2c) = pcVar12;
      }
      else {
        warning("Version search suppressed in %s because version (%s) of object %s in liblist is not an sgi interface version.\n"
                ,*(undefined4 *)(param_2 + 0x18),pcVar3,*piVar7 + *(int *)(param_2 + 0x60));
        uVar1 = 0;
      }
    }
  }
  return uVar1;
}



// === FUN_0fb6d270 @ 0fb6d270 (336 bytes) ===

bool FUN_0fb6d270(int param_1,int param_2,undefined4 *param_3,char *param_4,int param_5)

{
  ssize_t sVar2;
  size_t sVar3;
  longlong lVar1;
  size_t sVar4;
  char *pcVar5;
  
  if (param_2 == 0) {
    if (interact != '\0') {
      trace("**** Runtime linker cannot find a \"%s\"  ****\n",*(undefined4 *)(param_1 + 0x2c));
    }
  }
  else if (interact != '\0') {
    pcVar5 = *(char **)(param_2 + 0x2c);
    if (pcVar5 == (char *)0x0) {
      pcVar5 = "<unknownname>";
    }
    trace("**** Runtime linker cannot find a \"%s\" whose version matches the one used in \"%s\" ****\n"
          ,*(undefined4 *)(param_1 + 0x2c),pcVar5);
  }
  if (interact != '\0') {
    trace("Please input another \"%s\" with its full path or just hit RETURN:\n",
          *(undefined4 *)(param_1 + 0x2c));
  }
  *param_4 = '\0';
  sVar2 = read(0,param_4,param_5 - 1);
  if (sVar2 == -1) {
    return (bool)2;
  }
  param_4[sVar2] = '\0';
  sVar3 = strlen(param_4);
  sVar4 = sVar3;
  if (param_4[sVar3 - 1] == '\n') {
    sVar4 = sVar3 - 1;
    param_4[sVar3 - 1] = '\0';
  }
  if (sVar4 == 0) {
    return (bool)2;
  }
  lVar1 = elfmap(param_4,0,5,param_1);
  if (lVar1 != -1) {
    *param_3 = (int)lVar1;
  }
  return lVar1 == -1;
}



// === FUN_0fb6d3c0 @ 0fb6d3c0 (164 bytes) ===

void FUN_0fb6d3c0(int param_1,undefined8 param_2,int param_3,longlong param_4)

{
  char *__dest;
  
  if (*(char *)(param_1 + 0xde) != '\0') {
    free(*(undefined4 *)(param_1 + 0x18));
    *(undefined4 *)(param_1 + 0x18) = 0;
    *(undefined1 *)(param_1 + 0xde) = 0;
  }
  __dest = (char *)_rld_alloc_obj_space(param_1,param_3 + 1,1,param_1 + 0xde);
  strcpy(__dest,(char *)param_2);
  *(undefined4 *)(param_1 + 0x38) = 0;
  *(char **)(param_1 + 0x18) = __dest;
  *(int *)(param_1 + 0x1c) = (int)(short)param_3;
  if (param_4 != 0) {
    free(param_2);
  }
  return;
}



// === map_object_into_mem_and_init_object_info @ 0fb6d470 (2464 bytes) ===

undefined8
map_object_into_mem_and_init_object_info
          (int param_1,int param_2,undefined8 param_3,longlong param_4,longlong param_5,
          undefined4 param_6,longlong param_7,undefined4 *param_8)

{
  uint uVar1;
  bool bVar2;
  bool bVar3;
  size_t sVar7;
  longlong lVar4;
  ulonglong uVar5;
  __uid_t _Var8;
  undefined8 uVar6;
  int iVar9;
  undefined *puVar10;
  undefined *puVar11;
  int iVar12;
  int *piVar13;
  char *pcVar14;
  longlong lVar15;
  code *UNRECOVERED_JUMPTABLE;
  char acStack_14a0 [4096];
  int aiStack_4a0 [2];
  char acStack_498 [1040];
  
  acStack_14a0[0] = '\0';
  piVar13 = (int *)0x0;
  if (param_2 != 0) {
    piVar13 = (int *)0x0;
    if (*(int *)(param_2 + 0x74) != 0) {
      piVar13 = (int *)(*(int *)(param_2 + 0x74) + (int)param_3 * 0x14);
    }
  }
  if (debug_trace != '\0') {
    trace("map_object_into_mem_and_init_object_info: o 0x%lx, name %s\n",param_1,
          *(undefined4 *)(param_1 + 0x18));
  }
  if (execrld != '\0') {
    debug("((o)->o_name)= %s pt_interp_name = %s\n",*(undefined4 *)(param_1 + 0x2c),pt_interp_name);
  }
  if (param_8[2] != 0) {
    param_8[2] = 0;
    *(undefined1 *)*param_8 = 0;
  }
  iVar12 = *(int *)(param_1 + 0x34);
  if (iVar12 == 0) {
    if (*(char *)(param_1 + 0xc3) == '\0') {
      lVar15 = 2;
    }
    else {
      sVar7 = strlen(*(char **)(param_1 + 0x2c));
      FUN_0fb6d3c0(param_1,*(undefined4 *)(param_1 + 0x2c),sVar7,0);
      lVar15 = 1;
      *(undefined4 *)(param_1 + 0x34) = param_6;
    }
  }
  else {
    if (debug_map != '\0') {
      debug("map_object_into_mem_and_init_object_info: Mapping main object with fd = %ld\n");
      iVar12 = *(int *)(param_1 + 0x34);
    }
    aiStack_4a0[0] = elfmap(0,iVar12,4,param_1);
    if (aiStack_4a0[0] == -1) {
      if (debug_map != '\0') {
        debug("elfmap -- can\'t map Main with fd %ld",*(undefined4 *)(param_1 + 0x34));
      }
      fatal("Unable to map main object.\n");
    }
    if (debug_map != '\0') {
      debug("map_object_into_mem_and_init_object_info: mapped Main at 0x%lx\n");
    }
    *(undefined1 *)(param_1 + 0xc3) = 1;
    *(int *)(param_1 + 0x10) = aiStack_4a0[0];
    sVar7 = strlen(*(char **)(param_1 + 0x2c));
    FUN_0fb6d3c0(param_1,*(undefined4 *)(param_1 + 0x2c),sVar7,0);
    lVar15 = 0;
  }
  if (debug_malloc != '\0') {
    debug("tried_path malloc\'ed, addr = 0x%lx\n",*param_8);
  }
  iVar12 = 0;
  bVar2 = false;
  bVar3 = false;
  do {
    if (lVar15 != 2) {
LAB_0fb6d62c:
      if (execrld != '\0') {
        debug("Entering obj_init \n");
      }
      obj_init(param_1);
      dumpobj(param_1);
      if (execrld != '\0') {
        debug("Exiting obj_init \n");
      }
      if (param_2 == 0) {
joined_r0x0fb6d91c:
        pcVar14 = "";
        if (verbose != '\0') {
LAB_0fb6d924:
          trace("mapped %s at 0x%lx %s\n",*(undefined4 *)(param_1 + 0x18),
                *(undefined4 *)(param_1 + 0x10),pcVar14);
        }
      }
      else {
        lVar15 = FUN_0fb6de30(param_1,param_2,piVar13);
        if ((lVar15 != 0) ||
           (lVar15 = match_interface_version(param_1,param_2,piVar13), lVar15 == 0)) {
          if (verbose != '\0') {
            trace("failed version match %s ver %s against req ver %s\n",
                  *(undefined4 *)(param_1 + 0x18),
                  *(int *)(param_1 + 0x60) + *(int *)(param_1 + 0x98),
                  piVar13[3] + *(int *)(param_2 + 0x60));
          }
          lVar15 = elfunmap(param_1);
          aiStack_4a0[0] = (int)lVar15;
          if (lVar15 == -1) {
            error("elfunmap -- can\'t unmap text of %s ",*(undefined4 *)(param_1 + 0x18));
          }
          else if (debug_map != '\0') {
            debug("map_object_into_mem_and_init_object_info: Unmapped %s successfully \n",
                  *(undefined4 *)(param_1 + 0x18));
          }
          FUN_0fb6c390(param_1);
          bVar2 = true;
          lVar15 = 2;
          goto LAB_0fb6d618;
        }
        lVar15 = FUN_0fb6de30(param_1,param_2,piVar13);
        if (lVar15 != 0) {
          lVar15 = match_interface_version(param_1,param_2,piVar13);
          if (lVar15 == 0) {
            if (verbose != '\0') {
              trace("Impossible version case: match fails %s ver %s against req ver %s\n",
                    *(undefined4 *)(param_1 + 0x18),
                    *(int *)(param_1 + 0x60) + *(int *)(param_1 + 0x98),
                    piVar13[3] + *(int *)(param_2 + 0x60));
            }
          }
          else if (verbose != '\0') {
            trace("version match  passed %s ver %s against req ver %s\n",
                  *(undefined4 *)(param_1 + 0x18),
                  *(int *)(param_1 + 0x60) + *(int *)(param_1 + 0x98),
                  piVar13[3] + *(int *)(param_2 + 0x60));
          }
          goto joined_r0x0fb6d91c;
        }
        if (verbose != '\0') {
          pcVar14 = " exact same DSO as in liblist ";
          goto LAB_0fb6d924;
        }
      }
      if (user_tracking != '\0') {
        track("mapped %s at 0x%lx\n",*(undefined4 *)(param_1 + 0x18),*(undefined4 *)(param_1 + 0x10)
             );
      }
      if (iVar12 != 5000) goto LAB_0fb6d964;
      break;
    }
    aiStack_4a0[0] =
         FUN_0fb6c850(*(undefined4 *)(param_1 + 0x2c),acStack_14a0,0x1000,param_1,param_8);
    if ((aiStack_4a0[0] == -1) && (param_2 != 0)) {
      if ((*(uint *)(param_2 + 0x84) & 0x10) == 0) {
        if (debug_map != '\0') {
          debug("so %s is not \'sgi-only\': version not used to locate so\n",
                *(undefined4 *)(param_1 + 0x2c));
        }
      }
      else if (bVar3) {
        if (debug_map != '\0') {
          debug("..already has version..not reading.\n");
        }
      }
      else {
        lVar4 = FUN_0fb6ce20(param_1,param_2,param_3);
        if (lVar4 != 0) {
          aiStack_4a0[0] =
               FUN_0fb6c850(*(undefined4 *)(param_1 + 0x2c),acStack_14a0,0x1000,param_1,param_8);
        }
        bVar3 = true;
      }
    }
    if (aiStack_4a0[0] != -1) {
      sVar7 = strlen(acStack_14a0);
      FUN_0fb6d3c0(param_1,acStack_14a0,sVar7,0);
LAB_0fb6d79c:
      *(undefined1 *)(param_1 + 0xc3) = 1;
      *(int *)(param_1 + 0x10) = aiStack_4a0[0];
      if (debug_map != '\0') {
        debug("map_object_into_mem_and_init_object_info: mapped %s at 0x%lx\n",
              *(undefined4 *)(param_1 + 0x18),aiStack_4a0[0]);
      }
      if (debug_general != '\0') {
        debug("map_object_into_mem_and_init_object_info: Mapped obj : %s\n",
              *(undefined4 *)(param_1 + 0x18));
      }
      goto LAB_0fb6d62c;
    }
    iVar9 = -1;
    if (interact == '\0') {
LAB_0fb6d80c:
      if (iVar9 == -1) {
        if (param_2 == 0) {
          puVar10 = *(undefined **)(param_1 + 0x2c);
        }
        else {
          puVar10 = &DAT_0fbd8e68;
          if (piVar13 != (int *)0x0) {
            puVar10 = (undefined *)(*piVar13 + *(int *)(param_2 + 0x60));
          }
        }
        if (bVar2) {
          if (param_5 == 0) {
                    /* WARNING: Could not recover jumptable at 0x0fb6de20. Too many branches */
                    /* WARNING: Treating indirect jump as call */
            uVar6 = (*UNRECOVERED_JUMPTABLE)();
            return uVar6;
          }
          puVar11 = &DAT_0fbd86b8;
          if (piVar13 != (int *)0x0) {
            puVar11 = (undefined *)(piVar13[3] + *(int *)(param_2 + 0x60));
          }
          fatal("Cannot Successfully map soname \'%s\' version \'%s\' under any of the filenames %s \n"
                ,puVar10,puVar11,*param_8);
        }
        else {
          fatal("Cannot Successfully map soname \'%s\' under any of the filenames %s \n",puVar10,
                *param_8);
        }
      }
      goto LAB_0fb6d79c;
    }
    lVar4 = FUN_0fb6d270(param_1,param_2,aiStack_4a0,acStack_498,0x401);
    if (lVar4 == 0) {
      uVar6 = rld_obj_newstr(acStack_498);
      sVar7 = strlen(acStack_498);
      FUN_0fb6d3c0(param_1,uVar6,sVar7,1);
      __rld_strlcpy(acStack_14a0,acStack_498,0x1000);
      iVar9 = aiStack_4a0[0];
      goto LAB_0fb6d80c;
    }
    iVar9 = aiStack_4a0[0];
    if (lVar4 != 1) goto LAB_0fb6d80c;
LAB_0fb6d618:
    iVar12 = iVar12 + 1;
  } while (iVar12 != 5000);
  fatal("Attempted %ld DSO mapping iterations to map in %s: failed\n",iVar12,
        *(undefined4 *)(param_1 + 0x18));
LAB_0fb6d964:
  dumpobj(param_1);
  uVar5 = (ulonglong)*(int *)(param_1 + 0x84);
  if ((longlong)(uVar5 << 0x2f) < 0) {
    if (verbose != '\0') {
      trace("Setting LOCAL_LD_BIND_NOW on %s\n",*(undefined4 *)(param_1 + 0x18));
      uVar5 = (ulonglong)*(int *)(param_1 + 0x84);
    }
    *(undefined1 *)(param_1 + 0xe1) = 1;
  }
  if (((uVar5 & 1) == 0) && (quickstart_info != '\0')) {
    track("%s %s: QUICKSTART bit is off\n","Quickstart failed for object",
          *(undefined4 *)(param_1 + 0x18));
    uVar5 = (ulonglong)*(int *)(param_1 + 0x84);
  }
  if (((ignore_no_library_replacement == '\0') && ((uVar5 & 4) != 0)) &&
     (no_library_replace == '\0')) {
    _Var8 = getuid();
    if (_Var8 != 0) {
      if (debug_map != '\0') {
        debug("%s had RHF_NO_LIBRARY_REPLACEMENT, setting ld_path and global_path_prefix to nothing"
              ,*(undefined4 *)(param_1 + 0x18));
      }
      if (verbose != '\0') {
        trace("%s had RHF_NO_LIBRARY_REPLACEMENT, setting ld_path and global_path_prefix to nothing"
              ,*(undefined4 *)(param_1 + 0x18));
      }
      no_library_replace = '\x01';
      ld_path = 0;
      global_path_prefix = &DAT_0fbd86b8;
    }
    uVar5 = (ulonglong)*(int *)(param_1 + 0x84);
  }
  if ((uVar5 & 0x100) != 0) {
    is_pixie = 1;
    uVar5 = (ulonglong)*(int *)(param_1 + 0x84);
  }
  if ((uVar5 & 0x8000) != 0) {
    if (verbose != '\0') {
      trace("%s had RHF_LD_BIND_NOW, setting as if LD_BIND_NOW",*(undefined4 *)(param_1 + 0x18));
    }
    _rld_environment_ld_bind_now = 1;
    uVar5 = (ulonglong)*(int *)(param_1 + 0x84);
  }
  uVar5 = (ulonglong)guarantee_init | uVar5 & 0x20;
  guarantee_init = (byte)uVar5;
  uVar5 = (ulonglong)guarantee_start_init | (longlong)*(int *)(param_1 + 0x84) & 0x80U | uVar5;
  guarantee_start_init = (byte)uVar5;
  if (uVar5 != 0) {
    _rld_environment_ld_bind_now = 1;
    if (quickstart_info != '\0') {
      track("%s %s: RHF_GUARANTEE_[START_]INIT bit(s) are on\n","Quickstart failed for object",
            *(undefined4 *)(param_1 + 0x18));
    }
    all_fully_quickstarted = 0;
  }
  if ((*(int *)(param_1 + 0xbc) == -1) || (*(int *)(param_1 + 0xb0) == -1)) {
    if (debug_trace != '\0') {
      trace("map_object_into_mem_and_init_object_info: %s has NO conflict or liblist\n",
            *(undefined4 *)(param_1 + 0x18));
    }
    *(undefined1 *)(param_1 + 0xc6) = 1;
  }
  if (*(int *)(param_1 + 0x3c) != *(int *)(param_1 + 0x10)) {
    if ((*(uint *)(param_1 + 0x84) & 8) == 0) {
      if (verbose != '\0') {
        trace("obj %s got MOVED a distance of 0x%lx\n",*(undefined4 *)(param_1 + 0x18));
      }
      *(undefined1 *)(param_1 + 0xc2) = 1;
      if (quickstart_info != '\0') {
        track("%s %s: object was moved\n","Quickstart failed for object",
              *(undefined4 *)(param_1 + 0x18));
      }
      all_fully_quickstarted = 0;
    }
    else {
      fatal("obj %s got MOVED a distance of 0x%lx -- NOT ALLOWED since NO_LIBRARY_REPLACEMENT flag is set\n"
            ,*(undefined4 *)(param_1 + 0x18));
    }
  }
  if ((param_4 != 0) && (lVar15 = list_last(param_4), lVar15 != -1)) {
    iVar12 = (int)lVar15;
    if ((*(char *)(iVar12 + 0xc6) == '\0') && (*(char *)(iVar12 + 199) == '\0')) {
      uVar1 = *(uint *)(param_1 + 0x40);
      goto LAB_0fb6daa4;
    }
    if (verbose != '\0') {
      trace("obj %s FOLLOWED a CHECKSUM CHANGED obj %s (%d %d)\n",*(undefined4 *)(param_1 + 0x18),
            *(undefined4 *)(iVar12 + 0x18),*(char *)(iVar12 + 0xc6),*(undefined1 *)(iVar12 + 199));
    }
    *(undefined1 *)(param_1 + 199) = 1;
  }
  uVar1 = *(uint *)(param_1 + 0x40);
LAB_0fb6daa4:
  if (*(uint *)(param_1 + 0x48) < uVar1) {
    *(undefined1 *)(param_1 + 0xdf) = 1;
  }
  if (ld_trace_loaded_objects != '\0') {
    if (DAT_0fbde378 == 0) {
      if (piVar13 == (int *)0x0) {
        iVar12 = 0;
      }
      else {
        iVar12 = piVar13[3] + *(int *)(param_2 + 0x60);
      }
      if (param_7 == 0) {
        pcVar14 = "";
      }
      else {
        pcVar14 = "delay-load";
      }
      if (_rld_extended_ldd_output != '\0') {
        rld_fdprintf(1,"    find %s; required by %s",*(undefined4 *)(param_1 + 0x2c),
                     *(undefined4 *)(param_2 + 0x18));
        if (iVar12 != 0) {
          rld_fdprintf(1," (required-version %s )",iVar12);
        }
        rld_fdprintf(1,&DAT_0fbd9400);
      }
      rld_fdprintf(1,"\t%s  =>\t %s\t%s\n",*(undefined4 *)(param_1 + 0x2c),
                   *(undefined4 *)(param_1 + 0x18),pcVar14);
      return 1;
    }
    DAT_0fbde378 = 0;
    return 1;
  }
  return 1;
}



// === FUN_0fb6de30 @ 0fb6de30 (220 bytes) ===

undefined8 FUN_0fb6de30(int param_1,int param_2,int *param_3)

{
  longlong lVar1;
  
  if (debug_trace != '\0') {
    trace("entry exact_match_fails: compare %s, liblist entry %s, to %s\n",
          *(undefined4 *)(param_2 + 0x18),*param_3 + *(int *)(param_2 + 0x60),
          *(undefined4 *)(param_1 + 0x18));
  }
  if (debug_map != '\0') {
    debug("entry exact_match_fails: compare %s, liblist entry %s, to %s\n",
          *(undefined4 *)(param_2 + 0x18),*param_3 + *(int *)(param_2 + 0x60),
          *(undefined4 *)(param_1 + 0x18));
  }
  if (((param_3[4] & 1U) != 0) &&
     (lVar1 = essentially_identical_object(param_1,param_2,param_3), lVar1 == 0)) {
    if (debug_map != '\0') {
      debug("exact_match_fails returns FAILED (LL_EXACT_MATCH set, not identical object)");
    }
    return 1;
  }
  return 0;
}



// === essentially_identical_object @ 0fb6df10 (332 bytes) ===

/* WARNING: Instruction at (ram,0x0fb6dfc8) overlaps instruction at (ram,0x0fb6dfc4)
    */

undefined8 essentially_identical_object(int param_1,int param_2,int *param_3)

{
  longlong lVar1;
  int iVar2;
  undefined *puVar3;
  char *pcVar4;
  
  if (debug_trace != '\0') {
    trace("essentially_identical_object: compare %s, liblist entry %s, to %s\n",
          *(undefined4 *)(param_2 + 0x18),*param_3 + *(int *)(param_2 + 0x60),
          *(undefined4 *)(param_1 + 0x18));
  }
  if (debug_map != '\0') {
    puVar3 = *(undefined **)(param_1 + 0x30);
    pcVar4 = "<NULL>";
    if (puVar3 == (undefined *)0x0) {
      puVar3 = &DAT_0fbd86b8;
    }
    if (*(int *)(param_1 + 0x98) != 0) {
      pcVar4 = (char *)(*(int *)(param_1 + 0x60) + *(int *)(param_1 + 0x98));
    }
    debug("essentially_identical_object: [%s] tstamp %lx, csum %lx, version %s == [%s] tstamp %lx, csum %lx, version %s\n"
          ,*param_3 + *(int *)(param_2 + 0x60),param_3[1],param_3[2],
          param_3[3] + *(int *)(param_2 + 0x60),puVar3,*(undefined4 *)(param_1 + 0x90),
          *(undefined4 *)(param_1 + 0x94),pcVar4);
  }
  if ((((param_3[1] == *(int *)(param_1 + 0x90)) && (param_3[2] == *(int *)(param_1 + 0x94))) &&
      (lVar1 = match_interface_version(param_1,param_2,param_3), lVar1 != 0)) &&
     ((*(char **)(param_1 + 0x30) != (char *)0x0 &&
      (iVar2 = strcmp((char *)(*param_3 + *(int *)(param_2 + 0x60)),*(char **)(param_1 + 0x30)),
      iVar2 == 0)))) {
    return 1;
  }
  if (debug_map != '\0') {
    debug(" FAIL essentially_identical_object test\n");
  }
  return 0;
}



// === FUN_0fb6e060 @ 0fb6e060 (352 bytes) ===

/* WARNING: Instruction at (ram,0x0fb6e0cc) overlaps instruction at (ram,0x0fb6e0c8)
    */

void FUN_0fb6e060(int param_1,int param_2,int param_3)

{
  char *__s1;
  longlong lVar1;
  int iVar2;
  code *UNRECOVERED_JUMPTABLE;
  
  lVar1 = found_matching_string_ignore_comment_and_whitespace
                    (*(int *)(param_1 + 0x60) + *(int *)(param_1 + 0x98),
                     *(int *)(param_3 + 0xc) + *(int *)(param_2 + 0x60),0);
  if (lVar1 != 0) {
                    /* WARNING: Could not recover jumptable at 0x0fb6e0a4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*UNRECOVERED_JUMPTABLE)();
    return;
  }
  if ((((*(uint *)(param_1 + 0x84) & 0x10) != 0) && ((*(uint *)(param_3 + 0x10) & 4) == 0)) &&
     (lVar1 = found_matching_string_ignore_comment_and_whitespace
                        (*(int *)(param_1 + 0x60) + *(int *)(param_1 + 0x98),
                         *(int *)(param_3 + 0xc) + *(int *)(param_2 + 0x60),1), lVar1 != 0)) {
    __s1 = *(char **)(param_2 + 0x30);
    if (__s1 == (char *)0x0) {
      __s1 = *(char **)(param_2 + 0x2c);
    }
    if ((__s1 == (char *)0x0) || (iVar2 = strcmp(__s1,""), iVar2 == 0)) {
      __s1 = "Main";
    }
    if (verbose != '\0') {
      iVar2 = *(int *)(param_1 + 0x30);
      if (iVar2 == 0) {
        iVar2 = *(int *)(param_1 + 0x2c);
      }
      trace("Ignoring minor version number of %s, which does not\nmatch that in liblist of %s. sgi only %d  require minor %d\n"
            ,iVar2,__s1,*(uint *)(param_1 + 0x84) & 0x10,*(uint *)(param_3 + 0x10) & 4);
    }
                    /* WARNING: Could not recover jumptable at 0x0fb6e1a0. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*UNRECOVERED_JUMPTABLE)();
    return;
  }
  if (debug_map != '\0') {
    debug(" FAIL match_interface_with_exact_version test\n");
  }
                    /* WARNING: Could not recover jumptable at 0x0fb6e0ec. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
}



// === match_interface_version @ 0fb6e1c0 (336 bytes) ===

undefined8 match_interface_version(int param_1,int param_2,int *param_3)

{
  longlong lVar1;
  undefined8 uVar2;
  int iVar3;
  
  if (ignore_all_versions == '\0') {
    iVar3 = *(int *)(param_1 + 0x30);
    if (iVar3 == 0) {
      iVar3 = *(int *)(param_1 + 0x2c);
    }
    lVar1 = FUN_0fb6e5d0(iVar3);
    if (((lVar1 == 0) && ((param_3[4] & 2U) == 0)) && ((*(uint *)(param_2 + 0x84) & 0x10) != 0)) {
      if (param_3[3] == 0) {
        if (debug_trace != '\0') {
          trace("obj in liblist %s has no interface version\n",*param_3 + *(int *)(param_2 + 0x60));
        }
        return 1;
      }
      if (*(int *)(param_1 + 0x98) != 0) {
        uVar2 = FUN_0fb6e060(param_1,param_2,param_3);
        return uVar2;
      }
      if (debug_trace != '\0') {
        trace("obj %s has no interface version\n",*(undefined4 *)(param_1 + 0x18));
      }
      if (verbose != '\0') {
        trace("version-less %s is accepted even though version (%s) is specified for %s in the executable because of ABI compliance\n"
              ,*(undefined4 *)(param_1 + 0x18),param_3[3] + *(int *)(param_2 + 0x60),
              *param_3 + *(int *)(param_2 + 0x60));
      }
      return 1;
    }
  }
  if (debug_trace != '\0') {
    trace("version of obj %s is ignored\n",*(undefined4 *)(param_1 + 0x18));
  }
  return 1;
}



// === load_single_object @ 0fb6e310 (704 bytes) ===

/* WARNING: Instruction at (ram,0x0fb6e428) overlaps instruction at (ram,0x0fb6e424)
    */

undefined8
load_single_object(undefined8 param_1,char *param_2,ulonglong param_3,longlong param_4,
                  undefined4 *param_5,int *param_6)

{
  int iVar1;
  longlong lVar2;
  char *pcVar4;
  int iVar5;
  int iVar6;
  size_t sVar7;
  longlong lVar3;
  char *__s;
  int iVar8;
  char acStack_b0 [16];
  char *pcStack_40;
  uint uStack_3c;
  
  __s = (char *)param_1;
  if (param_4 == 0) {
    pcStack_40 = __s;
    pcVar4 = strchr(__s,0x2f);
    uStack_3c = (uint)(pcVar4 == (char *)0x0);
    lVar2 = foreach_sublist(pObj_Head,pathname_matches,&pcStack_40);
    if (lVar2 != -1) {
      *param_6 = (int)lVar2;
      return 1;
    }
  }
  else {
    lVar2 = foreach_sublist(pObj_Head,soname_matches,param_1);
    if (lVar2 != -1) {
      *param_6 = (int)lVar2;
      return 1;
    }
  }
  iVar5 = create_object(param_1,1);
  if (verbose != '\0') {
    pcVar4 = param_2;
    if (param_2 == (char *)0x0) {
      pcVar4 = "NULL";
    }
    trace("loading obj %s with version %s\n",param_1,pcVar4);
  }
  if ((param_2 == (char *)0x0) || (*param_2 == '\0')) {
    map_object_into_mem_and_init_object_info(iVar5,0,0,pObj_Head,1,0,param_3 & 0x10,param_5);
  }
  else {
    iVar6 = create_object(&DAT_0fbd86b8,0);
    sVar7 = strlen(__s);
    iVar8 = sVar7 + 2;
    sVar7 = strlen(param_2);
    iVar1 = -(sVar7 + iVar8 + 0x10 & 0xfffffff0);
    acStack_b0[iVar1] = 0;
    strcpy(acStack_b0 + iVar1 + 1,__s);
    strcpy(acStack_b0 + iVar8 + iVar1,param_2);
    *(char **)(iVar6 + 0x60) = acStack_b0 + iVar1;
    lVar2 = obj_create_liblist_entry(1,0,0,iVar8,param_3 & 0xfffffffffffffffe);
    if (lVar2 == 0) {
      fatal("load_single_object: cannot create liblist \n");
      return 2;
    }
    *(undefined4 *)(iVar6 + 0x84) = 0x10;
    *(int *)(iVar6 + 0x74) = (int)lVar2;
    lVar3 = map_object_into_mem_and_init_object_info
                      (iVar5,iVar6,0,pObj_Head,0,0,param_3 & 0x10,param_5);
    free(lVar2);
    delete_object(iVar6);
    if (lVar3 == 0) {
      fatal("Cannot find object %s with version %s in any of the filenames %s\n",param_1,param_2,
            *param_5);
      return 2;
    }
  }
  if (**(char **)(iVar5 + 0x88) != '\0') {
    global_library_search_rpath =
         generate_global_lib_search_path_from_obj
                   (*(char **)(iVar5 + 0x88),global_library_search_rpath,&user_defined_path);
  }
  *param_6 = iVar5;
  return 0;
}



// === FUN_0fb6e5d0 @ 0fb6e5d0 (172 bytes) ===

void FUN_0fb6e5d0(longlong param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  code *UNRECOVERED_JUMPTABLE_00;
  
  if (((ignore_so_num != 0) && (param_1 != 0)) && (iVar3 = 0, 0 < ignore_so_num)) {
    iVar2 = 0;
    do {
      iVar1 = strcmp((char *)param_1,*(char **)(ignore_version_list + iVar2));
      iVar2 = iVar2 + 4;
      if (iVar1 == 0) {
                    /* WARNING: Could not recover jumptable at 0x0fb6e674. Too many branches */
                    /* WARNING: Treating indirect jump as call */
        (*UNRECOVERED_JUMPTABLE_00)();
        return;
      }
      iVar3 = iVar3 + 1;
    } while (iVar3 < ignore_so_num);
  }
                    /* WARNING: Could not recover jumptable at 0x0fb6e654. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE_00)();
  return;
}



// === find_rld_allocated_common_symbol @ 0fb6e680 (164 bytes) ===

void find_rld_allocated_common_symbol(char *param_1,int param_2)

{
  int iVar1;
  undefined4 *puVar2;
  code *UNRECOVERED_JUMPTABLE_00;
  
  for (puVar2 = comm_list; puVar2 != (undefined4 *)0x0; puVar2 = (undefined4 *)puVar2[3]) {
    while (puVar2[2] != param_2) {
      puVar2 = (undefined4 *)puVar2[3];
      if (puVar2 == (undefined4 *)0x0) goto LAB_0fb6e6e8;
    }
    iVar1 = strcmp(param_1,(char *)*puVar2);
    if (iVar1 == 0) {
                    /* WARNING: Could not recover jumptable at 0x0fb6e71c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (*UNRECOVERED_JUMPTABLE_00)();
      return;
    }
  }
LAB_0fb6e6e8:
                    /* WARNING: Could not recover jumptable at 0x0fb6e6fc. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE_00)();
  return;
}



// === FUN_0fb6e730 @ 0fb6e730 (140 bytes) ===

void FUN_0fb6e730(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  undefined4 *puVar1;
  code *UNRECOVERED_JUMPTABLE;
  
  puVar1 = (undefined4 *)malloc(0x10);
  if (puVar1 == (undefined4 *)0x0) {
    fatal("Cannot rld_malloc in insert_into_common_list");
    uRam0000000c = 0;
  }
  else {
    puVar1[3] = 0;
  }
  *puVar1 = param_1;
  puVar1[1] = param_3;
  puVar1[2] = param_2;
  if (comm_list != (undefined4 *)0x0) {
    puVar1[3] = comm_list;
    comm_list = puVar1;
                    /* WARNING: Could not recover jumptable at 0x0fb6e794. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*UNRECOVERED_JUMPTABLE)();
    return;
  }
  comm_list = puVar1;
  return;
}



// === common_handling @ 0fb6e7c0 (1580 bytes) ===

int common_handling(int param_1,undefined8 param_2,undefined8 param_3)

{
  longlong lVar1;
  int iVar2;
  int iVar3;
  int *piVar4;
  int iVar5;
  int iStack_40;
  int iStack_3c;
  undefined1 *puStack_38;
  int *piStack_34;
  int iStack_30;
  int iStack_2c;
  undefined8 uStack_28;
  
  iStack_40 = 0;
  iVar3 = (int)param_2;
  uStack_28 = param_3;
  if (debug_trace != '\0') {
    iVar5 = *(int *)(*(int *)(param_1 + 0x68) + iVar3 * 8);
    if (iVar5 == 0) {
      iVar5 = get_dynsym_hash_value();
    }
    trace("common_handling:  %s, from %s hash value 0x%lx\n",
          *(int *)(*(int *)(param_1 + 100) + iVar3 * 0x10) + *(int *)(param_1 + 0x60),
          *(undefined4 *)(param_1 + 0x18),iVar5);
  }
  if (rld_has_listed_symbols != '\0') {
    lVar1 = rld_is_listed_symbol
                      (*(int *)(*(int *)(param_1 + 100) + iVar3 * 0x10) + *(int *)(param_1 + 0x60));
    if (lVar1 != 0) {
      iVar5 = *(int *)(*(int *)(param_1 + 0x68) + iVar3 * 8);
      if (iVar5 == 0) {
        iVar5 = get_dynsym_hash_value(param_1,param_2);
      }
      debug("common_handling:  resolving %s from %s, with hash value 0x%lx\n",
            *(int *)(*(int *)(param_1 + 100) + iVar3 * 0x10) + *(int *)(param_1 + 0x60),
            *(undefined4 *)(param_1 + 0x18),iVar5);
    }
  }
  if (debug_symbol != '\0') {
    iVar5 = *(int *)(*(int *)(param_1 + 0x68) + iVar3 * 8);
    if (iVar5 == 0) {
      iVar5 = get_dynsym_hash_value(param_1,param_2);
    }
    debug("common_handling:  resolving %s from %s, with hash value 0x%lx\n",
          *(int *)(*(int *)(param_1 + 100) + iVar3 * 0x10) + *(int *)(param_1 + 0x60),
          *(undefined4 *)(param_1 + 0x18),iVar5);
  }
  if ((param_1 != 0) &&
     ((iVar5 = iVar3 * 0x10, (*(byte *)(param_1 + 0xc9) & 1) != 0 ||
      (*(char *)(*(int *)(param_1 + 100) + iVar5 + 0xd) == '\x03')))) {
    if ((rld_has_listed_symbols != '\0') &&
       (lVar1 = rld_is_listed_symbol
                          (*(int *)(*(int *)(param_1 + 100) + iVar5) + *(int *)(param_1 + 0x60)),
       lVar1 != 0)) {
      debug("common_handling: shared object %s is defined with DT_SYMBOLIC or symbol %s has STO_PROTECTED on\n"
            ,*(undefined4 *)(param_1 + 0x18),
            *(int *)(*(int *)(param_1 + 100) + iVar5) + *(int *)(param_1 + 0x60));
    }
    if (debug_symbol != '\0') {
      debug("common_handling: shared object %s is defined with DT_SYMBOLIC or symbol %s has STO_PROTECTED on\n"
            ,*(undefined4 *)(param_1 + 0x18),
            *(int *)(*(int *)(param_1 + 100) + iVar5) + *(int *)(param_1 + 0x60));
    }
    iVar2 = *(int *)(*(int *)(param_1 + 0x68) + iVar3 * 8);
    if (iVar2 == 0) {
      iVar2 = get_dynsym_hash_value(param_1,param_2,0);
    }
    iStack_3c = find_symbol_in_object
                          (param_1,*(int *)(*(int *)(param_1 + 100) + iVar5) +
                                   *(int *)(param_1 + 0x60),iVar2,is_global_strong_symbol,uStack_28)
    ;
    if ((iStack_3c == -1) &&
       (piVar4 = (int *)(*(int *)(param_1 + 100) + iVar5), *(char *)((int)piVar4 + 0xd) == '\x03'))
    {
      fatal("%s has STO_PROTECTED on but is not defined in its shared obj %s\n",
            *piVar4 + *(int *)(param_1 + 0x60),*(undefined4 *)(param_1 + 0x18));
    }
    if (iStack_3c != -1) {
      return *(int *)(*(int *)(param_1 + 100) + iStack_3c * 0x10 + 4) -
             (*(int *)(param_1 + 0x3c) - *(int *)(param_1 + 0x10));
    }
  }
  piStack_34 = &iStack_3c;
  puStack_38 = (undefined1 *)&iStack_40;
  iStack_30 = param_1;
  iStack_2c = iVar3;
  foreach_obj(pObj_Head,&LAB_0fb6edf0,&puStack_38);
  if (iStack_40 != 0) {
    if (rld_has_listed_symbols != '\0') {
      lVar1 = rld_is_listed_symbol
                        (*(int *)(*(int *)(param_1 + 100) + iVar3 * 0x10) + *(int *)(param_1 + 0x60)
                        );
      if (lVar1 != 0) {
        debug("common_handling: resolved %s from %s\n",
              *(int *)(*(int *)(param_1 + 100) + iVar3 * 0x10) + *(int *)(param_1 + 0x60),
              *(undefined4 *)(iStack_40 + 0x18));
      }
    }
    if (debug_symbol != '\0') {
      debug("common_handling: resolved %s from %s\n",
            *(int *)(*(int *)(param_1 + 100) + iVar3 * 0x10) + *(int *)(param_1 + 0x60),
            *(undefined4 *)(iStack_40 + 0x18));
    }
    if (user_tracking != '\0') {
      track("found %s in %s for %s at 0x%lx...\n",
            *(int *)(*(int *)(param_1 + 100) + iVar3 * 0x10) + *(int *)(param_1 + 0x60),
            *(undefined4 *)(iStack_40 + 0x18),*(undefined4 *)(param_1 + 0x18),
            *(undefined4 *)(*(int *)(iStack_40 + 100) + iStack_3c * 0x10 + 4));
    }
    return *(int *)(*(int *)(iStack_40 + 100) + iStack_3c * 0x10 + 4) -
           (*(int *)(iStack_40 + 0x3c) - *(int *)(iStack_40 + 0x10));
  }
  foreach_obj(pObj_Head,FUN_0fb6efa0,&puStack_38);
  if (debug_symbol != '\0') {
    debug("common_handling: saved_o is 0x%lx: if 0 is really UNDEF\n",iStack_40);
  }
  if (iStack_40 == 0) {
    return -1;
  }
  if ((rld_has_listed_symbols != '\0') &&
     (lVar1 = rld_is_listed_symbol
                        (*(int *)(*(int *)(param_1 + 100) + iVar3 * 0x10) + *(int *)(param_1 + 0x60)
                        ), lVar1 != 0)) {
    piVar4 = (int *)(*(int *)(iStack_40 + 100) + iStack_3c * 0x10);
    debug("common_handling: resolved %s (align = %ld, size = %ld) by allocating it at runtime \n",
          *piVar4 + *(int *)(iStack_40 + 0x60),piVar4[1],piVar4[2]);
  }
  if (debug_symbol != '\0') {
    piVar4 = (int *)(*(int *)(iStack_40 + 100) + iStack_3c * 0x10);
    debug("common_handling: resolved %s (align = %ld, size = %ld) by allocating it at runtime \n",
          *piVar4 + *(int *)(iStack_40 + 0x60),piVar4[1],piVar4[2]);
  }
  piVar4 = (int *)(*(int *)(iStack_40 + 100) + iStack_3c * 0x10);
  if (*(short *)((int)piVar4 + 0xe) == -0xe) {
    iVar3 = *(int *)(*(int *)(iStack_40 + 0x68) + iStack_3c * 8);
    if (iVar3 == 0) {
      iVar3 = get_dynsym_hash_value(iStack_40,iStack_3c);
      piVar4 = (int *)(*(int *)(iStack_40 + 100) + iStack_3c * 0x10);
    }
    iVar3 = find_rld_allocated_common_symbol(*piVar4 + *(int *)(iStack_40 + 0x60),iVar3);
    if (iVar3 == -1) {
      iVar3 = *(int *)(iStack_40 + 100) + iStack_3c * 0x10;
      iVar5 = FUN_0fb6f0d0(*(undefined4 *)(iVar3 + 8),*(undefined4 *)(iVar3 + 4));
      iVar3 = *(int *)(*(int *)(iStack_40 + 0x68) + iStack_3c * 8);
      if (iVar3 == 0) {
        iVar3 = get_dynsym_hash_value(iStack_40,iStack_3c);
      }
      FUN_0fb6e730(*(int *)(*(int *)(iStack_40 + 100) + iStack_3c * 0x10) +
                   *(int *)(iStack_40 + 0x60),iVar3,iVar5);
      return iVar5;
    }
    return iVar3;
  }
  return -1;
}



// === FUN_0fb6efa0 @ 0fb6efa0 (300 bytes) ===

undefined8 FUN_0fb6efa0(int param_1,undefined4 *param_2)

{
  int *piVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  longlong lVar5;
  longlong lVar6;
  int unaff_gp_lo;
  int iStack_40;
  undefined1 uStack_3c;
  undefined4 uStack_38;
  undefined4 uStack_34;
  longlong lStack_30;
  longlong lStack_28;
  
  lVar5 = (longlong)(int)param_2[3];
  piVar1 = (int *)param_2[1];
  piVar2 = (int *)*param_2;
  lVar6 = (longlong)(int)param_2[2];
  uStack_3c = 0;
  iStack_40 = unaff_gp_lo + -0x1740;
  uStack_34 = 0;
  uStack_38 = 0;
  iVar3 = *(int *)(*(int *)(param_2[2] + 0x68) + param_2[3] * 8);
  if (iVar3 == 0) {
    lStack_30 = lVar6;
    lStack_28 = lVar5;
    iVar3 = get_dynsym_hash_value(lVar6,lVar5);
    lVar5 = lStack_28;
    lVar6 = lStack_30;
  }
  lVar5 = find_symbol_in_object
                    (param_1,*(int *)(*(int *)((int)lVar6 + 100) + (int)lVar5 * 0x10) +
                             *(int *)((int)lVar6 + 0x60),iVar3,&LAB_0fb6f190,&iStack_40);
  iVar3 = (int)lVar5;
  if (lVar5 != -1) {
    if (*piVar2 == 0) {
      *piVar2 = param_1;
      *piVar1 = iVar3;
    }
    else {
      iVar4 = *(int *)(param_1 + 100) + iVar3 * 0x10;
      if ((*(uint *)(*(int *)(*piVar2 + 100) + *piVar1 * 0x10 + 8) < *(uint *)(iVar4 + 8)) &&
         (*(short *)(iVar4 + 0xe) == -0xe)) {
        *piVar2 = param_1;
        *piVar1 = iVar3;
      }
    }
  }
  free_things_tried(&iStack_40);
  return 0xffffffffffffffff;
}



// === FUN_0fb6f0d0 @ 0fb6f0d0 (184 bytes) ===

void * FUN_0fb6f0d0(undefined8 param_1,longlong param_2)

{
  void *__s;
  uint uVar1;
  size_t __n;
  
  uVar1 = (uint)param_2;
  __n = ((int)param_1 + uVar1) - 1;
  __s = (void *)malloc(__n);
  if (__s == (void *)0x0) {
    fatal("Unable to malloc %ld bytes for common with alignment %ld",param_1,param_2);
  }
  memset(__s,0,__n);
  if (param_2 == 0) {
    trap(7);
  }
  if ((uint)__s % uVar1 != 0) {
    __s = (void *)((int)__s + (uVar1 - (uint)__s % uVar1));
  }
  if (debug_symbol != '\0') {
    debug("allocate_common: data with align = %ld, size = %ld is allocated at 0x%lx\n",param_2,
          param_1,__s);
  }
  return __s;
}



// === FUN_0fb6f1c0 @ 0fb6f1c0 (84 bytes) ===

void FUN_0fb6f1c0(uint *param_1)

{
  sginap((*param_1 & 0x7f) == 0x7f);
  *param_1 = *param_1 + 1;
  return;
}



// === init_signal_mask @ 0fb6f220 (60 bytes) ===

int init_signal_mask(void)

{
  int iVar1;
  
  sigfillset((sigset_t *)&rld_sigmask);
  sigdelset((sigset_t *)&rld_sigmask,3);
  sigdelset((sigset_t *)&rld_sigmask,0xb);
  iVar1 = sigdelset((sigset_t *)&rld_sigmask,10);
  return iVar1;
}



// === enter_LTR @ 0fb6f260 (408 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void enter_LTR(void)

{
  ulonglong uVar1;
  int iVar2;
  int iVar3;
  code *UNRECOVERED_JUMPTABLE;
  
  if (pthreads_involved != '\0') {
    do {
      iVar3 = 0;
      do {
        rld_app_bridge(pthreads_interface_data._8_4_,0,0,0,is_pixie);
        uVar1 = (*(code *)__test_then_add)(0xfbd3388,1);
        if (((uVar1 & 0xffffffffffff0000) == 0) ||
           (iVar2 = rld_app_bridge(pthreads_interface_data._4_4_,0,0,0,is_pixie),
           mpinfo._132_4_ == iVar2)) goto LAB_0fb6f314;
        (*(code *)__test_then_add)(0xfbd3388,0xffffffffffffffff);
        rld_app_bridge(pthreads_interface_data._12_4_,0,0,0,is_pixie);
        iVar3 = iVar3 + 1;
      } while (iVar3 < 5);
      sginap(1);
    } while( true );
  }
  while( true ) {
    while( true ) {
      if (_DAT_00200e00 == mpinfo._132_4_) {
                    /* WARNING: Could not recover jumptable at 0x0fb6f3cc. Too many branches */
                    /* WARNING: Treating indirect jump as call */
        (*(code *)__test_then_add)(0xfbd3388,1);
        return;
      }
      if ((mpinfo._128_4_ & 0xffff0000) == 0) break;
      sginap(1);
    }
    uVar1 = (*(code *)__test_then_add)(0xfbd3388,1);
    if ((uVar1 & 0xffffffffffff0000) == 0) break;
    (*(code *)__test_then_add)(0xfbd3388,0xffffffffffffffff);
    sginap(1);
  }
  if (debug_threads != '\0') {
    trace("enter_LTR exit E: we have the lock, sync lock 0x%lx\n",mpinfo._128_4_);
  }
LAB_0fb6f314:
                    /* WARNING: Could not recover jumptable at 0x0fb6f324. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
}



// === exit_LTR @ 0fb6f410 (144 bytes) ===

void exit_LTR(void)

{
  if ((pthreads_involved == '\0') && (debug_threads != '\0')) {
    trace("exit_LTR sync lock 0x%lx\n",mpinfo._128_4_);
  }
  if ((int)mpinfo._128_4_ < 1) {
    fatal("Locking Error in rld sproc/pthreads locking! sync lock %d id 0x%x (%d)\n",mpinfo._128_4_,
          mpinfo._132_4_,mpinfo._132_4_);
  }
  (*(code *)__test_then_add)(0xfbd3388,0xffffffffffffffff);
  if (pthreads_involved != '\0') {
    rld_app_bridge(pthreads_interface_data._12_4_,0,0,0,is_pixie);
  }
  return;
}



// === enter_libdl @ 0fb6f4a0 (636 bytes) ===

/* WARNING: Instruction at (ram,0x0fb6f674) overlaps instruction at (ram,0x0fb6f670)
    */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void enter_libdl(longlong param_1)

{
  ulong auVar1 [2];
  ulong auVar2 [2];
  int iVar5;
  longlong lVar3;
  int iVar6;
  ulonglong uVar4;
  undefined4 auStack_50 [2];
  ulong auStack_48 [2];
  ulong auStack_40 [2];
  
  iVar5 = _DAT_00200e00;
  auStack_50[0] = 0;
  if (pthreads_involved == '\0') {
    iVar6 = sigprocmask(1,(sigset_t *)&rld_sigmask,(sigset_t *)auStack_48);
    if ((iVar6 != 0) && (verbose != '\0')) {
      trace("sigprocmask fails: errno = %ld, sigmask = 0x%lx\n",errno,rld_sigmask);
    }
    while (uVar4 = (*(code *)__test_then_add)(0xfbd3388,0x10000), auVar1 = auStack_48,
          auVar2 = auStack_40, (uVar4 & 0xffffffffffff0000) != 0) {
      if (mpinfo._132_4_ == iVar5) {
        if (debug_threads != '\0') {
          trace("enter_libdl returns Ba sync lock 0x%lx\n",mpinfo._128_4_);
        }
        if (param_1 != 0) {
          return;
        }
        _rld_libdl_depth = _rld_libdl_depth + 1;
        return;
      }
      (*(code *)__test_then_add)(0xfbd3388,0xffffffffffff0000);
      sginap(1);
    }
    while (DAT_0fbd3828 = auVar2, rld_save_sigmask = auVar1, (mpinfo._128_4_ & 0xffff) != 0) {
      FUN_0fb6f1c0(auStack_50);
      auVar1 = (ulong  [2])rld_save_sigmask;
      auVar2 = (ulong  [2])DAT_0fbd3828;
    }
    if (param_1 == 0) {
      _rld_libdl_depth = _rld_libdl_depth + 1;
    }
    mpinfo._132_4_ = iVar5;
    if (debug_threads != '\0') {
      trace("enter_libdl returns B sync lock 0x%lx\n",mpinfo._128_4_);
    }
  }
  else {
    iVar5 = rld_app_bridge(pthreads_interface_data._4_4_,0,0,0,is_pixie);
    while( true ) {
      rld_app_bridge(pthreads_interface_data._8_4_,0,0,0,is_pixie);
      lVar3 = (*(code *)__test_then_add)(0xfbd3388,0x10000);
      if (lVar3 == 0) {
        if (param_1 == 0) {
          _rld_libdl_depth = _rld_libdl_depth + 1;
        }
        mpinfo._132_4_ = iVar5;
        return;
      }
      if (mpinfo._132_4_ == iVar5) break;
      (*(code *)__test_then_add)(0xfbd3388,0xffffffffffff0000);
      rld_app_bridge(pthreads_interface_data._12_4_,0,0,0,is_pixie);
      sginap(1);
    }
    if (param_1 == 0) {
      _rld_libdl_depth = _rld_libdl_depth + 1;
      return;
    }
  }
  return;
}



// === exit_libdl @ 0fb6f720 (236 bytes) ===

/* WARNING: Instruction at (ram,0x0fb6f7a4) overlaps instruction at (ram,0x0fb6f7a0)
    */

void exit_libdl(void)

{
  int iVar1;
  ulong auStack_20 [2];
  ulong auStack_18 [2];
  
  iVar1 = _rld_libdl_depth;
  if ((pthreads_involved == '\0') && (debug_threads != '\0')) {
    trace("exit_libdl sync_lock 0x%lx depth %u\n",mpinfo._128_4_,_rld_libdl_depth);
  }
  if (iVar1 == 1) {
    mpinfo._132_4_ = 0;
  }
  auStack_20[0] = rld_save_sigmask._0_4_;
  auStack_20[1] = rld_save_sigmask._4_4_;
  auStack_18[0] = DAT_0fbd3828._0_4_;
  auStack_18[1] = DAT_0fbd3828._4_4_;
  _rld_libdl_depth = _rld_libdl_depth + -1;
  (*(code *)__test_then_add)(0xfbd3388,0xffffffffffff0000);
  if (pthreads_involved == '\0') {
    if (((iVar1 == 1) && (iVar1 = sigprocmask(3,(sigset_t *)auStack_20,(sigset_t *)0x0), iVar1 != 0)
        ) && (verbose != '\0')) {
      trace("sigprocmask reset fails: errno = %ld, sigmask = 0x%lx\n",errno,auStack_20);
    }
  }
  else {
    rld_app_bridge(pthreads_interface_data._12_4_,0,0,0,is_pixie);
  }
  return;
}



// === enter_malloc @ 0fb6f810 (412 bytes) ===

void enter_malloc(void)

{
  longlong lVar1;
  int iVar2;
  code *UNRECOVERED_JUMPTABLE;
  int iStack_20;
  
  if (inside_non_thread_libdl_critical_region == '\0') {
    if (pthreads_involved == '\0') {
      while( true ) {
        while (malloc_sigmaskinfo._128_4_ != 0) {
          iStack_20 = iStack_20 + 1;
          if (4 < iStack_20) {
            iStack_20 = 0;
            sginap(1);
          }
        }
        lVar1 = (*(code *)__test_then_add)(0xfbd3790,1);
        if (lVar1 == 0) break;
        (*(code *)__test_then_add)(0xfbd3790,0xffffffffffffffff);
        iStack_20 = iStack_20 + 1;
        if (4 < iStack_20) {
          iStack_20 = 0;
          sginap(1);
        }
      }
      iVar2 = sigprocmask(1,(sigset_t *)&rld_sigmask,(sigset_t *)&malloc_save_sigmask);
      if ((iVar2 != 0) && (verbose != '\0')) {
        trace("enter_malloc sigprocmask fails: errno = %ld, sigmask = 0x%lx\n",errno,rld_sigmask);
      }
    }
    else {
      rld_app_bridge(pthreads_interface_data._8_4_,0,0,0,is_pixie);
    }
  }
  while( true ) {
    while (mallocinfo._128_4_ != 0) {
      iStack_20 = iStack_20 + 1;
      if (4 < iStack_20) {
        iStack_20 = 0;
        sginap(1);
      }
    }
    lVar1 = (*(code *)__test_then_add)(0xfbd3590,1);
    if (lVar1 == 0) break;
    (*(code *)__test_then_add)(0xfbd3590,0xffffffffffffffff);
    iStack_20 = iStack_20 + 1;
    if (4 < iStack_20) {
      iStack_20 = 0;
      sginap(1);
    }
  }
                    /* WARNING: Could not recover jumptable at 0x0fb6f8c4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
}



// === exit_malloc @ 0fb6f9c0 (168 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void exit_malloc(void)

{
  int iVar1;
  ulong auStack_20 [2];
  ulong auStack_18 [6];
  
  (*(code *)__test_then_add)(0xfbd3590,0xffffffffffffffff);
  if (inside_non_thread_libdl_critical_region == '\0') {
    if (pthreads_involved == '\0') {
      auStack_18[0] = _DAT_0fbd3848;
      auStack_18[1] = DAT_0fbd3848_4;
      auStack_20[0] = malloc_save_sigmask._0_4_;
      auStack_20[1] = malloc_save_sigmask._4_4_;
      (*(code *)__test_then_add)(0xfbd3790,0xffffffffffffffff);
      iVar1 = sigprocmask(3,(sigset_t *)auStack_20,(sigset_t *)0x0);
      if ((iVar1 != 0) && (verbose != '\0')) {
        trace("exit_malloc sigprocmask reset fails: errno = %ld, sigmask = 0x%lx\n",errno,auStack_20
             );
      }
    }
    else {
      rld_app_bridge(pthreads_interface_data._12_4_,0,0,0,is_pixie);
    }
  }
  return;
}



// === enter_print @ 0fb6fa70 (388 bytes) ===

void enter_print(void)

{
  longlong lVar1;
  int iVar2;
  int iVar3;
  code *UNRECOVERED_JUMPTABLE;
  
  if (inside_non_thread_libdl_critical_region == '\0') {
    iVar3 = 0;
    if (pthreads_involved == '\0') {
      while( true ) {
        while (print_sigmaskinfo._128_4_ != 0) {
          iVar3 = iVar3 + 1;
          if (4 < iVar3) {
            sginap(1);
            iVar3 = 0;
          }
        }
        lVar1 = (*(code *)__test_then_add)(0xfbd3690,1);
        if (lVar1 == 0) break;
        (*(code *)__test_then_add)(0xfbd3690,0xffffffffffffffff);
        iVar3 = iVar3 + 1;
        if (4 < iVar3) {
          sginap(1);
          iVar3 = 0;
        }
      }
      iVar2 = sigprocmask(1,(sigset_t *)&rld_sigmask,(sigset_t *)&print_save_sigmask);
      if ((iVar2 != 0) && (verbose != '\0')) {
        trace("enter_print sigprocmask fails: errno = %ld, sigmask = 0x%lx\n",errno,rld_sigmask);
      }
    }
    else {
      rld_app_bridge(pthreads_interface_data._8_4_,0,0,0,is_pixie);
      iVar3 = 0;
    }
  }
  else {
    iVar3 = 0;
  }
  while( true ) {
    while (printinfo._128_4_ != 0) {
      iVar3 = iVar3 + 1;
      if (4 < iVar3) {
        sginap(1);
        iVar3 = 0;
      }
    }
    lVar1 = (*(code *)__test_then_add)(0xfbd3490,1);
    if (lVar1 == 0) break;
    (*(code *)__test_then_add)(0xfbd3490,0xffffffffffffffff);
    iVar3 = iVar3 + 1;
    if (4 < iVar3) {
      sginap(1);
      iVar3 = 0;
    }
  }
                    /* WARNING: Could not recover jumptable at 0x0fb6fb1c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
}



// === exit_print @ 0fb6fc00 (168 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void exit_print(void)

{
  int iVar1;
  ulong auStack_20 [2];
  ulong auStack_18 [6];
  
  (*(code *)__test_then_add)(0xfbd3490,0xffffffffffffffff);
  if (inside_non_thread_libdl_critical_region == '\0') {
    if (pthreads_involved == '\0') {
      auStack_18[0] = _DAT_0fbd3838;
      auStack_18[1] = DAT_0fbd3838_4;
      auStack_20[0] = print_save_sigmask._0_4_;
      auStack_20[1] = print_save_sigmask._4_4_;
      (*(code *)__test_then_add)(0xfbd3690,0xffffffffffffffff);
      iVar1 = sigprocmask(3,(sigset_t *)auStack_20,(sigset_t *)0x0);
      if ((iVar1 != 0) && (verbose != '\0')) {
        trace("exit_print sigprocmask reset fails: errno = %ld, sigmask = 0x%lx\n",errno,auStack_20)
        ;
      }
    }
    else {
      rld_app_bridge(pthreads_interface_data._12_4_,0,0,0,is_pixie);
    }
  }
  return;
}



// === FUN_0fb6fcb0 @ 0fb6fcb0 (872 bytes) ===

/* WARNING: Instruction at (ram,0x0fb6fe4c) overlaps instruction at (ram,0x0fb6fe48)
    */
/* WARNING: Removing unreachable block (ram,0x0fb6feb0) */
/* WARNING: Removing unreachable block (ram,0x0fb6feb8) */

void FUN_0fb6fcb0(longlong param_1,longlong param_2,char param_3,undefined8 param_4,
                 undefined8 param_5,undefined8 param_6)

{
  bool bVar1;
  size_t sVar3;
  longlong lVar2;
  __pid_t _Var4;
  int iVar5;
  int iVar6;
  char *pcVar7;
  int iVar8;
  
  bVar1 = multi_threaded != '\0';
  if (bVar1) {
    enter_print();
  }
  iVar8 = 0x1e;
  if (main_exec_name != (char *)0x0) {
    sVar3 = strlen(main_exec_name);
    iVar8 = sVar3 + 0x1f;
  }
  lVar2 = malloc(10000);
  if (lVar2 == 0) {
    iVar8 = iVar8 + 10000;
  }
  else {
    sVar3 = strlen((char *)param_4);
    iVar5 = iVar8 + sVar3 + 1;
    iVar8 = rld_vsnprintfap(lVar2,iVar5,param_5,param_6);
    iVar8 = iVar8 + iVar5 + 1;
    free(lVar2);
  }
  if ((cur_last_error_message_len < iVar8) &&
     (last_error_message = (char *)realloc(last_error_message,iVar8),
     cur_last_error_message_len = iVar8, last_error_message == (char *)0x0)) {
    cur_last_error_message_len = 0;
    rld_fdprintf(2,"cannot malloc for last_error_message\n");
    if (!bVar1) {
      return;
    }
    exit_print();
    return;
  }
  iVar8 = cur_last_error_message_len;
  pcVar7 = last_error_message;
  _Var4 = getpid();
  iVar5 = rld_snprintf(pcVar7,iVar8,&DAT_0fbd9d58,_Var4);
  pcVar7 = pcVar7 + iVar5;
  iVar8 = iVar8 - iVar5;
  if (main_exec_name != (char *)0x0) {
    iVar5 = rld_snprintf(pcVar7,iVar8,&DAT_0fbd9d60);
    pcVar7 = pcVar7 + iVar5;
    iVar8 = iVar8 - iVar5;
  }
  iVar5 = rld_snprintf(pcVar7,iVar8,&DAT_0fbd9d68,param_4);
  iVar6 = rld_vsnprintfap(pcVar7 + iVar5,iVar8 - iVar5,param_5,param_6);
  pcVar7 = pcVar7 + iVar5 + iVar6;
  if (pcVar7[-1] == '\n') {
    pcVar7[-1] = '\0';
    pcVar7 = pcVar7 + -1;
  }
  if (param_2 != 0) {
    rld_snprintf(pcVar7,(iVar8 - iVar5) - iVar6,"errno = %ld",errno);
  }
  if (((param_3 == '\0') && (save_error_message_for_dlerror != '\0')) && (in_delay_load == '\0')) {
    last_error = last_error_message;
    if (param_1 != 0) {
      if (bVar1) {
        exit_print();
      }
                    /* WARNING: Subroutine does not return */
      longjmp((__jmp_buf_tag *)saved_error_exit,1);
    }
    goto LAB_0fb6fee8;
  }
  if (error_file_name == (char *)0x0) {
LAB_0fb6ff40:
    iVar8 = open64("/dev/tty",9,0x1ff);
  }
  else {
    iVar8 = open64(error_file_name,0x109,0x1ff);
    if (iVar8 == -1) {
      rld_fdprintf(2,"_RLD_ARGS: -log: cannot create %s\n",error_file_name);
      goto LAB_0fb6ff40;
    }
  }
  if (iVar8 == -1) {
    openlog("rld",1,8);
    if (param_1 != 0) {
      syslog(3,last_error_message);
      closelog();
                    /* WARNING: Subroutine does not return */
      exit(1);
    }
    syslog(4,last_error_message);
    closelog();
    if (!bVar1) {
      return;
    }
    exit_print();
    return;
  }
  rld_fdprintf(iVar8,&DAT_0fbd9dbc,last_error_message);
  close(iVar8);
  if (param_1 != 0) {
                    /* WARNING: Subroutine does not return */
    exit(1);
  }
LAB_0fb6fee8:
  if (bVar1) {
    exit_print();
  }
  return;
}



// === FUN_0fb70020 @ 0fb70020 (448 bytes) ===

int FUN_0fb70020(undefined8 param_1,char *param_2,undefined8 param_3)

{
  bool bVar1;
  char cVar2;
  int iVar3;
  size_t sVar4;
  __pid_t _Var5;
  tm *ptVar6;
  time_t atStack_50 [2];
  longlong lStack_48;
  ulonglong uStack_40;
  undefined8 uStack_38;
  undefined8 uStack_30;
  
  bVar1 = multi_threaded != '\0';
  uStack_38 = param_1;
  uStack_30 = param_3;
  if (bVar1) {
    enter_print();
  }
  if (error_file_name != (char *)0x0) {
    iVar3 = open64(error_file_name,0x109,0x1ff);
    if (iVar3 != -1) goto LAB_0fb70080;
    rld_fdprintf(2,"_RLD_ARGS: -log: cannot create %s\n",error_file_name);
  }
  iVar3 = open64("/dev/tty",9,0x1ff);
LAB_0fb70080:
  if (iVar3 == -1) {
    iVar3 = -1;
    if (bVar1) {
      iVar3 = exit_print();
    }
  }
  else {
    sVar4 = strlen(param_2);
    cVar2 = (param_2 + sVar4)[-2];
    lStack_48 = (longlong)(int)(sVar4 - 1);
    if (cVar2 == '\n') {
      param_2 = (char *)rld_obj_newstr(param_2);
      param_2[(int)lStack_48 + -1] = '\0';
    }
    uStack_40 = (ulonglong)(cVar2 == '\n');
    _Var5 = getpid();
    rld_fdprintf(iVar3,&DAT_0fbd9d58,_Var5);
    atStack_50[0] = time((time_t *)0x0);
    ptVar6 = localtime(atStack_50);
    rld_fdprintf(iVar3," %02d:%02d:%02d ",ptVar6->tm_hour,ptVar6->tm_min,ptVar6->tm_sec);
    if (main_exec_name != 0) {
      rld_fdprintf(iVar3,&DAT_0fbd9d60);
    }
    rld_fdprintf(iVar3,uStack_38);
    rld_fdprintfap(iVar3,param_2,uStack_30);
    iVar3 = close(iVar3);
    if (uStack_40 != 0) {
      iVar3 = free(param_2);
    }
    if (bVar1) {
      iVar3 = exit_print();
    }
  }
  return iVar3;
}



// === warning @ 0fb701e0 (80 bytes) ===

void warning(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,
            undefined8 param_5,undefined8 param_6,undefined8 param_7,undefined8 param_8)

{
  undefined8 uStack_38;
  undefined8 uStack_30;
  undefined8 uStack_28;
  undefined8 uStack_20;
  undefined8 uStack_18;
  undefined8 uStack_10;
  undefined8 uStack_8;
  
  uStack_38 = param_2;
  uStack_30 = param_3;
  uStack_28 = param_4;
  uStack_20 = param_5;
  uStack_18 = param_6;
  uStack_10 = param_7;
  uStack_8 = param_8;
  FUN_0fb6fcb0(0,0,0,"rld: Warning: ",param_1,&uStack_38);
  return;
}



// === force_warning @ 0fb70230 (80 bytes) ===

void force_warning(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,
                  undefined8 param_5,undefined8 param_6,undefined8 param_7,undefined8 param_8)

{
  undefined8 uStack_38;
  undefined8 uStack_30;
  undefined8 uStack_28;
  undefined8 uStack_20;
  undefined8 uStack_18;
  undefined8 uStack_10;
  undefined8 uStack_8;
  
  uStack_38 = param_2;
  uStack_30 = param_3;
  uStack_28 = param_4;
  uStack_20 = param_5;
  uStack_18 = param_6;
  uStack_10 = param_7;
  uStack_8 = param_8;
  FUN_0fb6fcb0(0,0,1,"rld: Warning: ",param_1,&uStack_38);
  return;
}



// === force_error @ 0fb70280 (80 bytes) ===

void force_error(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,
                undefined8 param_5,undefined8 param_6,undefined8 param_7,undefined8 param_8)

{
  undefined8 uStack_38;
  undefined8 uStack_30;
  undefined8 uStack_28;
  undefined8 uStack_20;
  undefined8 uStack_18;
  undefined8 uStack_10;
  undefined8 uStack_8;
  
  uStack_38 = param_2;
  uStack_30 = param_3;
  uStack_28 = param_4;
  uStack_20 = param_5;
  uStack_18 = param_6;
  uStack_10 = param_7;
  uStack_8 = param_8;
  FUN_0fb6fcb0(0,0,1,"rld: Error: ",param_1,&uStack_38);
  return;
}



// === error @ 0fb702d0 (80 bytes) ===

void error(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,
          undefined8 param_5,undefined8 param_6,undefined8 param_7,undefined8 param_8)

{
  undefined8 uStack_38;
  undefined8 uStack_30;
  undefined8 uStack_28;
  undefined8 uStack_20;
  undefined8 uStack_18;
  undefined8 uStack_10;
  undefined8 uStack_8;
  
  uStack_38 = param_2;
  uStack_30 = param_3;
  uStack_28 = param_4;
  uStack_20 = param_5;
  uStack_18 = param_6;
  uStack_10 = param_7;
  uStack_8 = param_8;
  FUN_0fb6fcb0(0,0,0,"rld: Error: ",param_1,&uStack_38);
  return;
}



// === fatal @ 0fb70320 (112 bytes) ===

void fatal(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,
          undefined8 param_5,undefined8 param_6,undefined8 param_7,undefined8 param_8)

{
  undefined8 uStack_38;
  undefined8 uStack_30;
  undefined8 uStack_28;
  undefined8 uStack_20;
  undefined8 uStack_18;
  undefined8 uStack_10;
  undefined8 uStack_8;
  
  uStack_38 = param_2;
  uStack_30 = param_3;
  uStack_28 = param_4;
  uStack_20 = param_5;
  uStack_18 = param_6;
  uStack_10 = param_7;
  uStack_8 = param_8;
  FUN_0fb70020("rld: Fatal Error exit/longjmp: ",param_1,&uStack_38);
  FUN_0fb6fcb0(1,0,0,"rld: Fatal Error: ",(int)param_1,&uStack_38);
  return;
}



// === trace @ 0fb70390 (68 bytes) ===

void trace(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,
          undefined8 param_5,undefined8 param_6,undefined8 param_7,undefined8 param_8)

{
  undefined8 uStack_38;
  undefined8 uStack_30;
  undefined8 uStack_28;
  undefined8 uStack_20;
  undefined8 uStack_18;
  undefined8 uStack_10;
  undefined8 uStack_8;
  
  uStack_38 = param_2;
  uStack_30 = param_3;
  uStack_28 = param_4;
  uStack_20 = param_5;
  uStack_18 = param_6;
  uStack_10 = param_7;
  uStack_8 = param_8;
  FUN_0fb70020(&DAT_0fbd9e30,param_1,&uStack_38);
  return;
}



// === debug @ 0fb703e0 (68 bytes) ===

void debug(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,
          undefined8 param_5,undefined8 param_6,undefined8 param_7,undefined8 param_8)

{
  undefined8 uStack_38;
  undefined8 uStack_30;
  undefined8 uStack_28;
  undefined8 uStack_20;
  undefined8 uStack_18;
  undefined8 uStack_10;
  undefined8 uStack_8;
  
  uStack_38 = param_2;
  uStack_30 = param_3;
  uStack_28 = param_4;
  uStack_20 = param_5;
  uStack_18 = param_6;
  uStack_10 = param_7;
  uStack_8 = param_8;
  FUN_0fb70020("rld: Debug: ",param_1,&uStack_38);
  return;
}



// === track @ 0fb70430 (68 bytes) ===

void track(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,
          undefined8 param_5,undefined8 param_6,undefined8 param_7,undefined8 param_8)

{
  undefined8 uStack_38;
  undefined8 uStack_30;
  undefined8 uStack_28;
  undefined8 uStack_20;
  undefined8 uStack_18;
  undefined8 uStack_10;
  undefined8 uStack_8;
  
  uStack_38 = param_2;
  uStack_30 = param_3;
  uStack_28 = param_4;
  uStack_20 = param_5;
  uStack_18 = param_6;
  uStack_10 = param_7;
  uStack_8 = param_8;
  FUN_0fb70020("[rld] ",param_1,&uStack_38);
  return;
}



// === FUN_0fb70480 @ 0fb70480 (172 bytes) ===

void FUN_0fb70480(int param_1,void *param_2,size_t param_3)

{
  ssize_t sVar1;
  code *UNRECOVERED_JUMPTABLE_00;
  
  while( true ) {
    if (param_3 == 0) {
                    /* WARNING: Could not recover jumptable at 0x0fb70524. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (*UNRECOVERED_JUMPTABLE_00)();
      return;
    }
    errno = 0;
    sVar1 = write(param_1,param_2,param_3);
    if ((sVar1 < 1) && ((-1 < sVar1 || (sVar1 = 0, errno != 4)))) break;
    param_3 = param_3 - sVar1;
    param_2 = (void *)(sVar1 + (int)param_2);
  }
                    /* WARNING: Could not recover jumptable at 0x0fb70504. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE_00)();
  return;
}



// === rld_fdprintf @ 0fb70530 (56 bytes) ===

void rld_fdprintf(undefined8 param_1,undefined4 param_2,undefined8 param_3,undefined8 param_4,
                 undefined8 param_5,undefined8 param_6,undefined8 param_7,undefined8 param_8)

{
  undefined8 uStack_30;
  undefined8 uStack_28;
  undefined8 uStack_20;
  undefined8 uStack_18;
  undefined8 uStack_10;
  undefined8 uStack_8;
  
  uStack_30 = param_3;
  uStack_28 = param_4;
  uStack_20 = param_5;
  uStack_18 = param_6;
  uStack_10 = param_7;
  uStack_8 = param_8;
  rld_fdprintfap(param_1,param_2,&uStack_30);
  return;
}



// === rld_fdprintfap @ 0fb70570 (304 bytes) ===

void rld_fdprintfap(undefined8 param_1,undefined8 param_2,undefined8 param_3)

{
  int iVar1;
  uint uVar3;
  int iVar4;
  longlong lVar2;
  size_t sVar5;
  
  if (DAT_0fbd3940 == 0) {
    DAT_0fbd3940 = 0x400;
    lVar2 = malloc();
    DAT_0fbd393c = (int)lVar2;
    if (lVar2 == 0) {
      sVar5 = strlen("Unable to malloc print buffer: giving up\n");
      FUN_0fb70480(param_1,"Unable to malloc print buffer: giving up\n",sVar5);
                    /* WARNING: Subroutine does not return */
      exit(3);
    }
  }
  while( true ) {
    uVar3 = rld_vsnprintfap(DAT_0fbd393c,DAT_0fbd3940,param_2,param_3);
    if ((int)uVar3 < 0) {
      sVar5 = strlen((char *)param_2);
      FUN_0fb70480(param_1,param_2,sVar5);
      return;
    }
    iVar1 = DAT_0fbd3940 * 2;
    if ((uVar3 < DAT_0fbd3940 - 2U) || (DAT_0fbd3940 = iVar1, iVar4 = malloc(iVar1), iVar4 == 0))
    break;
    free(DAT_0fbd393c);
    DAT_0fbd393c = iVar4;
    DAT_0fbd3940 = iVar1;
  }
  FUN_0fb70480(param_1,DAT_0fbd393c,uVar3);
  return;
}



// === rld_snprintf @ 0fb706a0 (52 bytes) ===

void rld_snprintf(void)

{
  rld_vsnprintfap();
  return;
}



// === rld_vsnprintfap @ 0fb706e0 (884 bytes) ===

/* WARNING: Instruction at (ram,0x0fb70a10) overlaps instruction at (ram,0x0fb70a0c)
    */
/* WARNING: Type propagation algorithm not settling */

void rld_vsnprintfap(byte *param_1,int param_2,byte *param_3)

{
  byte *pbVar1;
  byte *pbVar2;
  byte *pbVar3;
  byte bVar5;
  uint uVar4;
  code *UNRECOVERED_JUMPTABLE;
  
  bVar5 = *param_3;
  if (bVar5 != 0) {
    pbVar1 = param_1 + param_2 + -1;
    do {
      if (bVar5 == 0x25) {
        uVar4 = (uint)param_3[1];
        pbVar2 = param_3 + 1;
        pbVar3 = pbVar2;
        if (uVar4 != 0) {
          while ((((uVar4 == 0x2d || (uVar4 == 0x2b)) || (uVar4 == 0x20)) ||
                 ((uVar4 == 0x23 || (uVar4 == 0x30))))) {
            uVar4 = (uint)*(byte *)((int)pbVar3 + 1);
            pbVar3 = (byte *)((int)pbVar3 + 1);
            if (uVar4 == 0) goto LAB_0fb708d8;
          }
          if (uVar4 != 0x2a) {
            while ((0x2f < uVar4 && (uVar4 < 0x3a))) {
              uVar4 = (uint)pbVar3[1];
              pbVar3 = pbVar3 + 1;
              if (uVar4 == 0) goto LAB_0fb708d8;
            }
          }
          if (uVar4 == 0x2e) {
            uVar4 = (uint)pbVar3[1];
            pbVar3 = pbVar3 + 1;
            if (uVar4 != 0) {
              if (uVar4 != 0x2a) {
                while ((0x2f < uVar4 && (uVar4 < 0x3a))) {
                  uVar4 = (uint)pbVar3[1];
                  pbVar3 = pbVar3 + 1;
                  if (uVar4 == 0) goto LAB_0fb708d8;
                }
                goto LAB_0fb70a08;
              }
              goto code_r0x0fb70a14;
            }
          }
          else {
LAB_0fb70a08:
            if (uVar4 == 0x68) {
              pbVar2 = pbVar3 + 1;
              uVar4 = (uint)*pbVar2;
              if (uVar4 == 0) goto LAB_0fb708d8;
            }
            else {
code_r0x0fb70a14:
              pbVar2 = pbVar3;
              if (uVar4 == 0x6c) {
                pbVar2 = pbVar3 + 1;
                uVar4 = (uint)*pbVar2;
                if (uVar4 == 0x6c) {
                  uVar4 = (uint)pbVar3[2];
                  pbVar2 = pbVar3 + 2;
                }
              }
            }
            if (uVar4 != 0) {
              if (uVar4 - 0x58 < 0x21) {
                return;
              }
              if (param_1 < pbVar1) {
                *param_1 = (byte)uVar4;
                goto LAB_0fb70750;
              }
              break;
            }
          }
LAB_0fb708d8:
          bVar5 = *param_3;
          pbVar3 = param_1;
          if (bVar5 != 0) goto LAB_0fb708e4;
          break;
        }
        if (pbVar1 <= param_1) break;
        *param_1 = 0x25;
      }
      else {
        if (pbVar1 <= param_1) break;
        *param_1 = bVar5;
        pbVar2 = param_3;
      }
LAB_0fb70750:
      param_1 = param_1 + 1;
      bVar5 = pbVar2[1];
      param_3 = pbVar2 + 1;
    } while (bVar5 != 0);
  }
LAB_0fb707e8:
  *param_1 = 0;
                    /* WARNING: Could not recover jumptable at 0x0fb707f8. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
LAB_0fb708e4:
  param_1 = pbVar3;
  if (pbVar1 <= pbVar3) goto LAB_0fb707e8;
  *pbVar3 = bVar5;
  param_1 = pbVar3 + 1;
  if ((param_3[1] == 0) || (pbVar1 <= param_1)) goto LAB_0fb707e8;
  *param_1 = param_3[1];
  bVar5 = param_3[2];
  param_3 = param_3 + 2;
  param_1 = pbVar3 + 2;
  pbVar3 = param_1;
  if (bVar5 == 0) goto LAB_0fb707e8;
  goto LAB_0fb708e4;
}



// === sprintf @ 0fb71030 (64 bytes) ===

int sprintf(char *__s,char *__format,...)

{
  int iVar1;
  undefined8 in_a2;
  undefined8 in_a3;
  undefined8 in_t0;
  undefined8 in_t1;
  undefined8 in_t2;
  undefined8 in_t3;
  undefined8 uStack_30;
  undefined8 uStack_28;
  undefined8 uStack_20;
  undefined8 uStack_18;
  undefined8 uStack_10;
  undefined8 uStack_8;
  
  uStack_30 = in_a2;
  uStack_28 = in_a3;
  uStack_20 = in_t0;
  uStack_18 = in_t1;
  uStack_10 = in_t2;
  uStack_8 = in_t3;
  iVar1 = rld_vsnprintfap(__s,2000,__format,&uStack_30);
  return iVar1;
}



// === fprintf @ 0fb71070 (216 bytes) ===

int fprintf(FILE *__stream,char *__format,...)

{
  undefined2 uVar1;
  longlong lVar2;
  int iVar3;
  size_t sVar4;
  undefined8 in_a2;
  undefined8 in_a3;
  undefined8 in_t0;
  undefined8 in_t1;
  undefined8 in_t2;
  undefined8 in_t3;
  undefined8 uStack_30;
  undefined8 uStack_28;
  undefined8 uStack_20;
  undefined8 uStack_18;
  undefined8 uStack_10;
  undefined8 uStack_8;
  
  uVar1 = *(undefined2 *)((int)&__stream->_IO_read_base + 2);
  uStack_30 = in_a2;
  uStack_28 = in_a3;
  uStack_20 = in_t0;
  uStack_18 = in_t1;
  uStack_10 = in_t2;
  uStack_8 = in_t3;
  if (DAT_0fbd3940 == 0) {
    DAT_0fbd3940 = 0x400;
    lVar2 = malloc();
    DAT_0fbd393c = (undefined4)lVar2;
    if (lVar2 == 0) {
      sVar4 = strlen("Unable to malloc printf buffer: giving up\n");
      FUN_0fb70480(uVar1,"Unable to malloc printf buffer: giving up\n",sVar4);
                    /* WARNING: Subroutine does not return */
      exit(3);
    }
  }
  lVar2 = rld_vsnprintfap(DAT_0fbd393c,DAT_0fbd3940,__format,&uStack_30);
  if (lVar2 < 0) {
    sVar4 = strlen(__format);
    iVar3 = FUN_0fb70480(uVar1,__format,sVar4);
    return iVar3;
  }
  iVar3 = FUN_0fb70480(uVar1,DAT_0fbd393c,lVar2);
  return iVar3;
}



// === printf @ 0fb71150 (204 bytes) ===

int printf(char *__format,...)

{
  longlong lVar1;
  int iVar2;
  size_t sVar3;
  undefined8 in_a1;
  undefined8 in_a2;
  undefined8 in_a3;
  undefined8 in_t0;
  undefined8 in_t1;
  undefined8 in_t2;
  undefined8 in_t3;
  undefined8 uStack_38;
  undefined8 uStack_30;
  undefined8 uStack_28;
  undefined8 uStack_20;
  undefined8 uStack_18;
  undefined8 uStack_10;
  undefined8 uStack_8;
  
  uStack_38 = in_a1;
  uStack_30 = in_a2;
  uStack_28 = in_a3;
  uStack_20 = in_t0;
  uStack_18 = in_t1;
  uStack_10 = in_t2;
  uStack_8 = in_t3;
  if (DAT_0fbd3940 == 0) {
    DAT_0fbd3940 = 0x400;
    lVar1 = malloc();
    DAT_0fbd393c = (undefined4)lVar1;
    if (lVar1 == 0) {
      sVar3 = strlen("Unable to malloc printf buffer: giving up\n");
      FUN_0fb70480(1,"Unable to malloc printf buffer: giving up\n",sVar3);
                    /* WARNING: Subroutine does not return */
      exit(3);
    }
  }
  lVar1 = rld_vsnprintfap(DAT_0fbd393c,DAT_0fbd3940,__format,&uStack_38);
  if (lVar1 < 0) {
    sVar3 = strlen(__format);
    iVar2 = FUN_0fb70480(1,__format,sVar3);
    return iVar2;
  }
  iVar2 = FUN_0fb70480(1,DAT_0fbd393c,lVar1);
  return iVar2;
}



// === _vsprintf @ 0fb71220 (20 bytes) ===

int _vsprintf(char *__s,char *__format,__gnuc_va_list __arg)

{
  int iVar1;
  
  iVar1 = rld_vsnprintfap(__s,8000,__format,__arg);
  return iVar1;
}



// === vsnprintf @ 0fb71240 (8 bytes) ===

/* WARNING: Instruction at (ram,0x0fb70a10) overlaps instruction at (ram,0x0fb70a0c)
    */
/* WARNING: Type propagation algorithm not settling */

void vsnprintf(byte *param_1,int param_2,byte *param_3)

{
  byte *pbVar1;
  byte *pbVar2;
  byte *pbVar3;
  byte bVar5;
  uint uVar4;
  code *UNRECOVERED_JUMPTABLE;
  
  bVar5 = *param_3;
  if (bVar5 != 0) {
    pbVar1 = param_1 + param_2 + -1;
    do {
      if (bVar5 == 0x25) {
        uVar4 = (uint)param_3[1];
        pbVar2 = param_3 + 1;
        pbVar3 = pbVar2;
        if (uVar4 != 0) {
          while ((((uVar4 == 0x2d || (uVar4 == 0x2b)) || (uVar4 == 0x20)) ||
                 ((uVar4 == 0x23 || (uVar4 == 0x30))))) {
            uVar4 = (uint)*(byte *)((int)pbVar3 + 1);
            pbVar3 = (byte *)((int)pbVar3 + 1);
            if (uVar4 == 0) goto LAB_0fb708d8;
          }
          if (uVar4 != 0x2a) {
            while ((0x2f < uVar4 && (uVar4 < 0x3a))) {
              uVar4 = (uint)pbVar3[1];
              pbVar3 = pbVar3 + 1;
              if (uVar4 == 0) goto LAB_0fb708d8;
            }
          }
          if (uVar4 == 0x2e) {
            uVar4 = (uint)pbVar3[1];
            pbVar3 = pbVar3 + 1;
            if (uVar4 != 0) {
              if (uVar4 != 0x2a) {
                while ((0x2f < uVar4 && (uVar4 < 0x3a))) {
                  uVar4 = (uint)pbVar3[1];
                  pbVar3 = pbVar3 + 1;
                  if (uVar4 == 0) goto LAB_0fb708d8;
                }
                goto LAB_0fb70a08;
              }
              goto code_r0x0fb70a14;
            }
          }
          else {
LAB_0fb70a08:
            if (uVar4 == 0x68) {
              pbVar2 = pbVar3 + 1;
              uVar4 = (uint)*pbVar2;
              if (uVar4 == 0) goto LAB_0fb708d8;
            }
            else {
code_r0x0fb70a14:
              pbVar2 = pbVar3;
              if (uVar4 == 0x6c) {
                pbVar2 = pbVar3 + 1;
                uVar4 = (uint)*pbVar2;
                if (uVar4 == 0x6c) {
                  uVar4 = (uint)pbVar3[2];
                  pbVar2 = pbVar3 + 2;
                }
              }
            }
            if (uVar4 != 0) {
              if (uVar4 - 0x58 < 0x21) {
                return;
              }
              if (param_1 < pbVar1) {
                *param_1 = (byte)uVar4;
                goto LAB_0fb70750;
              }
              break;
            }
          }
LAB_0fb708d8:
          bVar5 = *param_3;
          pbVar3 = param_1;
          if (bVar5 != 0) goto LAB_0fb708e4;
          break;
        }
        if (pbVar1 <= param_1) break;
        *param_1 = 0x25;
      }
      else {
        if (pbVar1 <= param_1) break;
        *param_1 = bVar5;
        pbVar2 = param_3;
      }
LAB_0fb70750:
      param_1 = param_1 + 1;
      bVar5 = pbVar2[1];
      param_3 = pbVar2 + 1;
    } while (bVar5 != 0);
  }
LAB_0fb707e8:
  *param_1 = 0;
                    /* WARNING: Could not recover jumptable at 0x0fb707f8. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
LAB_0fb708e4:
  param_1 = pbVar3;
  if (pbVar1 <= pbVar3) goto LAB_0fb707e8;
  *pbVar3 = bVar5;
  param_1 = pbVar3 + 1;
  if ((param_3[1] == 0) || (pbVar1 <= param_1)) goto LAB_0fb707e8;
  *param_1 = param_3[1];
  bVar5 = param_3[2];
  param_3 = param_3 + 2;
  param_1 = pbVar3 + 2;
  pbVar3 = param_1;
  if (bVar5 == 0) goto LAB_0fb707e8;
  goto LAB_0fb708e4;
}



// === _cleanup @ 0fb71250 (8 bytes) ===

void _cleanup(EVP_PKEY_CTX *ctx)

{
  return;
}



// === fflush @ 0fb71260 (8 bytes) ===

int fflush(FILE *__stream)

{
  return 0;
}



// === fclose @ 0fb71270 (8 bytes) ===

int fclose(FILE *__stream)

{
  return 0;
}



// === _setbufend @ 0fb71280 (8 bytes) ===

void _setbufend(void)

{
  return;
}



// === _findiop @ 0fb71290 (8 bytes) ===

undefined8 _findiop(void)

{
  return 0;
}



// === _realbufend @ 0fb712a0 (8 bytes) ===

undefined8 _realbufend(void)

{
  return 0;
}



// === _incbufend @ 0fb712b0 (8 bytes) ===

void _incbufend(void)

{
  return;
}



// === _resetbufend @ 0fb712c0 (8 bytes) ===

void _resetbufend(void)

{
  return;
}



// === _realbufendadj @ 0fb712d0 (8 bytes) ===

undefined8 _realbufendadj(void)

{
  return 0;
}



// === _realbufdirty @ 0fb712e0 (8 bytes) ===

undefined8 _realbufdirty(void)

{
  return 0;
}



// === _setbufdirty @ 0fb712f0 (8 bytes) ===

void _setbufdirty(void)

{
  return;
}



// === _setbufclean @ 0fb71300 (8 bytes) ===

void _setbufclean(void)

{
  return;
}



// === _bufsync @ 0fb71310 (8 bytes) ===

void _bufsync(void)

{
  return;
}



// === _xflsbuf @ 0fb71320 (8 bytes) ===

undefined8 _xflsbuf(void)

{
  return 0;
}



// === _flushlbf @ 0fb71330 (8 bytes) ===

void _flushlbf(void)

{
  return;
}



// === FUN_0fb71340 @ 0fb71340 (120 bytes) ===

undefined4 * FUN_0fb71340(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  undefined4 *__s;
  
  __s = (undefined4 *)malloc(0xc);
  if (__s == (undefined4 *)0x0) {
    fatal("Failed malloc of struct iface_struct. rld Cannot continue.\n");
  }
  memset(__s,0,0xc);
  *__s = param_1;
  __s[1] = param_2;
  __s[2] = param_3;
  return __s;
}



// === FUN_0fb713c0 @ 0fb713c0 (40 bytes) ===

undefined8 FUN_0fb713c0(ulonglong param_1)

{
  undefined8 uVar1;
  
  if (param_1 < 0x29) {
                    /* WARNING: Could not recover jumptable at 0x0fb713d8. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    uVar1 = (*(code *)(&PTR_LAB_0fbda048)[(int)param_1])();
    return uVar1;
  }
  return 0;
}



// === FUN_0fb71450 @ 0fb71450 (96 bytes) ===

ushort FUN_0fb71450(int param_1,int *param_2)

{
  ushort uVar1;
  int iVar2;
  ushort *puVar3;
  
  iVar2 = *param_2;
  puVar3 = (ushort *)(iVar2 + 8);
  *param_2 = (int)puVar3;
  uVar1 = *(ushort *)(param_1 + 4);
  if ((uVar1 & 0x38) != 0) {
    puVar3 = (ushort *)(iVar2 + 10);
    *param_2 = (int)puVar3;
    uVar1 = *(ushort *)(param_1 + 4);
  }
  if ((uVar1 & 0x20) != 0) {
    puVar3 = puVar3 + 2;
    *param_2 = (int)puVar3;
  }
  if (*(byte *)(param_1 + 6) != 0xff) {
    return (ushort)*(byte *)(param_1 + 6);
  }
  uVar1 = *puVar3;
  *param_2 = (int)(puVar3 + 1);
  return uVar1;
}



// === FUN_0fb714b0 @ 0fb714b0 (216 bytes) ===

undefined8 FUN_0fb714b0(ushort *param_1)

{
  char cVar1;
  ushort uVar2;
  longlong lVar3;
  char *pcVar4;
  char *pcVar5;
  int iVar6;
  int iVar7;
  
  uVar2 = *param_1;
  if ((uVar2 & 0x4000) != 0) {
    return 1;
  }
  lVar3 = FUN_0fb713c0(uVar2 & 0xff);
  iVar6 = 2;
  if (lVar3 == 0xc) {
    if ((uVar2 & 0x2000) == 0) {
      iVar6 = 3;
    }
    else {
      iVar6 = 6;
    }
  }
  iVar7 = (int)(uVar2 & 0xf00) >> 8;
  pcVar4 = (char *)(iVar6 + (int)param_1);
  pcVar5 = pcVar4 + iVar7;
  if (iVar7 != 0) {
    cVar1 = *pcVar4;
    while( true ) {
      if ((cVar1 == '\x01') || (pcVar4 = pcVar4 + 1, cVar1 == '\x02')) {
        return 1;
      }
      if (pcVar4 == pcVar5) break;
      cVar1 = *pcVar4;
    }
  }
  return 0;
}



// === FUN_0fb71590 @ 0fb71590 (140 bytes) ===

void FUN_0fb71590(ushort *param_1)

{
  ushort uVar1;
  longlong lVar2;
  code *UNRECOVERED_JUMPTABLE;
  
  uVar1 = *param_1;
  lVar2 = FUN_0fb713c0(uVar1 & 0xff);
  if (lVar2 != 0xc) {
                    /* WARNING: Could not recover jumptable at 0x0fb715f4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*UNRECOVERED_JUMPTABLE)();
    return;
  }
  if ((uVar1 & 0x2000) != 0) {
                    /* WARNING: Could not recover jumptable at 0x0fb715d8. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*UNRECOVERED_JUMPTABLE)();
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x0fb71614. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
}



// === FUN_0fb71620 @ 0fb71620 (52 bytes) ===

uint FUN_0fb71620(ushort *param_1)

{
  if ((*param_1 & 0x2000) != 0) {
    return *(uint *)(param_1 + 1);
  }
  return (uint)*(byte *)(param_1 + 1);
}



// === FUN_0fb71660 @ 0fb71660 (540 bytes) ===

undefined8 FUN_0fb71660(int *param_1,int *param_2)

{
  ushort *puVar1;
  ushort *puVar2;
  longlong lVar3;
  int iVar6;
  int iVar7;
  undefined8 uVar4;
  longlong lVar5;
  code *UNRECOVERED_JUMPTABLE_00;
  
  puVar1 = (ushort *)*param_2;
  puVar2 = (ushort *)*param_1;
  lVar3 = FUN_0fb714b0();
  if ((lVar3 == 0) || (lVar3 = FUN_0fb714b0(*param_2), lVar3 != 0)) {
    lVar3 = FUN_0fb714b0(*param_2);
    if (lVar3 != 0) {
      lVar3 = FUN_0fb714b0(*param_1);
      if (lVar3 == 0) goto LAB_0fb71794;
    }
    iVar6 = FUN_0fb713c0(*puVar2);
    iVar7 = FUN_0fb713c0(*puVar1);
    if ((&DAT_0fbd9f58)[iVar7 + iVar6 * 0xd] != '\0') {
      iVar6 = FUN_0fb71590(*param_1);
      *param_1 = iVar6 + *param_1;
      iVar6 = FUN_0fb71590(*param_2);
      *param_2 = iVar6 + *param_2;
                    /* WARNING: Could not recover jumptable at 0x0fb71744. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      uVar4 = (*UNRECOVERED_JUMPTABLE_00)();
      return uVar4;
    }
    lVar3 = FUN_0fb713c0(*puVar2 & 0xff);
    if (lVar3 != 0xc) {
LAB_0fb7177c:
                    /* WARNING: Could not recover jumptable at 0x0fb7178c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      uVar4 = (*UNRECOVERED_JUMPTABLE_00)();
      return uVar4;
    }
    lVar3 = FUN_0fb713c0(*puVar1 & 0xff);
    if (lVar3 == 0xc) {
      lVar3 = FUN_0fb71620(*param_1);
      lVar5 = FUN_0fb71620(*param_2);
      if (lVar3 == lVar5) goto LAB_0fb7177c;
      iVar6 = FUN_0fb71590(*param_1);
      *param_1 = iVar6 + *param_1;
      iVar6 = FUN_0fb71590(*param_2);
      uVar4 = 1;
      *param_2 = iVar6 + *param_2;
    }
    else {
      iVar6 = FUN_0fb71590();
      *param_1 = iVar6 + *param_1;
      iVar6 = FUN_0fb71590(*param_2);
      uVar4 = 2;
      *param_2 = iVar6 + *param_2;
    }
  }
  else {
LAB_0fb71794:
    iVar6 = FUN_0fb71590(*param_1);
    *param_1 = iVar6 + *param_1;
    iVar6 = FUN_0fb71590(*param_2);
    uVar4 = 2;
    *param_2 = iVar6 + *param_2;
  }
  return uVar4;
}



// === FUN_0fb71880 @ 0fb71880 (276 bytes) ===

uint FUN_0fb71880(undefined8 param_1,undefined8 param_2,undefined8 param_3,int param_4,int param_5)

{
  uint uVar2;
  uint uVar3;
  longlong lVar1;
  uint uVar4;
  undefined1 auStack_50 [4];
  undefined1 auStack_4c [4];
  
  uVar2 = FUN_0fb71450(param_1,auStack_50);
  uVar2 = uVar2 & 0xffff;
  uVar3 = FUN_0fb71450(param_2,auStack_4c);
  uVar3 = uVar3 & 0xffff;
  if (uVar2 != uVar3) {
    warning("parameter count for %s in %s\'s interface section is %ld, it does not match the one in %s with count %ld\n"
            ,param_3,*(undefined4 *)(param_5 + 0x18),uVar3,*(undefined4 *)(param_4 + 0x18),uVar2);
  }
  uVar4 = (uint)(uVar2 != uVar3);
  uVar3 = 0;
  if (uVar2 != 0) {
    do {
      lVar1 = FUN_0fb71660(auStack_50,auStack_4c);
      if (lVar1 != 0) {
        if (lVar1 == 2) {
          warning("the type of the %ld-th parameter for %s in the interface section of %s does not match the one in %s\n"
                  ,uVar3,param_3,*(undefined4 *)(param_5 + 0x18),*(undefined4 *)(param_4 + 0x18));
          uVar4 = uVar4 + 1;
        }
        else {
          warning("the size of the %ld-th parameter for %s in the in terface section of %s does not match the one in %s\n"
                  ,uVar3,param_3,*(undefined4 *)(param_5 + 0x18),*(undefined4 *)(param_4 + 0x18));
          uVar4 = uVar4 + 1;
        }
      }
      uVar3 = uVar3 + 1;
    } while (uVar3 != uVar2);
  }
  return uVar4;
}



// === FUN_0fb719a0 @ 0fb719a0 (268 bytes) ===

int FUN_0fb719a0(undefined8 param_1,undefined8 param_2,undefined8 param_3,int param_4,int param_5)

{
  uint uVar2;
  longlong lVar1;
  int iVar3;
  int iVar4;
  int iVar5;
  code *UNRECOVERED_JUMPTABLE;
  undefined1 auStack_50 [4];
  undefined1 auStack_4c [4];
  
  uVar2 = FUN_0fb71450(param_1,auStack_50);
  iVar3 = (uVar2 & 0xffff) - 1;
  iVar5 = 0;
  if (0 < iVar3) {
    iVar4 = 0;
    do {
      lVar1 = FUN_0fb71660(auStack_50,auStack_4c);
      if (lVar1 != 0) {
        if (lVar1 == 2) {
          warning("the type of the %ld-th parameter for %s in the interface section of %s does not match the one in %s\n"
                  ,iVar4,param_3,*(undefined4 *)(param_5 + 0x18),*(undefined4 *)(param_4 + 0x18));
          iVar5 = iVar5 + 1;
        }
        else {
          warning("the size of the %ld-th parameter for %s in the interface section of %s does not match the one in %s\n"
                  ,iVar4,param_3,*(undefined4 *)(param_5 + 0x18),*(undefined4 *)(param_4 + 0x18));
          iVar5 = iVar5 + 1;
        }
      }
      iVar4 = iVar4 + 1;
    } while (iVar4 < iVar3);
    return iVar5;
  }
                    /* WARNING: Could not recover jumptable at 0x0fb71aac. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  iVar3 = (*UNRECOVERED_JUMPTABLE)();
  return iVar3;
}



// === FUN_0fb71ac0 @ 0fb71ac0 (372 bytes) ===

undefined8 FUN_0fb71ac0(int param_1,int param_2,undefined8 param_3,int param_4,int param_5)

{
  undefined8 uVar1;
  ushort uVar2;
  
  uVar1 = 0;
  if ((*(ushort *)(param_1 + 4) & 0x4080) == 0) {
    if ((*(ushort *)(param_1 + 4) & 0x4000) == 0) {
      if (*(char *)(param_1 + 7) == *(char *)(param_2 + 7)) {
        uVar1 = 0;
      }
      else {
        warning("fpmask for %s specified in %s\'s interface section does not match the one in %s\'s interface section. \n"
                ,param_3,*(undefined4 *)(param_4 + 0x18),*(undefined4 *)(param_5 + 0x18));
        uVar1 = 1;
      }
    }
    uVar2 = *(ushort *)(param_2 + 4);
  }
  else {
    uVar2 = *(ushort *)(param_2 + 4);
    if ((uVar2 & 0x8000) == 0) {
      uVar1 = 0;
      if (*(char *)(param_2 + 7) != '\0') {
        warning("floating-point parameters exist in the call for \"%s\", a VARARG function, in object \"%s\" without a prototype -- would result in invalid result.  Definition can be found in object \"%s\""
                ,param_3,*(undefined4 *)(param_5 + 0x18),*(undefined4 *)(param_4 + 0x18));
        uVar1 = 1;
        uVar2 = *(ushort *)(param_2 + 4);
      }
    }
    else {
      uVar1 = 0;
    }
  }
  if (((uVar2 & 0x10) != 0) && ((*(ushort *)(param_1 + 4) & 0x10) != 0)) {
    if ((*(ushort *)(param_1 + 4) & 0x4000) == 0) {
      uVar1 = FUN_0fb71880(param_1,param_2,param_3,param_4,param_5);
      return uVar1;
    }
    uVar1 = FUN_0fb719a0(param_1,param_2);
    return uVar1;
  }
  return uVar1;
}



// === FUN_0fb71c40 @ 0fb71c40 (672 bytes) ===

void FUN_0fb71c40(int param_1,int *param_2,undefined8 param_3,int param_4)

{
  int *piVar1;
  undefined4 *puVar2;
  longlong lVar3;
  undefined4 uVar4;
  int iVar5;
  int *piVar6;
  int *piVar7;
  undefined4 *puVar8;
  code *UNRECOVERED_JUMPTABLE;
  
  piVar6 = *(int **)(param_1 + 4);
  piVar7 = (int *)*piVar6;
  if (piVar7 == (int *)0x0) {
    if ((*(ushort *)(param_2 + 1) & 0x80) != 0) {
      uVar4 = FUN_0fb71340(param_2,param_4,0);
      **(undefined4 **)(param_1 + 4) = uVar4;
      piVar6 = *(int **)(param_1 + 4);
      piVar7 = (int *)*piVar6;
    }
  }
  else {
    piVar1 = (int *)*piVar7;
    if (((*(ushort *)(piVar1 + 1) & 0x80) != 0) && ((*(ushort *)(param_2 + 1) & 0x80) != 0)) {
      iVar5 = (int)(uint)*(byte *)(*piVar1 * 0x10 + *(int *)(piVar7[1] + 100) + 0xc) >> 4;
      if (iVar5 != 2) {
        if ((int)(uint)*(byte *)(*(int *)(param_4 + 100) + *param_2 * 0x10 + 0xc) >> 4 != 2) {
          warning("Definition of %s in %s\'s interface section appears at least once before\n",
                  param_3,*(undefined4 *)(param_4 + 0x18));
          piVar6 = *(int **)(param_1 + 4);
          piVar7 = (int *)*piVar6;
          iVar5 = (int)(uint)*(byte *)(*piVar1 * 0x10 + *(int *)(piVar7[1] + 100) + 0xc) >> 4;
        }
        if (iVar5 != 2) goto joined_r0x0fb71e00;
      }
      if ((int)(uint)*(byte *)(*(int *)(param_4 + 100) + *param_2 * 0x10 + 0xc) >> 4 != 2) {
        *piVar7 = (int)param_2;
        *(int *)(**(int **)(param_1 + 4) + 4) = param_4;
        piVar6 = *(int **)(param_1 + 4);
        piVar7 = (int *)*piVar6;
      }
    }
  }
joined_r0x0fb71e00:
  if (piVar7 == (int *)0x0) {
    puVar8 = (undefined4 *)piVar6[1];
    uVar4 = 0;
    if (puVar8 != (undefined4 *)0x0) {
      do {
        lVar3 = FUN_0fb71ac0(param_2,*puVar8,param_3,param_4,puVar8[1]);
        if (lVar3 == 0) goto LAB_0fb71da4;
        puVar8 = (undefined4 *)puVar8[2];
      } while (puVar8 != (undefined4 *)0x0);
      if (lVar3 == 0) goto LAB_0fb71da4;
      uVar4 = *(undefined4 *)(*(int *)(param_1 + 4) + 4);
    }
    uVar4 = FUN_0fb71340(param_2,param_4,uVar4);
    *(undefined4 *)(*(int *)(param_1 + 4) + 4) = uVar4;
                    /* WARNING: Could not recover jumptable at 0x0fb71e88. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*UNRECOVERED_JUMPTABLE)();
    return;
  }
  if ((*(ushort *)(param_2 + 1) & 0x80) == 0) {
    FUN_0fb71ac0(*piVar7,param_2,param_3,piVar7[1],param_4);
    piVar6 = *(int **)(param_1 + 4);
  }
  puVar8 = (undefined4 *)piVar6[1];
  if ((undefined4 *)piVar6[1] != (undefined4 *)0x0) {
    do {
      FUN_0fb71ac0(*(undefined4 *)**(int **)(param_1 + 4),*puVar8,param_3,
                   ((undefined4 *)**(int **)(param_1 + 4))[1],puVar8[1]);
      puVar2 = (undefined4 *)puVar8[2];
      free(puVar8);
      puVar8 = puVar2;
    } while (puVar2 != (undefined4 *)0x0);
    *(undefined4 *)(*(int *)(param_1 + 4) + 4) = 0;
                    /* WARNING: Could not recover jumptable at 0x0fb71d9c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*UNRECOVERED_JUMPTABLE)();
    return;
  }
LAB_0fb71da4:
                    /* WARNING: Could not recover jumptable at 0x0fb71dc0. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
}



// === FUN_0fb71ee0 @ 0fb71ee0 (108 bytes) ===

void FUN_0fb71ee0(undefined8 param_1)

{
  undefined8 uVar1;
  longlong lVar2;
  code *UNRECOVERED_JUMPTABLE_00;
  
  uVar1 = hashmod(param_1,&DAT_0fbd3954);
  lVar2 = hashmake(uVar1,param_1,1,&DAT_0fbd3948);
  if (lVar2 != 0) {
                    /* WARNING: Could not recover jumptable at 0x0fb71f2c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*UNRECOVERED_JUMPTABLE_00)();
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x0fb71f44. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE_00)(UNRECOVERED_JUMPTABLE_00,0);
  return;
}



// === FUN_0fb71f50 @ 0fb71f50 (152 bytes) ===

undefined4 * FUN_0fb71f50(undefined4 param_1)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  
  if (DAT_0fbd394c <= DAT_0fbd3950) {
    realloc_tbl(&DAT_0fbd3948);
  }
  puVar1 = (undefined4 *)(DAT_0fbd3948 + DAT_0fbd3950 * 8);
  *puVar1 = param_1;
  puVar2 = (undefined4 *)malloc(8);
  if (puVar2 == (undefined4 *)0x0) {
    fatal("cannot malloc for psym_iface\n");
  }
  puVar2[1] = 0;
  *puVar2 = 0;
  puVar1[1] = puVar2;
  DAT_0fbd3950 = DAT_0fbd3950 + 1;
  return puVar1;
}



// === FUN_0fb71ff0 @ 0fb71ff0 (212 bytes) ===

void FUN_0fb71ff0(int *param_1,int param_2)

{
  longlong lVar1;
  undefined8 uVar2;
  
  lVar1 = FUN_0fb71ee0(*(int *)(*(int *)(param_2 + 100) + *param_1 * 0x10) +
                       *(int *)(param_2 + 0x60));
  if (lVar1 != 0) {
    FUN_0fb71c40(lVar1,param_1,
                 *(int *)(*(int *)(param_2 + 100) + *param_1 * 0x10) + *(int *)(param_2 + 0x60),
                 param_2);
    return;
  }
  uVar2 = FUN_0fb71f50(*(int *)(*(int *)(param_2 + 100) + *param_1 * 0x10) +
                       *(int *)(param_2 + 0x60));
  FUN_0fb71c40(uVar2,param_1,
               *(int *)(*(int *)(param_2 + 100) + *param_1 * 0x10) + *(int *)(param_2 + 0x60),
               param_2);
  return;
}



// === process_interface @ 0fb720d0 (132 bytes) ===

undefined8 process_interface(int param_1)

{
  uint uVar1;
  uint uVar2;
  
  uVar1 = *(uint *)(param_1 + 0x118);
  uVar2 = *(int *)(param_1 + 0x11c) + uVar1;
  for (; uVar1 < uVar2; uVar1 = uVar1 + 8) {
    FUN_0fb71ff0(uVar1,param_1);
    if ((*(ushort *)(uVar1 + 4) & 0x38) != 0) {
      uVar1 = *(ushort *)(uVar1 + 8) + uVar1;
    }
  }
  return 0xffffffffffffffff;
}



// === create_interface_symtab @ 0fb72160 (60 bytes) ===

void create_interface_symtab(void)

{
  alloc_tbl(&DAT_0fbd3948,0x2000);
  hashinit(&DAT_0fbd3954,0x2000,0x2011);
  FUN_0fb71ee0(&DAT_0fbda428);
  FUN_0fb71f50(&DAT_0fbda428);
  return;
}



// === FUN_0fb721a0 @ 0fb721a0 (68 bytes) ===

undefined8 FUN_0fb721a0(int *param_1,int *param_2)

{
  int iVar1;
  int iVar2;
  
  iVar1 = *(int *)(*(int *)(*param_1 + 0xc) + 0xd4);
  iVar2 = *(int *)(*(int *)(*param_2 + 0xc) + 0xd4);
  if (iVar2 < iVar1) {
    return 0xffffffffffffffff;
  }
  if (iVar1 == iVar2) {
    return 0;
  }
  return 1;
}



// === FUN_0fb721f0 @ 0fb721f0 (96 bytes) ===

void FUN_0fb721f0(longlong param_1)

{
  if (param_1 != 0) {
    if (size_of_fini_array <= next_fini_to_use) {
      FUN_0fb72250();
    }
    *(int *)(fini_array + next_fini_to_use * 4) = (int)param_1;
    next_fini_to_use = next_fini_to_use + 1;
    return;
  }
  return;
}



// === FUN_0fb72250 @ 0fb72250 (156 bytes) ===

/* WARNING: Instruction at (ram,0x0fb722b0) overlaps instruction at (ram,0x0fb722ac)
    */

void FUN_0fb72250(void)

{
  int iVar1;
  longlong lVar2;
  int iVar3;
  int iVar4;
  
  iVar1 = size_of_fini_array;
  iVar4 = (next_fini_to_use >> 4) + 0x28;
  if (verbose != '\0') {
    trace("in grow_fini_array alloc goes from %ld to %ld entries\n",size_of_fini_array,
          (next_fini_to_use >> 4) + size_of_fini_array + 0x28);
  }
  size_of_fini_array = size_of_fini_array + iVar4;
  iVar3 = size_of_fini_array * 4;
  lVar2 = realloc(fini_array);
  if (lVar2 == 0) {
    fatal("Cannot malloc fini_array of %lu bytes, so cannot run fini code. Execution cancelled.",
          iVar3);
  }
  fini_array = (int)lVar2;
  blkclr((void *)(fini_array + iVar1 * 4),iVar4 * 4);
  return;
}



// === basename @ 0fb722f0 (68 bytes) ===

char * basename(char *__filename)

{
  char *pcVar1;
  code *UNRECOVERED_JUMPTABLE_00;
  
  pcVar1 = strrchr(__filename,0x2f);
  if (pcVar1 != (char *)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0fb7231c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    pcVar1 = (char *)(*UNRECOVERED_JUMPTABLE_00)();
    return pcVar1;
  }
                    /* WARNING: Could not recover jumptable at 0x0fb7232c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  pcVar1 = (char *)(*UNRECOVERED_JUMPTABLE_00)();
  return pcVar1;
}



// === locate_obj_by_name @ 0fb72340 (52 bytes) ===

void locate_obj_by_name(undefined4 param_1,longlong param_2)

{
  undefined4 uStack_10;
  uint uStack_c;
  
  uStack_c = (uint)(param_2 == 0);
  uStack_10 = param_1;
  foreach_sublist(pObj_Head,pathname_matches,&uStack_10);
  return;
}



// === mark_handles_on_obj_obsolete @ 0fb72380 (172 bytes) ===

void mark_handles_on_obj_obsolete(int param_1)

{
  int iVar1;
  code *UNRECOVERED_JUMPTABLE;
  
  iVar1 = handle_list_anchor;
  do {
    while( true ) {
      if (iVar1 == 0) {
                    /* WARNING: Could not recover jumptable at 0x0fb72424. Too many branches */
                    /* WARNING: Treating indirect jump as call */
        (*UNRECOVERED_JUMPTABLE)();
        return;
      }
      if ((*(char *)(iVar1 + 4) == '\x01') && (*(int *)(iVar1 + 0xc) == param_1)) break;
LAB_0fb723c0:
      iVar1 = *(int *)(iVar1 + 0x10);
    }
    if (verbose == '\0') {
      *(undefined4 *)(iVar1 + 0xc) = 0;
      *(undefined1 *)(iVar1 + 4) = 3;
      goto LAB_0fb723c0;
    }
    trace("handle: removed obj %s\n",*(undefined4 *)(param_1 + 0x18));
    *(undefined4 *)(iVar1 + 0xc) = 0;
    *(undefined1 *)(iVar1 + 4) = 3;
    iVar1 = *(int *)(iVar1 + 0x10);
  } while( true );
}



// === mark_handles_on_obj_switched @ 0fb72430 (160 bytes) ===

void mark_handles_on_obj_switched(int param_1,int param_2)

{
  char cVar1;
  int iVar2;
  code *UNRECOVERED_JUMPTABLE;
  
  if (handle_list_anchor != 0) {
    cVar1 = *(char *)(handle_list_anchor + 4);
    iVar2 = handle_list_anchor;
    while( true ) {
      if (((cVar1 == '\x01') && (*(int *)(iVar2 + 0xc) == param_1)) &&
         (*(int *)(iVar2 + 0xc) = param_2, verbose != '\0')) {
        trace("Handle: switched from %s to %s\n",*(undefined4 *)(param_1 + 0x18),
              *(undefined4 *)(param_2 + 0x18));
        iVar2 = *(int *)(iVar2 + 0x10);
      }
      else {
        iVar2 = *(int *)(iVar2 + 0x10);
      }
      if (iVar2 == 0) break;
      cVar1 = *(char *)(iVar2 + 4);
    }
  }
                    /* WARNING: Could not recover jumptable at 0x0fb724c8. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
}



// === FUN_0fb724d0 @ 0fb724d0 (356 bytes) ===

void FUN_0fb724d0(int param_1)

{
  uint uVar1;
  int iVar2;
  int *piVar3;
  int *piVar4;
  
  uVar1 = *(uint *)(param_1 + 4);
  piVar3 = *(int **)(param_1 + 8);
  if (*(int **)(param_1 + 8) == (int *)0x0) {
    uVar1 = DAT_0fbd31d4;
    piVar3 = DAT_0fbd31d8;
  }
  if (uVar1 != 0) {
    piVar4 = piVar3 + uVar1;
    if ((uVar1 & 1) != 0) {
      iVar2 = *piVar3;
      if (*(char *)(iVar2 + 0xd9) == '\0') {
        objlist_delete_element(&pObj_Head,iVar2);
        if (*(int *)(iVar2 + 0x10) != 0) {
          elfunmap(iVar2);
        }
        delete_object(iVar2);
      }
      piVar3 = piVar3 + 1;
    }
    if ((int)uVar1 >> 1 != 0) {
      iVar2 = *piVar3;
      while( true ) {
        if (*(char *)(iVar2 + 0xd9) == '\0') {
          objlist_delete_element(&pObj_Head,iVar2);
          if (*(int *)(iVar2 + 0x10) != 0) {
            elfunmap(iVar2);
          }
          delete_object(iVar2);
          iVar2 = piVar3[1];
        }
        else {
          iVar2 = piVar3[1];
        }
        if (*(char *)(iVar2 + 0xd9) == '\0') {
          objlist_delete_element(&pObj_Head,iVar2);
          if (*(int *)(iVar2 + 0x10) != 0) {
            elfunmap(iVar2);
          }
          delete_object(iVar2);
        }
        piVar3 = piVar3 + 2;
        if (piVar3 == piVar4) break;
        iVar2 = *piVar3;
      }
    }
  }
  if (_rld_libdl_depth == 1) {
    foreach_obj(pObj_Head,unset_tmp_unhidden_bit,0);
  }
  return;
}



// === FUN_0fb72640 @ 0fb72640 (116 bytes) ===

longlong FUN_0fb72640(undefined8 param_1,undefined8 param_2,undefined8 param_3)

{
  longlong lVar1;
  longlong lVar2;
  code *UNRECOVERED_JUMPTABLE;
  
  lVar1 = FUN_0fb726c0(param_1,param_3);
  if (lVar1 == 0) {
    lVar2 = locate_obj_by_name(param_1,param_3);
    if ((lVar2 != -1) && (lVar2 != 0)) {
      FUN_0fb728d0(lVar2,0,param_2);
                    /* WARNING: Could not recover jumptable at 0x0fb7269c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      lVar1 = (*UNRECOVERED_JUMPTABLE)();
      return lVar1;
    }
  }
  return lVar1;
}



// === FUN_0fb726c0 @ 0fb726c0 (200 bytes) ===

/* WARNING: Instruction at (ram,0x0fb72744) overlaps instruction at (ram,0x0fb72740)
    */

void FUN_0fb726c0(char *param_1,longlong param_2)

{
  char cVar1;
  int iVar3;
  longlong lVar2;
  int iVar4;
  undefined8 in_ra;
  char *pcStack_30;
  uint uStack_2c;
  undefined8 UNRECOVERED_JUMPTABLE;
  
  UNRECOVERED_JUMPTABLE._4_4_ = (code *)in_ra;
  uStack_2c = (uint)(param_2 == 0);
  pcStack_30 = param_1;
  if (handle_list_anchor != 0) {
    cVar1 = *(char *)(handle_list_anchor + 4);
    iVar4 = handle_list_anchor;
    UNRECOVERED_JUMPTABLE = in_ra;
    do {
      if (cVar1 == '\x01') {
        if (param_2 == 0) {
          lVar2 = pathname_matches(*(int *)(iVar4 + 0xc),&pcStack_30);
          if (lVar2 != -1) break;
        }
        else {
          iVar3 = strcmp(*(char **)(*(int *)(iVar4 + 0xc) + 0x18),param_1);
          if (iVar3 == 0) break;
        }
        iVar4 = *(int *)(iVar4 + 0x10);
      }
      else {
        iVar4 = *(int *)(iVar4 + 0x10);
      }
      if (iVar4 == 0) break;
      cVar1 = *(char *)(iVar4 + 4);
    } while( true );
  }
                    /* WARNING: Could not recover jumptable at 0x0fb72760. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE._4_4_)();
  return;
}



// === FUN_0fb72790 @ 0fb72790 (232 bytes) ===

void * FUN_0fb72790(void)

{
  int iVar1;
  void *pvVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  
  pvVar2 = handle_list_freelist;
  if (handle_list_freelist != (void *)0x0) {
    handle_list_freelist = *(void **)((int)handle_list_freelist + 0x10);
    memset(pvVar2,0,0x14);
    return pvVar2;
  }
  pvVar2 = (void *)malloc(0x400);
  if (pvVar2 == (void *)0x0) {
    fatal("Out of memory to malloc handle block for dlopen\n");
  }
  memset(pvVar2,0,0x400);
  iVar5 = (int)pvVar2 + 0x3c;
  *(int *)((int)pvVar2 + 0x24) = (int)pvVar2 + 0x28;
  iVar3 = (int)pvVar2 + 0x28;
  do {
    *(int *)(iVar3 + 0x9c) = iVar5 + 0x8c;
    *(int *)(iVar3 + 0x88) = iVar5 + 0x78;
    *(int *)(iVar3 + 0x74) = iVar5 + 100;
    *(int *)(iVar3 + 0x60) = iVar5 + 0x50;
    *(int *)(iVar3 + 0x4c) = iVar5 + 0x3c;
    iVar4 = iVar5 + 0x28;
    iVar1 = iVar5 + 0x14;
    *(int *)(iVar3 + 0x10) = iVar5;
    iVar5 = iVar5 + 0xa0;
    *(int *)(iVar3 + 0x38) = iVar4;
    *(int *)(iVar3 + 0x24) = iVar1;
    iVar3 = iVar3 + 0xa0;
  } while (iVar5 != (int)pvVar2 + 0x3fc);
  handle_list_freelist = (void *)((int)pvVar2 + 0x14);
  return pvVar2;
}



// === FUN_0fb72880 @ 0fb72880 (72 bytes) ===

void FUN_0fb72880(int param_1)

{
  if (handle_list_anchor == param_1) {
    fatal("rld internal error in free_a_handle: handle_list_anchor\n");
  }
  *(undefined4 *)(param_1 + 0xc) = 0;
  *(undefined1 *)(param_1 + 4) = 0;
  *(int *)(param_1 + 0x10) = handle_list_freelist;
  handle_list_freelist = param_1;
  return;
}



// === FUN_0fb728d0 @ 0fb728d0 (136 bytes) ===

void FUN_0fb728d0(undefined4 param_1,undefined1 param_2,undefined4 param_3)

{
  undefined4 *puVar1;
  
  puVar1 = (undefined4 *)FUN_0fb72790();
  *(undefined1 *)(puVar1 + 1) = 1;
  puVar1[3] = param_1;
  *(undefined1 *)((int)puVar1 + 5) = param_2;
  *puVar1 = 0xdeadbeef;
  puVar1[2] = param_3;
  if (handle_list_anchor == (undefined4 *)0x0) {
    handle_list_tail = puVar1;
    handle_list_anchor = puVar1;
    return;
  }
  if (handle_list_tail != (undefined4 *)0x0) {
    *(undefined4 **)((int)handle_list_tail + 0x10) = puVar1;
    handle_list_tail = puVar1;
    return;
  }
  handle_list_tail = puVar1;
  return;
}



// === FUN_0fb72960 @ 0fb72960 (764 bytes) ===

void FUN_0fb72960(int param_1,longlong param_2)

{
  int iVar1;
  bool bVar2;
  char *pcVar5;
  int iVar6;
  undefined8 uVar3;
  longlong lVar4;
  int iVar7;
  int *piVar8;
  int iVar10;
  int iVar11;
  int *piVar12;
  char *__s;
  int *piVar13;
  int *piVar14;
  undefined1 *puVar15;
  undefined1 *puVar16;
  int aiStack_490 [4];
  undefined1 auStack_418 [1048];
  int iVar9;
  
  if (*(char *)(param_1 + 6) == '\0') {
    piVar14 = aiStack_490;
    piVar12 = aiStack_490;
    piVar13 = aiStack_490;
    aiStack_490[1] = 0;
    aiStack_490[0] = param_1;
    if (&stack0x00000000 != (undefined1 *)0x490) {
LAB_0fb72a24:
      iVar1 = *piVar12;
      if (*(char *)(iVar1 + 6) == '\0') {
        *(undefined1 *)(iVar1 + 6) = 1;
        *(int *)(iVar1 + 8) = *(int *)(iVar1 + 8) + 1;
        if (verbose != '\0') {
          trace("do_dlopen_dependencies ref count for %s incremented to %ld\n",
                *(undefined4 *)(*(int *)(iVar1 + 0xc) + 0x18));
        }
        iVar10 = *(int *)(iVar1 + 0xc);
        iVar6 = *(int *)(iVar10 + 0xb0);
        iVar7 = 0;
        iVar11 = 0;
        if (0 < iVar6) {
          iVar9 = *(int *)(iVar10 + 0x74);
          do {
            piVar8 = (int *)(iVar9 + iVar7);
            if ((_rld_do_delay_loaded_libs_instantly != '\0') || ((piVar8[4] & 0x10U) == 0)) {
              __s = (char *)(*piVar8 + *(int *)(iVar10 + 0x60));
              pcVar5 = strchr(__s,0x2f);
              bVar2 = pcVar5 != (char *)0x0;
              iVar6 = FUN_0fb726c0(__s);
              if (iVar6 == 0) {
                if (piVar8[3] != 0) {
                  uVar3 = add_version_to_dlopen_name
                                    (__s,*(int *)(iVar10 + 0x60) + piVar8[3],auStack_418,0x400);
                  iVar6 = FUN_0fb726c0(uVar3,bVar2);
                }
                if (iVar6 == 0) {
                  iVar6 = FUN_0fb72640(__s,0,bVar2);
                  if ((iVar6 == 0) && (piVar8[3] != 0)) {
                    uVar3 = add_version_to_dlopen_name
                                      (__s,*(int *)(iVar10 + 0x60) + piVar8[3],auStack_418,0x400);
                    iVar6 = FUN_0fb72640(uVar3,0,bVar2);
                  }
                  goto LAB_0fb72b70;
                }
LAB_0fb72b78:
                lVar4 = match_interface_version(*(undefined4 *)(iVar6 + 0xc),iVar10,piVar8);
                if (lVar4 == 0) {
                  iVar9 = *(int *)(iVar6 + 0xc);
                  if (*(int *)(iVar9 + 0x98) == 0) {
                    pcVar5 = "<NULL>";
                  }
                  else {
                    pcVar5 = (char *)(*(int *)(iVar9 + 0x60) + *(int *)(iVar9 + 0x98));
                  }
                  fatal("object %s from liblist in %s has version \"%s\", which does not match the found object: %s (with version \"%s\")\n"
                        ,__s,*(undefined4 *)(*(int *)(iVar1 + 0xc) + 0x18),
                        piVar8[3] + *(int *)(iVar10 + 0x60),*(undefined4 *)(iVar9 + 0x18),pcVar5);
                }
                if ((((param_2 == 2) || (param_2 == 4)) || (param_2 == 3)) &&
                   (*(undefined1 *)(*(int *)(iVar6 + 0xc) + 0xca) = 0, debug_trace != '\0')) {
                  trace("reset o_is_hidden on %s\n",*(undefined4 *)(*(int *)(iVar6 + 0xc) + 0x18));
                }
                puVar15 = (undefined1 *)((int)piVar14 + -0x10);
                puVar16 = (undefined1 *)((int)piVar14 + -0x10);
                *(undefined4 *)((int)piVar14 + -0xc) = 0;
                *(int *)((int)piVar14 + -0x10) = iVar6;
                *(undefined1 **)((int)piVar13 + 4) = (undefined1 *)((int)piVar14 + -0x10);
              }
              else {
LAB_0fb72b70:
                puVar15 = (undefined1 *)piVar13;
                puVar16 = (undefined1 *)piVar14;
                if (iVar6 != 0) goto LAB_0fb72b78;
              }
              iVar10 = *(int *)(iVar1 + 0xc);
              iVar6 = *(int *)(iVar10 + 0xb0);
              piVar14 = (int *)puVar16;
              piVar13 = (int *)puVar15;
            }
            iVar11 = iVar11 + 1;
            iVar7 = iVar7 + 0x14;
            if (iVar6 <= iVar11) goto LAB_0fb72bd8;
            iVar9 = *(int *)(iVar10 + 0x74);
          } while( true );
        }
      }
      else if (verbose != '\0') {
        trace("Already is_processed %s\n",*(undefined4 *)(*(int *)(iVar1 + 0xc) + 0x18));
      }
      piVar12 = (int *)piVar12[1];
      goto LAB_0fb72a1c;
    }
  }
  return;
LAB_0fb72bd8:
  piVar12 = (int *)piVar12[1];
LAB_0fb72a1c:
  if (piVar12 == (int *)0x0) {
    return;
  }
  goto LAB_0fb72a24;
}



// === FUN_0fb72ca0 @ 0fb72ca0 (20 bytes) ===

void FUN_0fb72ca0(void)

{
  foreach_sublist(pObj_Head,&LAB_0fb72c70,0);
  return;
}



// === FUN_0fb72cc0 @ 0fb72cc0 (40 bytes) ===

void FUN_0fb72cc0(void)

{
  int iVar1;
  
  for (iVar1 = handle_list_anchor; iVar1 != 0; iVar1 = *(int *)(iVar1 + 0x10)) {
    *(undefined1 *)(iVar1 + 6) = 0;
  }
  return;
}



// === FUN_0fb72cf0 @ 0fb72cf0 (140 bytes) ===

void FUN_0fb72cf0(int param_1,longlong param_2)

{
  if (param_2 == 4) {
    if (debug_trace != '\0') {
      trace("Failing to set o_is_hidden in %s to mirror old error\n",*(undefined4 *)(param_1 + 0x18)
            ,0);
    }
  }
  else if ((param_2 != 2) && (param_2 != 3)) {
    foreach_obj(param_1,set_hidden_bit,0);
  }
  foreach_obj(pObj_Head,unset_tmp_unhidden_bit,0);
  foreach_obj(param_1,commit_obj,0);
  return;
}



// === FUN_0fb72d80 @ 0fb72d80 (1160 bytes) ===

/* WARNING: Instruction at (ram,0x0fb72fec) overlaps instruction at (ram,0x0fb72fe8)
    */

int FUN_0fb72d80(longlong param_1,longlong param_2,undefined8 param_3,longlong param_4,
                undefined4 *param_5)

{
  char *pcVar3;
  int iVar4;
  char *pcVar5;
  undefined8 uVar1;
  longlong lVar2;
  int aiStack_c70 [2];
  undefined1 auStack_c68 [2048];
  undefined1 *puStack_468;
  undefined1 uStack_464;
  undefined4 uStack_460;
  undefined4 uStack_45c;
  undefined1 auStack_458 [1024];
  longlong lStack_58;
  longlong lStack_50;
  
  uStack_464 = 0;
  uStack_460 = 0;
  puStack_468 = auStack_c68;
  auStack_c68[0] = 0;
  aiStack_c70[0] = 0;
  uStack_45c = 0x800;
  if (param_4 == 1) {
    lStack_50 = 4;
  }
  else if (param_4 == 2) {
    lStack_50 = 3;
  }
  else {
    lStack_50 = param_4;
    if ((param_4 != 4) && (param_4 != 3)) {
      if (verbose != '\0') {
        trace("Bad do_dlopen category %d setting LIBDL_DLADD_LIBLIST\n",param_4);
      }
      lStack_50 = 3;
    }
  }
  if (handle_list_anchor == 0) {
    FUN_0fb72ca0();
  }
  if (param_1 == 0) {
    pcVar3 = strchr(main_exec_name,0x2f);
    iVar4 = FUN_0fb72640(main_exec_name,1,pcVar3 != (char *)0x0);
    if (verbose != '\0') {
      trace("reference_count for %s initialized to %ld category %d\n",
            *(undefined4 *)(*(int *)(iVar4 + 0xc) + 0x18),*(undefined4 *)(iVar4 + 8),param_4);
    }
  }
  else {
    pcVar3 = strchr((char *)param_1,0x2f);
    iVar4 = FUN_0fb72640(param_1,0);
    if (iVar4 == 0) {
      pcVar5 = strchr((char *)param_1,0x2f);
      iVar4 = locate_obj(param_1,pcVar5 != (char *)0x0);
      if (iVar4 == -1) {
        lVar2 = load_single_object(param_1,param_2,param_3,0,&puStack_468,aiStack_c70);
        if (lVar2 == 0) {
          objlist_add_end(&pObj_Head,aiStack_c70[0]);
          add_to_global_cleanup_record(aiStack_c70[0]);
          iVar4 = aiStack_c70[0];
          if (debug_olist != '\0') {
            print_rld_object_list("_RLD_OP_ADD name, not orig_pathname, done");
          }
          verify_o_arch_is_ok(pObj_Head,aiStack_c70[0]);
          lStack_58 = 1;
        }
        else {
          lStack_58 = 0;
        }
      }
      else {
        lStack_58 = 0;
        aiStack_c70[0] = iVar4;
      }
      if (debug_trace != '\0') {
        trace("_RLD_OP_ADD set temp_unhidden on %s\n",*(undefined4 *)(aiStack_c70[0] + 0x18));
      }
      *(undefined1 *)(aiStack_c70[0] + 0xd8) = 1;
      if (param_4 == 2) {
        *(undefined1 *)(aiStack_c70[0] + 0xcc) = 1;
      }
      else if ((param_4 == 3) || (param_4 == 4)) {
        *(undefined1 *)(aiStack_c70[0] + 0xcd) = 1;
      }
      load_dependent_libs(iVar4,&puStack_468);
      if (param_4 == 2) {
        if (lStack_58 == 0) {
          foreach_obj(pObj_Head,unset_undefineds_are_resolved,0);
        }
        else {
          foreach_obj(pObj_Head,unset_undefineds_are_resolved_if_has_unres_undefs,0);
        }
      }
      else if (param_4 == 3) {
        if (lStack_58 == 0) {
          foreach_obj(pObj_Head,unset_undefineds_are_resolved,0);
        }
        else {
          foreach_obj(pObj_Head,unset_undefineds_are_resolved_if_has_unres_undefs,0);
        }
      }
      set_postcksum_then_handle_undefineds_and_conflicts(iVar4,1);
      foreach_obj(pObj_Head,call_pixie_init,0);
      __rld_calls_on_rld_list_change();
      do_oex_setting(iVar4,1,param_1);
      *param_5 = _rld_global_cleanup_record;
      param_5[1] = DAT_0fbd31d4;
      param_5[2] = DAT_0fbd31d8;
      DAT_0fbd31d4 = 0;
      _rld_global_cleanup_record = 0;
      DAT_0fbd31d8 = 0;
      execute_all_init_sections();
      if (_rld_libdl_depth == 1) {
        FUN_0fb72cf0(iVar4,param_4);
      }
      iVar4 = FUN_0fb72640(param_1,0,pcVar3 != (char *)0x0);
      if ((iVar4 == 0) && (param_2 != 0)) {
        uVar1 = add_version_to_dlopen_name(param_1,param_2,auStack_458,0x400);
        iVar4 = FUN_0fb72640(uVar1,0,pcVar3 != (char *)0x0);
      }
      if ((iVar4 != 0) && (verbose != '\0')) {
        trace("Handle ref count better be 0 here: %d. increment to 1 later \n",
              *(undefined4 *)(iVar4 + 8));
      }
    }
    else if (verbose != '\0') {
      trace("reference_count for %s is %ld category %d\n",
            *(undefined4 *)(*(int *)(iVar4 + 0xc) + 0x18),*(undefined4 *)(iVar4 + 8),param_4);
    }
  }
  free_things_tried(&puStack_468);
  if (iVar4 != 0) {
    if ((((param_4 == 2) || (param_4 == 4)) || (param_4 == 3)) &&
       (*(undefined1 *)(*(int *)(iVar4 + 0xc) + 0xca) = 0, debug_trace != '\0')) {
      trace("B reset o_is_hidden on %s\n",*(undefined4 *)(*(int *)(iVar4 + 0xc) + 0x18));
    }
    lVar2 = lStack_50;
    FUN_0fb72cc0();
    FUN_0fb72960(iVar4,lVar2);
    return iVar4;
  }
  return 0;
}



// === _rld_dlopen @ 0fb73210 (1328 bytes) ===

/* WARNING: Instruction at (ram,0x0fb734a8) overlaps instruction at (ram,0x0fb734a4)
    */

undefined8
_rld_dlopen(ulonglong param_1,ulonglong param_2,char *param_3,undefined8 param_4,ulonglong param_5)

{
  int iVar4;
  size_t sVar5;
  ulonglong uVar1;
  undefined8 uVar2;
  longlong lVar3;
  char *pcVar6;
  char *pcVar7;
  code *UNRECOVERED_JUMPTABLE;
  undefined4 uStack_170;
  undefined4 uStack_16c;
  undefined4 uStack_168;
  undefined1 uStack_160;
  undefined1 uStack_15f;
  undefined1 uStack_15e;
  undefined1 auStack_158 [224];
  longlong lStack_78;
  longlong lStack_70;
  ulonglong uStack_68;
  longlong lStack_60;
  ulonglong uStack_58;
  longlong lStack_50;
  ulonglong uStack_48;
  ulonglong uStack_40;
  ulonglong uStack_38;
  undefined8 uStack_30;
  
  uStack_170 = DAT_0fbd3978;
  uStack_16c = DAT_0fbd397c;
  uStack_168 = DAT_0fbd3980;
  uStack_30 = param_4;
  cleanup_record_memory_free(&_rld_global_cleanup_record);
  uStack_160 = rtld_lazy_mode;
  uStack_15f = override_dso_hidden_flag;
  uStack_15e = save_error_message_for_dlerror;
  memmove(auStack_158,saved_error_exit,0xe0);
  if (verbose != '\0') {
    uVar1 = param_1;
    if (param_1 == 0) {
      uVar1 = ZEXT48("(null)");
    }
    pcVar7 = param_3;
    if (param_3 == (char *)0x0) {
      pcVar7 = "(null)";
    }
    pcVar6 = "unknown category";
    if (param_5 < 9) {
      if ((longlong)param_5 < 0) {
        pcVar6 = "unknown category";
      }
      else {
        pcVar6 = *(char **)(category_names + (int)param_5 * 4);
      }
    }
    trace("dlopening -------- %s mode 0x%x version %s l_flags 0x%x category 0x%x %s\n",uVar1,param_2
          ,pcVar7,uStack_30,param_5,pcVar6);
  }
  iVar4 = setjmp((__jmp_buf_tag *)saved_error_exit);
  if (iVar4 != 0) {
    FUN_0fb724d0(&uStack_170);
    cleanup_record_memory_free(&uStack_170);
    cleanup_record_memory_free(&_rld_global_cleanup_record);
    save_error_message_for_dlerror = uStack_15e;
    override_dso_hidden_flag = uStack_15f;
    rtld_lazy_mode = uStack_160;
    memmove(saved_error_exit,auStack_158,0xe0);
    return 0;
  }
  if (param_1 == 0) {
    uStack_38 = 0;
  }
  else {
    pcVar7 = (char *)param_1;
    if (perf_suffix == (char *)0x0) {
      uStack_38 = param_1;
      if (is_pixie != '\0') {
        sVar5 = strlen(pcVar7);
        if (6 < (ulonglong)(longlong)(int)sVar5) {
          uStack_68 = (longlong)(int)sVar5;
          iVar4 = strcmp(pcVar7 + (sVar5 - 6),".pixie");
          sVar5 = (size_t)uStack_68;
          if (iVar4 == 0) {
            lStack_78 = (longlong)(int)(sVar5 + 1);
            uStack_40 = malloc();
            uStack_38 = uStack_40;
            if (uStack_40 == 0) {
              fatal("Out of space malloc %ld string bytesC \n",lStack_78);
            }
            __rld_strlcpy(uStack_40,param_1,lStack_78);
            goto LAB_0fb7340c;
          }
        }
        lStack_60 = (longlong)(int)(sVar5 + 7);
        uStack_40 = malloc();
        uStack_38 = uStack_40;
        if (uStack_40 == 0) {
          fatal("Out of space malloc %ld string bytesD \n",lStack_60);
        }
        __rld_strlcpy(uStack_40,param_1,lStack_60);
        __rld_strlcat(uStack_40,".pixie",lStack_60);
      }
    }
    else {
      sVar5 = strlen(perf_suffix);
      uStack_48 = (ulonglong)(int)sVar5;
      sVar5 = strlen(pcVar7);
      uVar1 = (ulonglong)(int)sVar5;
      if ((uStack_48 < uVar1) &&
         (uStack_58 = uVar1, iVar4 = strcmp(pcVar7 + (sVar5 - (int)uStack_48),perf_suffix),
         uVar1 = uStack_58, iVar4 == 0)) {
        lStack_70 = (longlong)((int)uStack_58 + 1);
        uStack_40 = malloc();
        uStack_38 = uStack_40;
        if (uStack_40 == 0) {
          fatal("Out of space malloc %ld string bytesA \n",lStack_70);
          pcVar6 = (char *)uStack_40;
        }
        else {
          pcVar6 = (char *)uStack_40;
        }
        strcpy(pcVar6,pcVar7);
      }
      else {
        lStack_50 = (longlong)((int)uVar1 + (int)uStack_48 + 1);
        uStack_40 = malloc();
        uStack_38 = uStack_40;
        if (uStack_40 == 0) {
          fatal("Out of space malloc %ld string bytesB \n",lStack_50);
        }
        __rld_strlcpy(uStack_40,param_1,lStack_50);
        __rld_strlcat(uStack_40,perf_suffix,lStack_50);
      }
    }
  }
LAB_0fb7340c:
  uVar1 = param_2 & 0xb;
  if (uVar1 != 1) {
    if (uVar1 != 2) {
      if (uVar1 != 8) {
        rld_snprintf(last_error_message,cur_last_error_message_len,"dlopen: invalid mode 0%ld",
                     param_2);
        override_dso_hidden_flag = uStack_15f;
        rtld_lazy_mode = uStack_160;
        last_error = last_error_message;
        memmove(saved_error_exit,auStack_158,0xe0);
        cleanup_record_memory_free(&uStack_170);
        cleanup_record_memory_free(&_rld_global_cleanup_record);
                    /* WARNING: Could not recover jumptable at 0x0fb73494. Too many branches */
                    /* WARNING: Treating indirect jump as call */
        uVar2 = (*UNRECOVERED_JUMPTABLE)();
        return uVar2;
      }
      if (((param_5 == 0) == true) || (param_5 == 3)) {
        rld_snprintf(last_error_message,cur_last_error_message_len,"dlopen: invalid mode %ld",
                     param_2);
        override_dso_hidden_flag = uStack_15f;
        rtld_lazy_mode = uStack_160;
        last_error = last_error_message;
        cleanup_record_memory_free(&uStack_170);
        cleanup_record_memory_free(&_rld_global_cleanup_record);
        return 0;
      }
      goto LAB_0fb734ac;
    }
    if ((param_5 != 2) && (param_5 != 3)) goto LAB_0fb734ac;
  }
  rtld_lazy_mode = 1;
LAB_0fb734ac:
  last_error = 0;
  save_error_message_for_dlerror = 1;
  if ((param_2 & 4) != 0) {
    override_dso_hidden_flag = 1;
  }
  lVar3 = FUN_0fb72d80(uStack_38,param_3,uStack_30,param_5,&uStack_170);
  save_error_message_for_dlerror = uStack_15e;
  override_dso_hidden_flag = uStack_15f;
  rtld_lazy_mode = uStack_160;
  memmove(saved_error_exit,auStack_158,0xe0);
  if ((is_pixie != '\0') && (lVar3 == 0)) {
    rld_snprintf(last_error_message,cur_last_error_message_len,
                 "dlopen: Cannot find %s, please use pixie to create one",uStack_38);
    last_error = last_error_message;
    cleanup_record_memory_free(&uStack_170);
    cleanup_record_memory_free(&_rld_global_cleanup_record);
                    /* WARNING: Could not recover jumptable at 0x0fb7355c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    uVar2 = (*UNRECOVERED_JUMPTABLE)();
    return uVar2;
  }
  cleanup_record_memory_free(&uStack_170);
  cleanup_record_memory_free(&_rld_global_cleanup_record);
                    /* WARNING: Could not recover jumptable at 0x0fb73590. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  uVar2 = (*UNRECOVERED_JUMPTABLE)();
  return uVar2;
}



// === FUN_0fb73740 @ 0fb73740 (144 bytes) ===

/* WARNING: Instruction at (ram,0x0fb7376c) overlaps instruction at (ram,0x0fb73768)
    */

undefined8 FUN_0fb73740(int *param_1)

{
  int *piVar1;
  char *pcVar2;
  undefined8 uVar3;
  undefined4 uVar4;
  char cVar5;
  char cVar6;
  
  cVar6 = s_libdl__invalid_handle_0fbda988[0x15];
  cVar5 = s_libdl__invalid_handle_0fbda988[0x14];
  uVar4 = s_libdl__invalid_handle_0fbda988._16_4_;
  uVar3 = s_libdl__invalid_handle_0fbda988._8_8_;
  pcVar2 = last_error_message;
  piVar1 = handle_list_anchor;
  do {
    if (piVar1 == (int *)0x0) {
LAB_0fb73770:
      *(undefined8 *)last_error_message = s_libdl__invalid_handle_0fbda988._0_8_;
      *(undefined8 *)(pcVar2 + 8) = uVar3;
      *(undefined4 *)(pcVar2 + 0x10) = uVar4;
      pcVar2[0x15] = cVar6;
      pcVar2[0x14] = cVar5;
      last_error = last_error_message;
      return 0xffffffffffffffff;
    }
    if (param_1 == piVar1) {
      if ((*piVar1 == DAT_0fbde4c8) && (*(char *)(piVar1 + 1) == '\x01')) {
        return 0;
      }
      goto LAB_0fb73770;
    }
    piVar1 = (int *)piVar1[4];
  } while( true );
}



// === FUN_0fb738a0 @ 0fb738a0 (636 bytes) ===

longlong FUN_0fb738a0(int param_1,undefined8 param_2)

{
  undefined1 uVar1;
  undefined1 uVar2;
  longlong lVar3;
  int iVar4;
  char *pcVar5;
  code *UNRECOVERED_JUMPTABLE;
  undefined1 auStack_108 [224];
  
  uVar2 = save_error_message_for_dlerror;
  uVar1 = override_dso_hidden_flag;
  memmove(auStack_108,saved_error_exit,0xe0);
  last_error = 0;
  lVar3 = FUN_0fb73740(param_1);
  if (lVar3 == 0) {
    iVar4 = setjmp((__jmp_buf_tag *)saved_error_exit);
    if (iVar4 != 0) {
      override_dso_hidden_flag = uVar1;
      memmove(saved_error_exit,auStack_108,0xe0);
                    /* WARNING: Could not recover jumptable at 0x0fb73934. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      lVar3 = (*UNRECOVERED_JUMPTABLE)();
      return lVar3;
    }
    if (*(char *)(param_1 + 4) != '\x01') {
      if (*(int *)(param_1 + 0xc) == 0) {
        pcVar5 = "(unknown object)";
      }
      else {
        pcVar5 = *(char **)(*(int *)(param_1 + 0xc) + 0x18);
        if (pcVar5 == (char *)0x0) {
          pcVar5 = "(unknown object)";
        }
      }
      rld_snprintf(last_error_message,cur_last_error_message_len,
                   "dlsym: attempt to find symbol %s in closed object %s\n",param_2,pcVar5);
      last_error = last_error_message;
      override_dso_hidden_flag = uVar1;
      memmove(saved_error_exit,auStack_108,0xe0);
                    /* WARNING: Could not recover jumptable at 0x0fb739a4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      lVar3 = (*UNRECOVERED_JUMPTABLE)();
      return lVar3;
    }
    FUN_0fb72cc0();
    last_error = 0;
    save_error_message_for_dlerror = 1;
    iVar4 = *(int *)(param_1 + 0xc);
    if (iVar4 == -1) {
      override_dso_hidden_flag = uVar1;
      save_error_message_for_dlerror = uVar2;
      memmove(saved_error_exit,auStack_108,0xe0);
      rld_snprintf(last_error_message,cur_last_error_message_len,
                   "dlsym: Unable to locate symbol %s, cannot find object for libdl handle\n",
                   param_2);
      lVar3 = 0;
      last_error = last_error_message;
    }
    else if (pObj_Head == iVar4) {
      lVar3 = foreach_obj(iVar4,&LAB_0fb737d0,param_2);
      override_dso_hidden_flag = uVar1;
      save_error_message_for_dlerror = uVar2;
      memmove(saved_error_exit,auStack_108,0xe0);
      if (lVar3 == -1) {
        rld_snprintf(last_error_message,cur_last_error_message_len,
                     "dlsym: Unable to locate symbol %s\n",param_2);
        last_error = last_error_message;
        lVar3 = 0;
      }
    }
    else {
      override_dso_hidden_flag = 1;
      lVar3 = FUN_0fb73b20(param_2,param_1);
      override_dso_hidden_flag = uVar1;
      save_error_message_for_dlerror = uVar2;
      memmove(saved_error_exit,auStack_108,0xe0);
      if (lVar3 != 0) {
                    /* WARNING: Could not recover jumptable at 0x0fb73a84. Too many branches */
                    /* WARNING: Treating indirect jump as call */
        lVar3 = (*UNRECOVERED_JUMPTABLE)();
        return lVar3;
      }
      rld_snprintf(last_error_message,cur_last_error_message_len,
                   "dlsym: Unable to locate symbol %s\n",param_2);
      lVar3 = 0;
      last_error = last_error_message;
    }
  }
  else {
    lVar3 = 0;
  }
  return lVar3;
}



// === FUN_0fb73b20 @ 0fb73b20 (600 bytes) ===

int FUN_0fb73b20(undefined8 param_1,undefined4 param_2)

{
  int iVar1;
  undefined8 uVar2;
  longlong lVar3;
  undefined8 uVar4;
  char *pcVar5;
  int *piVar6;
  int iVar7;
  int iVar8;
  char *__s;
  int iVar9;
  int *piVar10;
  int *piVar11;
  undefined1 *puVar12;
  code *UNRECOVERED_JUMPTABLE;
  int aiStack_4b0 [5];
  undefined4 uStack_49c;
  undefined1 auStack_428 [1024];
  undefined1 *puStack_28;
  undefined1 uStack_24;
  int iStack_20;
  undefined4 uStack_1c;
  
  puStack_28 = &DAT_0fbda9a0;
  iStack_20 = 0;
  uStack_24 = 0;
  uStack_1c = 0;
  piVar11 = aiStack_4b0 + 4;
  piVar6 = aiStack_4b0 + 4;
  piVar10 = aiStack_4b0 + 4;
  uStack_49c = 0;
  aiStack_4b0[4] = param_2;
  uVar2 = elfhash();
  if (&stack0x00000000 != (undefined1 *)0x4a0) {
    do {
      iVar7 = *piVar6;
      if (((iVar7 != 0) && (iVar1 = *(int *)(iVar7 + 0xc), *(char *)(iVar7 + 6) == '\0')) &&
         (*(undefined1 *)(iVar7 + 6) = 1, iVar1 != 0)) {
        if (iStack_20 != 0) {
          iStack_20 = 0;
          *puStack_28 = 0;
        }
        lVar3 = find_symbol_in_object(iVar1,param_1,uVar2,is_weak_or_global_symbol,&puStack_28);
        iVar7 = 0;
        iVar9 = 0;
        if (lVar3 != -1) {
          free_things_tried(&puStack_28);
          iVar9 = *(int *)(iVar1 + 100) + (int)lVar3 * 0x10;
          iVar7 = *(int *)(iVar9 + 4);
          if (*(short *)(iVar9 + 0xe) != -0xf) {
            iVar7 = iVar7 - (*(int *)(iVar1 + 0x3c) - *(int *)(iVar1 + 0x10));
          }
          return iVar7;
        }
        if (0 < *(int *)(iVar1 + 0xb0)) {
          iVar8 = *(int *)(iVar1 + 0x74);
          puVar12 = (undefined1 *)piVar11;
          while( true ) {
            __s = (char *)(*(int *)(iVar8 + iVar7) + *(int *)(iVar1 + 0x60));
            pcVar5 = strchr(__s,0x2f);
            lVar3 = FUN_0fb72640(__s,1);
            if ((lVar3 == 0) && (iVar8 = ((int *)(iVar8 + iVar7))[3], iVar8 != 0)) {
              uVar4 = add_version_to_dlopen_name
                                (__s,*(int *)(iVar1 + 0x60) + iVar8,auStack_428,0x400);
              lVar3 = FUN_0fb72640(uVar4,1,pcVar5 != (char *)0x0);
            }
            iVar9 = iVar9 + 1;
            piVar11 = (int *)puVar12;
            if (lVar3 != 0) {
              piVar11 = (int *)(puVar12 + -0x10);
              *(int *)(puVar12 + -0x10) = (int)lVar3;
              *(undefined4 *)(puVar12 + -0xc) = 0;
              *(undefined1 **)((int)piVar10 + 4) = puVar12 + -0x10;
              piVar10 = (int *)(puVar12 + -0x10);
            }
            iVar7 = iVar7 + 0x14;
            if (*(int *)(iVar1 + 0xb0) <= iVar9) break;
            iVar8 = *(int *)(iVar1 + 0x74);
            puVar12 = (undefined1 *)piVar11;
          }
        }
      }
      piVar6 = (int *)piVar6[1];
    } while (piVar6 != (int *)0x0);
  }
  free_things_tried(&puStack_28);
                    /* WARNING: Could not recover jumptable at 0x0fb73c4c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  iVar7 = (*UNRECOVERED_JUMPTABLE)();
  return iVar7;
}



// === FUN_0fb73d80 @ 0fb73d80 (28 bytes) ===

int FUN_0fb73d80(void)

{
  int iVar1;
  
  iVar1 = last_error;
  if (last_error != 0) {
    last_error = 0;
    return iVar1;
  }
  return 0;
}



// === FUN_0fb73da0 @ 0fb73da0 (412 bytes) ===

/* WARNING: Instruction at (ram,0x0fb73dec) overlaps instruction at (ram,0x0fb73de8)
    */

void FUN_0fb73da0(int param_1,longlong param_2)

{
  int iVar1;
  undefined8 uVar2;
  longlong lVar3;
  char cVar4;
  
  cVar4 = _rld_short_circuit_delete;
  if (verbose != '\0') {
    trace("Actually beginning unmap DSO %s\n",*(undefined4 *)(param_1 + 0x18));
  }
  if (pObj_Head == param_1) {
    fatal("Deleting MAIN is not allowed\n");
  }
  if ((param_2 == 7) ||
     (((((cVar4 == '\0' && (*(char *)(param_1 + 0xce) == '\0')) &&
        (*(char *)(param_1 + 0xcc) == '\0')) &&
       ((*(char *)(param_1 + 0xcd) == '\0' && (param_2 == 6)))) && (*(int *)(param_1 + 0xb0) == 0)))
     ) {
    cVar4 = '\x01';
  }
  uVar2 = objlist_delete_element(&pObj_Head,param_1);
  if (debug_olist != '\0') {
    print_rld_object_list("after _RLD_OP_DELETE done");
  }
  if (cVar4 == '\0') {
    foreach_obj(pObj_Head,unset_undefineds_are_resolved,0);
    set_postcksum_then_handle_undefineds_and_conflicts(uVar2,_rld_environment_ld_bind_now);
    iVar1 = *(int *)(param_1 + 300);
  }
  else {
    if (verbose != '\0') {
      trace("Short ciruit delete object from list!\n");
    }
    iVar1 = *(int *)(param_1 + 300);
  }
  if (iVar1 != 0) {
    __rld_recent_update_callback = 1;
  }
  __rld_calls_on_rld_list_change();
  if ((cVar4 == '\0') && (_rld_environment_ld_bind_now == '\0')) {
    unset_undefineds_and_reresolve();
  }
  lVar3 = elfunmap(param_1);
  if (lVar3 == -1) {
    if (verbose != '\0') {
      trace(" elfunmap -- can\'t unmap %s \n",*(undefined4 *)(param_1 + 0x18));
    }
  }
  else if (verbose != '\0') {
    trace("Completed unmap DSO %s\n",*(undefined4 *)(param_1 + 0x18));
  }
  return;
}



// === FUN_0fb73f40 @ 0fb73f40 (452 bytes) ===

void FUN_0fb73f40(int param_1)

{
  int iVar1;
  char *pcVar4;
  longlong lVar2;
  undefined8 uVar3;
  int iVar5;
  char *__s;
  int iVar6;
  int iVar7;
  int *piVar8;
  int *piVar9;
  int *piVar10;
  undefined1 *puVar11;
  undefined1 *puVar12;
  int *piVar13;
  int aiStack_480 [6];
  undefined1 auStack_418 [1048];
  
  if (*(char *)(param_1 + 6) == '\0') {
    piVar13 = aiStack_480;
    piVar8 = aiStack_480;
    piVar10 = aiStack_480;
    aiStack_480[1] = 0;
    aiStack_480[0] = param_1;
    if (&stack0x00000000 != (undefined1 *)0x480) {
      do {
        iVar1 = *piVar8;
        if (*(char *)(iVar1 + 6) == '\0') {
          *(undefined1 *)(iVar1 + 6) = 1;
          *(int *)(iVar1 + 8) = *(int *)(iVar1 + 8) + -1;
          if (verbose != '\0') {
            trace("dlclosing %s ref count decremented to %ld lh_is_processed %ld\n",
                  *(undefined4 *)(*(int *)(iVar1 + 0xc) + 0x18),*(undefined4 *)(iVar1 + 8),1);
          }
          iVar1 = *(int *)(iVar1 + 0xc);
          iVar5 = *(int *)(iVar1 + 0xb0);
          iVar7 = 0;
          iVar6 = 0;
          puVar11 = (undefined1 *)piVar13;
          if (iVar5 < 1) goto LAB_0fb73fd8;
          do {
            piVar9 = (int *)(*(int *)(iVar1 + 0x74) + iVar6);
            piVar13 = (int *)puVar11;
            if ((piVar9[4] & 0x10U) == 0) {
              __s = (char *)(*piVar9 + *(int *)(iVar1 + 0x60));
              pcVar4 = strchr(__s,0x2f);
              lVar2 = FUN_0fb726c0(__s);
              if ((lVar2 == 0) && (piVar9[3] != 0)) {
                uVar3 = add_version_to_dlopen_name
                                  (__s,*(int *)(iVar1 + 0x60) + piVar9[3],auStack_418,0x400);
                lVar2 = FUN_0fb726c0(uVar3,pcVar4 != (char *)0x0);
              }
              puVar12 = (undefined1 *)piVar10;
              if (lVar2 != 0) {
                piVar13 = (int *)(puVar11 + -0x10);
                puVar12 = puVar11 + -0x10;
                *(undefined4 *)(puVar11 + -0xc) = 0;
                *(int *)(puVar11 + -0x10) = (int)lVar2;
                *(undefined1 **)((int)piVar10 + 4) = puVar11 + -0x10;
              }
              iVar5 = *(int *)(iVar1 + 0xb0);
              piVar10 = (int *)puVar12;
            }
            iVar7 = iVar7 + 1;
            iVar6 = iVar6 + 0x14;
            puVar11 = (undefined1 *)piVar13;
          } while (iVar7 < iVar5);
          piVar8 = (int *)piVar8[1];
        }
        else {
          if (verbose != '\0') {
            trace("Already is_processed. %s\n",*(undefined4 *)(*(int *)(iVar1 + 0xc) + 0x18));
          }
LAB_0fb73fd8:
          piVar8 = (int *)piVar8[1];
        }
      } while (piVar8 != (int *)0x0);
    }
  }
  return;
}



// === FUN_0fb74110 @ 0fb74110 (1248 bytes) ===

/* WARNING: Instruction at (ram,0x0fb74308) overlaps instruction at (ram,0x0fb74304)
    */

void FUN_0fb74110(int param_1)

{
  undefined1 uVar1;
  undefined1 uVar2;
  longlong lVar3;
  longlong lVar4;
  int iVar5;
  char *pcVar7;
  int *piVar8;
  longlong lVar6;
  int iVar9;
  undefined8 in_ra;
  undefined1 auStack_148 [224];
  longlong lStack_68;
  longlong lStack_60;
  longlong lStack_58;
  longlong lStack_50;
  longlong lStack_48;
  longlong lStack_40;
  undefined8 UNRECOVERED_JUMPTABLE_00;
  
  uVar2 = save_error_message_for_dlerror;
  uVar1 = override_dso_hidden_flag;
  UNRECOVERED_JUMPTABLE_00 = in_ra;
  memmove(auStack_148,saved_error_exit,0xe0);
  lVar4 = FUN_0fb73740(param_1);
  if (lVar4 != 0) {
    if (verbose != '\0') {
      trace("exit dlclose invalid handle\n");
    }
                    /* WARNING: Could not recover jumptable at 0x0fb74170. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*UNRECOVERED_JUMPTABLE_00._4_4_)();
    return;
  }
  if (verbose != '\0') {
    if (*(int *)(param_1 + 0xc) == 0) {
      pcVar7 = "<unknown name, null obj ptr>";
    }
    else {
      pcVar7 = *(char **)(*(int *)(param_1 + 0xc) + 0x18);
    }
    trace("Beginning dlclose of %s\n",pcVar7);
  }
  if (*(char *)(param_1 + 4) != '\x01') {
    if (*(int *)(param_1 + 0xc) == 0) {
      pcVar7 = "<unknown name, null obj ptr>";
    }
    else {
      pcVar7 = *(char **)(*(int *)(param_1 + 0xc) + 0x18);
    }
    rld_snprintf(last_error_message,cur_last_error_message_len,
                 "dlclose: attempt to close already closed object %s\n",pcVar7);
    last_error = last_error_message;
                    /* WARNING: Could not recover jumptable at 0x0fb741e0. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*UNRECOVERED_JUMPTABLE_00._4_4_)();
    return;
  }
  iVar5 = setjmp((__jmp_buf_tag *)saved_error_exit);
  if (iVar5 != 0) {
    override_dso_hidden_flag = uVar1;
    save_error_message_for_dlerror = uVar2;
    memmove(saved_error_exit,auStack_148,0xe0);
    if (verbose != '\0') {
      if (*(int *)(param_1 + 0xc) == 0) {
        pcVar7 = "<unknown name, null obj ptr>";
      }
      else {
        pcVar7 = *(char **)(*(int *)(param_1 + 0xc) + 0x18);
      }
      trace("Ending dlclose of %s with error = \'%s\'\n",pcVar7,last_error);
    }
                    /* WARNING: Could not recover jumptable at 0x0fb74260. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*UNRECOVERED_JUMPTABLE_00._4_4_)();
    return;
  }
  last_error = 0;
  save_error_message_for_dlerror = 1;
  FUN_0fb72cc0();
  FUN_0fb73f40(param_1);
  lVar4 = (longlong)handle_list_anchor;
  next_fini_to_use = 0;
  if (lVar4 == 0) {
    lStack_50 = 0;
  }
  else {
    lStack_50 = 0;
    iVar5 = *(int *)(handle_list_anchor + 8);
    while( true ) {
      iVar9 = (int)lVar4;
      if ((iVar5 < 1) && (*(char *)(iVar9 + 4) == '\x01')) {
        if (*(char *)(iVar9 + 5) == '\0') {
          lStack_50 = (longlong)((int)lStack_50 + 1);
          lStack_40 = lVar4;
          FUN_0fb721f0(lVar4);
          *(undefined1 *)((int)lStack_40 + 4) = 4;
          iVar5 = *(int *)((int)lStack_40 + 0x10);
        }
        else {
          iVar5 = *(int *)(iVar9 + 0x10);
        }
      }
      else {
        iVar5 = *(int *)(iVar9 + 0x10);
      }
      lVar4 = (longlong)iVar5;
      if (lVar4 == 0) break;
      iVar5 = *(int *)(iVar5 + 8);
    }
    if (1 < lStack_50) {
      qsort(fini_array,next_fini_to_use,4,FUN_0fb721a0);
    }
  }
  if ((verbose != '\0') &&
     (trace("_rld_dlclose: about to unmap %ld libraries\n",lStack_50), verbose != '\0')) {
    trace("_rld_dlclose: The libraries in fini order are\n");
  }
  iVar5 = 0;
  if (0 < (int)next_fini_to_use) {
    lVar4 = 1;
    iVar9 = 0;
    do {
      piVar8 = (int *)((int)fini_array + iVar5);
      while (verbose == '\0') {
        iVar9 = iVar9 + 1;
        piVar8 = piVar8 + 1;
        iVar5 = iVar5 + 4;
        lVar4 = (longlong)((int)lVar4 + 1);
        if ((int)next_fini_to_use <= iVar9) goto LAB_0fb743c0;
      }
      lStack_48 = lVar4;
      trace("   %ld %s (init_order = %ld)\n",lVar4,*(undefined4 *)(*(int *)(*piVar8 + 0xc) + 0x18),
            *(undefined4 *)(*(int *)(*piVar8 + 0xc) + 0xd4));
      iVar9 = iVar9 + 1;
      iVar5 = iVar5 + 4;
      lVar4 = (longlong)((int)lStack_48 + 1);
    } while (iVar9 < (int)next_fini_to_use);
  }
LAB_0fb743c0:
  if (0 < lStack_50) {
    iVar5 = 0;
    if (0 < (int)next_fini_to_use) {
      iVar9 = 0;
      do {
        piVar8 = (int *)((int)fini_array + iVar5);
        while (*(int *)(*(int *)(*piVar8 + 0xc) + 0xec) == 0) {
          iVar9 = iVar9 + 1;
          piVar8 = piVar8 + 1;
          iVar5 = iVar5 + 4;
          if ((int)next_fini_to_use <= iVar9) goto LAB_0fb7443c;
        }
        call_fini();
        iVar9 = iVar9 + 1;
        iVar5 = iVar5 + 4;
        last_error = 0;
      } while (iVar9 < (int)next_fini_to_use);
LAB_0fb7443c:
      iVar5 = 0;
      if (0 < (int)next_fini_to_use) {
        lVar4 = 0;
        do {
          lVar4 = (longlong)((int)lVar4 + 1);
          iVar9 = *(int *)((int)fini_array + iVar5);
          lStack_40 = (longlong)iVar9;
          *(undefined1 *)(iVar9 + 4) = 2;
          lStack_58 = (longlong)*(int *)(iVar9 + 0xc);
          lStack_68 = lVar4;
          if (lVar4 < lStack_50) {
            FUN_0fb73da0(lStack_58,7);
          }
          else {
            FUN_0fb73da0(lStack_58,8);
          }
          delete_object(lStack_58);
          *(undefined4 *)((int)lStack_40 + 0xc) = 0;
          iVar5 = iVar5 + 4;
        } while (lStack_68 < (int)next_fini_to_use);
      }
    }
    if ((longlong)handle_list_anchor == 0) {
      lVar4 = 0;
    }
    else {
      lVar3 = (longlong)handle_list_anchor;
      lVar6 = 0;
      do {
        while( true ) {
          lVar4 = lVar3;
          iVar5 = (int)lVar4;
          if (*(char *)(iVar5 + 4) != '\x02') break;
          lStack_60 = lVar6;
          if (lVar6 == 0) {
            handle_list_anchor = *(int *)(iVar5 + 0x10);
            FUN_0fb72880();
            iVar5 = handle_list_anchor;
          }
          else {
            *(int *)((int)lVar6 + 0x10) = *(int *)(iVar5 + 0x10);
            FUN_0fb72880();
            iVar5 = *(int *)((int)lStack_60 + 0x10);
          }
          lVar3 = (longlong)iVar5;
          lVar4 = lStack_60;
          lVar6 = lStack_60;
          if ((longlong)iVar5 == 0) goto LAB_0fb74540;
        }
        lVar3 = (longlong)*(int *)(iVar5 + 0x10);
        lVar6 = lVar4;
      } while ((longlong)*(int *)(iVar5 + 0x10) != 0);
    }
LAB_0fb74540:
    handle_list_tail = (undefined4)lVar4;
  }
  override_dso_hidden_flag = uVar1;
  save_error_message_for_dlerror = uVar2;
  memmove(saved_error_exit,auStack_148,0xe0);
  if (verbose != '\0') {
    if (*(int *)(param_1 + 0xc) == 0) {
      pcVar7 = "<unknown name, null obj ptr>";
    }
    else {
      pcVar7 = *(char **)(*(int *)(param_1 + 0xc) + 0x18);
    }
    trace("Ending dlclose of %s\n",pcVar7);
  }
                    /* WARNING: Could not recover jumptable at 0x0fb74598. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE_00._4_4_)();
  return;
}



// === __rld_libdl_interface @ 0fb745f0 (408 bytes) ===

/* WARNING: Instruction at (ram,0x0fb746dc) overlaps instruction at (ram,0x0fb746d8)
    */

undefined8
__rld_libdl_interface
          (uint param_1,undefined8 param_2,ulonglong param_3,undefined8 param_4,undefined8 param_5)

{
  undefined1 uVar1;
  undefined1 uVar2;
  ulonglong uVar3;
  int iVar4;
  undefined8 uVar5;
  ulong uStack_48;
  ulong auStack_38 [2];
  ulong auStack_30 [2];
  longlong lStack_28;
  ulong auStack_20 [2];
  
  uVar2 = inside_non_thread_libdl_critical_region;
  uVar1 = in_delay_load;
  in_delay_load = 0;
  auStack_38 = (ulong  [2])param_4;
  auStack_30 = (ulong  [2])param_5;
  auStack_20 = (ulong  [2])param_2;
  if (multi_threaded == '\0') {
    inside_non_thread_libdl_critical_region = 1;
    if (_rld_running_under_speedshop == '\0') {
      lStack_28 = 0;
    }
    else if (_rld_libdl_depth == 0) {
      if (((param_1 == 0) || (param_1 == 4)) || (param_1 == 5)) {
        iVar4 = sigprocmask(3,(sigset_t *)&rld_sigmask,(sigset_t *)&uStack_48);
        if ((iVar4 != 0) && (verbose != '\0')) {
          trace("sigprocmask fails: errno = %ld, sigmask = 0x%lx\n",errno,rld_sigmask);
        }
        lStack_28 = 1;
      }
      else {
        lStack_28 = 0;
      }
    }
    else {
      lStack_28 = 0;
    }
    _rld_libdl_depth = _rld_libdl_depth + 1;
  }
  else {
    enter_libdl(0);
    lStack_28 = 0;
  }
  if (param_1 < 8) {
    switch(param_1) {
    case 0:
      uVar5 = 2;
      if ((param_3 & 4) == 0) {
        uVar5 = 1;
      }
      uVar5 = _rld_dlopen(auStack_20,param_3,0,0,uVar5);
      break;
    case 1:
      uVar5 = FUN_0fb738a0(auStack_20,param_3);
      break;
    case 2:
      uVar5 = FUN_0fb74110(auStack_20);
      break;
    case 3:
      uVar5 = FUN_0fb73d80();
      break;
    case 4:
      uVar5 = _rld_dlopen(auStack_20,param_3 | 4,0,0,2);
      break;
    case 5:
      uVar5 = 2;
      if ((param_3 & 4) == 0) {
        uVar5 = 1;
      }
      uVar5 = _rld_dlopen(auStack_20,param_3,auStack_38,auStack_30,uVar5);
      break;
    default:
      uVar5 = 3;
      uVar3 = 4;
      in_delay_load = 1;
      if (param_1 == 7) {
        uVar3 = 0;
        uVar5 = 4;
      }
      uVar5 = _rld_dlopen(auStack_20,uVar3 | param_3,auStack_38,auStack_30,uVar5);
    }
  }
  else {
    fatal("libdl: Unknown argument to rld_libdl_interface\n");
    uVar5 = 0;
  }
  in_delay_load = uVar1;
  inside_non_thread_libdl_critical_region = uVar2;
  if (multi_threaded == '\0') {
    _rld_libdl_depth = _rld_libdl_depth + -1;
    if (((lStack_28 != 0) && (_rld_libdl_depth == 0)) &&
       ((iVar4 = sigprocmask(3,(sigset_t *)&uStack_48,(sigset_t *)0x0), iVar4 != 0 &&
        (verbose != '\0')))) {
      trace("sigprocmask fails: errno = %ld, sigmask = 0x%lx\n",errno,uStack_48);
    }
  }
  else {
    exit_libdl();
  }
  return uVar5;
}



// === add_version_to_dlopen_name @ 0fb74880 (696 bytes) ===

undefined1  [16]
add_version_to_dlopen_name(undefined8 param_1,char *param_2,undefined8 param_3,undefined8 param_4)

{
  byte *pbVar1;
  byte bVar2;
  char *pcVar3;
  int iVar4;
  size_t sVar5;
  char *pcVar6;
  char cVar7;
  byte bVar8;
  byte *pbVar9;
  undefined8 unaff_s8;
  code *UNRECOVERED_JUMPTABLE;
  undefined1 auVar10 [16];
  char acStack_70 [8];
  
  pcVar3 = strchr(param_2,0x23);
  if (pcVar3 != (char *)0x0) {
    param_2 = pcVar3 + 1;
  }
  cVar7 = *param_2;
  pcVar3 = param_2;
  if (cVar7 != '\0') {
    while ((cVar7 == ' ' || (pcVar3 = param_2, cVar7 == '\t'))) {
      cVar7 = param_2[1];
      pcVar3 = param_2 + 1;
      if ((cVar7 == '\0') || ((cVar7 != ' ' && (cVar7 != '\t')))) break;
      cVar7 = param_2[2];
      pcVar3 = param_2 + 2;
      param_2 = pcVar3;
      if (cVar7 == '\0') break;
    }
  }
  iVar4 = strncmp(pcVar3,"sgi",3);
  if (iVar4 != 0) {
                    /* WARNING: Could not recover jumptable at 0x0fb74910. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    auVar10 = (*UNRECOVERED_JUMPTABLE)(unaff_s8);
    return auVar10;
  }
  sVar5 = strlen(pcVar3);
  if (3 < sVar5 + 1) {
    iVar4 = -(sVar5 + 0x10 & 0xfffffff0);
    strcpy(acStack_70 + iVar4,pcVar3);
    pcVar3 = acStack_70 + iVar4 + 3;
    pcVar6 = strchr(pcVar3,0x2e);
    if ((pcVar6 != (char *)0x0) && (pcVar6 != pcVar3)) {
      bVar8 = pcVar6[1];
      *pcVar6 = '\0';
      if ((0x2f < bVar8) && (bVar8 < 0x3a)) {
        pbVar9 = (byte *)(pcVar6 + 1);
        if (bVar8 == 0) {
          bVar2 = pcVar6[1];
        }
        else {
          do {
            if ((bVar8 < 0x30) || (0x39 < bVar8)) {
              bVar2 = *pbVar9;
              break;
            }
            bVar8 = pbVar9[1];
            pbVar9 = pbVar9 + 1;
            bVar2 = 0;
          } while (bVar8 != 0);
        }
        for (; ((bVar8 = bVar2, bVar8 != 0 &&
                (((bVar8 == 0x20 || (bVar8 == 9)) && (bVar8 = pbVar9[1], bVar8 != 0)))) &&
               ((bVar8 == 0x20 || (bVar8 == 9)))); pbVar9 = pbVar9 + 2) {
          pbVar1 = pbVar9 + 2;
          bVar2 = *pbVar1;
        }
      }
      if (bVar8 == 0) {
        bVar8 = acStack_70[iVar4 + 3];
        for (; ((bVar8 != 0 && (0x2f < bVar8)) &&
               ((bVar8 < 0x3a &&
                (((bVar8 = pcVar3[1], bVar8 != 0 && (0x2f < bVar8)) && (bVar8 < 0x3a))))));
            pcVar3 = pcVar3 + 2) {
          bVar8 = pcVar3[2];
        }
        if (bVar8 == 0) {
          acStack_70[iVar4 + 2] = '.';
          __rld_strlcpy(param_3,param_1,param_4);
          __rld_strlcat(param_3,acStack_70 + iVar4 + 2,param_4);
          param_1 = param_3;
        }
      }
    }
  }
  auVar10._0_8_ = param_1;
  auVar10._8_8_ = unaff_s8;
  return auVar10;
}



// === _sbrk @ 0fb74b40 (416 bytes) ===

void * _sbrk(int param_1)

{
  int __fd;
  void *pvVar1;
  size_t __len;
  void *pvVar2;
  
  if (real_break == (void *)0x0) {
    _minbrk = 0xfbde528;
    _curbrk = (void *)0xfbde528;
    real_break = (void *)(pagesize + 0xfbde527U & ~(pagesize - 1U));
  }
  pvVar2 = _curbrk;
  if ((void *)(param_1 + (int)_curbrk) <= real_break) {
    _curbrk = (void *)(param_1 + (int)_curbrk);
    return pvVar2;
  }
  if (grow_len == 0) {
    grow_len = 0x8000;
    __len = 0x8000 - ((uint)real_break & 0x7fff);
  }
  else {
    __len = (pagesize + grow_len) - 1U & ~(pagesize - 1U);
    grow_len = grow_len * 2;
  }
  if ((int)__len < param_1) {
    __len = __len + ((param_1 - __len) + 0x7fff & 0xffff8000);
  }
  __fd = open64("/dev/zero",0);
  if (__fd == -1) {
    write(2,"rld_sbrk: Can\'t open /dev/zero\n",0x20);
                    /* WARNING: Subroutine does not return */
    exit(1);
  }
  pvVar1 = mmap64(real_break,__len,3,2,__fd,0);
  if (pvVar1 == (void *)0xffffffff) {
    write(2,"rld_sbrk: Can\'t map /dev/zero -- probably not enough virtual memory\n",0x44);
                    /* WARNING: Subroutine does not return */
    exit(1);
  }
  close(__fd);
  if (real_break != pvVar1) {
    pvVar2 = pvVar1;
  }
  real_break = (void *)(__len + (int)pvVar1);
  _curbrk = (void *)(param_1 + (int)pvVar2);
  return pvVar2;
}



// === sbrk @ 0fb74ce0 (8 bytes) ===

void * sbrk(int param_1)

{
  int __fd;
  void *pvVar1;
  size_t __len;
  void *pvVar2;
  
  if (real_break == (void *)0x0) {
    _minbrk = 0xfbde528;
    _curbrk = (void *)0xfbde528;
    real_break = (void *)(pagesize + 0xfbde527U & ~(pagesize - 1U));
  }
  pvVar2 = _curbrk;
  if ((void *)(param_1 + (int)_curbrk) <= real_break) {
    _curbrk = (void *)(param_1 + (int)_curbrk);
    return pvVar2;
  }
  if (grow_len == 0) {
    grow_len = 0x8000;
    __len = 0x8000 - ((uint)real_break & 0x7fff);
  }
  else {
    __len = (pagesize + grow_len) - 1U & ~(pagesize - 1U);
    grow_len = grow_len * 2;
  }
  if ((int)__len < param_1) {
    __len = __len + ((param_1 - __len) + 0x7fff & 0xffff8000);
  }
  __fd = open64("/dev/zero",0);
  if (__fd == -1) {
    write(2,"rld_sbrk: Can\'t open /dev/zero\n",0x20);
                    /* WARNING: Subroutine does not return */
    exit(1);
  }
  pvVar1 = mmap64(real_break,__len,3,2,__fd,0);
  if (pvVar1 == (void *)0xffffffff) {
    write(2,"rld_sbrk: Can\'t map /dev/zero -- probably not enough virtual memory\n",0x44);
                    /* WARNING: Subroutine does not return */
    exit(1);
  }
  close(__fd);
  if (real_break != pvVar1) {
    pvVar2 = pvVar1;
  }
  real_break = (void *)(__len + (int)pvVar1);
  _curbrk = (void *)(param_1 + (int)pvVar2);
  return pvVar2;
}



// === FUN_0fb74cf0 @ 0fb74cf0 (20 bytes) ===

void FUN_0fb74cf0(undefined8 param_1)

{
  rld_app_bridge(param_1,1,0,0,is_pixie);
  return;
}



// === FUN_0fb74d10 @ 0fb74d10 (172 bytes) ===

void FUN_0fb74d10(void)

{
  int iVar1;
  longlong lVar2;
  int iVar3;
  
  if (DAT_0fbd3988 == 0) {
    lVar2 = malloc(0x3fc);
    DAT_0fbd398c = (undefined4)lVar2;
    if (lVar2 == 0) {
      fatal("Out of memory allocating buffer %ld bytes for DT_MIPS_RLD_OBJ_UPDATE callback\n",0x3fc)
      ;
    }
    DAT_0fbd3988 = 0x55;
  }
  else {
    iVar1 = DAT_0fbd3988 * 2;
    iVar3 = DAT_0fbd3988 * 0x18;
    lVar2 = realloc(DAT_0fbd398c);
    if (lVar2 == 0) {
      fatal("Out of memory reallocating buffer %ld bytes for DT_MIPS_RLD_OBJ_UPDATE callback\n",
            iVar3);
    }
    DAT_0fbd398c = (undefined4)lVar2;
    DAT_0fbd3988 = iVar1;
  }
  if (debug_map != '\0') {
    debug("callback array available size now %d entries\n",DAT_0fbd3988);
  }
  return;
}



// === __rld_refresh_callback_list @ 0fb74f00 (24 bytes) ===

void __rld_refresh_callback_list(void)

{
  __rld_callback_list_count = 0;
  foreach_obj(pObj_Head,&LAB_0fb74dc0,0);
  return;
}



// === __rld_callback_list_execute @ 0fb74f20 (136 bytes) ===

void __rld_callback_list_execute(void)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  
  puVar2 = DAT_0fbd398c + __rld_callback_list_count * 3;
  for (puVar1 = DAT_0fbd398c; puVar1 < puVar2; puVar1 = puVar1 + 3) {
    if (debug_map != '\0') {
      debug("Now callback rld update func obj 0x%lx dynsym %ld funcp 0x%lx\n",*puVar1,puVar1[1],
            puVar1[2]);
    }
    FUN_0fb74cf0(puVar1[2]);
  }
  return;
}



// === endofstack @ 0fb74fb0 (96 bytes) ===

int endofstack(uint param_1)

{
  int iVar1;
  int unaff_gp_lo;
  
  iVar1 = *(int *)(unaff_gp_lo + -0x7f54);
  if (iVar1 < 0x1000) {
    *(undefined4 *)(unaff_gp_lo + -0x7f54) = 0x1000;
    return (param_1 & 0xfffff000) - 0x4000;
  }
  return (param_1 & ~(iVar1 - 1U)) + iVar1 * -4;
}



// === hashinit @ 0fb75010 (196 bytes) ===

void * hashinit(undefined4 *param_1,int param_2,int param_3)

{
  longlong lVar1;
  void *pvVar2;
  
  lVar1 = malloc(param_3 << 2);
  if (lVar1 == 0) {
    fatal("rld_malloc hashinit failed");
  }
  memset((void *)lVar1,0,param_3 << 2);
  param_1[3] = param_3;
  *param_1 = (void *)lVar1;
  lVar1 = malloc();
  if (lVar1 == 0) {
    fatal("rld_malloc hashinit failed");
  }
  pvVar2 = memset((void *)lVar1,0,param_2 << 2);
  param_1[4] = param_2;
  param_1[2] = (void *)lVar1;
  return pvVar2;
}



// === num_hash_bucket @ 0fb750e0 (8 bytes) ===

undefined4 num_hash_bucket(int param_1)

{
  return *(undefined4 *)(param_1 + 0xc);
}



// === num_hash_chain @ 0fb750f0 (8 bytes) ===

undefined4 num_hash_chain(int param_1)

{
  return *(undefined4 *)(param_1 + 0x10);
}



// === hashmod @ 0fb75100 (52 bytes) ===

uint hashmod(undefined8 param_1,int param_2)

{
  uint uVar1;
  
  uVar1 = elfhash();
  if (*(uint *)(param_2 + 0xc) == 0) {
    trap(7);
  }
  return uVar1 % *(uint *)(param_2 + 0xc);
}



// === expand_tab @ 0fb75140 (120 bytes) ===

void * expand_tab(int param_1)

{
  int iVar1;
  longlong lVar2;
  void *pvVar3;
  int iVar4;
  
  iVar1 = *(int *)(param_1 + 0x10);
  lVar2 = realloc(*(undefined4 *)(param_1 + 8),iVar1 << 3);
  if (lVar2 == 0) {
    fatal("expanding hash failed");
    iVar4 = *(int *)(param_1 + 0x10);
  }
  else {
    iVar4 = *(int *)(param_1 + 0x10);
  }
  *(int *)(param_1 + 8) = (int)lVar2;
  pvVar3 = memset((void *)((int)lVar2 + iVar4 * 4),0,iVar4 * 4);
  *(int *)(param_1 + 0x10) = iVar1 * 2;
  return pvVar3;
}



// === hashmake @ 0fb751c0 (428 bytes) ===

/* WARNING: Instruction at (ram,0x0fb7533c) overlaps instruction at (ram,0x0fb75338)
    */

uint hashmake(int param_1,char *param_2,longlong param_3,int *param_4)

{
  char cVar1;
  uint uVar2;
  char *__s2;
  int iVar3;
  uint *puVar4;
  uint uVar5;
  int iVar6;
  
  cVar1 = *param_2;
  iVar6 = param_4[5];
  puVar4 = (uint *)(param_4[3] + param_1 * 4);
  uVar5 = *puVar4;
  uVar2 = param_4[4];
  if (uVar5 == 0) {
    if (param_3 != 0) {
      *puVar4 = uVar2;
      uVar5 = param_4[7];
      while ((uVar5 == uVar2 || (uVar5 <= uVar2))) {
        expand_tab(param_4 + 3);
        iVar6 = param_4[5];
        uVar5 = param_4[7];
      }
      *(undefined4 *)(iVar6 + uVar2 * 4) = 0;
      param_4[4] = param_4[4] + 1;
    }
  }
  else {
    do {
      while( true ) {
        __s2 = *(char **)(*param_4 + uVar5 * 8);
        if (*__s2 == cVar1) break;
        puVar4 = (uint *)(iVar6 + uVar5 * 4);
        uVar5 = *puVar4;
        if (uVar5 == 0) goto LAB_0fb75248;
      }
      iVar3 = strcmp(param_2,__s2);
      if (iVar3 == 0) {
        return uVar5;
      }
      puVar4 = (uint *)(iVar6 + uVar5 * 4);
      uVar5 = *puVar4;
    } while (uVar5 != 0);
LAB_0fb75248:
    if (param_3 != 0) {
      *puVar4 = uVar2;
      if (param_4[7] == uVar2) {
        expand_tab(param_4 + 3);
        iVar6 = param_4[5];
      }
      *(undefined4 *)(iVar6 + uVar2 * 4) = 0;
      param_4[4] = param_4[4] + 1;
      return 0;
    }
  }
  return 0;
}



// === alloc_tbl @ 0fb75370 (84 bytes) ===

void alloc_tbl(undefined4 *param_1,int param_2)

{
  longlong lVar1;
  
  lVar1 = malloc(param_2 << 3);
  if (lVar1 == 0) {
    fatal("allocating symbol table failed");
  }
  *param_1 = (int)lVar1;
  param_1[1] = param_2;
  return;
}



// === realloc_tbl @ 0fb753d0 (84 bytes) ===

/* WARNING: Instruction at (ram,0x0fb753fc) overlaps instruction at (ram,0x0fb753f8)
    */

void realloc_tbl(undefined4 *param_1)

{
  int iVar1;
  longlong lVar2;
  
  iVar1 = param_1[1];
  param_1[1] = iVar1 * 2;
  lVar2 = realloc(*param_1,iVar1 << 4);
  if (lVar2 == 0) {
    fatal("reallocating symbol table failed");
  }
  *param_1 = (int)lVar2;
  return;
}



// === rld_add_to_listed_symbols @ 0fb75430 (220 bytes) ===

void rld_add_to_listed_symbols(longlong param_1)

{
  size_t sVar1;
  int iVar2;
  undefined4 uVar3;
  
  if ((param_1 != 0) && (sVar1 = strlen((char *)param_1), sVar1 != 0)) {
    rld_has_listed_symbols = 1;
    if (DAT_0fbd3994 == 0) {
      DAT_0fbd399c = 10;
      DAT_0fbd3994 = malloc(0x28);
      if (DAT_0fbd3994 == 0) {
        force_error("Unable to malloc %ld bytes for desired symbol array\n",DAT_0fbd399c << 2);
                    /* WARNING: Subroutine does not return */
        exit(1);
      }
    }
    iVar2 = DAT_0fbd3994;
    if (DAT_0fbd399c <= DAT_0fbd3998) {
      DAT_0fbd399c = DAT_0fbd399c + 10;
      iVar2 = realloc(DAT_0fbd3994,DAT_0fbd399c * 4);
      if (iVar2 == 0) {
        force_error("Unable to realloc %ld bytes for desired symbol array\n",DAT_0fbd399c << 2);
                    /* WARNING: Subroutine does not return */
        exit(1);
      }
    }
    DAT_0fbd3994 = iVar2;
    uVar3 = rld_obj_newstr(param_1);
    iVar2 = DAT_0fbd3998 * 4;
    DAT_0fbd3998 = DAT_0fbd3998 + 1;
    *(undefined4 *)(DAT_0fbd3994 + iVar2) = uVar3;
    return;
  }
  return;
}



// === rld_is_listed_symbol @ 0fb75510 (156 bytes) ===

void rld_is_listed_symbol(char *param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  code *UNRECOVERED_JUMPTABLE_00;
  
  if (0 < DAT_0fbd3998) {
    iVar3 = 0;
    iVar2 = 0;
    do {
      iVar1 = strcmp(*(char **)(DAT_0fbd3994 + iVar2),param_1);
      iVar2 = iVar2 + 4;
      if (iVar1 == 0) {
                    /* WARNING: Could not recover jumptable at 0x0fb755a4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
        (*UNRECOVERED_JUMPTABLE_00)();
        return;
      }
      iVar3 = iVar3 + 1;
    } while (iVar3 < DAT_0fbd3998);
  }
                    /* WARNING: Could not recover jumptable at 0x0fb75584. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE_00)();
  return;
}



// === print_rld_object_list @ 0fb755b0 (116 bytes) ===

void print_rld_object_list(undefined8 param_1)

{
  int iVar1;
  int iVar2;
  code *UNRECOVERED_JUMPTABLE;
  
  iVar1 = pObj_Head;
  force_warning("rld object list %s\n",param_1);
  if (iVar1 != 0) {
    iVar2 = 0;
    do {
      force_warning("OL [%3d] 0x%08lx nxt 0x%08lx prev 0x%08lx %s next\n",iVar2,iVar1,
                    *(undefined4 *)(iVar1 + 8),*(undefined4 *)(iVar1 + 0xc),
                    *(undefined4 *)(iVar1 + 0x2c));
      iVar1 = *(int *)(iVar1 + 8);
      iVar2 = iVar2 + 1;
    } while (iVar1 != 0);
  }
                    /* WARNING: Could not recover jumptable at 0x0fb7561c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
}



// === execute_all_fini_sections @ 0fb75630 (364 bytes) ===

void execute_all_fini_sections(void)

{
  undefined1 *puVar1;
  int iVar2;
  int *piVar3;
  int *piVar4;
  size_t __nmemb;
  int iVar5;
  int iStack_54;
  int aiStack_50 [2];
  
  if (pObj_Head == 0) {
    __nmemb = 0;
  }
  else {
    __nmemb = 0;
    iVar2 = pObj_Head;
    do {
      iVar2 = *(int *)(iVar2 + 8);
      __nmemb = __nmemb + 1;
    } while (iVar2 != 0);
  }
  puVar1 = (undefined1 *)(__nmemb * 4 + 0xf & 0xfffffff0);
  iVar2 = -(int)puVar1;
  iVar5 = pObj_Head;
  piVar4 = (int *)((int)aiStack_50 + iVar2);
  if (aiStack_50 == (int *)puVar1) {
    fatal("execute_all_fini_sections: unable to allocate order array; length = %ld\n",__nmemb);
    iVar5 = pObj_Head;
    piVar4 = (int *)((int)aiStack_50 + iVar2);
  }
  for (; iVar5 != 0; iVar5 = *(int *)(iVar5 + 8)) {
    *piVar4 = iVar5;
    piVar4 = piVar4 + 1;
  }
  qsort((void *)((int)aiStack_50 + iVar2),__nmemb,4,ascending_init_order);
  if (verbose != '\0') {
    trace("execute_all_fini_sections: libs in fini order are\n");
  }
  piVar4 = (int *)((int)aiStack_50 + (__nmemb - 1) * 4 + iVar2);
  if (-1 < (int)(__nmemb - 1)) {
    iVar5 = 1;
    piVar3 = piVar4;
    do {
      if (verbose != '\0') {
        trace("   %ld %s (init_order = %ld)\n",iVar5,*(undefined4 *)(*piVar3 + 0x18),
              *(undefined4 *)(*piVar3 + 0xd4));
      }
      piVar3 = piVar3 + -1;
      iVar5 = iVar5 + 1;
    } while (piVar3 != (int *)((int)&iStack_54 + iVar2));
    do {
      call_fini(*piVar4);
      piVar4 = piVar4 + -1;
    } while (piVar4 != (int *)((int)&iStack_54 + iVar2));
  }
  return;
}



// === FUN_0fb75850 @ 0fb75850 (168 bytes) ===

longlong FUN_0fb75850(longlong param_1,undefined8 param_2)

{
  int iVar1;
  longlong lVar2;
  undefined4 uStack_20;
  undefined4 uStack_1c;
  undefined8 uStack_18;
  longlong lStack_10;
  
  if (param_1 != 0) {
    if (debug_trace != '\0') {
      iVar1 = (int)param_1;
      uStack_18 = param_2;
      lStack_10 = param_1;
      trace("redund_check: %s from %s index %ld\n",
            *(int *)(*(int *)(iVar1 + 0x74) + (int)param_2 * 0x14) + *(int *)(iVar1 + 0x60),
            *(undefined4 *)(iVar1 + 0x18));
      param_2 = uStack_18;
      param_1 = lStack_10;
    }
    uStack_1c = (undefined4)param_2;
    uStack_20 = (undefined4)param_1;
    lVar2 = foreach_obj(pObj_Head,&LAB_0fb78270,&uStack_20);
    if (lVar2 == -1) {
      lVar2 = 0;
    }
    return lVar2;
  }
  return 0;
}



// === FUN_0fb75900 @ 0fb75900 (140 bytes) ===

undefined8 FUN_0fb75900(undefined8 param_1,undefined8 param_2,undefined8 param_3)

{
  undefined8 uVar1;
  longlong lVar2;
  undefined *puStack_40;
  undefined1 uStack_3c;
  undefined4 uStack_38;
  undefined4 uStack_34;
  undefined8 uStack_20;
  
  uVar1 = elfhash(param_2);
  uStack_3c = 0;
  puStack_40 = &DAT_0fbdb348;
  uStack_34 = 0;
  uStack_38 = 0;
  lVar2 = find_symbol_in_object(param_1,param_2,uVar1,param_3,&puStack_40);
  if (lVar2 == -1) {
    uStack_20 = 0xffffffffffffffff;
  }
  else {
    uStack_20 = obj_dynsym_got(pObj_Head,lVar2);
  }
  free_things_tried(&puStack_40);
  return uStack_20;
}



// === FUN_0fb75990 @ 0fb75990 (704 bytes) ===

/* WARNING: Instruction at (ram,0x0fb75b54) overlaps instruction at (ram,0x0fb75b50)
    */

void FUN_0fb75990(int param_1,undefined8 param_2,undefined8 param_3)

{
  bool bVar1;
  char *pcVar2;
  int *piVar3;
  int *piVar4;
  int iVar6;
  size_t sVar7;
  longlong lVar5;
  undefined4 uVar8;
  
  if (debug_trace != '\0') {
    trace("build_entire_object_list, rld_mapped_addr 0x%lx\n",rld_mapped_addr);
  }
  if (rld_mapped_addr != 0) {
    pt_interp_name = (undefined *)elf_get_pt_interp(param_1);
    if (pt_interp_name == (undefined *)0x0) {
      pt_interp_name = &DAT_0fbdb348;
    }
    if (debug_general != '\0') {
      debug("build_entire_object_list: pt_interp_name - %s\n",pt_interp_name);
    }
  }
  objlist_add_beginning(&pObj_Head,param_1);
  if (debug_olist != '\0') {
    print_rld_object_list("build_entire_object_list added at beginning done");
  }
  if (we_are_ldd == '\0') {
    lVar5 = FUN_0fb75900(param_1,&DAT_0fbdb3f0,is_weak_or_global_symbol);
    piVar4 = Main_Argv;
    if (lVar5 != -1) {
      piVar3 = (int *)*(int *)lVar5;
      if (((piVar3 != (int *)0x0) ||
          (bVar1 = Main_Argv != (int *)0x0, *(int *)lVar5 = (int)Main_Argv, piVar3 = piVar4, bVar1))
         && (main_exec_name == (char *)0x0)) {
        main_exec_name = (char *)*piVar3;
      }
    }
    if (execrld != '\0') {
      debug("_argv = 0x%lx\n",lVar5);
    }
    lVar5 = FUN_0fb75900(param_1,"__Argc",is_weak_or_global_symbol);
    if (lVar5 != -1) {
      *(undefined4 *)lVar5 = Main_Argc;
    }
    if (execrld != '\0') {
      if (lVar5 == 0) {
        uVar8 = 0;
      }
      else {
        uVar8 = *(undefined4 *)lVar5;
      }
      debug("_argc(0x%lx) = %ld\n",lVar5,uVar8);
      pcVar2 = *(char **)(param_1 + 0x88);
      goto LAB_0fb75a2c;
    }
  }
  pcVar2 = *(char **)(param_1 + 0x88);
LAB_0fb75a2c:
  if (*pcVar2 != '\0') {
    global_library_search_rpath =
         generate_global_lib_search_path_from_obj(pcVar2,0,&user_defined_path);
  }
  if ((*(char **)(param_1 + 0x30) == (char *)0x0) ||
     (iVar6 = strcmp(*(char **)(param_1 + 0x30),""), iVar6 == 0)) {
    pcVar2 = main_exec_name;
    *(undefined1 *)(param_1 + 0xdd) = 0;
    *(char **)(param_1 + 0x2c) = pcVar2;
  }
  else {
    *(undefined1 *)(param_1 + 0xdd) = 0;
    *(undefined4 *)(param_1 + 0x2c) = *(undefined4 *)(param_1 + 0x30);
  }
  if ((*(uint *)(param_1 + 0x84) & 0x10) == 0) {
    clearFlag = 1;
  }
  if (main_exec_name != (char *)0x0) {
    *(char **)(param_1 + 0x18) = main_exec_name;
    sVar7 = strlen(main_exec_name);
    *(undefined1 *)(param_1 + 0xde) = 0;
    *(undefined4 *)(param_1 + 0x38) = 0;
    *(int *)(param_1 + 0x1c) = (int)(short)sVar7;
  }
  if (debug_map != '\0') {
    iVar6 = *(int *)(param_1 + 0xb0);
    if (iVar6 == -1) {
      iVar6 = -1;
    }
    debug("build_entire_object_list: %s has liblist size of %ld\n",*(undefined4 *)(param_1 + 0x18),
          iVar6);
  }
  FUN_0fb77df0(pObj_Head,param_3);
  iVar6 = oex_global;
  oex_global = 0x1f00;
  foreach_obj(pObj_Head,apply_oex_flags,0);
  if (oex_global != iVar6) {
    update_oex_state(0,iVar6);
  }
  lVar5 = FUN_0fb77660(param_2,pObj_Head,param_3);
  iVar6 = oex_global;
  if (0 < lVar5) {
    oex_global = 0x1f00;
    foreach_obj(pObj_Head,apply_oex_flags,0);
    if (oex_global != iVar6) {
      update_oex_state(0,iVar6);
    }
  }
  return;
}



// === FUN_0fb75c60 @ 0fb75c60 (2476 bytes) ===

/* WARNING: Instruction at (ram,0x0fb75e88) overlaps instruction at (ram,0x0fb75e84)
    */

void FUN_0fb75c60(longlong param_1,int param_2,int *param_3)

{
  char cVar1;
  char *pcVar2;
  int iVar3;
  size_t sVar4;
  undefined4 uVar5;
  ulonglong uVar6;
  int iVar7;
  longlong lVar8;
  int iVar9;
  int iVar10;
  undefined4 *puVar11;
  int iStack_80;
  int iStack_7c;
  int iStack_78;
  int iStack_74;
  longlong lStack_70;
  ulonglong uStack_68;
  ulonglong uStack_60;
  longlong lStack_58;
  
  iVar7 = *param_3;
  if (1 < param_1) {
    lVar8 = 1;
    lStack_58 = 0;
    uStack_68 = ZEXT48(&DAT_0fbd39ac);
    uStack_60 = (ulonglong)((int)param_1 * 4 + param_2);
    iVar10 = 4;
    do {
      iVar9 = (int)lVar8;
      puVar11 = (undefined4 *)(param_2 + iVar10);
      iVar3 = strcmp((char *)*puVar11,"-f");
      iVar10 = iVar9;
      if (iVar3 == 0) {
        _rld_short_circuit_delete = 1;
LAB_0fb75cd4:
        lVar8 = (longlong)(iVar10 + 1);
      }
      else {
        iVar3 = strcmp((char *)*puVar11,"-s");
        if (iVar3 == 0) {
          _rld_short_circuit_delete = 0;
          goto LAB_0fb75cd4;
        }
        iVar3 = strcmp((char *)*puVar11,"-idv");
        if (iVar3 == 0) {
          _rld_delay_load_versions_checking_on = 0;
          goto LAB_0fb75cd4;
        }
        iVar3 = strcmp((char *)*puVar11,"-ignore_unresolved");
        if (iVar3 == 0) {
          ignore_unresolved = 1;
          goto LAB_0fb75cd4;
        }
        iVar3 = strcmp((char *)*puVar11,"-big_page_load");
        if (iVar3 == 0) {
          if (iVar7 != 2) {
            if (iVar7 == 0) {
              uVar6 = syssgi(0x5c,0);
              if (uVar6 == 0xffffffffffffffff) {
                getreuid(&iStack_80,&iStack_7c);
                getregid(&iStack_78,&iStack_74);
                if ((iStack_7c == iStack_80) && (iStack_74 == iStack_78)) {
                  uVar6 = 0;
                }
                else {
                  uVar6 = 0;
                  if (iStack_80 != 0) {
                    uVar6 = 1;
                  }
                }
              }
              iVar7 = 1;
              if (uVar6 == 0) {
                iVar7 = 2;
              }
            }
            else {
              uVar6 = (ulonglong)(iVar7 != 2);
            }
            if (uVar6 != 0) goto LAB_0fb75cd4;
          }
          rld_big_page_load = 1;
          goto LAB_0fb75cd4;
        }
        iVar3 = strcmp((char *)*puVar11,"-big_page_pagesize");
        if (iVar3 == 0) {
          iVar10 = iVar9 + 1;
          if (iVar10 < param_1) {
            sVar4 = strlen((char *)puVar11[1]);
            lVar8 = __rld_bp_parse_value(puVar11[1],sVar4,&__rld_loading_data);
            if ((lVar8 == -1) && (verbose != '\0')) {
              trace("%s erroneous, ignored\n",*puVar11);
              lVar8 = (longlong)(iVar9 + 2);
              goto LAB_0fb75cd8;
            }
          }
          goto LAB_0fb75cd4;
        }
        iVar3 = strcmp((char *)*puVar11,"-big_page_mmap_size");
        if (iVar3 == 0) {
          iVar10 = iVar9 + 1;
          if (iVar10 < param_1) {
            sVar4 = strlen((char *)puVar11[1]);
            lVar8 = __rld_bp_parse_value(puVar11[1],sVar4,uStack_68);
            if ((lVar8 == -1) && (verbose != '\0')) {
              trace("%s erroneous, ignored\n",*puVar11);
              lVar8 = (longlong)(iVar9 + 2);
              goto LAB_0fb75cd8;
            }
          }
          goto LAB_0fb75cd4;
        }
        iVar3 = strcmp((char *)*puVar11,"-big_page_cache_misalign");
        if (iVar3 == 0) {
          DAT_0fbd39c4 = 1;
          goto LAB_0fb75cd4;
        }
        iVar3 = strcmp((char *)*puVar11,"-big_page_allocation_preference");
        if (iVar3 == 0) {
          iVar10 = iVar9 + 1;
          lVar8 = (longlong)iVar10;
          if (iVar7 != 2) {
            if (iVar7 == 0) {
              lStack_70 = lVar8;
              uVar6 = syssgi(0x5c,0);
              if (uVar6 == 0xffffffffffffffff) {
                getreuid(&iStack_80,&iStack_7c);
                getregid(&iStack_78,&iStack_74);
                if ((iStack_7c == iStack_80) && (iStack_74 == iStack_78)) {
                  uVar6 = 0;
                }
                else {
                  uVar6 = 0;
                  if (iStack_80 != 0) {
                    uVar6 = 1;
                  }
                }
              }
              iVar7 = 1;
              lVar8 = lStack_70;
              if (uVar6 == 0) {
                iVar7 = 2;
              }
            }
            else {
              uVar6 = (ulonglong)(iVar7 != 2);
            }
            if (uVar6 != 0) goto LAB_0fb75cd4;
          }
          if (((lVar8 < param_1) &&
              (lVar8 = __rld_setup_mmap_alloc_preference(puVar11[1]), lVar8 == -1)) &&
             (verbose != '\0')) {
            trace("%s erroneous, ignored\n",*puVar11);
            lVar8 = (longlong)(iVar9 + 2);
            goto LAB_0fb75cd8;
          }
          goto LAB_0fb75cd4;
        }
        iVar3 = strcmp((char *)*puVar11,"-debug");
        if (iVar3 == 0) {
          iVar9 = iVar9 + 1;
          lVar8 = (longlong)iVar9;
          uVar6 = (ulonglong)(int)(puVar11 + 1);
          if (lVar8 < param_1) {
            pcVar2 = (char *)puVar11[1];
            cVar1 = *pcVar2;
            while (cVar1 != '-') {
              iVar10 = strcmp(pcVar2,"general");
              puVar11 = (undefined4 *)uVar6;
              if (iVar10 == 0) {
                debug_general = 1;
              }
              else {
                iVar10 = strcmp((char *)*puVar11,"map");
                if (iVar10 == 0) {
                  debug_map = 1;
                }
                else {
                  iVar10 = strcmp((char *)*puVar11,"symbol");
                  if (iVar10 == 0) {
                    debug_symbol = 1;
                  }
                  else {
                    iVar10 = strcmp((char *)*puVar11,"conflict");
                    if (iVar10 == 0) {
                      debug_conflict = 1;
                    }
                    else {
                      iVar10 = strcmp((char *)*puVar11,"trace");
                      if (iVar10 == 0) {
                        debug_trace = 1;
                      }
                      else {
                        iVar10 = strcmp((char *)*puVar11,"MALLOC");
                        if (iVar10 == 0) {
                          debug_malloc = 1;
                        }
                        else {
                          iVar10 = strcmp((char *)*puVar11,"hash");
                          if (iVar10 == 0) {
                            debug_hash = 1;
                          }
                          else {
                            iVar10 = strcmp((char *)*puVar11,"olist");
                            if (iVar10 == 0) {
                              debug_olist = 1;
                            }
                            else {
                              iVar10 = strcmp((char *)*puVar11,"interface");
                              if (iVar10 == 0) {
                                interface = 1;
                              }
                              else {
                                iVar10 = strcmp((char *)*puVar11,"execrld");
                                if (iVar10 == 0) {
                                  execrld = 1;
                                }
                                else {
                                  iVar10 = strcmp((char *)*puVar11,"threads");
                                  if (iVar10 == 0) {
                                    debug_threads = 1;
                                  }
                                  else {
                                    lStack_58 = (longlong)((int)lStack_58 + 1);
                                    error("rld: unknown flag: %s\n",*puVar11);
                                  }
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
              uVar6 = (ulonglong)(int)(puVar11 + 1);
              iVar9 = (int)lVar8 + 1;
              lVar8 = (longlong)iVar9;
              if (uStack_60 <= uVar6) break;
              pcVar2 = (char *)puVar11[1];
              cVar1 = *pcVar2;
            }
          }
          iVar10 = iVar9 + -1;
          goto LAB_0fb75cd4;
        }
        iVar3 = strcmp((char *)*puVar11,"-force");
        if (iVar3 == 0) {
          iVar9 = iVar9 + 1;
          lVar8 = (longlong)iVar9;
          uVar6 = (ulonglong)(int)(puVar11 + 1);
          if (lVar8 < param_1) {
            pcVar2 = (char *)puVar11[1];
            cVar1 = *pcVar2;
            while (cVar1 != '-') {
              iVar10 = strcmp(pcVar2,"moved");
              puVar11 = (undefined4 *)uVar6;
              if (iVar10 == 0) {
                force_moved = 1;
              }
              else {
                iVar10 = strcmp((char *)*puVar11,"timestamp");
                if (iVar10 == 0) {
                  force_timestamp = 1;
                }
                else {
                  iVar10 = strcmp((char *)*puVar11,"checksum");
                  if (iVar10 == 0) {
                    force_checksum = 1;
                  }
                  else {
                    lStack_58 = (longlong)((int)lStack_58 + 1);
                    error("_RLD_ARGS: Unknown flag: %s\n",*puVar11);
                  }
                }
              }
              uVar6 = (ulonglong)(int)(puVar11 + 1);
              iVar9 = (int)lVar8 + 1;
              lVar8 = (longlong)iVar9;
              if (uStack_60 <= uVar6) break;
              pcVar2 = (char *)puVar11[1];
              cVar1 = *pcVar2;
            }
          }
          iVar10 = iVar9 + -1;
          goto LAB_0fb75cd4;
        }
        iVar3 = strcmp((char *)*puVar11,"-v");
        if (iVar3 == 0) {
          verbose = '\x01';
          goto LAB_0fb75cd4;
        }
        iVar3 = strcmp((char *)*puVar11,"-trace");
        if (iVar3 == 0) {
          quickstart_info = 1;
          user_tracking = 1;
          goto LAB_0fb75cd4;
        }
        iVar3 = strcmp((char *)*puVar11,"-warnconflict");
        if (iVar3 == 0) {
          warnconflict = 1;
          goto LAB_0fb75cd4;
        }
        iVar3 = strcmp((char *)*puVar11,"-check_interface");
        if (iVar3 == 0) {
          check_interface = 1;
          goto LAB_0fb75cd4;
        }
        iVar3 = strcmp((char *)*puVar11,"-clearstack");
        if (iVar3 == 0) {
          clearFlag = 1;
          goto LAB_0fb75cd4;
        }
        iVar3 = strcmp((char *)*puVar11,"-noclearstack");
        if (iVar3 == 0) {
          clearFlag = 0;
          goto LAB_0fb75cd4;
        }
        iVar3 = strcmp((char *)*puVar11,"-stat");
        if (iVar3 == 0) {
          var_stat = 1;
          goto LAB_0fb75cd4;
        }
        iVar3 = strcmp((char *)*puVar11,"-ignore_all_versions");
        if (iVar3 == 0) {
          ignore_all_versions = 1;
          goto LAB_0fb75cd4;
        }
        iVar3 = strcmp((char *)*puVar11,"-ignore_exact_match");
        if (iVar3 == 0) {
          ignore_exact_match = 1;
          goto LAB_0fb75cd4;
        }
        iVar3 = strcmp((char *)*puVar11,"-ignore_no_library_replacement");
        if (iVar3 == 0) {
          ignore_no_library_replacement = 1;
          goto LAB_0fb75cd4;
        }
        iVar3 = strcmp((char *)*puVar11,"-interact");
        if (iVar3 == 0) {
          interact = 1;
          goto LAB_0fb75cd4;
        }
        iVar3 = strcmp((char *)*puVar11,"-old_interface");
        if (iVar3 == 0) {
          old_interface = 1;
          goto LAB_0fb75cd4;
        }
        iVar3 = strcmp((char *)*puVar11,"-quickstart_info");
        if (iVar3 == 0) {
          quickstart_info = 1;
          goto LAB_0fb75cd4;
        }
        iVar3 = strcmp((char *)*puVar11,"-quickstart_only");
        if (iVar3 == 0) {
          quickstart_only = 1;
          goto LAB_0fb75cd4;
        }
        iVar3 = strcmp((char *)*puVar11,"-ignore_version");
        if (iVar3 == 0) {
          iVar9 = iVar9 + 1;
          lVar8 = (longlong)iVar9;
          uVar6 = (ulonglong)(int)(puVar11 + 1);
          lStack_70 = lVar8;
          if ((ignore_version_list == 0) &&
             (ignore_version_list = malloc(ignore_version_list_size << 2), ignore_version_list == 0)
             ) {
            fatal("parse_args: unable to malloc for ignore_version_list\n");
          }
          if (lStack_70 < param_1) {
            cVar1 = *(char *)puVar11[1];
            while (cVar1 != '-') {
              if (ignore_version_list_size <= ignore_so_num) {
                ignore_version_list_size = ignore_version_list_size + 0x14;
                ignore_version_list = realloc(ignore_version_list,ignore_version_list_size * 4);
                if (ignore_version_list == 0) {
                  fatal("parse_args: unable to realloc for ignore_version_list\n");
                }
              }
              ignore_so_num = ignore_so_num + 1;
              uVar5 = rld_obj_newstr(*(undefined4 *)uVar6);
              puVar11 = (undefined4 *)uVar6 + 1;
              uVar6 = (ulonglong)(int)puVar11;
              iVar9 = (int)lVar8 + 1;
              lVar8 = (longlong)iVar9;
              *(undefined4 *)(ignore_version_list + ignore_so_num * 4 + -4) = uVar5;
              if (uStack_60 <= uVar6) break;
              cVar1 = *(char *)*puVar11;
            }
          }
          iVar10 = iVar9 + -1;
          goto LAB_0fb75cd4;
        }
        iVar3 = strcmp((char *)*puVar11,"-log");
        if (iVar3 == 0) {
          if (iVar7 != 2) {
            if (iVar7 == 0) {
              uVar6 = syssgi(0x5c,0);
              if (uVar6 == 0xffffffffffffffff) {
                getreuid(&iStack_80,&iStack_7c);
                getregid(&iStack_78,&iStack_74);
                if ((iStack_7c == iStack_80) && (iStack_74 == iStack_78)) {
                  uVar6 = 0;
                }
                else {
                  uVar6 = 0;
                  if (iStack_80 != 0) {
                    uVar6 = 1;
                  }
                }
              }
              iVar7 = 1;
              if (uVar6 == 0) {
                iVar7 = 2;
              }
            }
            else {
              uVar6 = (ulonglong)(iVar7 != 2);
            }
            if (uVar6 != 0) {
              if (verbose != '\0') {
                trace("-log ignored because of setuid or setgid programs\n");
              }
              iVar10 = iVar9 + 1;
              goto LAB_0fb75cd4;
            }
          }
          error_file_name = rld_obj_newstr(puVar11[1]);
          iVar10 = iVar9 + 1;
          goto LAB_0fb75cd4;
        }
        iVar3 = strcmp((char *)*puVar11,"-guarantee_init");
        if (iVar3 == 0) {
          guarantee_init = 1;
          guarantee_start_init = guarantee_start_init | 1;
          goto LAB_0fb75cd4;
        }
        iVar3 = strncmp((char *)*puVar11,"-y",2);
        if (iVar3 == 0) {
          rld_add_to_listed_symbols((char *)*puVar11 + 2);
          lVar8 = (longlong)(iVar9 + 1);
        }
        else {
          iVar3 = strcmp((char *)*puVar11,"-speedshop");
          if (iVar3 == 0) {
            _rld_running_under_speedshop = 1;
            goto LAB_0fb75cd4;
          }
          lStack_58 = (longlong)((int)lStack_58 + 1);
          error("rld: Unknown Flag: %s\n",*puVar11);
          lVar8 = (longlong)(iVar9 + 1);
        }
      }
LAB_0fb75cd8:
      iVar10 = (int)lVar8 << 2;
    } while (lVar8 < param_1);
    if (lStack_58 != 0) {
      error(
           "_RLD_ARGS:\n[-v]\n[-clearstack]\n[-f]\n[-s]\n[-idv\n[-quickstart_info]\n[-y<symname> ]\n[-warnconflict][-log <file>]\n[-trace]\n[-debug [map symbol conflict trace general MALLOC hash interface execrld olist] \n[-force [moved checksum timestamp]\n----------the following are not very useful----------\n[-ignore_all_versions]\n[-ignore_version soname]\n[-ignore_exact_match]\n[-ignore_unresolved]\n[-noclearstack]\n[-quickstart_only]\n[-ignore_no_library_replacement]\n[-stat]\n"
           );
    }
  }
  set_ltr_extra();
  return;
}



// === FUN_0fb76620 @ 0fb76620 (148 bytes) ===

void FUN_0fb76620(void)

{
  bool bVar1;
  undefined8 uVar2;
  undefined **ppuVar3;
  int iVar4;
  int iVar5;
  code *UNRECOVERED_JUMPTABLE;
  
  if (old_interface == '\0') {
    iVar5 = 1;
  }
  else {
    iVar5 = 8;
  }
  ppuVar3 = &_rld_special_funcs;
  iVar4 = 0;
  do {
    uVar2 = elfhash(*ppuVar3);
    bVar1 = interface != '\0';
    ppuVar3[1] = (undefined *)uVar2;
    if (bVar1) {
      trace("_rld_special_funcs: (%ld) : %s(hash=0x%lx)\t0x%lx\n",iVar4,*ppuVar3,uVar2,ppuVar3[2]);
    }
    iVar4 = iVar4 + 1;
    ppuVar3 = ppuVar3 + 3;
  } while (iVar4 != iVar5);
                    /* WARNING: Could not recover jumptable at 0x0fb7669c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
}



// === FUN_0fb766c0 @ 0fb766c0 (1236 bytes) ===

void FUN_0fb766c0(int *param_1)

{
  int iVar1;
  int iVar2;
  ulonglong uVar3;
  char *pcVar6;
  size_t sVar7;
  undefined8 uVar4;
  longlong lVar5;
  char *pcVar8;
  int iVar9;
  int iVar10;
  char **ppcVar11;
  undefined1 *puVar12;
  undefined1 *puVar13;
  int iStack_30;
  int iStack_2c;
  int iStack_28;
  int iStack_24;
  char *apcStack_20 [8];
  undefined1 *puVar14;
  
  puVar12 = &stack0xffffff80;
  apcStack_20[1] = "_RLD_ARGS_BIG_PAGES";
  apcStack_20[0] = "_RLD_ARGS";
  iVar9 = *param_1;
  iVar10 = 0;
  ppcVar11 = apcStack_20;
  do {
    if ((iVar10 == 1) && (iVar9 != 2)) {
      if (iVar9 == 0) {
        uVar3 = syssgi(0x5c,0);
        if (uVar3 == 0xffffffffffffffff) {
          getreuid(&iStack_28,&iStack_30);
          getregid(&iStack_2c,&iStack_24);
          if (((iStack_30 != iStack_28) || (uVar3 = 0, iStack_24 != iStack_2c)) &&
             (uVar3 = 0, iStack_28 != 0)) {
            uVar3 = 1;
          }
        }
        iVar9 = 1;
        if (uVar3 == 0) {
          iVar9 = 2;
        }
      }
      else {
        uVar3 = (ulonglong)(iVar9 != 2);
      }
      if (uVar3 != 0) break;
    }
    pcVar8 = *ppcVar11;
    pcVar6 = getenv(pcVar8);
    if (pcVar6 != (char *)0x0) {
      sVar7 = strlen(pcVar6);
      iVar1 = -(sVar7 * 4 + 0x13 & 0xfffffff0);
      iVar2 = -(sVar7 + 0x10 & 0xfffffff0);
      puVar13 = puVar12 + iVar2 + iVar1;
      puVar14 = puVar12 + iVar2 + iVar1;
      uVar4 = split(pcVar6,(int)puVar12 + iVar1,puVar12 + iVar2 + iVar1,sVar7 + 1,&DAT_0fbdba20,1);
      FUN_0fb75c60(uVar4,(int)puVar12 + iVar1,param_1);
      if (verbose != '\0') {
        trace("%s = \"%s\"\n",pcVar8,pcVar6);
      }
      puVar12 = puVar12 + iVar2 + iVar1;
      if ((iVar10 == 0) && (puVar12 = puVar14, verbose != '\0')) {
        trace("rld version 7.4.1a n32 ABI %s %s \n","Oct 24 2005","20:52:01");
        puVar12 = puVar13;
      }
    }
    iVar10 = iVar10 + 1;
    ppcVar11 = ppcVar11 + 1;
  } while (iVar10 != 2);
  if ((rld_big_page_load != '\0') && (lVar5 = __rld_complete_big_page_control_setup(), lVar5 < 0)) {
    rld_big_page_load = '\0';
  }
  global_path_prefix = getenv("_RLDN32_ROOT");
  if (global_path_prefix == (char *)0x0) {
    global_path_prefix = getenv("_RLD_ROOT");
    if (global_path_prefix == (char *)0x0) {
      pcVar8 = "_RLDN32_ROOT";
    }
    else {
      pcVar8 = "_RLD_ROOT";
    }
  }
  else {
    pcVar8 = "_RLDN32_ROOT";
  }
  if (global_path_prefix == (char *)0x0) {
    global_path_prefix = "";
    if (verbose != '\0') {
      trace("%s is empty\n",pcVar8);
    }
  }
  else {
    if (verbose != '\0') {
      trace("%s = %s\n",pcVar8);
    }
    if (iVar9 != 2) {
      if (iVar9 == 0) {
        uVar3 = syssgi(0x5c,0);
        if (uVar3 == 0xffffffffffffffff) {
          getreuid(&iStack_28,&iStack_30);
          getregid(&iStack_2c,&iStack_24);
          if (((iStack_30 != iStack_28) || (uVar3 = 0, iStack_24 != iStack_2c)) &&
             (uVar3 = 0, iStack_28 != 0)) {
            uVar3 = 1;
          }
        }
        iVar9 = 1;
        if (uVar3 == 0) {
          iVar9 = 2;
        }
      }
      else {
        uVar3 = (ulonglong)(iVar9 != 2);
      }
      if (uVar3 != 0) {
        if (debug_map != '\0') {
          debug("get_environ_vars: %s set to NULL because of setuid or setgid programs\n",pcVar8);
        }
        if (verbose != '\0') {
          trace("%s set to default because of setuid or setgid programs\n",pcVar8);
        }
        global_path_prefix = "";
        goto LAB_0fb7686c;
      }
    }
    user_defined_path = 1;
  }
LAB_0fb7686c:
  ld_path = getenv("LD_LIBRARYN32_PATH");
  if (ld_path == (char *)0x0) {
    ld_path = getenv("LD_LIBRARY_PATH");
    if (ld_path == (char *)0x0) {
      pcVar8 = "LD_LIBRARYN32_PATH";
    }
    else {
      pcVar8 = "LD_LIBRARY_PATH";
    }
  }
  else {
    pcVar8 = "LD_LIBRARYN32_PATH";
  }
  if (ld_path != (char *)0x0) {
    if (iVar9 != 2) {
      if (iVar9 == 0) {
        uVar3 = syssgi(0x5c,0);
        if (uVar3 == 0xffffffffffffffff) {
          getreuid(&iStack_28,&iStack_30);
          getregid(&iStack_2c,&iStack_24);
          if (((iStack_30 != iStack_28) || (uVar3 = 0, iStack_24 != iStack_2c)) &&
             (uVar3 = 0, iStack_28 != 0)) {
            uVar3 = 1;
          }
        }
        if (uVar3 == 0) {
          iVar9 = 2;
        }
        else {
          iVar9 = 1;
        }
      }
      else {
        uVar3 = (ulonglong)(iVar9 != 2);
      }
      if (uVar3 != 0) {
        if (debug_map != '\0') {
          debug("get_environ_vars: %s ignored because of either NO_LIBRARY_REPLACEMENT or setuid or setgid programs\n"
                ,pcVar8);
        }
        if (verbose != '\0') {
          trace("%s ignored because of either NO_LIBRARY_REPLACEMENT or setuid or setgid programs\n"
                ,pcVar8);
        }
        ld_path = (char *)0x0;
        goto LAB_0fb768d4;
      }
    }
    user_defined_path = 1;
  }
LAB_0fb768d4:
  if (verbose != '\0') {
    pcVar6 = ld_path;
    if (ld_path == (char *)0x0) {
      pcVar6 = "none";
    }
    trace("%s = %s\n",pcVar8,pcVar6);
  }
  pcVar8 = getenv("LD_BIND_NOW");
  if (pcVar8 != (char *)0x0) {
    iVar10 = strcmp(pcVar8,"1");
    if ((iVar10 == 0) || (iVar10 = strcmp(pcVar8,"on"), iVar10 == 0)) {
      _rld_environment_ld_bind_now = 1;
    }
    if (verbose != '\0') {
      trace("LD_BIND_NOW = %s (and is on",pcVar8);
    }
  }
  if (user_tracking != '\0') {
    track("Entering RLD through MAIN\n");
  }
  *param_1 = iVar9;
  return;
}



// === FUN_0fb76ba0 @ 0fb76ba0 (712 bytes) ===

/* WARNING: Instruction at (ram,0x0fb76bc4) overlaps instruction at (ram,0x0fb76bc0)
    */

void FUN_0fb76ba0(void)

{
  undefined4 uVar1;
  undefined8 uVar2;
  char cVar3;
  undefined4 *puVar4;
  int unaff_gp_lo;
  int iStack_30;
  undefined1 uStack_2c;
  undefined4 uStack_28;
  undefined4 uStack_24;
  longlong lStack_20;
  ulonglong uStack_18;
  
  uStack_18 = (ulonglong)*(byte *)(unaff_gp_lo + -0x7eac);
  FUN_0fb76620();
  if (*(char *)(unaff_gp_lo + -0x7eb0) == '\0') {
    cVar3 = *(char *)(unaff_gp_lo + -0x7ee1);
  }
  else {
    cVar3 = *(char *)(unaff_gp_lo + -0x7ee1);
    if (*(char *)(unaff_gp_lo + 0x3210) == '\0') {
      fatal(unaff_gp_lo + 0xb48);
      cVar3 = *(char *)(unaff_gp_lo + -0x7ee1);
    }
  }
  if (cVar3 != '\0') {
    create_interface_symtab();
    foreach_obj(*(undefined4 *)(unaff_gp_lo + -0x7f8c),process_interface,0);
  }
  foreach_obj(*(undefined4 *)(unaff_gp_lo + -0x7f8c),commit_obj,0);
  uVar1 = *(undefined4 *)(unaff_gp_lo + -0x7f8c);
  if (*(char *)(unaff_gp_lo + -0x7ee9) != '\0') {
    trace(unaff_gp_lo + 0xb80);
    uVar1 = *(undefined4 *)(unaff_gp_lo + -0x7f8c);
  }
  foreach_obj(uVar1,handle_undefineds,uStack_18);
  foreach_obj(*(undefined4 *)(unaff_gp_lo + -0x7f8c),handle_conflicts,0);
  if (*(char *)(unaff_gp_lo + -0x7ea9) != '\0') {
    if (*(char *)(unaff_gp_lo + -0x7eeb) != '\0') {
      debug(unaff_gp_lo + 0xba0);
    }
    fatal(unaff_gp_lo + 0xbd8);
  }
  if (*(int *)(unaff_gp_lo + -0x7f10) != 0) {
    lStack_20 = FUN_0fb75900(*(undefined4 *)(unaff_gp_lo + -0x7f8c),unaff_gp_lo + 0xc08,
                             is_symbol_in_got);
    if (lStack_20 == -1) {
      uVar2 = elfhash(unaff_gp_lo + 0xc08,0xffffffffffffffff);
      uStack_2c = 0;
      iStack_30 = unaff_gp_lo + 0x208;
      uStack_24 = 0;
      uStack_28 = 0;
      lStack_20 = resolve_symbol(unaff_gp_lo + 0xc08,unaff_gp_lo + 0xc18,uVar2,0,0,0,&iStack_30);
      if (*(char *)(unaff_gp_lo + -0x7edb) != '\0') {
        debug(unaff_gp_lo + 0xc20,unaff_gp_lo + 0xc08,lStack_20);
      }
      free_things_tried(&iStack_30);
    }
    else if (*(char *)(unaff_gp_lo + -0x7edb) != '\0') {
      debug(unaff_gp_lo + 0xc48,unaff_gp_lo + 0xc08,lStack_20);
    }
    if (lStack_20 == -1) {
      if (*(char *)(unaff_gp_lo + -0x7ec0) == '\0') {
        fatal(unaff_gp_lo + 0xc68);
      }
    }
    else {
      *(undefined4 *)lStack_20 = *(undefined4 *)(unaff_gp_lo + -0x7f1c);
    }
  }
  *(undefined4 *)(unaff_gp_lo + -0x7f84) = 1;
  foreach_obj(*(undefined4 *)(unaff_gp_lo + -0x7f8c),set_obj_initial_bit,0);
  *(undefined4 *)(unaff_gp_lo + -0x7f08) = *(undefined4 *)(*(int *)(unaff_gp_lo + -0x7f8c) + 0xf8);
  if (*(char *)(unaff_gp_lo + -0x7edb) != '\0') {
    debug(unaff_gp_lo + 0xc88);
    puVar4 = *(undefined4 **)(unaff_gp_lo + -0x7f08);
    if (*(char *)(unaff_gp_lo + -0x7edb) == '\0') goto LAB_0fb76d20;
    debug(unaff_gp_lo + 0xca0);
    if (*(char *)(unaff_gp_lo + -0x7edb) != '\0') {
      debug(unaff_gp_lo + 0xcc0,*(undefined4 *)(unaff_gp_lo + -0x7f8c));
    }
  }
  puVar4 = *(undefined4 **)(unaff_gp_lo + -0x7f08);
LAB_0fb76d20:
  if (puVar4 != (undefined4 *)0x0) {
    *puVar4 = *(undefined4 *)(unaff_gp_lo + -0x7f8c);
  }
  foreach_obj(*(undefined4 *)(unaff_gp_lo + -0x7f8c),call_pixie_init,0);
  *(undefined1 *)(unaff_gp_lo + -0x7f80) = 1;
  __rld_calls_on_rld_list_change();
  execute_all_init_sections(0);
  cleanup_record_memory_free(unaff_gp_lo + -0x7f70);
  syssgi(0x6c);
  if (*(char *)(unaff_gp_lo + -0x7eda) != '\0') {
    trace(unaff_gp_lo + 0xcd8,*(undefined4 *)(unaff_gp_lo + -0x7e58));
    if (*(char *)(unaff_gp_lo + -0x7eda) != '\0') {
      trace(unaff_gp_lo + 0xd00,*(undefined4 *)(unaff_gp_lo + -0x7e54));
      if (*(char *)(unaff_gp_lo + -0x7eda) != '\0') {
        trace(unaff_gp_lo + 0xd28,*(undefined4 *)(unaff_gp_lo + -0x7e50));
        if (*(char *)(unaff_gp_lo + -0x7eda) != '\0') {
          trace(unaff_gp_lo + 0xd58,*(undefined4 *)(unaff_gp_lo + -0x7e4c));
        }
      }
    }
  }
  if (*(char *)(unaff_gp_lo + -0x7ee4) != '\0') {
    track(unaff_gp_lo + 0xd88);
  }
  return;
}



// === FUN_0fb76e70 @ 0fb76e70 (1320 bytes) ===

void FUN_0fb76e70(undefined8 param_1,longlong param_2,int *param_3)

{
  uint uVar1;
  int iVar2;
  size_t sVar4;
  int iVar5;
  longlong lVar3;
  int iVar6;
  char *pcVar7;
  uint uVar8;
  int *piVar9;
  int iVar10;
  undefined1 auStack_970 [2048];
  undefined1 *puStack_170;
  undefined1 uStack_16c;
  undefined4 uStack_168;
  undefined4 uStack_164;
  char acStack_160 [264];
  ulonglong uStack_58;
  longlong lStack_50;
  undefined8 uStack_48;
  
  puStack_170 = auStack_970;
  uStack_16c = 0;
  uStack_164 = 0x800;
  uStack_168 = 0;
  auStack_970[0] = 0;
  iVar5 = *param_3;
  uStack_48 = param_1;
  if (iVar5 == 0) {
    lStack_50 = 0;
    uStack_58 = 0;
    iVar10 = 0;
    iVar6 = -1;
  }
  else {
    lStack_50 = 0;
    uStack_58 = 0;
    iVar10 = 0;
    iVar6 = -1;
    puStack_170 = auStack_970;
    do {
      if (5 < iVar5) {
        if (iVar5 < 7) {
          pagesize = param_3[1];
          if (execrld != '\0') {
            debug("AT_PAGESZ set to %ld\n");
          }
          if (pagesize != 0) goto LAB_0fb76ee4;
          getpagesize();
          iVar5 = param_3[2];
          goto LAB_0fb76ee8;
        }
        if (iVar5 < 9) {
          if (iVar5 < 8) {
            if ((iVar5 != 7) || (rld_mapped_addr = param_3[1], execrld == '\0')) goto LAB_0fb76ee4;
            debug("AT_BASE set to 0x%lx\n");
            iVar5 = param_3[2];
          }
          else {
            if (execrld == '\0') goto LAB_0fb76ee4;
            debug("AT_FLAGS set to %ld\n",param_3[1]);
            iVar5 = param_3[2];
          }
        }
        else if (iVar5 < 10) {
          if (execrld == '\0') goto LAB_0fb76ee4;
          debug("AT_ENTRY set to 0x%lx\n",param_3[1]);
          iVar5 = param_3[2];
        }
        else {
          if ((iVar5 != 100) || (iVar10 = param_3[1], execrld == '\0')) goto LAB_0fb76ee4;
          debug("AT_PFETCHFD set to %ld\n");
          iVar5 = param_3[2];
        }
        goto LAB_0fb76ee8;
      }
      if (iVar5 < 4) {
        if (iVar5 < 3) {
          if ((iVar5 != 2) || (iVar6 = param_3[1], execrld == '\0')) goto LAB_0fb76ee4;
          debug("AT_EXECFD set to %ld\n");
          iVar5 = param_3[2];
        }
        else {
          lStack_50 = (longlong)param_3[1];
          if (execrld == '\0') goto LAB_0fb76ee4;
          debug("AT_PHDR set to 0x%lx\n");
          iVar5 = param_3[2];
        }
      }
      else if (iVar5 < 5) {
        if (execrld == '\0') {
LAB_0fb76ee4:
          iVar5 = param_3[2];
        }
        else {
          debug("AT_PHENT set to %ld\n",param_3[1]);
          iVar5 = param_3[2];
        }
      }
      else {
        if ((iVar5 != 5) || (uStack_58 = (ulonglong)param_3[1], execrld == '\0')) goto LAB_0fb76ee4;
        debug("AT_PHNUM set to %ld\n");
        iVar5 = param_3[2];
      }
LAB_0fb76ee8:
      param_3 = param_3 + 2;
    } while (iVar5 != 0);
  }
  if (rld_mapped_addr != 0) {
    if (param_2 == 0) {
      main_exec_name = (char *)0x0;
    }
    else {
      sVar4 = strlen((char *)param_2);
      main_exec_name = (char *)malloc(sVar4 + 1);
      strcpy(main_exec_name,(char *)param_2);
    }
    iVar5 = sysinfo((sysinfo *)0x6d);
    if ((iVar5 != -1) && (iVar5 = strncmp(acStack_160,"R8000",5), iVar5 == 0)) {
      R8000_arch = 1;
    }
    if (iVar6 == -1) {
      if ((longlong)uStack_58 < 1) {
        iVar5 = 0;
      }
      else {
        iVar5 = 0;
        uVar8 = 0;
        piVar9 = (int *)lStack_50;
        lVar3 = lStack_50;
        if ((((uStack_58 & 1) != 0) && (lVar3 = (longlong)(int)(piVar9 + 8), *piVar9 == 1)) &&
           ((piVar9[6] & 6U) == 4)) {
          uVar8 = piVar9[1];
          iVar5 = piVar9[2] - uVar8;
        }
        if ((int)uStack_58 >> 1 != 0) {
          iVar6 = *(int *)lVar3;
          while( true ) {
            iVar2 = (int)lVar3;
            if ((iVar6 == 1) && ((*(uint *)(iVar2 + 0x18) & 6) == 4)) {
              uVar1 = *(uint *)(iVar2 + 4);
              if (iVar5 == 0) {
                iVar5 = *(int *)(iVar2 + 8) - uVar1;
                uVar8 = uVar1;
              }
              else if (uVar1 < uVar8) {
                iVar5 = *(int *)(iVar2 + 8) - uVar1;
                uVar8 = uVar1;
              }
            }
            if ((*(int *)(iVar2 + 0x20) == 1) && ((*(uint *)(iVar2 + 0x38) & 6) == 4)) {
              uVar1 = *(uint *)(iVar2 + 0x24);
              if (iVar5 == 0) {
                iVar5 = *(int *)(iVar2 + 0x28) - uVar1;
                uVar8 = uVar1;
              }
              else if (uVar1 < uVar8) {
                iVar5 = *(int *)(iVar2 + 0x28) - uVar1;
                uVar8 = uVar1;
              }
            }
            lVar3 = (longlong)(iVar2 + 0x40);
            if (lVar3 == (int)(piVar9 + (int)uStack_58 * 8)) break;
            iVar6 = *(int *)(iVar2 + 0x40);
          }
        }
      }
      if (execrld != '\0') {
        debug("main_obj_addr = 0x%lx\n",iVar5);
      }
      if (iVar5 != 0) {
        if (debug_trace != '\0') {
          trace("build_entire_object_list_by_addr: addr 0x%lx\n",iVar5);
        }
        pcVar7 = main_exec_name;
        if (main_exec_name == (char *)0x0) {
          pcVar7 = "Main";
        }
        iVar6 = create_object(pcVar7,0);
        *(undefined1 *)(iVar6 + 0xc3) = 1;
        *(int *)(iVar6 + 0x10) = iVar5;
        *(int *)(iVar6 + 0x20) = (int)lStack_50;
        map_object_into_mem_and_init_object_info(iVar6,0,0,pObj_Head,1,iVar10,0,&puStack_170);
        FUN_0fb75990(iVar6,uStack_48,&puStack_170);
      }
    }
    else {
      if (execrld != '\0') {
        debug("pathname = %s\n",param_2);
      }
      if (debug_trace != '\0') {
        trace("build_entire_object_list_by_fd: fd %ld\n",iVar6);
      }
      pcVar7 = main_exec_name;
      if (main_exec_name == (char *)0x0) {
        pcVar7 = "Main";
      }
      iVar5 = create_object(pcVar7,0);
      *(int *)(iVar5 + 0x34) = iVar6;
      map_object_into_mem_and_init_object_info(iVar5,0,0,pObj_Head,1,iVar10,0,&puStack_170);
      FUN_0fb75990(iVar5,uStack_48,&puStack_170);
    }
    free_things_tried(&puStack_170);
    return;
  }
                    /* WARNING: Subroutine does not return */
  exit(1);
}



// === main @ 0fb773b0 (8 bytes) ===

void main(void)

{
  return;
}



// === sgi_main @ 0fb773c0 (668 bytes) ===

void sgi_main(int param_1,int *param_2,int *param_3)

{
  int *piVar1;
  char *__s1;
  int iVar3;
  longlong lVar2;
  int *piVar4;
  int *piVar5;
  code *UNRECOVERED_JUMPTABLE;
  undefined4 auStack_50 [2];
  undefined4 uStack_48;
  int iStack_44;
  undefined4 uStack_40;
  Elf32_Ehdr *pEStack_3c;
  undefined4 uStack_38;
  
  auStack_50[0] = 0;
  pagesize = getpagesize();
  Main_Argv = param_2;
  Main_Argc = param_1;
  if ((char *)*param_2 == (char *)0x0) {
    __s1 = "";
  }
  else {
    __s1 = basename((char *)*param_2);
  }
  iVar3 = strcmp(__s1,"_sgi_internal_ldd_name_of_rld");
  if (iVar3 != 0) {
    piVar5 = param_3;
    piVar4 = param_3;
    if (*param_3 != 0) {
      while (((piVar5 = piVar4 + 1, piVar4[1] != 0 && (piVar5 = piVar4 + 2, piVar4[2] != 0)) &&
             (piVar5 = piVar4 + 3, piVar4[3] != 0))) {
        piVar5 = piVar4 + 4;
        if ((piVar4[4] == 0) ||
           (piVar1 = piVar4 + 5, piVar5 = piVar4 + 5, piVar4 = piVar5, *piVar1 == 0)) break;
      }
    }
    _environ = param_3;
    lVar2 = malloc(0x1000);
    last_error_message = (undefined4)lVar2;
    if (lVar2 == 0) {
      fatal("cannot malloc for last_error_message\n");
    }
    FUN_0fb766c0(auStack_50);
    init_signal_mask();
    FUN_0fb76e70(auStack_50[0],*param_2,piVar5 + 1);
    FUN_0fb76ba0();
                    /* WARNING: Could not recover jumptable at 0x0fb774d4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*UNRECOVERED_JUMPTABLE)();
    return;
  }
  _rld_do_delay_loaded_libs_instantly = 1;
  if ((char *)param_2[2] != (char *)0x0) {
    iVar3 = strcmp((char *)param_2[2],"-D");
    if (iVar3 == 0) {
      _rld_do_delay_loaded_libs_instantly = 0;
    }
    else if ((char *)param_2[2] != (char *)0x0) {
      iVar3 = strcmp((char *)param_2[2],"-Dv");
      if ((iVar3 == 0) || (iVar3 = strcmp((char *)param_2[2],"-vD"), iVar3 == 0)) {
        _rld_extended_ldd_output = 1;
        _rld_do_delay_loaded_libs_instantly = 0;
      }
      else if (((char *)param_2[2] != (char *)0x0) &&
              (iVar3 = strcmp((char *)param_2[2],"-v"), iVar3 == 0)) {
        _rld_extended_ldd_output = 1;
      }
    }
  }
  uStack_48 = 2;
  iStack_44 = open64((char *)param_2[1],0);
  __sgi_fixup_args = 1;
  uStack_38 = 0;
  pEStack_3c = &Elf32_Ehdr_0fb60000;
  uStack_40 = 7;
  we_are_ldd = 1;
  ld_trace_loaded_objects = 1;
  iVar3 = *param_2;
  param_2[-1] = param_1 + -1;
  piVar5 = param_2;
  while (piVar4 = piVar5, iVar3 != 0) {
    *piVar5 = piVar5[1];
    piVar4 = piVar5 + 1;
    if (piVar5[1] == 0) break;
    iVar3 = piVar5[2];
    piVar5[1] = iVar3;
    piVar5 = piVar5 + 2;
  }
  do {
    *piVar4 = piVar4[1];
    if (piVar4[1] == 0) break;
    piVar5 = piVar4 + 2;
    piVar4[1] = *piVar5;
    piVar4 = piVar4 + 2;
  } while (*piVar5 != 0);
  Main_Argv = param_2;
  _environ = param_3;
  lVar2 = malloc(0x1000);
  last_error_message = (undefined4)lVar2;
  if (lVar2 == 0) {
    fatal("cannot malloc for last_error_message\n");
  }
  FUN_0fb766c0(auStack_50);
  init_signal_mask();
  FUN_0fb76e70(auStack_50[0],*param_2,&uStack_48);
                    /* WARNING: Subroutine does not return */
  exit(0);
}



// === FUN_0fb77660 @ 0fb77660 (1920 bytes) ===

/* WARNING: Instruction at (ram,0x0fb778f0) overlaps instruction at (ram,0x0fb778ec)
    */

undefined1  [16] FUN_0fb77660(int param_1,undefined8 param_2,undefined8 param_3)

{
  undefined4 uVar1;
  char *pcVar5;
  size_t sVar6;
  undefined8 uVar2;
  int iVar7;
  longlong lVar3;
  longlong lVar4;
  ulonglong in_v1;
  ulonglong extraout_v1;
  ulonglong extraout_v1_00;
  ulonglong extraout_v1_01;
  ulonglong extraout_v1_02;
  ulonglong extraout_v1_03;
  undefined8 extraout_v1_04;
  ulonglong extraout_v1_05;
  int iVar8;
  undefined4 *puVar9;
  longlong lVar10;
  longlong lVar11;
  longlong lVar12;
  longlong lVar13;
  longlong lVar14;
  ulonglong unaff_s8;
  undefined1 auVar15 [16];
  undefined1 auVar16 [16];
  undefined1 auVar17 [16];
  undefined1 auVar18 [16];
  undefined1 auVar19 [16];
  undefined4 uStack_d4;
  longlong lStack_80;
  int iStack_3c;
  int iStack_38;
  int iStack_34;
  int iStack_30;
  int aiStack_2c [4];
  int iStack_1c;
  
  aiStack_2c[0] = 0;
  iStack_1c = param_1;
  if (debug_trace != '\0') {
    trace("process_RLD_LIST:  head 0x%lx\n");
    in_v1 = extraout_v1;
  }
  pcVar5 = getenv("_RLDN32_LIST");
  if (pcVar5 == (char *)0x0) {
    pcVar5 = getenv("_RLD_LIST");
  }
  if (pcVar5 == (char *)0x0) {
    auVar17._0_8_ = 0;
    auVar17._8_8_ = unaff_s8;
    return auVar17;
  }
  sVar6 = strlen(pcVar5);
  if (iStack_1c != 2) {
    if (iStack_1c == 0) {
      auVar19 = syssgi(0x5c,0);
      if (auVar19._0_8_ == -1) {
        getreuid(&iStack_30,&iStack_34);
        getregid(&iStack_38,&iStack_3c);
        if (((iStack_34 != iStack_30) ||
            (auVar19._0_8_ = 0, auVar19._8_8_ = extraout_v1_05, iStack_3c != iStack_38)) &&
           (auVar19._0_8_ = 0, auVar19._8_8_ = extraout_v1_05, iStack_30 != 0)) {
          auVar19._0_8_ = 1;
        }
      }
    }
    else {
      auVar19[7] = iStack_1c != 2;
      auVar19._8_8_ = in_v1;
      auVar19._0_7_ = 0;
    }
    in_v1 = auVar19._8_8_;
    if (auVar19._0_8_ != 0) {
      if (debug_map != '\0') {
        debug("processRLD_LIST: _RLD_LIST ignored because of setuid or setgid programs\n");
        in_v1 = extraout_v1_00;
      }
      if (verbose != '\0') {
        trace("_RLD_LIST ignored because of setuid or setgid programs\n");
        in_v1 = extraout_v1_01;
      }
      auVar16._0_8_ = 0;
      auVar16._8_8_ = in_v1;
      return auVar16;
    }
  }
  if (verbose != '\0') {
    trace("_RLDN32_LIST = %s\n",pcVar5);
    in_v1 = extraout_v1_02;
  }
  if (debug_trace != '\0') {
    trace("process_RLD_LIST: _RLD_LIST(_RLD64_LIST/_RLDN32_LIST) = %s\n",pcVar5);
    in_v1 = extraout_v1_03;
  }
  if (*pcVar5 == '\0') {
LAB_0fb776c4:
    auVar15._0_8_ = 0;
    auVar15._8_8_ = in_v1;
    return auVar15;
  }
  iVar8 = -(sVar6 * 4 + 0x13 & 0xfffffff0);
  uVar2 = rld_obj_newstr(pcVar5);
  auVar17 = split(pcVar5,&stack0xffffff30 + iVar8,uVar2,sVar6 + 1,&DAT_0fbdc1c8,0);
  lVar11 = auVar17._0_8_;
  in_v1 = auVar17._8_8_;
  if (lVar11 < 1) goto LAB_0fb776c4;
  if (lVar11 == 1) {
    iVar7 = strcmp(*(char **)(&stack0xffffff30 + iVar8),"DEFAULT");
    if (iVar7 == 0) goto LAB_0fb776c4;
    lVar10 = 0;
LAB_0fb77bb8:
    lVar12 = 0;
    lVar14 = 3;
    foreach_obj(*(undefined4 *)(pObj_Head + 8),&LAB_0fb757b0,0);
  }
  else {
    iVar7 = strcmp(*(char **)(&stack0xffffff30 + iVar8),"DEFAULT");
    if (iVar7 == 0) {
      lVar12 = 1;
      lVar10 = 0;
      lVar14 = 2;
    }
    else {
      iVar7 = strcmp(*(char **)((int)&uStack_d4 + auVar17._4_4_ * 4 + iVar8),"DEFAULT");
      if (iVar7 == 0) {
        lVar14 = 1;
        lVar11 = (longlong)(auVar17._4_4_ + -1);
        lVar10 = 0;
        lVar12 = 0;
      }
      else {
        lVar10 = default_in_middle(&stack0xffffff30 + iVar8,lVar11);
        if (lVar10 == 0) goto LAB_0fb77bb8;
        lVar14 = 4;
        lVar12 = 0;
      }
    }
  }
  if (debug_trace != '\0') {
    trace("process_RLD_LIST: add_list_where = %ld\n",lVar14);
  }
  if (lVar12 < lVar11) {
    puVar9 = (undefined4 *)(&stack0xffffff30 + (int)lVar12 * 4 + iVar8);
    lVar13 = lVar12;
    iVar8 = 0;
    do {
      iVar7 = aiStack_2c[0];
      if ((lVar10 == 0) || (lVar12 != lVar10)) {
        lVar3 = create_object(*puVar9,0);
        iVar7 = iVar8;
        if (lVar14 == 3) {
LAB_0fb77930:
          map_object_into_mem_and_init_object_info(lVar3,0,0,aiStack_2c[0],1,0,0,param_3);
          lStack_80 = lVar3;
        }
        else {
          lStack_80 = foreach_obj(pObj_Head,obj_already_mapped,*puVar9);
          if ((lStack_80 == -1) && (iVar8 != 0)) {
            lVar4 = foreach_obj(iVar8,obj_already_mapped,*puVar9);
            if (lVar4 != -1) goto LAB_0fb7786c;
            lStack_80 = -1;
          }
          if ((lStack_80 == -1) && (aiStack_2c[0] != 0)) {
            lVar4 = foreach_obj(aiStack_2c[0],obj_already_mapped,*puVar9);
            if (lVar4 != -1) goto LAB_0fb7786c;
            lStack_80 = -1;
          }
          if (lStack_80 == -1) goto LAB_0fb77930;
          if ((lVar14 != 1) && ((lVar14 != 4 || (lVar10 <= lVar12)))) goto LAB_0fb7786c;
          if (debug_olist != '\0') {
            print_rld_object_list("process_RLD_LIST LIST_BEGINNING+ before");
          }
          objlist_delete_element(&pObj_Head,lStack_80);
          if (debug_olist != '\0') {
            print_rld_object_list("process_RLD_LIST LIST_BEGINNING+ after");
          }
        }
        pcVar5 = *(char **)((int)lStack_80 + 0x88);
        if (*pcVar5 == '\0') {
          if (lVar12 != lVar13) goto LAB_0fb7797c;
LAB_0fb779c4:
          objlist_add_beginning(aiStack_2c,lStack_80);
          if (debug_olist != '\0') {
            print_rld_object_list("process_RLD_LIST i == start done");
          }
          verify_o_arch_is_ok(pObj_Head,lStack_80);
        }
        else {
          global_library_search_rpath =
               generate_global_lib_search_path_from_obj
                         (pcVar5,global_library_search_rpath,&user_defined_path);
          if (lVar12 == lVar13) goto LAB_0fb779c4;
LAB_0fb7797c:
          objlist_add_end(aiStack_2c,lStack_80);
          add_to_global_cleanup_record(lStack_80);
          if (debug_olist != '\0') {
            print_rld_object_list("process_RLD_LIST i != start done");
          }
          verify_o_arch_is_ok(pObj_Head,lStack_80);
        }
        *(undefined1 *)((int)lStack_80 + 0xc6) = 1;
      }
      else {
        aiStack_2c[0] = 0;
        lVar13 = (longlong)((int)lVar10 + 1);
      }
LAB_0fb7786c:
      lVar12 = (longlong)((int)lVar12 + 1);
      puVar9 = puVar9 + 1;
      iVar8 = iVar7;
    } while (lVar12 < lVar11);
    iVar8 = aiStack_2c[0];
    if (iVar7 == 0) goto LAB_0fb77a6c;
  }
  else {
LAB_0fb77a6c:
    iVar8 = 0;
    iVar7 = aiStack_2c[0];
  }
  if (lVar14 == 1) {
    foreach_obj(*(undefined4 *)(pObj_Head + 8),set_postchksum_bit,0);
    *(undefined4 *)(*(int *)(iVar7 + 0xc) + 8) = *(undefined4 *)(pObj_Head + 8);
    *(undefined4 *)(*(int *)(pObj_Head + 8) + 0xc) = *(undefined4 *)(iVar7 + 0xc);
    *(int *)(iVar7 + 0xc) = pObj_Head;
    *(int *)(pObj_Head + 8) = iVar7;
    if (debug_olist != '\0') {
      print_rld_object_list("RLD_LIST_BEGINNING  after");
    }
    goto LAB_0fb77aa4;
  }
  if (lVar14 == 2) {
    *(int *)(*(int *)(pObj_Head + 0xc) + 8) = iVar7;
    uVar1 = *(undefined4 *)(pObj_Head + 0xc);
    *(undefined4 *)(pObj_Head + 0xc) = *(undefined4 *)(iVar7 + 0xc);
    *(undefined4 *)(iVar7 + 0xc) = uVar1;
    if (debug_olist != '\0') {
      print_rld_object_list("RLD_LIST_END  after");
    }
    goto LAB_0fb77aa4;
  }
  if (lVar14 == 3) {
    *(int *)(pObj_Head + 8) = iVar7;
    *(undefined4 *)(pObj_Head + 0xc) = *(undefined4 *)(iVar7 + 0xc);
    *(int *)(iVar7 + 0xc) = pObj_Head;
    if (debug_olist != '\0') {
      print_rld_object_list("RLD_LIST_REPLACE  after");
    }
    goto LAB_0fb77aa4;
  }
  if (lVar14 != 4) {
    error("process_RLD_LIST:  Bad operation on add_list_where (%ld)\n",lVar14);
    goto LAB_0fb77aa4;
  }
  foreach_obj(*(undefined4 *)(pObj_Head + 8),set_postchksum_bit,0);
  if (*(int *)(pObj_Head + 8) == 0) {
    if (iVar7 != 0) goto LAB_0fb77c50;
    *(undefined4 *)(pObj_Head + 8) = 0;
  }
  else {
    if (iVar7 != 0) {
      *(undefined4 *)(*(int *)(pObj_Head + 8) + 0xc) = *(undefined4 *)(iVar7 + 0xc);
LAB_0fb77c50:
      if (*(int *)(iVar7 + 0xc) == 0) {
        if (iVar7 == 0) {
          *(undefined4 *)(pObj_Head + 8) = 0;
          goto LAB_0fb77c78;
        }
      }
      else {
        *(undefined4 *)(*(int *)(iVar7 + 0xc) + 8) = *(undefined4 *)(pObj_Head + 8);
        if (iVar7 == 0) goto LAB_0fb77c74;
      }
      *(int *)(iVar7 + 0xc) = pObj_Head;
    }
LAB_0fb77c74:
    *(int *)(pObj_Head + 8) = iVar7;
  }
LAB_0fb77c78:
  iVar7 = *(int *)(pObj_Head + 0xc);
  if (iVar7 != 0) {
    *(int *)(iVar7 + 8) = iVar8;
  }
  if (iVar8 != 0) {
    *(undefined4 *)(pObj_Head + 0xc) = *(undefined4 *)(iVar8 + 0xc);
    *(int *)(iVar8 + 0xc) = iVar7;
  }
  if (debug_olist != '\0') {
    print_rld_object_list("RLD_LIST_INSERT  after");
  }
LAB_0fb77aa4:
  if (user_tracking != '\0') {
    track("Quickstart disabled: RLD_LIST called\n");
  }
  all_fully_quickstarted = 0;
  foreach_obj(pObj_Head,print_obj,0);
  auVar18._0_8_ = 1;
  auVar18._8_8_ = extraout_v1_04;
  return auVar18;
}



// === FUN_0fb77df0 @ 0fb77df0 (616 bytes) ===

void FUN_0fb77df0(int param_1,undefined8 param_2)

{
  int iVar2;
  longlong lVar1;
  char *pcVar3;
  uint uVar4;
  int iVar5;
  int *piVar6;
  
  for (; param_1 != 0; param_1 = *(int *)(param_1 + 8)) {
    if (debug_trace != '\0') {
      trace("build_obj_list: cur_obj 0x%lx\n",param_1);
    }
    iVar5 = 0;
    uVar4 = 0;
    if ((*(int *)(param_1 + 0x74) != 0) && (*(int *)(param_1 + 0xb0) != 0)) {
      do {
        iVar2 = FUN_0fb78060(param_1,uVar4,param_2);
        if (iVar2 != 0) {
          piVar6 = (int *)(*(int *)(param_1 + 0x74) + iVar5);
          lVar1 = essentially_identical_object(iVar2,param_1,piVar6);
          if (lVar1 == 0) {
            lVar1 = match_interface_version(iVar2,param_1,piVar6);
            if (lVar1 == 0) {
              if (*(int *)(iVar2 + 0x98) == 0) {
                pcVar3 = "<NULL>";
              }
              else {
                pcVar3 = (char *)(*(int *)(iVar2 + 0x60) + *(int *)(iVar2 + 0x98));
              }
              fatal("object %s from liblist in %s has version \"%s\", which does not match the found object: %s (with version \"%s\")\n"
                    ,*piVar6 + *(int *)(param_1 + 0x60),*(undefined4 *)(param_1 + 0x18),
                    piVar6[3] + *(int *)(param_1 + 0x60),*(undefined4 *)(iVar2 + 0x18),pcVar3);
              pcVar3 = (char *)0x0;
            }
            else if (piVar6[2] == *(int *)(iVar2 + 0x94)) {
              if ((ignore_exact_match == '\0') && ((piVar6[4] & 1U) != 0)) {
                fatal("-exact_version is used when linked: object %s from liblist in %s has timestamp 0x%lx, which does not match the found object: %s (with timestamp 0x%lx)\n"
                      ,*piVar6 + *(int *)(param_1 + 0x60),*(undefined4 *)(param_1 + 0x18),piVar6[1],
                      *(undefined4 *)(iVar2 + 0x18),*(undefined4 *)(iVar2 + 0x90));
              }
              if (verbose != '\0') {
                trace("obj %s %s (liblist tstamp 0x%lx obj tstamp 0x%lx)\n",
                      *(undefined4 *)(iVar2 + 0x18),"TIMESTAMP CHANGED",piVar6[1],
                      *(undefined4 *)(iVar2 + 0x90));
              }
              pcVar3 = "TIMESTAMP CHANGED";
            }
            else {
              if ((ignore_exact_match == '\0') && ((piVar6[4] & 1U) != 0)) {
                fatal("-exact_version is used when linked: object %s from liblist in %s has checksum 0x%lx, which does not match the found object: %s (with checksum 0x%lx)\n"
                      ,*piVar6 + *(int *)(param_1 + 0x60),*(undefined4 *)(param_1 + 0x18),piVar6[2],
                      *(undefined4 *)(iVar2 + 0x18));
              }
              *(undefined1 *)(iVar2 + 0xc6) = 1;
              pcVar3 = "CHECKSUM CHANGED";
              if (verbose != '\0') {
                trace("obj %s %s (liblist cksum 0x%lx obj cksm 0x%lx)\n",
                      *(undefined4 *)(iVar2 + 0x18),"CHECKSUM CHANGED",piVar6[2],
                      *(undefined4 *)(iVar2 + 0x94));
                pcVar3 = "CHECKSUM CHANGED";
              }
            }
            if (quickstart_info != '\0') {
              track("%s %s: %s from that expected by %s\n","Quickstart failed for object",
                    *(undefined4 *)(iVar2 + 0x18),pcVar3,*(undefined4 *)(param_1 + 0x18));
            }
            all_fully_quickstarted = 0;
          }
        }
        iVar5 = iVar5 + 0x14;
        uVar4 = uVar4 + 1;
      } while ((*(int *)(param_1 + 0x74) != 0) && (uVar4 < *(uint *)(param_1 + 0xb0)));
    }
  }
  return;
}



// === FUN_0fb78060 @ 0fb78060 (516 bytes) ===

longlong FUN_0fb78060(int param_1,undefined8 param_2,undefined8 param_3)

{
  uint uVar1;
  longlong lVar2;
  longlong lVar3;
  int iVar4;
  int iVar5;
  int *piVar6;
  
  piVar6 = (int *)(*(int *)(param_1 + 0x74) + (int)param_2 * 0x14);
  uVar1 = piVar6[4];
  iVar4 = *piVar6 + *(int *)(param_1 + 0x60);
  if ((_rld_do_delay_loaded_libs_instantly == '\0') && ((uVar1 & 0x10) != 0)) {
    lVar2 = 0;
  }
  else {
    if (debug_trace != '\0') {
      trace("build_obj_list_by_name: name %s, head 0x%lx\n",iVar4,param_1);
    }
    lVar2 = FUN_0fb75850(param_1,param_2);
    iVar5 = (int)param_2 * 4;
    if (lVar2 != 0) {
      *(int *)(*(int *)(param_1 + 0xb4) + iVar5) = (int)lVar2;
      return lVar2;
    }
    lVar2 = create_object(iVar4,0);
    map_object_into_mem_and_init_object_info
              (lVar2,param_1,param_2,pObj_Head,1,0,uVar1 & 0x10,param_3);
    iVar4 = (int)lVar2;
    if (**(char **)(iVar4 + 0x88) != '\0') {
      global_library_search_rpath =
           generate_global_lib_search_path_from_obj
                     (*(char **)(iVar4 + 0x88),global_library_search_rpath,&user_defined_path);
    }
    lVar3 = locate_obj_with_soname(*(undefined4 *)(iVar4 + 0x30));
    if (lVar3 != -1) {
      if (verbose != '\0') {
        trace("Unmapped %s since a previously mapped shared object %s has the same SONAME %s\n",
              *(undefined4 *)(iVar4 + 0x18),*(undefined4 *)((int)lVar3 + 0x18),
              *(undefined4 *)(iVar4 + 0x30));
      }
      lVar2 = elfunmap(lVar2);
      if (lVar2 == -1) {
        error("elfunmap -- can\'t unmap text of %s ",*(undefined4 *)(iVar4 + 0x18));
      }
      *(int *)(*(int *)(param_1 + 0xb4) + iVar5) = (int)lVar3;
      return lVar3;
    }
    stat_num_lib_mapped = stat_num_lib_mapped + 1;
    objlist_add_end(&pObj_Head,lVar2);
    add_to_global_cleanup_record(lVar2);
    if (debug_olist != '\0') {
      print_rld_object_list("build_obj_list_by_liblist_index add at LIST_END done");
    }
    verify_o_arch_is_ok(pObj_Head,lVar2);
    *(int *)(*(int *)(param_1 + 0xb4) + iVar5) = iVar4;
  }
  return lVar2;
}



// === FUN_0fb783c0 @ 0fb783c0 (176 bytes) ===

int FUN_0fb783c0(undefined8 param_1,int param_2,undefined4 *param_3)

{
  int iVar1;
  
  if ((debug_map != '\0') &&
     (debug("Region list: %s num of segments %d\n",param_1,param_2), debug_map != '\0')) {
    debug("index vaddr length type\n");
  }
  if (0 < param_2) {
    iVar1 = 0;
    do {
      if (debug_map != '\0') {
        debug("[%d] priority %d addr 0x%lx sz 0x%lx is up? %d \n",iVar1,*(undefined2 *)(param_3 + 2)
              ,*param_3,param_3[1],*(undefined1 *)((int)param_3 + 10));
      }
      iVar1 = iVar1 + 1;
      param_3 = param_3 + 3;
    } while (param_2 != iVar1);
  }
  return param_2;
}



// === FUN_0fb78470 @ 0fb78470 (244 bytes) ===

/* WARNING: Instruction at (ram,0x0fb784bc) overlaps instruction at (ram,0x0fb784b8)
    */

void FUN_0fb78470(uint param_1,int param_2,int param_3,ulonglong param_4)

{
  uint *puVar1;
  
  puVar1 = (uint *)(DAT_0fbd39c0 + param_3 * 0xc);
  if (((param_3 < 0) || (DAT_0fbd39bc <= param_3)) && (debug_map != '\0')) {
    debug("Priority range ERROR, %d vs %d, addr 0x%lx\n",param_3,DAT_0fbd39bc,param_1);
  }
  if ((param_1 < *puVar1) && (debug_map != '\0')) {
    debug("Priority begin ERROR priority %d addr 0x%lx begin 0x%lx\n",param_3,param_1);
  }
  if ((puVar1[1] < param_1 + param_2) && (debug_map != '\0')) {
    debug("Priority end ERROR priority %d addr 0x%lx begin 0x%lx\n",param_3);
  }
  if ((param_4 != *(byte *)(puVar1 + 2)) && (debug_map != '\0')) {
    debug("Priority pref ERROR priority %d  %d vs %d addr 0x%lx \n",param_3,param_4,
          (ulonglong)*(byte *)(puVar1 + 2),param_1);
  }
  return;
}



// === FUN_0fb78570 @ 0fb78570 (408 bytes) ===

void FUN_0fb78570(undefined8 param_1,undefined8 param_2)

{
  uint uVar1;
  uint uVar2;
  uint *puVar3;
  int iVar4;
  char cVar5;
  uint *puVar6;
  uint *puVar7;
  int iVar8;
  
  iVar4 = DAT_0fbd39cc;
  if (1 < DAT_0fbd39cc) {
    iVar8 = 1;
    puVar6 = DAT_0fbd39c8;
    puVar3 = DAT_0fbd39c8;
    do {
      puVar7 = puVar3 + 3;
      FUN_0fb78470(*puVar7,puVar3[4],*(undefined2 *)(puVar3 + 5),*(undefined1 *)((int)puVar3 + 0x16)
                  );
      FUN_0fb78470(*puVar6,puVar6[1],*(undefined2 *)(puVar6 + 2),*(undefined1 *)((int)puVar6 + 10));
      if (*(short *)(puVar3 + 5) <= *(short *)(puVar6 + 2)) {
        if ((*(short *)(puVar3 + 5) < *(short *)(puVar6 + 2)) && (debug_map != '\0')) {
          debug("rld ERROR Invalid priority %s %d [%d] %d %d\n",param_1,param_2,iVar8);
        }
        cVar5 = *(char *)((int)puVar3 + 0x16);
        if ((cVar5 != *(char *)((int)puVar6 + 10)) && (debug_map != '\0')) {
          debug("rld ERROR Invalid is_up flag %s %d [%d] %d %d\n",param_1,param_2,iVar8,
                *(char *)((int)puVar6 + 10),cVar5);
          cVar5 = *(char *)((int)puVar3 + 0x16);
        }
        uVar1 = *puVar6;
        uVar2 = *puVar7;
        if (cVar5 == '\0') {
          if ((uVar1 < uVar2 + puVar3[4]) && (debug_map != '\0')) {
            debug("rld ERROR Invalid addrs bad, %s %d [%d] is_up %d  0x%lx 0x%lx,  0x%lx 0x%lx\n",
                  param_1,param_2,iVar8,*(undefined1 *)((int)puVar6 + 10),uVar1,puVar6[1],uVar2,
                  puVar3[4]);
          }
        }
        else if ((uVar2 < uVar1 + puVar6[1]) && (debug_map != '\0')) {
          debug("rld ERROR Invalid addrs bad. %s %d [%d] is_up %d  0x%lx 0x%lx,  0x%lx 0x%lx\n",
                param_1,param_2,iVar8,*(undefined1 *)((int)puVar6 + 10));
        }
      }
      iVar8 = iVar8 + 1;
      puVar6 = puVar6 + 3;
      puVar3 = puVar7;
    } while (iVar8 < iVar4);
  }
  return;
}



// === __rld_page_load_object @ 0fb78710 (1028 bytes) ===

longlong __rld_page_load_object(undefined8 param_1,int param_2,uint param_3,int param_4)

{
  longlong lVar1;
  longlong lVar2;
  undefined1 *puVar3;
  undefined *puVar4;
  char *pcVar5;
  uint uVar6;
  int iVar7;
  uint uVar8;
  int iVar9;
  int iVar10;
  uint uVar11;
  int aiStack_70 [5];
  undefined1 uStack_5c;
  undefined2 uStack_52;
  char acStack_50 [8];
  undefined8 uStack_48;
  
  if ((int)param_3 < 9) {
    if ((int)param_3 < 1) {
      uVar8 = 0;
      uVar6 = 0;
    }
    else {
      uVar6 = 0;
      uVar8 = 0;
      iVar10 = param_2;
      if ((param_3 & 1) != 0) {
        uVar11 = *(int *)(param_2 + 8) + *(int *)(param_2 + 0x14);
        if (uVar11 != 0) {
          uVar8 = uVar11;
        }
        if (*(uint *)(param_2 + 0x1c) != 0) {
          uVar6 = *(uint *)(param_2 + 0x1c);
        }
        iVar10 = param_2 + 0x20;
      }
      if ((int)param_3 >> 1 != 0) {
        iVar7 = *(int *)(iVar10 + 0x14);
        while( true ) {
          uVar11 = *(int *)(iVar10 + 8) + iVar7;
          if (uVar8 < uVar11) {
            uVar8 = uVar11;
          }
          if (uVar6 < *(uint *)(iVar10 + 0x1c)) {
            uVar6 = *(uint *)(iVar10 + 0x1c);
          }
          uVar11 = *(int *)(iVar10 + 0x28) + *(int *)(iVar10 + 0x34);
          if (uVar8 < uVar11) {
            uVar8 = uVar11;
          }
          if (uVar6 < *(uint *)(iVar10 + 0x3c)) {
            uVar6 = *(uint *)(iVar10 + 0x3c);
          }
          if (iVar10 + 0x40 == param_3 * 0x20 + param_2) break;
          iVar7 = *(int *)(iVar10 + 0x54);
          iVar10 = iVar10 + 0x40;
        }
      }
    }
    uStack_48 = param_1;
    FUN_0fb78b20(aiStack_70,uVar8 - *(int *)(param_2 + 8),uVar6);
    lVar1 = FUN_0fb78b70(param_3,aiStack_70);
    if (lVar1 < 0) {
      if (debug_map != '\0') {
        debug("Cannot map big page region in __rld_page_load_object ");
      }
      return -1;
    }
    puVar4 = *(undefined **)(param_4 + 0x18);
    if (puVar4 == (undefined *)0x0) {
      puVar4 = &DAT_0fbdca28;
    }
    if (debug_map != '\0') {
      debug("Now load in DSO %s\n",puVar4);
    }
    lVar1 = FUN_0fb79cf0(uStack_48,param_2,param_3,aiStack_70);
    if (lVar1 == -1) {
      if (debug_map != '\0') {
        debug("__rld_load_into_memory failed\n");
      }
      return -1;
    }
    lVar2 = FUN_0fb7a060(param_3,param_2,aiStack_70);
    if (lVar2 < 0) {
      if (debug_map != '\0') {
        debug("big page free space reservation fails \n");
      }
      return -1;
    }
    acStack_50[0] = '\0';
    iVar10 = aiStack_70[0] - *(int *)(param_2 + 8);
    puVar3 = (undefined1 *)_rld_alloc_obj_space(param_4,param_3 * 0xc + 4,4,acStack_50);
    if (puVar3 == (undefined1 *)0x0) {
      if (debug_map != '\0') {
        debug("o_mmap_big_page_data alloc fails\n");
      }
      lVar1 = -1;
    }
    else {
      *(undefined1 **)(param_4 + 0x124) = puVar3;
      *puVar3 = (char)param_3;
      if (0 < (int)param_3) {
        iVar7 = 0;
        for (uVar8 = param_3 & 3; uVar8 != 0; uVar8 = uVar8 - 1) {
          iVar9 = *(int *)(param_4 + 0x124) + iVar7;
          *(int *)(iVar9 + 4) = *(int *)(param_2 + 8) + iVar10;
          *(undefined4 *)(iVar9 + 8) = *(undefined4 *)(param_2 + 0x14);
          *(undefined1 *)(iVar9 + 0xe) = uStack_5c;
          param_2 = param_2 + 0x20;
          iVar7 = iVar7 + 0xc;
          *(undefined2 *)(iVar9 + 0xc) = uStack_52;
        }
        if ((int)param_3 >> 2 != 0) {
          do {
            iVar9 = *(int *)(param_4 + 0x124) + iVar7;
            *(int *)(iVar9 + 4) = *(int *)(param_2 + 8) + iVar10;
            *(undefined4 *)(iVar9 + 8) = *(undefined4 *)(param_2 + 0x14);
            *(undefined1 *)(iVar9 + 0xe) = uStack_5c;
            *(undefined2 *)(iVar9 + 0xc) = uStack_52;
            iVar9 = *(int *)(param_4 + 0x124) + iVar7 + 0xc;
            *(int *)(iVar9 + 4) = *(int *)(param_2 + 0x28) + iVar10;
            *(undefined4 *)(iVar9 + 8) = *(undefined4 *)(param_2 + 0x34);
            *(undefined1 *)(iVar9 + 0xe) = uStack_5c;
            *(undefined2 *)(iVar9 + 0xc) = uStack_52;
            iVar9 = *(int *)(param_4 + 0x124) + iVar7 + 0x18;
            *(int *)(iVar9 + 4) = *(int *)(param_2 + 0x48) + iVar10;
            *(undefined4 *)(iVar9 + 8) = *(undefined4 *)(param_2 + 0x54);
            *(undefined1 *)(iVar9 + 0xe) = uStack_5c;
            *(undefined2 *)(iVar9 + 0xc) = uStack_52;
            iVar9 = *(int *)(param_4 + 0x124) + iVar7 + 0x24;
            *(int *)(iVar9 + 4) = *(int *)(param_2 + 0x68) + iVar10;
            *(undefined4 *)(iVar9 + 8) = *(undefined4 *)(param_2 + 0x74);
            iVar7 = iVar7 + 0x30;
            *(undefined1 *)(iVar9 + 0xe) = uStack_5c;
            param_2 = param_2 + 0x80;
            *(undefined2 *)(iVar9 + 0xc) = uStack_52;
          } while (iVar7 != param_3 * 0xc);
        }
      }
      *(char *)(param_4 + 0xe0) = acStack_50[0];
      if (debug_map != '\0') {
        pcVar5 = "malloced";
        if (acStack_50[0] == '\0') {
          pcVar5 = "allocated in obj record";
        }
        debug("o_mmap_big_page_data is %s\n",pcVar5);
      }
      *(uint *)(param_4 + 0x28) = *(uint *)(param_4 + 0x28) | 1;
    }
  }
  else {
    if (debug_map != '\0') {
      debug("Cannot big page load %d PT_LOAD segments\n",param_3);
    }
    lVar1 = -1;
  }
  return lVar1;
}



// === FUN_0fb78b20 @ 0fb78b20 (80 bytes) ===

void FUN_0fb78b20(undefined4 *param_1,int param_2,int param_3)

{
  uint uVar1;
  
  if (DAT_0fbd39c4 != '\0') {
    param_2 = param_2 + 0x7000;
  }
  param_1[7] = 0xffffffff;
  param_1[6] = 0xffffffff;
  param_1[2] = param_3;
  *param_1 = 0;
  *(undefined1 *)(param_1 + 5) = 1;
  param_1[3] = 0;
  uVar1 = ~(param_3 - 1U) & (param_3 + param_2) - 1U;
  param_1[4] = uVar1;
  param_1[1] = uVar1;
  return;
}



// === FUN_0fb78b70 @ 0fb78b70 (184 bytes) ===

void FUN_0fb78b70(undefined8 param_1,int param_2)

{
  longlong lVar1;
  code *UNRECOVERED_JUMPTABLE_00;
  
  lVar1 = FUN_0fb78d50(param_2);
  if (-1 < lVar1) {
                    /* WARNING: Could not recover jumptable at 0x0fb78b98. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*UNRECOVERED_JUMPTABLE_00)();
    return;
  }
  lVar1 = FUN_0fb78ff0(param_1,param_2);
  if (lVar1 < 0) {
                    /* WARNING: Could not recover jumptable at 0x0fb78c10. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*UNRECOVERED_JUMPTABLE_00)();
    return;
  }
  lVar1 = FUN_0fb7a4d0(*(undefined4 *)(param_2 + 0xc),*(undefined4 *)(param_2 + 0x10),
                       *(undefined1 *)(param_2 + 0x14),*(undefined4 *)(param_2 + 0x1c));
  if (lVar1 < 0) {
                    /* WARNING: Could not recover jumptable at 0x0fb78c20. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*UNRECOVERED_JUMPTABLE_00)();
    return;
  }
  lVar1 = FUN_0fb78c30(param_1,param_2);
  if (lVar1 < 0) {
                    /* WARNING: Could not recover jumptable at 0x0fb78bf0. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*UNRECOVERED_JUMPTABLE_00)();
    return;
  }
  FUN_0fb78d50(param_2);
  return;
}



// === FUN_0fb78c30 @ 0fb78c30 (280 bytes) ===

void FUN_0fb78c30(undefined8 param_1,int param_2)

{
  size_t __len;
  void *__addr;
  int __fd;
  void *pvVar1;
  code *UNRECOVERED_JUMPTABLE;
  
  __len = *(size_t *)(param_2 + 0x10);
  __addr = *(void **)(param_2 + 0xc);
  __fd = open64("/dev/zero",0,0);
  if (__fd < 0) {
    if (debug_map != '\0') {
      debug("Unable to open /dev/zero to mmap \n");
    }
                    /* WARNING: Could not recover jumptable at 0x0fb78d40. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*UNRECOVERED_JUMPTABLE)();
    return;
  }
  if (debug_map != '\0') {
    debug("big page mmap64 at 0x%lx, end 0x%lx size 0x%lx, flags 0x%x\n",__addr,(int)__addr + __len,
          __len,7);
  }
  pvVar1 = mmap64(__addr,__len,7,0x12,__fd,0);
  close(__fd);
  if (pvVar1 == (void *)0xffffffff) {
    if (debug_map != '\0') {
      debug("mmap64 for dso at 0x%lx size 0x%lx failed, errno %d\n",__addr,__len,errno);
    }
                    /* WARNING: Could not recover jumptable at 0x0fb78cfc. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*UNRECOVERED_JUMPTABLE)();
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x0fb78d10. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
}



// === FUN_0fb78d50 @ 0fb78d50 (656 bytes) ===

undefined8 FUN_0fb78d50(uint *param_1)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  undefined8 uVar4;
  uint uVar5;
  uint uVar6;
  int *piVar7;
  int iVar8;
  uint uVar9;
  uint uVar10;
  uint uVar11;
  
  uVar3 = DAT_0fbd39cc;
  uVar11 = param_1[2];
  uVar1 = param_1[1];
  if ((debug_map != '\0') &&
     (debug("find existing region entry, load need size 0x%lx\n",uVar1), debug_map != '\0')) {
    uVar4 = FUN_0fb783c0("regions before finding",uVar3,DAT_0fbd39c8);
    debug("Find region from %d regions\n",uVar4);
  }
  uVar6 = 0;
  uVar9 = ~(uVar11 - 1);
  if (0 < (int)uVar3) {
    iVar8 = 0;
    do {
      piVar7 = (int *)(DAT_0fbd39c8 + iVar8);
      if (debug_map != '\0') {
        debug("avail[%d]  priority %d vaddr 0x%lx  len 0x%lx phdr space needed 0x%lx \n",uVar6,
              *(undefined2 *)(piVar7 + 2),*piVar7,piVar7[1],uVar1);
      }
      uVar2 = piVar7[1];
      if (uVar1 <= uVar2) {
        uVar10 = uVar9 & (*piVar7 + uVar11) - 1;
        uVar5 = uVar10 - *piVar7;
        if (uVar5 == 0) {
LAB_0fb78ed0:
          if (debug_map != '\0') {
            debug("found a segment prelim 0x%lx len 0x%lx i %d \n",*piVar7,piVar7[1],uVar6);
          }
          uVar11 = uVar10;
          if (*(char *)((int)piVar7 + 10) == '\0') {
            uVar9 = uVar9 & (*piVar7 + piVar7[1]) - uVar1;
            if (debug_map != '\0') {
              debug("Segment map addr 0x%lx take upper addr 0x%lx\n",uVar10,uVar9);
            }
            if (uVar9 < uVar10) {
              if (debug_map != '\0') {
                debug("rld ERROR: Segment map addr Impossible!\n");
              }
            }
            else if ((uVar10 < uVar9) && (uVar11 = uVar9, debug_map != '\0')) {
              debug("Segment map addr moved up to 0x%lx  newaddr \n",uVar10);
            }
          }
          if (debug_map != '\0') {
            debug("found a segment using 0x%lx len 0x%lx i %d \n",uVar11,piVar7[1],uVar6);
          }
          param_1[6] = uVar6;
          *param_1 = uVar11;
          *(undefined1 *)(param_1 + 5) = *(undefined1 *)((int)piVar7 + 10);
          param_1[7] = (int)*(short *)(piVar7 + 2);
          return 1;
        }
        if (uVar5 < uVar2) {
          if (debug_map != '\0') {
            debug("alignment issue, take care of it. have 0x%lx need 0x%lx\n",uVar2 - uVar5,uVar1);
          }
          if (uVar1 <= uVar2 - uVar5) goto LAB_0fb78ed0;
        }
        else if (debug_map != '\0') {
          debug("alignment issue, unfixable  length 0x%lx align_increment 0x%lx\n");
        }
      }
      uVar6 = uVar6 + 1;
      iVar8 = iVar8 + 0xc;
    } while (uVar3 != uVar6);
  }
  if (debug_map != '\0') {
    debug("Could not find available existing region \n");
  }
  return 0xffffffffffffffff;
}



// === FUN_0fb78ff0 @ 0fb78ff0 (520 bytes) ===

void FUN_0fb78ff0(undefined8 param_1,int param_2)

{
  uint uVar1;
  uint uVar2;
  longlong lVar3;
  int iVar4;
  int iVar5;
  uint uVar6;
  uint *puVar7;
  code *UNRECOVERED_JUMPTABLE;
  undefined4 auStack_40 [2];
  
  uVar2 = DAT_0fbd39ac;
  auStack_40[0] = 0;
  if (debug_map != '\0') {
    debug("__rld_create_new_region numloadseg %d\n",param_1);
  }
  lVar3 = FUN_0fb79890();
  if (-1 < lVar3) {
    *(uint *)(param_2 + 0x10) = uVar2;
    if (debug_map != '\0') {
      debug("df_mmap_length = %lx\n",uVar2);
    }
    uVar1 = *(uint *)(param_2 + 4);
    if (uVar2 < uVar1) {
      uVar6 = uVar1 / uVar2;
      if (uVar2 == 0) {
        trap(7);
      }
      if (debug_map != '\0') {
        debug("t = %ld\n",uVar6);
        uVar1 = *(uint *)(param_2 + 4);
        if (uVar2 == 0) {
          trap(7);
        }
      }
      if ((uVar1 % uVar2 != 0) && (uVar6 = uVar6 + 1, debug_map != '\0')) {
        debug("t = %ld\n",uVar6);
      }
      *(uint *)(param_2 + 0x10) = uVar2 * uVar6;
      if (debug_map != '\0') {
        debug("df_mmap_length = %lx\n",uVar2 * uVar6);
      }
    }
    auStack_40[0] = 0;
    iVar4 = 0;
    if (0 < DAT_0fbd39bc) {
      iVar5 = 0;
      puVar7 = DAT_0fbd39c0;
      do {
        if ((0 < iVar4) && (*puVar7 < puVar7[-3])) {
          auStack_40[0] = 0;
        }
        if (debug_map != '\0') {
          debug("Check for area. pref index %d, active region index %d\n",iVar4,auStack_40[0]);
        }
        lVar3 = FUN_0fb79200(iVar4,puVar7,param_2,auStack_40);
        if (lVar3 != 0) {
                    /* WARNING: Could not recover jumptable at 0x0fb79168. Too many branches */
                    /* WARNING: Treating indirect jump as call */
          (*UNRECOVERED_JUMPTABLE)();
          return;
        }
        iVar4 = iVar4 + 1;
        iVar5 = iVar5 + 0xc;
        puVar7 = (uint *)((int)DAT_0fbd39c0 + iVar5);
      } while (iVar4 < DAT_0fbd39bc);
    }
    if (debug_map != '\0') {
      debug("Unable to find available space in memory map for rld load\n");
    }
                    /* WARNING: Could not recover jumptable at 0x0fb791bc. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*UNRECOVERED_JUMPTABLE)();
    return;
  }
  if (debug_map != '\0') {
    debug(" Could not set up active regions info\n");
  }
                    /* WARNING: Could not recover jumptable at 0x0fb791f0. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
}



// === FUN_0fb79200 @ 0fb79200 (796 bytes) ===

undefined8 FUN_0fb79200(undefined4 param_1,uint *param_2,int param_3,int *param_4)

{
  char cVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  uint *puVar6;
  uint *puVar7;
  int iVar8;
  uint uVar9;
  uint uVar10;
  int iVar11;
  
  cVar1 = *(char *)(param_2 + 2);
  uVar2 = param_2[1];
  uVar3 = *param_2;
  iVar8 = *param_4;
  puVar6 = (uint *)(DAT_0fbd39b8 + iVar8 * 0xc);
  if (debug_map != '\0') {
    debug("Find unused. Pref rec 0x%lx thru 0x%lx, is up? %d  seg in use index %d\n",uVar3,uVar2,
          cVar1,iVar8);
  }
  iVar8 = iVar8 + 1;
  if (iVar8 < DAT_0fbd39b0) {
    iVar11 = 0;
    puVar7 = puVar6;
    do {
      puVar7 = puVar7 + 3;
      uVar9 = *puVar6;
      if (uVar2 <= uVar9) {
        if (debug_map != '\0') {
          debug("Nothing more to search, [%d]  0x%lx >= 0x%lx\n",iVar8,uVar9,uVar2);
        }
        break;
      }
      uVar4 = *puVar7;
      if (uVar4 < uVar3) {
        if (debug_map != '\0') {
          debug("Gap too low 0x%lx < 0x%lx\n",uVar4,uVar3);
        }
      }
      else {
        uVar5 = puVar6[1] + uVar9;
        if (puVar6[1] + uVar9 <= uVar3) {
          uVar5 = uVar3;
        }
        if (uVar2 <= uVar4) {
          uVar4 = uVar2;
        }
        if (uVar5 < uVar4) {
          uVar9 = __rld_loading_data;
          if (__rld_loading_data < *(uint *)(param_3 + 8)) {
            uVar9 = *(uint *)(param_3 + 8);
          }
          uVar10 = ~(uVar9 - 1) & (uVar5 + uVar9) - 1;
          uVar9 = ~(uVar9 - 1) & uVar4;
          if (uVar10 < uVar9) {
            if (uVar9 - uVar10 < *(uint *)(param_3 + 0x10)) {
              if (debug_map != '\0') {
                debug("length ck: 0x%lx 0x%lx\n",uVar9 - uVar10,*(uint *)(param_3 + 0x10));
              }
            }
            else {
              if ((debug_map != '\0') &&
                 (debug("Found space to mmap usable, 0x%lx to 0x%lx avail index %d\n",uVar5,uVar4,
                        iVar8), debug_map != '\0')) {
                debug("Found space to mmap aligned, 0x%lx to 0x%lx avail index %d\n",uVar10,uVar9,
                      iVar8);
              }
              if (cVar1 != '\0') {
                *(uint *)(param_3 + 0xc) = uVar10;
                *(undefined4 *)(param_3 + 0x1c) = param_1;
                *(char *)(param_3 + 0x14) = cVar1;
                if (debug_map != '\0') {
                  debug("Return region. Looking up\n");
                }
                return 1;
              }
              iVar11 = uVar9 - *(int *)(param_3 + 0x10);
            }
          }
          else if (debug_map != '\0') {
            debug("aligned_begin >= aligned_end 0x%lx 0x%lx, aligned amt 0x%lx\n",uVar10,uVar9);
          }
        }
        else if (debug_map != '\0') {
          debug("usable_begin >= usable_end 0x%lx 0x%lx\n");
        }
      }
      iVar8 = iVar8 + 1;
      puVar6 = puVar6 + 3;
    } while (iVar8 < DAT_0fbd39b0);
    if (cVar1 != '\0') goto LAB_0fb79498;
    if (iVar11 != 0) {
      *(undefined1 *)(param_3 + 0x14) = 0;
      *(int *)(param_3 + 0xc) = iVar11;
      *(undefined4 *)(param_3 + 0x1c) = param_1;
      if (debug_map != '\0') {
        debug("Return region. Looking down\n");
      }
      return 1;
    }
  }
  else if (cVar1 != '\0') {
LAB_0fb79498:
    *param_4 = iVar8 + -1;
    return 0;
  }
  *param_4 = iVar8 + -1;
  return 0;
}



// === FUN_0fb79530 @ 0fb79530 (228 bytes) ===

undefined8 FUN_0fb79530(int param_1)

{
  longlong lVar1;
  int iVar2;
  
  if (param_1 <= DAT_0fbd39b4) {
    return 1;
  }
  if (DAT_0fbd39b8 != 0) {
    free();
    DAT_0fbd39b0 = 0;
    DAT_0fbd39b8 = 0;
  }
  if (DAT_0fbd39b4 == 0) {
    iVar2 = 0x55;
    if (param_1 < 0x56) goto LAB_0fb79590;
  }
  else {
    iVar2 = DAT_0fbd39b4 * 2;
    if (param_1 <= iVar2) goto LAB_0fb79590;
  }
  iVar2 = param_1 + 1;
LAB_0fb79590:
  if (debug_map != '\0') {
    debug("Allocated %d segments for map info\n",iVar2);
  }
  lVar1 = malloc(iVar2 * 0xc);
  if (lVar1 != 0) {
    DAT_0fbd39b8 = (int)lVar1;
    DAT_0fbd39b4 = iVar2;
    return 1;
  }
  DAT_0fbd39b4 = 0;
  if (debug_map != '\0') {
    debug("No memory for segment map!\n");
  }
  return 0xffffffffffffffff;
}



// === FUN_0fb79620 @ 0fb79620 (288 bytes) ===

int FUN_0fb79620(undefined4 *param_1,uint param_2)

{
  int iVar1;
  undefined4 *puVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  
  iVar4 = 0;
  uVar3 = 0;
  if ((int)param_2 < 1) {
    iVar5 = 0;
  }
  else {
    iVar4 = 0;
    iVar5 = 0;
    puVar2 = param_1;
    if ((param_2 & 1) != 0) {
      uVar3 = 1;
      puVar2 = param_1 + 3;
      if (param_1[1] == 0) {
        iVar4 = 1;
      }
      else {
        iVar5 = 1;
        param_1 = param_1 + 3;
      }
    }
    if ((int)param_2 >> 1 != 0) {
      iVar1 = puVar2[1];
      while( true ) {
        uVar3 = uVar3 + 2;
        if (iVar1 == 0) {
          iVar4 = iVar4 + 1;
        }
        else {
          if (param_1 != puVar2) {
            *param_1 = *puVar2;
            param_1[1] = puVar2[1];
            *(undefined2 *)(param_1 + 2) = *(undefined2 *)(puVar2 + 2);
            *(undefined1 *)((int)param_1 + 10) = *(undefined1 *)((int)puVar2 + 10);
          }
          iVar5 = iVar5 + 1;
          param_1 = param_1 + 3;
        }
        if (puVar2[4] == 0) {
          iVar4 = iVar4 + 1;
        }
        else {
          if (param_1 != puVar2 + 3) {
            *param_1 = puVar2[3];
            param_1[1] = puVar2[4];
            *(undefined2 *)(param_1 + 2) = *(undefined2 *)(puVar2 + 5);
            *(undefined1 *)((int)param_1 + 10) = *(undefined1 *)((int)puVar2 + 0x16);
          }
          iVar5 = iVar5 + 1;
          param_1 = param_1 + 3;
        }
        if (param_2 == uVar3) break;
        iVar1 = puVar2[7];
        puVar2 = puVar2 + 6;
      }
    }
  }
  if (debug_map != '\0') {
    debug("Squeezed out %d 0 length entries\n",iVar4);
  }
  return iVar5;
}



// === FUN_0fb79740 @ 0fb79740 (320 bytes) ===

int FUN_0fb79740(uint *param_1,int param_2)

{
  uint uVar1;
  uint uVar2;
  uint *puVar3;
  uint uVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  int iVar8;
  
  if (1 < param_2) {
    iVar6 = 0;
    iVar5 = 1;
    iVar8 = 0;
    do {
      puVar3 = param_1 + 3;
      uVar1 = *param_1;
      uVar2 = *puVar3;
      uVar4 = param_1[1] + uVar1;
      if (uVar2 <= uVar4) {
        uVar7 = param_1[4] + uVar2;
        if (uVar2 == uVar4) {
          if (verbose != '\0') {
            trace("Region list entrymerge %d 0x%lx-0x%lx  %d 0x%lx-0x%lx\n",iVar6,uVar1,uVar4,iVar5,
                  uVar2,uVar7);
            uVar2 = *puVar3;
            uVar1 = *param_1;
          }
        }
        else if (verbose != '\0') {
          trace("Region list overlap    %d 0x%lx-0x%lx  %d 0x%lx-0x%lx\n",iVar6,uVar1,uVar4,iVar5,
                uVar2,uVar7);
          uVar2 = *puVar3;
          uVar1 = *param_1;
        }
        iVar8 = iVar8 + 1;
        if (uVar2 <= uVar1) {
          uVar1 = uVar2;
        }
        *puVar3 = uVar1;
        param_1[1] = 0;
        *param_1 = uVar1;
        if (uVar7 < uVar4) {
          uVar7 = uVar4;
        }
        param_1[4] = uVar7 - uVar1;
      }
      iVar5 = iVar5 + 1;
      iVar6 = iVar6 + 1;
      param_1 = puVar3;
    } while (param_2 != iVar5);
    return iVar8;
  }
  return 0;
}



// === FUN_0fb79890 @ 0fb79890 (1096 bytes) ===

void FUN_0fb79890(void)

{
  undefined4 uVar1;
  bool bVar2;
  undefined4 *__base;
  __pid_t _Var5;
  int __fd;
  uint *puVar6;
  int iVar7;
  longlong lVar3;
  undefined4 *puVar8;
  undefined8 uVar4;
  undefined4 *puVar9;
  uint uVar10;
  uint uVar11;
  int iVar12;
  int *piVar13;
  undefined8 in_ra;
  uint *puStack_c0;
  int iStack_bc;
  char acStack_b8 [104];
  ulonglong uStack_50;
  longlong lStack_48;
  undefined8 UNRECOVERED_JUMPTABLE_00;
  
  UNRECOVERED_JUMPTABLE_00 = in_ra;
  _Var5 = getpid();
  iStack_bc = 0;
  puStack_c0 = (uint *)0x0;
  if (debug_map != '\0') {
    debug("__rld_get_active_regions entry\n");
  }
  rld_snprintf(acStack_b8,100,"/proc/%05d",_Var5);
  __fd = open64(acStack_b8,0);
  uVar11 = DAT_0fbde3e8;
  if (__fd < 0) {
    if (debug_map != '\0') {
      debug("FAIL open /proc %d\n",errno);
    }
                    /* WARNING: Could not recover jumptable at 0x0fb79b18. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*UNRECOVERED_JUMPTABLE_00._4_4_)();
    return;
  }
  while( true ) {
    if (puStack_c0 != (uint *)0x0) {
      if (debug_map != '\0') {
        debug("Looping on PIOCMAP_SGI. free %ld bytes\n",iStack_bc);
      }
      free(puStack_c0);
    }
    iStack_bc = uVar11 * 0x50;
    puVar6 = (uint *)malloc();
    if (puVar6 == (uint *)0x0) {
      if (verbose != '\0') {
        trace("Unable to malloc %ld bytes of space for big page map\n",iStack_bc);
      }
      goto LAB_0fb79a4c;
    }
    DAT_0fbde3e8 = uVar11;
    if (debug_map != '\0') {
      debug("Allocate space for %ld regions, %ld bytes\n",uVar11,iStack_bc);
    }
    puStack_c0 = puVar6;
    iVar7 = ioctl(__fd,0x71f9,&puStack_c0);
    iVar12 = 0;
    if (iVar7 < 0) {
      if (verbose != '\0') {
        trace("FAIL PIOCMAP_SGI errno %d\n",errno);
      }
      free(puStack_c0);
      close(__fd);
                    /* WARNING: Could not recover jumptable at 0x0fb79aa4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (*UNRECOVERED_JUMPTABLE_00._4_4_)();
      return;
    }
    uVar10 = 0;
    if (uVar11 != 0) break;
LAB_0fb799f4:
    uVar10 = uVar11 * 2;
    bVar2 = 0x3fff < uVar11;
    uVar11 = uVar10;
    if ((0x4000 < uVar10) && (uVar11 = 0x4000, bVar2)) {
      close(__fd);
      free(puStack_c0);
                    /* WARNING: Could not recover jumptable at 0x0fb79adc. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (*UNRECOVERED_JUMPTABLE_00._4_4_)();
      return;
    }
  }
  while( true ) {
    piVar13 = (int *)((int)puStack_c0 + iVar12);
    if (debug_map != '\0') {
      debug("PIOCMAP_SGI Region %d addr 0x%lx end 0x%lx\n",uVar10,*piVar13,piVar13[1] + *piVar13);
    }
    iVar12 = iVar12 + 0x50;
    if ((piVar13[1] == 0) && (*piVar13 == 0)) break;
    uVar10 = uVar10 + 1;
    if (uVar10 == uVar11) goto LAB_0fb799f4;
  }
  if (uVar11 - 1 <= uVar10) goto LAB_0fb799f4;
  if (debug_map != '\0') {
    debug("PIOCMAP_SGI Region %d is empty. Found end of array.\n",uVar10);
  }
  close(__fd);
  lStack_48 = (longlong)(int)puStack_c0;
  bVar2 = __rld_loading_data + 0x400000U < *puStack_c0;
  if (bVar2) {
    uVar10 = uVar10 + 1;
  }
  uStack_50 = (ulonglong)bVar2;
  lVar3 = FUN_0fb79530(uVar10);
  __base = DAT_0fbd39b8;
  if (lVar3 < 0) goto LAB_0fb79a4c;
  DAT_0fbd39b0 = uVar10;
  if (uStack_50 == 0) {
    if (uVar10 == 0) goto LAB_0fb79c10;
    uVar11 = 0;
    lVar3 = lStack_48;
    puVar8 = DAT_0fbd39b8;
  }
  else {
    *(undefined2 *)(DAT_0fbd39b8 + 2) = 0;
    *(undefined1 *)((int)__base + 10) = 1;
    __base[1] = 0x400000;
    *__base = 0;
    if (uVar10 < 2) goto LAB_0fb79c10;
    uVar11 = 1;
    lVar3 = lStack_48;
    puVar8 = __base + 3;
  }
  do {
    puVar9 = (undefined4 *)lVar3;
    *puVar8 = *puVar9;
    uVar1 = puVar9[1];
    *(undefined2 *)(puVar8 + 2) = 0;
    uVar11 = uVar11 + 1;
    *(undefined1 *)((int)puVar8 + 10) = 1;
    puVar8[1] = uVar1;
    lVar3 = (longlong)(int)(puVar9 + 0x14);
    puVar8 = puVar8 + 3;
  } while (uVar11 != uVar10);
LAB_0fb79c10:
  free(lStack_48);
  qsort(__base,uVar10,0xc,(__compar_fn_t)&LAB_0fb7a460);
  if (debug_map != '\0') {
    uVar4 = FUN_0fb783c0("qsorted",uVar10,__base);
    debug("sorted active regions: %d \n",uVar4);
  }
  lVar3 = FUN_0fb79740(__base,uVar10);
  if (0 < lVar3) {
    DAT_0fbd39b0 = FUN_0fb79620(__base,uVar10);
    uVar10 = DAT_0fbd39b0;
  }
  if ((debug_map != '\0') &&
     (debug("__rld_get_active_regions set up %d regions\n",uVar10), debug_map != '\0')) {
    uVar4 = FUN_0fb783c0("active regions",uVar10,__base);
    debug("get active regions %d complete\n",uVar4);
  }
LAB_0fb79a4c:
                    /* WARNING: Could not recover jumptable at 0x0fb79a58. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE_00._4_4_)();
  return;
}



// === FUN_0fb79cf0 @ 0fb79cf0 (876 bytes) ===

/* WARNING: Instruction at (ram,0x0fb79e40) overlaps instruction at (ram,0x0fb79e3c)
    */

uint FUN_0fb79cf0(int param_1,int param_2,int param_3,uint *param_4)

{
  int iVar1;
  uint uVar2;
  bool bVar3;
  int iVar5;
  __off64_t _Var4;
  uint uVar6;
  int iVar7;
  void *__buf;
  uint uVar8;
  int iVar9;
  uint uVar10;
  void *__addr;
  size_t __len;
  
  uVar8 = *param_4;
  if (debug_map != '\0') {
    debug("verify tlb map into mem at addr 0x%lx,\n",uVar8);
  }
  if (DAT_0fbd39a4 == '\0') {
    DAT_0fbd39a0 = uVar8 & 0x7000;
    if (debug_map != '\0') {
      debug("Initial tlb bits next to use to 0x%lx\n");
    }
    DAT_0fbd39a4 = '\x01';
  }
  if (DAT_0fbd39c4 == '\0') {
    iVar7 = 0;
  }
  else {
    iVar7 = 0;
    if (DAT_0fbd39a0 != (uVar8 & 0x7000)) {
      uVar8 = uVar8 + 0x1000;
      iVar7 = 0x1000;
      if (DAT_0fbd39a0 == (uVar8 & 0x7000)) {
        iVar7 = 0x1000;
      }
      else {
        do {
          while( true ) {
            iVar7 = iVar7 + 0x1000;
            uVar8 = uVar8 + 0x1000;
            if (debug_map != '\0') break;
            if (DAT_0fbd39a0 == (uVar8 & 0x7000)) goto LAB_0fb79f4c;
          }
          debug("tlb map adder updated, addr 0x%lx,\n",uVar8);
        } while (DAT_0fbd39a0 != (uVar8 & 0x7000));
      }
LAB_0fb79f4c:
      if (debug_map != '\0') {
        debug("tlb misalign altered addr from 0x%lx to 0x%lx\n",uVar8 - iVar7,uVar8);
      }
    }
    DAT_0fbd39a0 = DAT_0fbd39a0 + 0x1000;
    if (DAT_0fbd39a0 == 0x7000) {
      DAT_0fbd39a0 = 0;
    }
  }
  uVar8 = uVar8 + iVar7;
  iVar1 = *(int *)(param_2 + 8);
  *param_4 = uVar8;
  if (0 < param_3) {
    __addr = (void *)0x0;
    uVar10 = 0;
    __len = 0;
    iVar9 = 0;
    do {
      __buf = (void *)(*(int *)(param_2 + 8) + (uVar8 - iVar1));
      bVar3 = (*(uint *)(param_2 + 0x18) & 1) != 0;
      if (debug_map != '\0') {
        debug("region %d load at 0x%lx after tlb if any, orig: 0x%lx\n",iVar9,__buf,
              (int)__buf - iVar7);
      }
      if (bVar3) {
        uVar10 = ~(__rld_loading_data - 1U) &
                 (int)__buf + __rld_loading_data + *(int *)(param_2 + 0x14) + -1;
        __addr = (void *)(~(__rld_loading_data - 1U) & (uint)__buf);
        __len = uVar10 - (int)__addr;
      }
      if ((debug_map != '\0') &&
         (debug("Now read from DSO to address 0x%lx \n",__buf), debug_map != '\0')) {
        debug("Now read in PT_LOAD offset 0x%lx into our addr at 0x%lx, fsize 0x%lx msize 0x%lx\n",
              *(undefined4 *)(param_2 + 4),__buf,*(undefined4 *)(param_2 + 0x10),
              *(undefined4 *)(param_2 + 0x14));
      }
      _Var4 = lseek64(param_1,(ulonglong)*(uint *)(param_2 + 4),0);
      if (_Var4 == -1) {
        if (debug_map != '\0') {
          debug("Unable to seek to 0x%lx   err %d\n",*(undefined4 *)(param_2 + 4),errno);
        }
        return 0xffffffff;
      }
      uVar6 = read(param_1,__buf,*(size_t *)(param_2 + 0x10));
      uVar2 = *(uint *)(param_2 + 0x10);
      if (uVar6 != uVar2) {
        if (debug_map != '\0') {
          debug("Unable to read from DSO   err %d\n",errno);
        }
        return 0xffffffff;
      }
      if (uVar2 < *(uint *)(param_2 + 0x14)) {
        memset((void *)(uVar2 + (int)__buf),0,*(uint *)(param_2 + 0x14) - uVar2);
      }
      if (bVar3) {
        if (debug_map != '\0') {
          debug("mprotect to read/write/exec 0x%lx to 0x%lx\n",__addr,uVar10);
        }
        iVar5 = mprotect(__addr,__len,7);
        if (iVar5 < 0) {
          if (debug_map != '\0') {
            debug("mprotect back to text/read/write fails %d\n",errno);
          }
          return 0xffffffff;
        }
      }
      iVar9 = iVar9 + 1;
      param_2 = param_2 + 0x20;
    } while (param_3 != iVar9);
  }
  return uVar8;
}



// === FUN_0fb7a060 @ 0fb7a060 (860 bytes) ===

int FUN_0fb7a060(int param_1,longlong param_2,int *param_3)

{
  undefined1 uVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  undefined4 uVar5;
  int iVar6;
  undefined8 uVar7;
  uint uVar8;
  int iVar9;
  uint uVar10;
  uint uVar11;
  longlong lVar12;
  int iVar13;
  undefined4 *puVar14;
  uint *puVar15;
  int iVar16;
  undefined4 *puVar17;
  int iVar18;
  undefined4 auStack_cc [3];
  undefined4 auStack_c0 [2];
  undefined2 uStack_b8;
  longlong lStack_60;
  
  iVar9 = param_3[6];
  lVar12 = (longlong)param_3[7];
  iVar16 = *param_3 - *(int *)((int)param_2 + 8);
  puVar15 = (uint *)(DAT_0fbd39c8 + iVar9 * 0xc);
  lStack_60 = param_2;
  if (debug_map != '\0') {
    uVar7 = FUN_0fb783c0("removal point, avail regions",DAT_0fbd39cc,DAT_0fbd39c8);
    debug("enter __rld_remove_regions_from_free_space %d base\n",uVar7);
    if (debug_map != '\0') {
      debug("__rld_remove_regions_from_free_space avail_seg %d, priority %d  diff 0x%lx\n",iVar9,
            lVar12,iVar16);
    }
  }
  if (*(short *)(puVar15 + 2) == lVar12) {
    FUN_0fb78570("remove regions",0x640);
    if (param_1 < 1) {
      iVar18 = 0;
    }
    else {
      puVar14 = auStack_cc;
      iVar18 = 0;
      iVar13 = 0;
      lVar12 = lStack_60;
      do {
        iVar6 = (int)lVar12;
        uVar2 = *puVar15;
        uVar3 = puVar15[1];
        uVar11 = *(int *)(iVar6 + 8) + iVar16;
        uVar1 = *(undefined1 *)(param_3 + 5);
        iVar4 = *(int *)(iVar6 + 0x14);
        uVar10 = uVar3 + uVar2;
        puVar17 = puVar14;
        if (uVar2 == uVar11) {
          puVar15[1] = uVar3 - iVar4;
          *puVar15 = uVar2 + iVar4;
          if (debug_map != '\0') {
            debug("remove region: new length %lx,i %d\n",uVar3 - iVar4,iVar13);
          }
        }
        else {
          uVar8 = uVar11 + iVar4;
          if (uVar10 <= uVar11) {
            if (debug_map != '\0') {
              debug(
                   "Unable to remove used space from available mem list! 0x%lx 0x%lx, 0x%lx 0x%lx\n"
                   );
            }
            return -1;
          }
          if (uVar8 == uVar10) {
            puVar15[1] = uVar3 - iVar4;
            if (debug_map != '\0') {
              debug("take from end [%d], new length %lx\n",iVar9,uVar3 - iVar4);
            }
          }
          else {
            puVar14[3] = uVar2;
            puVar14[4] = uVar11 - uVar2;
            puVar15[1] = uVar10 - uVar8;
            *puVar15 = uVar8;
            *(undefined1 *)((int)puVar14 + 0x16) = uVar1;
            puVar17 = puVar14 + 3;
            iVar18 = iVar18 + 1;
            *(undefined2 *)(puVar14 + 5) = *(undefined2 *)(puVar15 + 2);
            if ((debug_map != '\0') &&
               (debug("oldp new addr 0x%lx length %lx, i %d\n",uVar8,uVar10 - uVar8,iVar13),
               debug_map != '\0')) {
              debug("newslot  addr 0x%lx length %lx\n",*puVar17,puVar14[4]);
            }
          }
        }
        puVar14 = puVar17;
        iVar13 = iVar13 + 1;
        lVar12 = (longlong)(iVar6 + 0x20);
      } while (param_1 != iVar13);
    }
    FUN_0fb78570("remove regions done",0x682);
    if (iVar18 < 1) {
LAB_0fb7a2c8:
      FUN_0fb78570("remove regions local segs added",0x69a);
      iVar9 = DAT_0fbd39cc;
      while (iVar9 = iVar9 + -1, -1 < iVar9) {
        FUN_0fb7aba0(iVar9);
      }
      FUN_0fb78570("remove regions complete",0x6a2);
      if (debug_map != '\0') {
        uVar7 = FUN_0fb783c0("removal point, avail regions",DAT_0fbd39cc,DAT_0fbd39c8);
        debug("return __rld_remove_regions_from_free_space %d base\n",uVar7);
      }
      return param_1;
    }
    puVar14 = auStack_c0;
    while( true ) {
      uVar5 = *puVar14;
      iVar9 = FUN_0fb7a4d0(uVar5,puVar14[1],*(undefined1 *)((int)puVar14 + 10),uStack_b8);
      if (iVar9 < 0) break;
      if (puVar14 + 3 == auStack_c0 + iVar18 * 3) goto LAB_0fb7a2c8;
      uStack_b8 = *(undefined2 *)(puVar14 + 5);
      puVar14 = puVar14 + 3;
    }
    if (debug_map != '\0') {
      debug("Unable to add to avail 0x%lx \n",uVar5);
    }
  }
  else {
    if (debug_map != '\0') {
      debug("rld internal error seg %d priorities %d %d\n",iVar9,(longlong)*(short *)(puVar15 + 2),
            lVar12);
    }
    iVar9 = -1;
  }
  return iVar9;
}



// === __rld_pagemap_return_to_available @ 0fb7a3c0 (148 bytes) ===

void __rld_pagemap_return_to_available(byte *param_1)

{
  undefined8 uVar1;
  byte *pbVar2;
  int iVar3;
  
  if (*param_1 != 0) {
    iVar3 = 0;
    pbVar2 = param_1;
    do {
      FUN_0fb7a4d0(*(undefined4 *)(pbVar2 + 4),*(undefined4 *)(pbVar2 + 8),pbVar2[0xe],
                   *(undefined2 *)(pbVar2 + 0xc));
      iVar3 = iVar3 + 1;
      pbVar2 = pbVar2 + 0xc;
    } while (iVar3 < (int)(uint)*param_1);
  }
  if (debug_map != '\0') {
    uVar1 = FUN_0fb783c0("removal point, avail regions",DAT_0fbd39cc,DAT_0fbd39c8);
    debug("return __rld_pagemap_return_to_available done %d base\n",uVar1);
  }
  return;
}



// === FUN_0fb7a4d0 @ 0fb7a4d0 (920 bytes) ===

/* WARNING: Instruction at (ram,0x0fb7a600) overlaps instruction at (ram,0x0fb7a5fc)
    */

undefined8 FUN_0fb7a4d0(uint param_1,undefined8 param_2,longlong param_3,short param_4)

{
  bool bVar1;
  char cVar2;
  ushort uVar3;
  uint uVar4;
  uint uVar5;
  undefined8 uVar6;
  longlong lVar7;
  uint *puVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  int iVar12;
  uint uVar13;
  
  if ((debug_map != '\0') &&
     (debug("__rld_add_to_available_segs  segs avail mem used %d addr 0x%lx len 0x%lx is_up %d priority %d\n"
            ,DAT_0fbd39cc,param_1,param_2,param_3,param_4), debug_map != '\0')) {
    uVar6 = FUN_0fb783c0("avail set",DAT_0fbd39cc,DAT_0fbd39c8);
    debug("avail segments before add: %d\n",uVar6);
  }
  FUN_0fb78570("add to avail ",0x703);
  if ((DAT_0fbd39d0 == DAT_0fbd39cc) && (lVar7 = FUN_0fb7a920(), lVar7 < 0)) {
    return 1;
  }
  if (DAT_0fbd39cc < 1) {
    bVar1 = false;
  }
  else {
    iVar10 = 0;
    puVar8 = DAT_0fbd39c8;
    do {
      if (param_4 == *(short *)(puVar8 + 2)) {
        if (param_3 == 0) {
          if (*puVar8 <= param_1) {
            FUN_0fb7a880(iVar10,param_1,param_2,0,param_4);
            bVar1 = true;
            break;
          }
        }
        else if (param_1 <= *puVar8) {
          FUN_0fb7a880(iVar10,param_1,param_2,param_3,param_4);
          bVar1 = true;
          break;
        }
      }
      else if (param_4 <= *(short *)(puVar8 + 2)) {
        FUN_0fb7a880(iVar10,param_1,param_2,param_3,param_4);
        bVar1 = true;
        break;
      }
      iVar10 = iVar10 + 1;
      puVar8 = puVar8 + 3;
      bVar1 = false;
    } while (DAT_0fbd39c8 + DAT_0fbd39cc * 3 != puVar8);
  }
  FUN_0fb78570("add to availB ",0x73d);
  if (!bVar1) {
    puVar8 = (uint *)FUN_0fb7a9b0(0xffffffffffffffff,0x74f);
    *(short *)(puVar8 + 2) = param_4;
    *(char *)((int)puVar8 + 10) = (char)param_3;
    puVar8[1] = (uint)param_2;
    *puVar8 = param_1;
    if (debug_map != '\0') {
      uVar6 = FUN_0fb783c0("avail set after new entry",DAT_0fbd39cc,DAT_0fbd39c8);
      debug("inserted new avail entry at end, new set: %d\n",uVar6);
    }
  }
  FUN_0fb78570("add to availC ",0x74f);
  iVar12 = 0;
  iVar10 = 0xc;
  iVar11 = 1;
  if (DAT_0fbd39cc < 2) {
LAB_0fb7a720:
    FUN_0fb78570("add to availD ",0x780);
    if ((debug_map != '\0') &&
       (debug("end __rld_add_to_available_segs  segs avail mem used %d\n",DAT_0fbd39cc),
       debug_map != '\0')) {
      uVar6 = FUN_0fb783c0("avail regions list",DAT_0fbd39cc,DAT_0fbd39c8);
      debug("avail regions %d\n",uVar6);
    }
    return 1;
  }
  puVar8 = DAT_0fbd39c8 + 3;
  uVar3 = *(ushort *)(DAT_0fbd39c8 + 2);
  iVar9 = DAT_0fbd39cc;
  do {
    cVar2 = *(char *)((int)puVar8 + -2);
    uVar13 = *puVar8;
    if (*(ushort *)(puVar8 + 2) == uVar3) {
      uVar4 = puVar8[-3];
      if (cVar2 == '\0') {
        if (uVar4 == puVar8[1] + uVar13) goto LAB_0fb7a678;
      }
      else {
        bVar1 = uVar13 == puVar8[-2] + uVar4;
        uVar13 = uVar4;
        if (bVar1) {
LAB_0fb7a678:
          if (uVar13 != 0) {
            uVar4 = puVar8[-2];
            uVar5 = puVar8[1];
            if (debug_map != '\0') {
              debug("Merge adjacent %d %d into one\n",iVar12,iVar11);
            }
            FUN_0fb7ab10(DAT_0fbd39c8,iVar12);
            puVar8 = DAT_0fbd39c8;
            *(ushort *)((int)DAT_0fbd39c8 + iVar10 + -4) = uVar3 & 0xff;
            *(char *)((int)puVar8 + iVar10 + -2) = cVar2;
            *(uint *)((int)puVar8 + iVar10 + -8) = uVar4 + uVar5;
            *(uint *)((int)puVar8 + iVar10 + -0xc) = uVar13;
            iVar9 = DAT_0fbd39cc;
            if (debug_map != '\0') {
              uVar6 = FUN_0fb783c0("avail regions list after  merge",DAT_0fbd39cc);
              debug("avail regions  after merge %d\n",uVar6);
              iVar9 = DAT_0fbd39cc;
            }
            goto LAB_0fb7a63c;
          }
        }
      }
      iVar10 = iVar10 + 0xc;
      iVar12 = iVar12 + 1;
      iVar11 = iVar11 + 1;
    }
    else {
      iVar10 = iVar10 + 0xc;
      iVar12 = iVar12 + 1;
      iVar11 = iVar11 + 1;
    }
LAB_0fb7a63c:
    puVar8 = (uint *)(iVar10 + (int)DAT_0fbd39c8);
    if (iVar9 <= iVar11) goto LAB_0fb7a720;
    uVar3 = *(ushort *)(puVar8 + -1);
  } while( true );
}



// === FUN_0fb7a880 @ 0fb7a880 (152 bytes) ===

void FUN_0fb7a880(undefined8 param_1,undefined4 param_2,undefined4 param_3,undefined1 param_4,
                 undefined2 param_5)

{
  undefined4 *puVar2;
  undefined8 uVar1;
  
  puVar2 = (undefined4 *)FUN_0fb7a9b0();
  if (debug_map != '\0') {
    debug("New %d in avail\n",param_1);
  }
  *(undefined2 *)(puVar2 + 2) = param_5;
  *(undefined1 *)((int)puVar2 + 10) = param_4;
  puVar2[1] = param_3;
  *puVar2 = param_2;
  if (debug_map != '\0') {
    uVar1 = FUN_0fb783c0("avail set after new entry",DAT_0fbd39cc,DAT_0fbd39c8);
    debug("inserted new avail entry, new set: %d\n",uVar1);
  }
  FUN_0fb78570("insert in front",0x7b1);
  return;
}



// === FUN_0fb7a920 @ 0fb7a920 (132 bytes) ===

undefined8 FUN_0fb7a920(void)

{
  longlong lVar1;
  int iVar2;
  
  if (DAT_0fbd39d0 == 0) {
    iVar2 = 0x55;
  }
  else {
    iVar2 = DAT_0fbd39d0 * 2;
  }
  lVar1 = realloc(DAT_0fbd39c8,iVar2 * 0xc);
  if (lVar1 == 0) {
    if (debug_map != '\0') {
      debug("Out of memory allocating segments b\n");
    }
    return 0xffffffffffffffff;
  }
  DAT_0fbd39c8 = (int)lVar1;
  DAT_0fbd39d0 = iVar2;
  return 0;
}



// === FUN_0fb7a9b0 @ 0fb7a9b0 (340 bytes) ===

int FUN_0fb7a9b0(int param_1)

{
  undefined1 uVar1;
  undefined1 uVar2;
  undefined2 uVar3;
  undefined2 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  int iVar8;
  undefined4 *puVar9;
  undefined4 *puVar10;
  uint uVar11;
  undefined4 *puVar12;
  
  iVar8 = DAT_0fbd39c8;
  if (param_1 < 0) {
    DAT_0fbd39cc = DAT_0fbd39cc + 1;
    return DAT_0fbd39c8 + DAT_0fbd39cc * 0xc + -0xc;
  }
  uVar11 = DAT_0fbd39cc - param_1 & 3;
  if (param_1 <= DAT_0fbd39cc + -1) {
    puVar12 = (undefined4 *)(DAT_0fbd39c8 + (param_1 + -1) * 0xc);
    puVar9 = (undefined4 *)((DAT_0fbd39cc + -1) * 0xc + DAT_0fbd39c8);
    for (; uVar11 != 0; uVar11 = uVar11 - 1) {
      *(undefined1 *)((int)puVar9 + 0x16) = *(undefined1 *)((int)puVar9 + 10);
      *(undefined2 *)(puVar9 + 5) = *(undefined2 *)(puVar9 + 2);
      puVar9[4] = puVar9[1];
      puVar9[3] = *puVar9;
      puVar9 = puVar9 + -3;
    }
    if (DAT_0fbd39cc - param_1 >> 2 != 0) {
      do {
        uVar1 = *(undefined1 *)((int)puVar9 + -0xe);
        *(undefined1 *)((int)puVar9 + -0xe) = *(undefined1 *)((int)puVar9 + -0x1a);
        uVar3 = *(undefined2 *)(puVar9 + -4);
        *(undefined2 *)(puVar9 + -4) = *(undefined2 *)(puVar9 + -7);
        uVar5 = puVar9[-5];
        puVar9[-5] = puVar9[-8];
        uVar6 = puVar9[-6];
        puVar9[-6] = puVar9[-9];
        uVar2 = *(undefined1 *)((int)puVar9 + -2);
        *(undefined1 *)((int)puVar9 + -2) = uVar1;
        uVar4 = *(undefined2 *)(puVar9 + -1);
        *(undefined2 *)(puVar9 + -1) = uVar3;
        uVar7 = puVar9[-2];
        puVar9[-2] = uVar5;
        uVar5 = puVar9[-3];
        puVar9[-3] = uVar6;
        uVar1 = *(undefined1 *)((int)puVar9 + 10);
        *(undefined1 *)((int)puVar9 + 10) = uVar2;
        uVar3 = *(undefined2 *)(puVar9 + 2);
        *(undefined2 *)(puVar9 + 2) = uVar4;
        uVar6 = puVar9[1];
        puVar9[1] = uVar7;
        uVar7 = *puVar9;
        *puVar9 = uVar5;
        *(undefined1 *)((int)puVar9 + 0x16) = uVar1;
        *(undefined2 *)(puVar9 + 5) = uVar3;
        puVar10 = puVar9 + -0xc;
        puVar9[4] = uVar6;
        puVar9[3] = uVar7;
        puVar9 = puVar10;
      } while (puVar10 != puVar12);
    }
  }
  DAT_0fbd39cc = DAT_0fbd39cc + 1;
  return iVar8 + param_1 * 0xc;
}



// === FUN_0fb7ab10 @ 0fb7ab10 (132 bytes) ===

void FUN_0fb7ab10(int param_1,int param_2)

{
  undefined8 uVar1;
  void *__dest;
  
  if (0 < (DAT_0fbd39cc - param_2) + -1) {
    __dest = (void *)(param_1 + param_2 * 0xc);
    memmove(__dest,(void *)((int)__dest + 0xc),(DAT_0fbd39cc - param_2) * 0xc - 0xc);
  }
  DAT_0fbd39cc = DAT_0fbd39cc + -1;
  if (debug_map != '\0') {
    uVar1 = FUN_0fb783c0("copy_down_onto done",DAT_0fbd39cc,DAT_0fbd39c8);
    debug(" after copy_down_onto done %d\n",uVar1);
  }
  return;
}



// === FUN_0fb7aba0 @ 0fb7aba0 (152 bytes) ===

void FUN_0fb7aba0(undefined8 param_1)

{
  int iVar1;
  int unaff_gp_lo;
  code *UNRECOVERED_JUMPTABLE_00;
  
  iVar1 = *(int *)(unaff_gp_lo + -0x7778) + (int)param_1 * 0xc;
  FUN_0fb78570(unaff_gp_lo + 0x29b8,0x837);
  if (*(char *)(unaff_gp_lo + -0x7eec) != '\0') {
    debug(unaff_gp_lo + 0x29d0,param_1,*(undefined4 *)(iVar1 + 4));
  }
  if (*(int *)(iVar1 + 4) != 0) {
    FUN_0fb78570(unaff_gp_lo + 0x2a08,0x83f);
                    /* WARNING: Could not recover jumptable at 0x0fb7ac10. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*UNRECOVERED_JUMPTABLE_00)();
    return;
  }
  FUN_0fb7ab10(*(undefined4 *)(unaff_gp_lo + -0x7778),param_1);
                    /* WARNING: Could not recover jumptable at 0x0fb7ac30. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE_00)();
  return;
}



// === FUN_0fb7ac40 @ 0fb7ac40 (204 bytes) ===

/* WARNING: Instruction at (ram,0x0fb7acc0) overlaps instruction at (ram,0x0fb7acbc)
    */

undefined8 FUN_0fb7ac40(ulonglong param_1,longlong param_2)

{
  if (param_1 < 0x40000) {
    if (param_1 < 0x4000) {
      if ((param_1 == 0x1000) && (param_2 == 0x1000)) {
        return 1;
      }
    }
    else {
      if (param_1 < 0x4001) {
        return 1;
      }
      if (param_1 == 0x10000) {
        return 1;
      }
    }
    goto LAB_0fb7acd8;
  }
  if (0x40000 < param_1) {
    if (param_1 < 0x400000) {
      if (param_1 != 0x100000) goto LAB_0fb7acd8;
    }
    else if ((0x400000 < param_1) && (param_1 != 0x1000000)) {
LAB_0fb7acd8:
      if (debug_map != '\0') {
        debug("Invalid pagesize requested %ld (must be (4K) 16K 64K 256K 1M 4M 16M)\n",param_1);
      }
      return 0;
    }
  }
  return 1;
}



// === FUN_0fb7ad10 @ 0fb7ad10 (544 bytes) ===

undefined8 FUN_0fb7ad10(byte *param_1,undefined4 *param_2,int *param_3)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  byte bVar4;
  byte *pbVar5;
  byte *pbVar6;
  int iVar7;
  
  bVar4 = *param_1;
  pbVar6 = param_1;
  if (bVar4 != 0) {
    while ((bVar4 == 0x20 || (pbVar6 = param_1, bVar4 == 9))) {
      bVar4 = param_1[1];
      pbVar6 = param_1 + 1;
      if ((bVar4 == 0) || ((bVar4 != 0x20 && (bVar4 != 9)))) break;
      bVar4 = param_1[2];
      pbVar6 = param_1 + 2;
      param_1 = pbVar6;
      if (bVar4 == 0) break;
    }
  }
  iVar2 = 0;
  iVar7 = 0;
  if (bVar4 == 0x30) {
    uVar1 = (uint)pbVar6[1];
    pbVar5 = pbVar6 + 1;
    if ((uVar1 == 0x78) || (uVar1 == 0x58)) {
      uVar1 = (uint)pbVar6[2];
      pbVar5 = pbVar6 + 2;
      iVar7 = 0;
      iVar3 = 0;
      if (uVar1 == 0) {
        return 0;
      }
      do {
        if ((uVar1 < 0x30) || (0x39 < uVar1)) {
          if ((uVar1 < 0x61) || (0x66 < uVar1)) {
            if ((uVar1 < 0x41) || (0x46 < uVar1)) break;
            iVar7 = uVar1 + iVar7 * 0x10 + -0x37;
          }
          else {
            iVar7 = uVar1 + iVar7 * 0x10 + -0x57;
          }
        }
        else {
          iVar7 = uVar1 + iVar7 * 0x10 + -0x30;
        }
        iVar3 = iVar3 + 1;
        uVar1 = (uint)pbVar5[1];
        pbVar5 = pbVar5 + 1;
      } while (uVar1 != 0);
    }
    else {
      iVar3 = 1;
      if (uVar1 == 0) {
        iVar7 = 0;
        goto LAB_0fb7add4;
      }
      iVar7 = 0;
      do {
        if ((uVar1 < 0x30) || (0x37 < uVar1)) break;
        pbVar6 = pbVar5 + 1;
        pbVar5 = pbVar5 + 1;
        iVar3 = iVar3 + 1;
        iVar7 = uVar1 + iVar7 * 8 + -0x30;
        uVar1 = (uint)*pbVar6;
      } while (*pbVar6 != 0);
    }
  }
  else {
    uVar1 = (uint)*pbVar6;
    if (uVar1 == 0) {
      return 0;
    }
    while ((iVar3 = iVar2, pbVar5 = pbVar6, 0x2f < uVar1 && (uVar1 < 0x3a))) {
      pbVar5 = pbVar6 + 1;
      bVar4 = *pbVar5;
      iVar3 = iVar2 + 1;
      iVar7 = uVar1 + iVar7 * 10 + -0x30;
      if ((bVar4 == 0) || ((bVar4 < 0x30 || (0x39 < bVar4)))) break;
      pbVar5 = pbVar6 + 2;
      uVar1 = (uint)*pbVar5;
      iVar3 = iVar2 + 2;
      iVar7 = (uint)bVar4 + iVar7 * 10 + -0x30;
      iVar2 = iVar3;
      pbVar6 = pbVar5;
      if (uVar1 == 0) break;
    }
  }
  if (iVar3 == 0) {
    return 0;
  }
LAB_0fb7add4:
  *param_2 = pbVar5;
  *param_3 = iVar7;
  return 1;
}



// === __rld_bp_parse_value @ 0fb7af30 (280 bytes) ===

void __rld_bp_parse_value(int param_1,undefined8 param_2,undefined8 param_3)

{
  char cVar1;
  longlong lVar2;
  int iVar3;
  code *in_ra_lo;
  int iStack_30;
  int iStack_2c;
  undefined8 uStack_28;
  undefined8 uStack_20;
  code *UNRECOVERED_JUMPTABLE_00;
  
  iStack_30 = 0;
  uStack_28 = param_3;
  uStack_20 = param_2;
  UNRECOVERED_JUMPTABLE_00 = in_ra_lo;
  lVar2 = FUN_0fb7ad10(param_1,&iStack_30,&iStack_2c);
  if (lVar2 == 0) {
    if (debug_map != '\0') {
      debug("__rld_bp_parse_value empty/garbage number in %s\n",param_1);
    }
                    /* WARNING: Could not recover jumptable at 0x0fb7afe8. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*UNRECOVERED_JUMPTABLE_00)();
    return;
  }
  iVar3 = (int)uStack_20 + param_1;
  if (iStack_30 != iVar3) {
    cVar1 = *(char *)(iVar3 + -1);
    if (cVar1 == 'K') {
      iStack_2c = iStack_2c << 10;
    }
    else if (cVar1 == 'M') {
      iStack_2c = iStack_2c << 0x14;
    }
    else {
      if (cVar1 != 'G') {
        if (debug_map != '\0') {
          debug("__rld_bp_parse_value garbage last char %s\n",param_1);
        }
                    /* WARNING: Could not recover jumptable at 0x0fb7afbc. Too many branches */
                    /* WARNING: Treating indirect jump as call */
        (*UNRECOVERED_JUMPTABLE_00)();
        return;
      }
      iStack_2c = iStack_2c << 0x1e;
    }
                    /* WARNING: Could not recover jumptable at 0x0fb7b034. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    *(int *)uStack_28 = iStack_2c;
    (*UNRECOVERED_JUMPTABLE_00)();
    return;
  }
  *(int *)uStack_28 = iStack_2c;
                    /* WARNING: Could not recover jumptable at 0x0fb7b00c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE_00)();
  return;
}



// === __rld_complete_big_page_control_setup @ 0fb7b050 (412 bytes) ===

/* WARNING: Instruction at (ram,0x0fb7b100) overlaps instruction at (ram,0x0fb7b0fc)
    */

undefined8 __rld_complete_big_page_control_setup(void)

{
  longlong lVar1;
  undefined8 uVar2;
  undefined *puVar3;
  
  if (__rld_loading_data == 0) {
    if (verbose != '\0') {
      trace("cd_pagesize zero error \n");
    }
    return 0xffffffffffffffff;
  }
  lVar1 = FUN_0fb7ac40(__rld_loading_data,pagesize);
  if (lVar1 == 0) {
    if (verbose != '\0') {
      trace("cd_pagesize invalid error \n");
    }
    return 0xffffffffffffffff;
  }
  if (__rld_loading_data == 0) {
    trap(7);
  }
  if (DAT_0fbd39ac % __rld_loading_data == 0) {
    if (DAT_0fbd39bc == 0) {
      DAT_0fbd39bc = 4;
      DAT_0fbd39c0 = &DAT_0fbde3b8;
    }
    if (DAT_0fbd39ac < __rld_loading_data) {
      DAT_0fbd39ac = __rld_loading_data;
    }
    lVar1 = FUN_0fb7ba20();
    if (lVar1 < 0) {
      if (verbose != '\0') {
        trace("__rld_complete_big_page_control_setup init failed\n");
      }
      uVar2 = 0xffffffffffffffff;
    }
    else {
      if (((verbose != '\0') &&
          (trace("__rld_complete_big_page_control_setup complete\n",DAT_0fbd39c0), verbose != '\0'))
         && (trace("pagesize  0x%lx\n",__rld_loading_data), verbose != '\0')) {
        trace("mmap size  0x%lx\n",DAT_0fbd39ac);
        puVar3 = &DAT_0fbddd18;
        if (verbose != '\0') {
          if (DAT_0fbd39c4 == '\0') {
            puVar3 = &DAT_0fbddd1c;
          }
          trace("attempt cache_misalign? %s\n",puVar3);
          if (verbose != '\0') {
            trace("allocation preference: count %d\n",DAT_0fbd39bc);
          }
        }
      }
      uVar2 = FUN_0fb7b260(DAT_0fbd39bc,DAT_0fbd39c0);
      FUN_0fb7b410(DAT_0fbd39bc,DAT_0fbd39c0);
    }
    return uVar2;
  }
  if (verbose != '\0') {
    trace("cd_mmap_size not multiple of cd_pagesize error\n");
  }
  return 0xffffffffffffffff;
}



// === FUN_0fb7b260 @ 0fb7b260 (420 bytes) ===

undefined8 FUN_0fb7b260(uint param_1,uint *param_2)

{
  uint uVar1;
  undefined4 *__base;
  undefined8 uVar2;
  undefined4 *puVar3;
  uint *puVar4;
  uint uVar5;
  uint *puVar6;
  uint *puVar7;
  code *UNRECOVERED_JUMPTABLE;
  
  __base = (undefined4 *)malloc(param_1 << 2);
  if (__base == (undefined4 *)0x0) {
    if (verbose != '\0') {
      trace("Could not malloc array of %d entries of big page regions\n",param_1);
    }
                    /* WARNING: Could not recover jumptable at 0x0fb7b2b8. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    uVar2 = (*UNRECOVERED_JUMPTABLE)();
    return uVar2;
  }
  if (0 < (int)param_1) {
    puVar4 = param_2;
    puVar3 = __base;
    for (uVar5 = param_1 & 7; uVar5 != 0; uVar5 = uVar5 - 1) {
      *puVar3 = puVar4;
      puVar4 = puVar4 + 3;
      puVar3 = puVar3 + 1;
    }
    if ((int)param_1 >> 3 != 0) {
      do {
        puVar3[7] = puVar4 + 0x15;
        puVar3[6] = puVar4 + 0x12;
        puVar3[5] = puVar4 + 0xf;
        puVar3[4] = puVar4 + 0xc;
        puVar3[3] = puVar4 + 9;
        puVar7 = puVar4 + 6;
        puVar6 = puVar4 + 3;
        *puVar3 = puVar4;
        puVar4 = puVar4 + 0x18;
        puVar3[2] = puVar7;
        puVar3[1] = puVar6;
        puVar3 = puVar3 + 8;
      } while (puVar4 != param_2 + param_1 * 3);
    }
  }
  qsort(__base,param_1,4,(__compar_fn_t)&LAB_0fb7b1f0);
  uVar5 = 1;
  if (1 < (int)param_1) {
    puVar4 = param_2;
    do {
      uVar1 = puVar4[3];
      if (uVar1 < *param_2 + param_2[1]) {
        if (verbose != '\0') {
          trace("ERROR:[%d]  0x%lx-0x%lx  up? %d  overlaps [%d]  0x%lx-0x%lx  up? %d, big page load ignored\n"
                ,uVar5 - 1,*param_2,param_2[1],*(undefined1 *)(param_2 + 2),uVar5,uVar1,puVar4[4],
                *(undefined1 *)(puVar4 + 5));
        }
        free(__base);
        return 0xffffffffffffffff;
      }
      uVar5 = uVar5 + 1;
      puVar4 = puVar4 + 3;
    } while (param_1 != uVar5);
  }
  free(__base);
  return 1;
}



// === FUN_0fb7b410 @ 0fb7b410 (120 bytes) ===

void FUN_0fb7b410(int param_1,undefined4 *param_2)

{
  int iVar1;
  
  if (0 < param_1) {
    iVar1 = 0;
    do {
      if (verbose != '\0') {
        trace("[%d]  0x%lx-0x%lx  up? %d\n",iVar1,*param_2,param_2[1],*(undefined1 *)(param_2 + 2));
      }
      iVar1 = iVar1 + 1;
      param_2 = param_2 + 3;
    } while (param_1 != iVar1);
  }
  return;
}



// === __rld_setup_mmap_alloc_preference @ 0fb7b490 (1412 bytes) ===

undefined8 __rld_setup_mmap_alloc_preference(char *param_1)

{
  longlong lVar1;
  undefined8 uVar2;
  char *pcVar3;
  char *pcVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  uint *puVar8;
  int iVar9;
  uint uStack_70;
  uint uStack_6c;
  
  if ((param_1 == (char *)0x0) || (*param_1 == '\0')) {
    uVar2 = 0xffffffffffffffff;
    if (debug_map != '\0') {
      debug("allocation region empty error\n");
      uVar2 = 0xffffffffffffffff;
    }
  }
  else {
    iVar5 = 0;
    iVar7 = 0;
    pcVar4 = param_1;
    while( true ) {
      if (*pcVar4 == ':') {
        iVar7 = iVar7 + 1;
      }
      if ((*pcVar4 == '\0') || (iVar5 == 1999)) break;
      if (pcVar4[1] == ':') {
        iVar7 = iVar7 + 1;
      }
      if ((pcVar4[1] == '\0') || (iVar5 == 0x7ce)) break;
      pcVar3 = pcVar4 + 3;
      if (pcVar4[2] == ':') {
        iVar7 = iVar7 + 1;
      }
      if ((pcVar4[2] == '\0') || (iVar5 == 0x7cd)) break;
      pcVar4 = pcVar4 + 4;
      iVar5 = iVar5 + 4;
      if (*pcVar3 == ':') {
        iVar7 = iVar7 + 1;
      }
      if ((*pcVar3 == '\0') || (iVar5 == 2000)) break;
    }
    iVar5 = iVar7 + 1;
    if (DAT_0fbd39bc < iVar5) {
      if (DAT_0fbd39c0 != 0) {
        free();
        DAT_0fbd39bc = 0;
      }
      DAT_0fbd39c0 = malloc(iVar5 * 0xc);
      if (DAT_0fbd39c0 == 0) {
        if (debug_map != '\0') {
          debug("-big_page_allocation_preference cannot malloc %d internal records\n",iVar5);
        }
        return 0xffffffffffffffff;
      }
    }
    DAT_0fbd39bc = iVar5;
    if (0 < iVar5) {
      iVar9 = 0;
      iVar6 = 0;
      do {
        puVar8 = (uint *)(DAT_0fbd39c0 + iVar9);
        pcVar4 = strchr(param_1,0x2d);
        if (pcVar4 == (char *)0x0) {
          if (debug_map != '\0') {
            debug("-big_page_allocation_preferencerestricted address string \"%s\" has wrong format, no -, giving up preferences\n"
                  ,param_1);
          }
          return 0xffffffffffffffff;
        }
        lVar1 = __rld_bp_parse_value(param_1,(int)pcVar4 - (int)param_1,&uStack_70);
        if (lVar1 == -1) {
          if (debug_map != '\0') {
            debug("-big_page_restricted_region \"%s\" first val wrong format, ignored.\n",param_1);
          }
          return 0xffffffffffffffff;
        }
        pcVar3 = strchr(pcVar4,0x2c);
        if (pcVar3 == (char *)0x0) {
          if (debug_map != '\0') {
            debug("-big_page_allocation_preferencerestricted address string \"%s\" has wrong format, no comma. preferences ignored \n"
                  ,param_1);
          }
          return 0xffffffffffffffff;
        }
        lVar1 = __rld_bp_parse_value(pcVar4 + 1,pcVar3 + (-1 - (int)pcVar4),&uStack_6c);
        if (lVar1 == -1) {
          if (debug_map != '\0') {
            debug("-big_page_allocation_preference \"%s\" second val wrong format, ignored..\n",
                  param_1);
          }
          return 0xffffffffffffffff;
        }
        if (uStack_6c <= uStack_70) {
          if (debug_map != '\0') {
            debug("-big_page_allocation_preference \"%s\" low >= high,  0x%lx 0x%lx, ignored..\n",
                  param_1);
          }
          return 0xffffffffffffffff;
        }
        uVar2 = 0;
        if (pcVar3[1] == 'u') {
          *(undefined1 *)(puVar8 + 2) = 1;
          uVar2 = 1;
        }
        else {
          if (pcVar3[1] != 'd') {
            if (debug_map != '\0') {
              debug("-big_page_allocation_preference \"%s\" missing u d \n",param_1,uStack_70,iVar7,
                    0);
            }
            return 0xffffffffffffffff;
          }
          *(undefined1 *)(puVar8 + 2) = 0;
        }
        if (((pcVar3[2] != '\0') || (iVar7 != iVar6)) && ((pcVar3[2] != ':' || (iVar6 >= iVar7)))) {
          if (debug_map != '\0') {
            debug("-big_page_allocation_preference \"%s\" not ended correctly\n",param_1,uStack_70,
                  iVar6 < iVar7,uVar2);
          }
          return 0xffffffffffffffff;
        }
        *puVar8 = uStack_70;
        puVar8[1] = uStack_6c;
        param_1 = pcVar3 + 3;
        if (debug_map != '\0') {
          debug("allocation_preference [%d] 0x%lx 0x%lx up? %d\n",iVar6,uStack_70,uStack_6c,uVar2);
        }
        iVar6 = iVar6 + 1;
        iVar9 = iVar9 + 0xc;
      } while (iVar6 != iVar5);
    }
    if (debug_map != '\0') {
      debug("-big_page_allocation_preference region constructed \n");
    }
    uVar2 = 1;
  }
  return uVar2;
}



// === FUN_0fb7ba20 @ 0fb7ba20 (176 bytes) ===

undefined8 FUN_0fb7ba20(undefined8 param_1)

{
  longlong lVar1;
  undefined1 auStack_40 [40];
  undefined4 uStack_18;
  
  if (debug_map != '\0') {
    debug("Pagesize:  data %ld (bytes)\n",param_1);
  }
  pm_filldefault(auStack_40);
  uStack_18 = (undefined4)param_1;
  lVar1 = pm_create(auStack_40);
  if (lVar1 < 0) {
    if (debug_map != '\0') {
      debug("pm_create failed, errno %d\n",errno);
    }
    return 0xffffffffffffffff;
  }
  lVar1 = pm_setdefault(lVar1,2);
  if (lVar1 < 0) {
    if (debug_map != '\0') {
      debug("pm_setdefault MEM_DATA errno %d\n",errno);
    }
    return 0xffffffffffffffff;
  }
  return 0;
}



// === qfcvt_r @ 0fb7bad0 (8 bytes) ===

int qfcvt_r(double __value,int __ndigit,int *__decpt,int *__sign,char *__buf,size_t __len)

{
  return 0;
}



// === qgcvt @ 0fb7bae0 (8 bytes) ===

char * qgcvt(double __value,int __ndigit,char *__buf)

{
  return (char *)0x0;
}



// === wctomb @ 0fb7baf0 (8 bytes) ===

size_t wctomb(wchar_t *__pwcs,char *__s,size_t __n)

{
  return 0;
}



// === sprocsp @ 0fb7bb00 (8 bytes) ===

undefined8 sprocsp(void)

{
  return 0;
}



// === _aqueue @ 0fb7bb10 (8 bytes) ===

undefined8 _aqueue(void)

{
  return 0;
}



// === __ns_iconv_lookup @ 0fb7bb20 (8 bytes) ===

undefined8 __ns_iconv_lookup(void)

{
  return 0;
}



// === __ns_iconv_open @ 0fb7bb30 (8 bytes) ===

undefined8 __ns_iconv_open(void)

{
  return 0;
}



// === iconv_specsize @ 0fb7bb40 (8 bytes) ===

int iconv_specsize(iconv_t __cd)

{
  return 0;
}



// === __iconv_utf8_lat1_flb @ 0fb7bb50 (8 bytes) ===

undefined8 __iconv_utf8_lat1_flb(void)

{
  return 0;
}



// === atfork_pre @ 0fb7bb60 (8 bytes) ===

undefined8 atfork_pre(void)

{
  return 0;
}



// === pthread_setconcurrency @ 0fb7bb70 (8 bytes) ===

int pthread_setconcurrency
              (pthread_t *__newthread,pthread_attr_t *__attr,__start_routine *__start_routine,
              void *__arg)

{
  return 0;
}



// === mkarglist @ 0fb7bb80 (8 bytes) ===

undefined8 mkarglist(void)

{
  return 0;
}



// === getdate @ 0fb7bb90 (8 bytes) ===

tm * getdate(char *__string)

{
  return (tm *)0x0;
}



// === fork @ 0fb7bba0 (8 bytes) ===

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

__pid_t fork(void)

{
  return 0;
}



// === atoi @ 0fb7bbb0 (8 bytes) ===

int atoi(char *__nptr)

{
  return 0;
}



// === FUN_0fb7bbb8 @ 0fb7bbb8 (164 bytes) ===

bool FUN_0fb7bbb8(longlong param_1)

{
  int iVar1;
  int unaff_gp_lo;
  
  if ((0 < param_1) && (param_1 < 0x81)) {
    if (*(int *)(unaff_gp_lo + -0x775c) == 0) {
      __sigfillset(&DAT_0fbde4d0);
      *(int *)(unaff_gp_lo + -0x775c) = *(int *)(unaff_gp_lo + -0x775c) + 1;
    }
    iVar1 = (int)param_1 + -1;
    return ((&DAT_0fbde4d0)[(int)(((uint)(iVar1 >> 4) >> 0x1b) + iVar1) >> 5] &
           1 << (iVar1 % 0x20 & 0x1fU)) != 0;
  }
  return false;
}



// === sigfillset @ 0fb7bc5c (108 bytes) ===

int sigfillset(sigset_t *__set)

{
  if (DAT_0fbd39e4 == 0) {
    __sigfillset(&DAT_0fbde4d0);
    DAT_0fbd39e4 = DAT_0fbd39e4 + 1;
  }
  __set->__val[0] = DAT_0fbde4d0;
  __set->__val[1] = DAT_0fbde4d4;
  __set->__val[2] = DAT_0fbde4d8;
  __set->__val[3] = DAT_0fbde4dc;
  return 0;
}



// === sigemptyset @ 0fb7bcc8 (24 bytes) ===

int sigemptyset(sigset_t *__set)

{
  __set->__val[3] = 0;
  __set->__val[2] = 0;
  __set->__val[1] = 0;
  __set->__val[0] = 0;
  return 0;
}



// === sigaddset @ 0fb7bce0 (140 bytes) ===

int sigaddset(sigset_t *__set,int __signo)

{
  longlong lVar1;
  int iVar2;
  
  lVar1 = FUN_0fb7bbb8(__signo);
  if (lVar1 == 0) {
    setoserror(0x16);
    return -1;
  }
  iVar2 = __signo + -1;
  __set->__val[(int)(((uint)(iVar2 >> 4) >> 0x1b) + iVar2) >> 5] =
       __set->__val[(int)(((uint)(iVar2 >> 4) >> 0x1b) + iVar2) >> 5] | 1 << (iVar2 % 0x20 & 0x1fU);
  return 0;
}



// === sigdelset @ 0fb7bd6c (144 bytes) ===

int sigdelset(sigset_t *__set,int __signo)

{
  longlong lVar1;
  int iVar2;
  
  lVar1 = FUN_0fb7bbb8(__signo);
  if (lVar1 == 0) {
    setoserror(0x16);
    return -1;
  }
  iVar2 = __signo + -1;
  __set->__val[(int)(((uint)(iVar2 >> 4) >> 0x1b) + iVar2) >> 5] =
       __set->__val[(int)(((uint)(iVar2 >> 4) >> 0x1b) + iVar2) >> 5] &
       ~(1 << (iVar2 % 0x20 & 0x1fU));
  return 0;
}



// === sigismember @ 0fb7bdfc (136 bytes) ===

int sigismember(sigset_t *__set,int __signo)

{
  longlong lVar1;
  int iVar2;
  
  lVar1 = FUN_0fb7bbb8(__signo);
  if (lVar1 == 0) {
    setoserror(0x16);
    return -1;
  }
  iVar2 = __signo + -1;
  return (int)((__set->__val[(int)(((uint)(iVar2 >> 4) >> 0x1b) + iVar2) >> 5] &
               1 << (iVar2 % 0x20 & 0x1fU)) != 0);
}



// === time @ 0fb7be84 (52 bytes) ===

time_t time(time_t *__timer)

{
  time_t tVar1;
  longlong in_a3;
  
  syscall(0);
  if (in_a3 == 0) {
    if (__timer != (time_t *)0x0) {
      *__timer = 0x3f5;
    }
    return 0x3f5;
  }
  tVar1 = _cerror();
  return tVar1;
}



// === getenv @ 0fb7beb8 (144 bytes) ===

char * getenv(char *__name)

{
  int iVar1;
  longlong lVar2;
  int *piVar3;
  
  if (_environ == (int *)0x0) {
    return (char *)0x0;
  }
  if (*_environ != 0) {
    iVar1 = *_environ;
    piVar3 = _environ;
    while( true ) {
      piVar3 = piVar3 + 1;
      lVar2 = FUN_0fb7bf48(__name,iVar1);
      if (lVar2 != 0) {
        return (char *)lVar2;
      }
      if (*piVar3 == 0) break;
      iVar1 = *piVar3;
    }
  }
  return (char *)0x0;
}



// === FUN_0fb7bf48 @ 0fb7bf48 (92 bytes) ===

char * FUN_0fb7bf48(char *param_1,char *param_2)

{
  char *pcVar1;
  char cVar2;
  char cVar3;
  
  cVar2 = *param_1;
  pcVar1 = param_2 + 1;
  cVar3 = *param_2;
  if (cVar2 == cVar3) {
    do {
      cVar2 = *param_1;
      param_1 = param_1 + 1;
      if (cVar2 == '=') {
        return pcVar1;
      }
      cVar2 = *param_1;
      cVar3 = *pcVar1;
      pcVar1 = pcVar1 + 1;
    } while (cVar2 == cVar3);
  }
  if ((cVar2 == '\0') && (cVar3 == '=')) {
    return pcVar1;
  }
  return (char *)0x0;
}



// === _test_and_set @ 0fb7bfa4 (32 bytes) ===

void _test_and_set(void)

{
  int unaff_gp_lo;
  
  (**(code **)(unaff_gp_lo + 0x32d4))();
  return;
}



// === atomic_op @ 0fb7bfc4 (36 bytes) ===

void atomic_op(code *param_1,undefined8 param_2,undefined8 param_3)

{
  (*param_1)(param_2,param_3);
  return;
}



// === atomic_op32 @ 0fb7bfe8 (36 bytes) ===

void atomic_op32(code *param_1,undefined8 param_2,undefined8 param_3)

{
  (*param_1)(param_2,param_3);
  return;
}



// === init_lock @ 0fb7c00c (12 bytes) ===

undefined8 init_lock(undefined4 *param_1)

{
  *param_1 = 0;
  return 0;
}



// === release_lock @ 0fb7c018 (12 bytes) ===

undefined8 release_lock(undefined4 *param_1)

{
  *param_1 = 0;
  return 0;
}



// === stat_lock @ 0fb7c024 (24 bytes) ===

bool stat_lock(int *param_1)

{
  return *param_1 != 0;
}



// === __havellsc @ 0fb7c03c (28 bytes) ===

void __havellsc(void)

{
  is_mips2();
  return;
}



// === getpagesize @ 0fb7c058 (28 bytes) ===

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

int getpagesize(void)

{
  int iVar1;
  
  iVar1 = sysmp(0xe);
  return iVar1;
}



// === _mips2_add_then_test @ 0fb7c074 (36 bytes) ===

/* WARNING: Removing unreachable block (ram,0x0fb7c088) */

void _mips2_add_then_test(int *param_1,int param_2)

{
  *param_1 = *param_1 + param_2;
  return;
}



// === _mips2_test_and_set @ 0fb7c098 (28 bytes) ===

/* WARNING: Removing unreachable block (ram,0x0fb7c0a4) */

undefined4 _mips2_test_and_set(undefined4 *param_1,undefined4 param_2)

{
  undefined4 uVar1;
  
  uVar1 = *param_1;
  *param_1 = param_2;
  return uVar1;
}



// === _mips2_test_then_and @ 0fb7c0b4 (28 bytes) ===

/* WARNING: Removing unreachable block (ram,0x0fb7c0c0) */

void _mips2_test_then_and(uint *param_1,uint param_2)

{
  *param_1 = *param_1 & param_2;
  return;
}



// === _mips2_test_then_nand @ 0fb7c0d0 (32 bytes) ===

/* WARNING: Removing unreachable block (ram,0x0fb7c0e0) */

void _mips2_test_then_nand(uint *param_1,uint param_2)

{
  *param_1 = ~(*param_1 & param_2);
  return;
}



// === _mips2_test_then_nor @ 0fb7c0f0 (28 bytes) ===

/* WARNING: Removing unreachable block (ram,0x0fb7c0fc) */

void _mips2_test_then_nor(uint *param_1,uint param_2)

{
  *param_1 = ~(*param_1 | param_2);
  return;
}



// === _mips2_test_then_not @ 0fb7c10c (28 bytes) ===

/* WARNING: Removing unreachable block (ram,0x0fb7c118) */

void _mips2_test_then_not(uint *param_1)

{
  *param_1 = ~*param_1;
  return;
}



// === _mips2_test_then_xor @ 0fb7c128 (28 bytes) ===

/* WARNING: Removing unreachable block (ram,0x0fb7c134) */

void _mips2_test_then_xor(uint *param_1,uint param_2)

{
  *param_1 = *param_1 ^ param_2;
  return;
}



// === _mips2_test_then_or @ 0fb7c144 (28 bytes) ===

/* WARNING: Removing unreachable block (ram,0x0fb7c150) */

void _mips2_test_then_or(uint *param_1,uint param_2)

{
  *param_1 = *param_1 | param_2;
  return;
}



// === _mips2_test_then_add @ 0fb7c160 (28 bytes) ===

/* WARNING: Removing unreachable block (ram,0x0fb7c16c) */

void _mips2_test_then_add(int *param_1,int param_2)

{
  *param_1 = *param_1 + param_2;
  return;
}



// === _mips2_add_then_test32 @ 0fb7c17c (36 bytes) ===

/* WARNING: Removing unreachable block (ram,0x0fb7c190) */

void _mips2_add_then_test32(int *param_1,int param_2)

{
  *param_1 = *param_1 + param_2;
  return;
}



// === _mips2_test_and_set32 @ 0fb7c1a0 (28 bytes) ===

/* WARNING: Removing unreachable block (ram,0x0fb7c1ac) */

undefined4 _mips2_test_and_set32(undefined4 *param_1,undefined4 param_2)

{
  undefined4 uVar1;
  
  uVar1 = *param_1;
  *param_1 = param_2;
  return uVar1;
}



// === _mips2_test_then_and32 @ 0fb7c1bc (28 bytes) ===

/* WARNING: Removing unreachable block (ram,0x0fb7c1c8) */

void _mips2_test_then_and32(uint *param_1,uint param_2)

{
  *param_1 = *param_1 & param_2;
  return;
}



// === _mips2_test_then_nand32 @ 0fb7c1d8 (32 bytes) ===

/* WARNING: Removing unreachable block (ram,0x0fb7c1e8) */

void _mips2_test_then_nand32(uint *param_1,uint param_2)

{
  *param_1 = ~(*param_1 & param_2);
  return;
}



// === _mips2_test_then_nor32 @ 0fb7c1f8 (28 bytes) ===

/* WARNING: Removing unreachable block (ram,0x0fb7c204) */

void _mips2_test_then_nor32(uint *param_1,uint param_2)

{
  *param_1 = ~(*param_1 | param_2);
  return;
}



// === _mips2_test_then_not32 @ 0fb7c214 (28 bytes) ===

/* WARNING: Removing unreachable block (ram,0x0fb7c220) */

void _mips2_test_then_not32(uint *param_1)

{
  *param_1 = ~*param_1;
  return;
}



// === _mips2_test_then_xor32 @ 0fb7c230 (28 bytes) ===

/* WARNING: Removing unreachable block (ram,0x0fb7c23c) */

void _mips2_test_then_xor32(uint *param_1,uint param_2)

{
  *param_1 = *param_1 ^ param_2;
  return;
}



// === _mips2_test_then_or32 @ 0fb7c24c (28 bytes) ===

/* WARNING: Removing unreachable block (ram,0x0fb7c258) */

void _mips2_test_then_or32(uint *param_1,uint param_2)

{
  *param_1 = *param_1 | param_2;
  return;
}



// === _mips2_test_then_add32 @ 0fb7c268 (28 bytes) ===

/* WARNING: Removing unreachable block (ram,0x0fb7c274) */

void _mips2_test_then_add32(int *param_1,int param_2)

{
  *param_1 = *param_1 + param_2;
  return;
}



// === is_mips2 @ 0fb7c284 (24 bytes) ===

/* WARNING: Removing unreachable block (ram,0x0fb7c288) */

undefined4 is_mips2(void)

{
  return 0xffffffff;
}



// === acquire_lock @ 0fb7c29c (28 bytes) ===

/* WARNING: Removing unreachable block (ram,0x0fb7c2a8) */

uint acquire_lock(uint *param_1)

{
  uint uVar1;
  uint in_t4_lo;
  
  uVar1 = *param_1;
  *param_1 = in_t4_lo | 1;
  return uVar1;
}



// === spin_lock @ 0fb7c2b8 (112 bytes) ===

/* WARNING: Removing unreachable block (ram,0x0fb7c2d8) */

void spin_lock(int *param_1)

{
  ulonglong in_t4;
  int iVar1;
  
  iVar1 = _ushlockdefspin;
  while (in_t4 = in_t4 | 1, *param_1 != 0) {
    iVar1 = iVar1 + -1;
    if (iVar1 < 1) {
      nanosleep((timespec *)&__usnano,(timespec *)0x0);
      iVar1 = _ushlockdefspin;
    }
  }
  *param_1 = (int)in_t4;
  return;
}



// === sginap @ 0fb7c328 (40 bytes) ===

undefined8 sginap(void)

{
  undefined8 uVar1;
  longlong in_a3;
  
  syscall(0);
  if (in_a3 == 0) {
    return 0x43a;
  }
  uVar1 = _cerror();
  return uVar1;
}



// === blkclr @ 0fb7c350 (8 bytes) ===

/* WARNING: Instruction at (ram,0x0fb7c3c0) overlaps instruction at (ram,0x0fb7c3bc)
    */
/* WARNING: Removing unreachable block (ram,0x0fb7c36c) */

void blkclr(void *__s,size_t __n)

{
  uint uVar1;
  uint uVar2;
  ulonglong *puVar3;
  int iVar4;
  longlong *plVar5;
  undefined1 *puVar6;
  longlong *extraout_a0_lo;
  longlong lVar8;
  longlong extraout_a1;
  ulonglong uVar9;
  ulonglong extraout_a3;
  longlong *plVar10;
  ulonglong uVar11;
  undefined1 *puVar7;
  
  uVar9 = (ulonglong)(int)__n;
  if ((longlong)uVar9 < 8) {
    puVar7 = __s;
    if (0 < (longlong)uVar9) {
      do {
        puVar6 = puVar7 + 1;
        *puVar7 = 0;
        puVar7 = puVar6;
      } while (puVar6 != (undefined1 *)(__n + (int)__s));
    }
    return;
  }
  if (((uint)__s & 7) != 0) {
    iVar4 = ((uint)__s & 7) - 8;
    uVar1 = (uint)__s & 7;
    *(ulonglong *)((int)__s - uVar1) =
         *(ulonglong *)((int)__s - uVar1) & -1L << (8 - uVar1) * 8 | 0UL >> uVar1 * 8;
    uVar9 = (ulonglong)(int)(__n + iVar4);
    __s = (void *)((int)__s - iVar4);
  }
  if (((uVar9 < 0x180) || (_blk_fp == 0)) || ((_blk_fp < 1 && (_blk_init(), _blk_fp == 0)))) {
    lVar8 = 0;
    if ((uVar9 & 0xffffffffffffffc0) != 0) {
      plVar10 = (longlong *)((int)(uVar9 & 0xffffffffffffffc0) + (int)__s);
      plVar5 = __s;
      do {
        __s = plVar5 + 8;
        *plVar5 = 0;
        plVar5[1] = 0;
        plVar5[2] = 0;
        plVar5[3] = 0;
        plVar5[4] = 0;
        plVar5[5] = 0;
        plVar5[6] = 0;
        plVar5[7] = 0;
        plVar5 = __s;
      } while (plVar10 != __s);
    }
    uVar11 = uVar9 & 0x10;
    if ((uVar9 & 0x20) != 0) {
      *(longlong *)__s = 0;
      *(longlong *)((int)__s + 8) = 0;
      *(longlong *)((int)__s + 0x10) = 0;
      *(longlong *)((int)__s + 0x18) = 0;
      __s = (void *)((int)__s + 0x20);
    }
  }
  else {
    _memset_fp(__s,0,(uint)uVar9 - ((uint)uVar9 & 0x1f));
    uVar11 = extraout_a3 & 0x10;
    lVar8 = extraout_a1;
    uVar9 = extraout_a3;
    __s = extraout_a0_lo;
    if (extraout_a3 == 0) {
      return;
    }
  }
  if (uVar11 != 0) {
    *(longlong *)__s = lVar8;
    *(longlong *)((int)__s + 8) = lVar8;
    __s = (void *)((int)__s + 0x10);
  }
  if ((uVar9 & 8) != 0) {
    *(longlong *)__s = lVar8;
    __s = (void *)((int)__s + 8);
  }
  if ((uVar9 & 7) != 0) {
    uVar1 = (int)__s + (int)(uVar9 & 7) + -1;
    uVar2 = uVar1 & 7;
    puVar3 = (ulonglong *)(uVar1 - uVar2);
    *puVar3 = lVar8 << (7 - uVar2) * 8 | *puVar3 & 0xffffffffffffffffU >> (uVar2 + 1) * 8;
  }
  return;
}



// === memset @ 0fb7c358 (388 bytes) ===

/* WARNING: Instruction at (ram,0x0fb7c3c0) overlaps instruction at (ram,0x0fb7c3bc)
    */

void * memset(void *__s,int __c,size_t __n)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  ulonglong *puVar4;
  ulonglong *puVar5;
  undefined1 *puVar6;
  ulonglong *extraout_a0_lo;
  ulonglong uVar8;
  ulonglong extraout_a1;
  ulonglong uVar9;
  ulonglong extraout_a3;
  ulonglong *puVar10;
  ulonglong uVar11;
  undefined1 *puVar7;
  
  uVar9 = (ulonglong)(int)__n;
  if ((longlong)uVar9 < 8) {
    puVar7 = __s;
    if (0 < (longlong)uVar9) {
      do {
        puVar6 = puVar7 + 1;
        *puVar7 = (char)__c;
        puVar7 = puVar6;
      } while (puVar6 != (undefined1 *)(__n + (int)__s));
    }
    return __s;
  }
  uVar8 = 0;
  if ((longlong)__c != 0) {
    uVar8 = (longlong)__c & 0xff;
    uVar8 = uVar8 | (longlong)((int)uVar8 << 8);
    uVar8 = uVar8 | uVar8 << 0x10;
    uVar8 = uVar8 | uVar8 << 0x20;
  }
  puVar4 = __s;
  if (((uint)__s & 7) != 0) {
    iVar3 = ((uint)__s & 7) - 8;
    uVar1 = (uint)__s & 7;
    *(ulonglong *)((int)__s - uVar1) =
         *(ulonglong *)((int)__s - uVar1) & -1L << (8 - uVar1) * 8 | uVar8 >> uVar1 * 8;
    uVar9 = (ulonglong)(int)(__n + iVar3);
    puVar4 = (ulonglong *)((int)__s - iVar3);
  }
  if (((uVar9 < 0x180) || (_blk_fp == 0)) || ((_blk_fp < 1 && (_blk_init(), _blk_fp == 0)))) {
    if ((uVar9 & 0xffffffffffffffc0) != 0) {
      puVar10 = (ulonglong *)((int)(uVar9 & 0xffffffffffffffc0) + (int)puVar4);
      puVar5 = puVar4;
      do {
        puVar4 = puVar5 + 8;
        *puVar5 = uVar8;
        puVar5[1] = uVar8;
        puVar5[2] = uVar8;
        puVar5[3] = uVar8;
        puVar5[4] = uVar8;
        puVar5[5] = uVar8;
        puVar5[6] = uVar8;
        puVar5[7] = uVar8;
        puVar5 = puVar4;
      } while (puVar10 != puVar4);
    }
    uVar11 = uVar9 & 0x10;
    if ((uVar9 & 0x20) != 0) {
      *puVar4 = uVar8;
      puVar4[1] = uVar8;
      puVar4[2] = uVar8;
      puVar4[3] = uVar8;
      puVar4 = puVar4 + 4;
    }
  }
  else {
    __s = (void *)_memset_fp(puVar4,uVar8,(uint)uVar9 - ((uint)uVar9 & 0x1f));
    uVar11 = extraout_a3 & 0x10;
    uVar8 = extraout_a1;
    uVar9 = extraout_a3;
    puVar4 = extraout_a0_lo;
    if (extraout_a3 == 0) {
      return __s;
    }
  }
  if (uVar11 != 0) {
    *puVar4 = uVar8;
    puVar4[1] = uVar8;
    puVar4 = puVar4 + 2;
  }
  if ((uVar9 & 8) != 0) {
    *puVar4 = uVar8;
    puVar4 = puVar4 + 1;
  }
  if ((uVar9 & 7) != 0) {
    uVar1 = (int)puVar4 + (int)(uVar9 & 7) + -1;
    uVar2 = uVar1 & 7;
    puVar4 = (ulonglong *)(uVar1 - uVar2);
    *puVar4 = uVar8 << (7 - uVar2) * 8 | *puVar4 & 0xffffffffffffffffU >> (uVar2 + 1) * 8;
  }
  return __s;
}



// === write @ 0fb7c4dc (40 bytes) ===

ssize_t write(int __fd,void *__buf,size_t __n)

{
  ssize_t sVar1;
  longlong in_a3;
  
  syscall(0);
  if (in_a3 == 0) {
    return 0x3ec;
  }
  sVar1 = _cerror(__fd,__buf,__n);
  return sVar1;
}



// === sysinfo @ 0fb7c504 (40 bytes) ===

int sysinfo(sysinfo *__info)

{
  int iVar1;
  longlong in_a3;
  
  syscall(0);
  if (in_a3 == 0) {
    return 0x484;
  }
  iVar1 = _cerror(__info);
  return iVar1;
}



// === strerror @ 0fb7c52c (116 bytes) ===

char * strerror(int __errnum)

{
  char *pcVar1;
  
  if (999 < __errnum) {
    pcVar1 = (char *)__irixerror(__errnum);
    return pcVar1;
  }
  if (sys_nerr <= __errnum) {
    pcVar1 = (char *)__svr4error(__errnum);
    return pcVar1;
  }
  if (-1 < __errnum) {
    pcVar1 = (char *)__sys_errlisterror(__errnum);
    return pcVar1;
  }
  return (char *)0x0;
}



// === mprotect @ 0fb7c5a0 (40 bytes) ===

int mprotect(void *__addr,size_t __len,int __prot)

{
  int iVar1;
  longlong in_a3;
  
  syscall(0);
  if (in_a3 == 0) {
    return 0x470;
  }
  iVar1 = _cerror(__addr,__len,__prot);
  return iVar1;
}



// === mld_create @ 0fb7c5c8 (48 bytes) ===

void mld_create(undefined4 param_1,undefined4 param_2)

{
  undefined4 uStack_20;
  undefined4 uStack_1c;
  
  uStack_20 = param_1;
  uStack_1c = param_2;
  syssgi(0xd0,1,&uStack_20,0);
  return;
}



// === mld_destroy @ 0fb7c5f8 (40 bytes) ===

void mld_destroy(undefined8 param_1)

{
  syssgi(0xd0,2,param_1,0);
  return;
}



// === mldset_create @ 0fb7c620 (48 bytes) ===

void mldset_create(undefined4 param_1,undefined4 param_2)

{
  undefined4 uStack_20;
  undefined4 uStack_1c;
  
  uStack_20 = param_1;
  uStack_1c = param_2;
  syssgi(0xd0,10,&uStack_20,0);
  return;
}



// === mldset_place @ 0fb7c650 (60 bytes) ===

void mldset_place(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
                 undefined4 param_5)

{
  undefined4 uStack_50;
  undefined4 uStack_4c;
  undefined4 uStack_48;
  undefined4 uStack_44;
  undefined4 uStack_40;
  
  uStack_50 = param_1;
  uStack_4c = param_2;
  uStack_48 = param_3;
  uStack_44 = param_4;
  uStack_40 = param_5;
  syssgi(0xd0,0xc,&uStack_50,0);
  return;
}



// === mldset_destroy @ 0fb7c68c (40 bytes) ===

void mldset_destroy(undefined8 param_1)

{
  syssgi(0xd0,0xb,param_1,0);
  return;
}



// === pm_create @ 0fb7c6b4 (200 bytes) ===

void pm_create(undefined4 *param_1)

{
  char acStack_190 [64];
  undefined4 uStack_150;
  char acStack_14c [64];
  undefined4 uStack_10c;
  char acStack_108 [64];
  undefined4 uStack_c8;
  char acStack_c4 [64];
  undefined4 uStack_84;
  char acStack_80 [64];
  undefined4 uStack_40;
  undefined4 uStack_3c;
  undefined4 uStack_38;
  
  strncpy(acStack_190,(char *)*param_1,0x3f);
  uStack_150 = param_1[1];
  strncpy(acStack_14c,(char *)param_1[2],0x3f);
  uStack_10c = param_1[3];
  strncpy(acStack_108,(char *)param_1[4],0x3f);
  uStack_c8 = param_1[5];
  strncpy(acStack_c4,(char *)param_1[6],0x3f);
  uStack_84 = param_1[7];
  strncpy(acStack_80,(char *)param_1[8],0x3f);
  uStack_40 = param_1[9];
  uStack_3c = param_1[10];
  uStack_38 = param_1[0xb];
  syssgi(0xd0,0x14,acStack_190,0);
  return;
}



// === pm_create_simple @ 0fb7c77c (220 bytes) ===

void pm_create_simple(char *param_1,undefined4 param_2,char *param_3,undefined8 param_4,
                     undefined4 param_5)

{
  undefined8 uVar1;
  char acStack_1d0 [64];
  undefined4 uStack_190;
  char acStack_18c [64];
  undefined4 uStack_14c;
  char acStack_148 [64];
  undefined4 uStack_108;
  char acStack_104 [64];
  undefined4 uStack_c4;
  char acStack_c0 [64];
  undefined4 uStack_80;
  undefined4 uStack_7c;
  undefined4 uStack_78;
  undefined8 uStack_68;
  
  uStack_68 = param_4;
  strncpy(acStack_1d0,param_1,0x3f);
  uStack_190 = param_2;
  strncpy(acStack_18c,s_FallbackDefault_0fbd0000,0x3f);
  uVar1 = uStack_68;
  uStack_14c = 0;
  strncpy(acStack_148,param_3,0x3f);
  uStack_108 = (undefined4)uVar1;
  strncpy(acStack_104,s_MigrationDefault_0fbd0010,0x3f);
  uStack_c4 = 0;
  strncpy(acStack_c0,s_PagingDefault_0fbd0028,0x3f);
  uStack_78 = 0;
  uStack_80 = 0;
  uStack_7c = param_5;
  syssgi(0xd0,0x14,acStack_1d0,0);
  return;
}



// === pm_filldefault @ 0fb7c858 (80 bytes) ===

void pm_filldefault(undefined4 *param_1)

{
  param_1[0xb] = 0;
  param_1[10] = 0;
  param_1[9] = 0;
  param_1[7] = 0;
  param_1[5] = 0;
  param_1[3] = 0;
  param_1[1] = 1;
  param_1[2] = s_FallbackDefault_0fbd0000;
  param_1[8] = s_PagingDefault_0fbd0028;
  param_1[6] = s_MigrationDefault_0fbd0010;
  param_1[4] = s_ReplicationDefault_0fbd0050;
  *param_1 = s_PlacementDefault_0fbd0038;
  return;
}



// === pm_destroy @ 0fb7c8a8 (40 bytes) ===

void pm_destroy(undefined8 param_1)

{
  syssgi(0xd0,0x15,param_1,0);
  return;
}



// === pm_attach @ 0fb7c8d0 (52 bytes) ===

void pm_attach(undefined8 param_1,undefined4 param_2,undefined4 param_3)

{
  undefined4 uStack_30;
  undefined4 uStack_2c;
  
  uStack_30 = param_2;
  uStack_2c = param_3;
  syssgi(0xd0,0x17,param_1,&uStack_30);
  return;
}



// === pm_getdefault @ 0fb7c904 (40 bytes) ===

void pm_getdefault(undefined8 param_1)

{
  syssgi(0xd0,0x18,param_1,0);
  return;
}



// === pm_setdefault @ 0fb7c92c (40 bytes) ===

void pm_setdefault(undefined8 param_1,undefined8 param_2)

{
  syssgi(0xd0,0x1c,param_1,param_2);
  return;
}



// === pm_getall @ 0fb7c954 (48 bytes) ===

void pm_getall(undefined4 param_1,undefined4 param_2,undefined8 param_3)

{
  undefined4 uStack_30;
  undefined4 uStack_2c;
  
  uStack_30 = param_1;
  uStack_2c = param_2;
  syssgi(0xd0,0x19,&uStack_30,param_3);
  return;
}



// === pm_getstat @ 0fb7c984 (40 bytes) ===

void pm_getstat(undefined8 param_1,undefined8 param_2)

{
  syssgi(0xd0,0x1a,param_1,param_2);
  return;
}



// === pm_setpagesize @ 0fb7c9ac (40 bytes) ===

void pm_setpagesize(undefined8 param_1,undefined8 param_2)

{
  syssgi(0xd0,0x1b,param_1,param_2);
  return;
}



// === process_mldlink @ 0fb7c9d4 (60 bytes) ===

void process_mldlink(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  undefined4 uStack_40;
  undefined4 uStack_3c;
  undefined4 uStack_38;
  undefined4 uStack_34;
  
  uStack_38 = 0xffffffff;
  uStack_40 = param_1;
  uStack_3c = param_2;
  uStack_34 = param_3;
  syssgi(0xd0,0x1e,&uStack_40,0);
  return;
}



// === process_cpulink @ 0fb7ca10 (64 bytes) ===

void process_cpulink(undefined4 param_1,undefined4 param_2,short param_3,undefined4 param_4)

{
  undefined4 uStack_40;
  undefined4 uStack_3c;
  int iStack_38;
  undefined4 uStack_34;
  
  iStack_38 = (int)param_3;
  uStack_40 = param_1;
  uStack_3c = param_2;
  uStack_34 = param_4;
  syssgi(0xd0,0x1e,&uStack_40,0);
  return;
}



// === migr_range_migrate @ 0fb7ca50 (48 bytes) ===

void migr_range_migrate(undefined4 param_1,undefined4 param_2,undefined8 param_3)

{
  undefined4 uStack_30;
  undefined4 uStack_2c;
  
  uStack_30 = param_1;
  uStack_2c = param_2;
  syssgi(0xd0,0x28,&uStack_30,param_3);
  return;
}



// === migr_policy_args_init @ 0fb7ca80 (152 bytes) ===

void migr_policy_args_init(ulonglong *param_1)

{
  ulonglong uVar1;
  
  *(undefined4 *)param_1 = 1;
  param_1[1] = param_1[1] & 0xffffffffffffff | 0x2800000000000000;
  uVar1 = *param_1;
  *param_1 = uVar1 & 0xffffffff807fffff | 0x8a000000;
  *param_1 = uVar1 & 0xffffffff80403fff | 0x8a4c8000;
  *param_1 = uVar1 & 0xffffffff8040201f | 0x8a4ca640;
  *param_1 = uVar1 & 0xffffffff8040201f | 0x8a4ca658;
  return;
}



// === numa_acreate @ 0fb7cb18 (368 bytes) ===

longlong numa_acreate(undefined8 param_1,longlong param_2,longlong param_3)

{
  int __fd;
  void *pvVar2;
  longlong lVar1;
  char *pcStack_80;
  undefined4 uStack_7c;
  char *pcStack_78;
  undefined4 uStack_74;
  char *pcStack_70;
  undefined4 uStack_6c;
  char *pcStack_68;
  undefined4 uStack_64;
  char *pcStack_60;
  undefined4 uStack_5c;
  undefined4 uStack_58;
  undefined4 uStack_54;
  undefined8 uStack_50;
  longlong lStack_40;
  
  if (param_2 == 0) {
    param_2 = 0x4000000;
  }
  if (param_3 == 0) {
    param_3 = 0;
  }
  uStack_50 = param_1;
  __fd = open64(s__dev_zero_0fbd0068,2);
  pvVar2 = mmap64((void *)0x0,(size_t)param_2,3,1,__fd,0);
  lStack_40 = (longlong)(int)pvVar2;
  if (lStack_40 == -1) {
    return 0;
  }
  uStack_54 = 0;
  uStack_58 = (undefined4)param_3;
  uStack_5c = 0;
  uStack_64 = 0;
  uStack_6c = 0;
  uStack_74 = 0;
  pcStack_78 = s_FallbackDefault_0fbd0000;
  uStack_7c = (undefined4)uStack_50;
  pcStack_60 = s_PagingDefault_0fbd0028;
  pcStack_68 = s_MigrationDefault_0fbd0010;
  pcStack_70 = s_ReplicationDefault_0fbd0050;
  pcStack_80 = s_PlacementFixed_0fbd0078;
  lVar1 = pm_create(&pcStack_80);
  if (lVar1 < 0) {
    return 0;
  }
  lVar1 = pm_attach(lVar1,lStack_40,param_2);
  if (lVar1 < 0) {
    return 0;
  }
  lVar1 = acreate(lStack_40,param_2,0,0,0);
  if (lVar1 == 0) {
    return 0;
  }
  return lVar1;
}



// === __pm_get_page_info @ 0fb7cc88 (56 bytes) ===

void __pm_get_page_info(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  undefined4 uStack_40;
  undefined4 uStack_3c;
  undefined4 uStack_38;
  undefined4 uStack_34;
  
  uStack_40 = param_1;
  uStack_3c = param_2;
  uStack_38 = param_3;
  uStack_34 = param_4;
  syssgi(0xd0,0x32,&uStack_40,&uStack_38);
  return;
}



// === __mld_to_node @ 0fb7ccc0 (40 bytes) ===

void __mld_to_node(undefined8 param_1)

{
  syssgi(0xd0,0x33,param_1,0);
  return;
}



// === get_fpc_csr @ 0fb7cce8 (12 bytes) ===

undefined4 get_fpc_csr(void)

{
  undefined4 in_fcsr;
  
  return in_fcsr;
}



// === set_fpc_csr @ 0fb7ccf4 (16 bytes) ===

undefined4 set_fpc_csr(void)

{
  undefined4 in_fcsr;
  
  return in_fcsr;
}



// === get_fpc_irr @ 0fb7cd04 (12 bytes) ===

undefined4 get_fpc_irr(void)

{
  undefined4 in_fir;
  
  return in_fir;
}



// === __irixerror @ 0fb7cd10 (96 bytes) ===

int __irixerror(int param_1)

{
  int iVar1;
  undefined4 *puVar2;
  
  puVar2 = &_sgi_errtable;
  iVar1 = _sgi_errtable;
  while( true ) {
    if (iVar1 == 0) {
      return 0;
    }
    if (iVar1 == param_1) break;
    iVar1 = puVar2[3];
    puVar2 = puVar2 + 3;
  }
  iVar1 = atoi(s_uxsgierr_0fbd0088);
  return iVar1;
}



// === __svr4error @ 0fb7cd70 (96 bytes) ===

int __svr4error(int param_1)

{
  int iVar1;
  undefined4 *puVar2;
  
  puVar2 = &_sys_errtable;
  iVar1 = _sys_errtable;
  while( true ) {
    if (iVar1 == 0) {
      return 0;
    }
    if (iVar1 == param_1) break;
    iVar1 = puVar2[3];
    puVar2 = puVar2 + 3;
  }
  iVar1 = atoi(s_uxsyserr_0fbd0098);
  return iVar1;
}



// === __sys_errlisterror @ 0fb7cdd0 (108 bytes) ===

int __sys_errlisterror(int param_1)

{
  int iVar1;
  char *__nptr;
  
  if ((*(uint *)(_sys_index + param_1 * 4) & 0x10000) == 0) {
    __nptr = s_uxsyserr_0fbd0098;
  }
  else {
    __nptr = s_uxsgierr_0fbd0088;
  }
  iVar1 = atoi(__nptr);
  return iVar1;
}



// === _cerror @ 0fb7ce3c (44 bytes) ===

undefined8 _cerror(void)

{
  undefined4 in_v0_lo;
  
  errno = in_v0_lo;
  if ((undefined4 *)__errnoaddr != &errno) {
    *(undefined4 *)__errnoaddr = in_v0_lo;
  }
  return 0xffffffffffffffff;
}



// === setjmp @ 0fb7ce70 (92 bytes) ===

int setjmp(__jmp_buf_tag *__env)

{
  undefined8 unaff_s0;
  undefined8 unaff_s1;
  undefined8 unaff_s2;
  undefined8 unaff_s3;
  undefined8 unaff_s4;
  undefined8 unaff_s5;
  undefined8 unaff_s6;
  undefined8 unaff_s7;
  undefined8 unaff_s8;
  undefined8 in_ra;
  undefined8 unaff_f20;
  undefined8 unaff_f22;
  undefined8 unaff_f24;
  undefined8 unaff_f26;
  undefined8 unaff_f28;
  undefined8 unaff_f30;
  undefined8 in_f31;
  int in_fcsr;
  
  *(undefined8 *)(__env->__jmpbuf + 2) = in_ra;
  *(ulonglong *)__env->__jmpbuf = ZEXT48(&stack0x00000000);
  __env->__mask_was_saved = (int)((ulonglong)unaff_s0 >> 0x20);
  (__env->__saved_mask).__val[0] = (int)unaff_s0;
  *(undefined8 *)((__env->__saved_mask).__val + 1) = unaff_s1;
  *(undefined8 *)((__env->__saved_mask).__val + 3) = unaff_s2;
  *(undefined8 *)((__env->__saved_mask).__val + 5) = unaff_s3;
  *(undefined8 *)((__env->__saved_mask).__val + 7) = unaff_s4;
  *(undefined8 *)((__env->__saved_mask).__val + 9) = unaff_s5;
  *(undefined8 *)((__env->__saved_mask).__val + 0xb) = unaff_s6;
  *(undefined8 *)((__env->__saved_mask).__val + 0xd) = unaff_s7;
  *(undefined8 *)((__env->__saved_mask).__val + 0xf) = unaff_s8;
  *(ulonglong *)(__env[1].__jmpbuf + 3) =
       ZEXT48("Unable to realloc %ld bytes for desired symbol array\n");
  *(longlong *)(__env[1].__jmpbuf + 1) = (longlong)in_fcsr;
  *(undefined8 *)((__env->__saved_mask).__val + 0x11) = unaff_f20;
  *(undefined8 *)((__env->__saved_mask).__val + 0x15) = unaff_f22;
  *(undefined8 *)((__env->__saved_mask).__val + 0x17) = unaff_f24;
  *(undefined8 *)((__env->__saved_mask).__val + 0x19) = unaff_f26;
  *(undefined8 *)((__env->__saved_mask).__val + 0x1b) = unaff_f28;
  *(undefined8 *)((__env->__saved_mask).__val + 0x1d) = unaff_f30;
  *(undefined8 *)((__env->__saved_mask).__val + 0x1f) = in_f31;
  return 0;
}



// === longjmp @ 0fb7ced0 (104 bytes) ===

void longjmp(__jmp_buf_tag *__env,int __val)

{
  return;
}



// === perror @ 0fb7cf38 (412 bytes) ===

void perror(char *__s)

{
  longlong lVar1;
  longlong lVar2;
  undefined8 uVar3;
  char *__s_00;
  int iVar4;
  size_t sVar5;
  char *__s_01;
  
  __s_00 = (char *)0x0;
  if (_us_rsthread_misc == 0) {
    lVar1 = 0;
  }
  else {
    lVar1 = ___libc_locklocale();
  }
  lVar2 = oserror();
  if (lVar2 < 1000) {
    iVar4 = oserror();
    if (iVar4 < sys_nerr) {
      lVar2 = oserror();
      if (-1 < lVar2) {
        uVar3 = oserror();
        __s_00 = (char *)__sys_errlisterror(uVar3);
      }
    }
    else {
      uVar3 = oserror();
      __s_00 = (char *)__svr4error(uVar3);
    }
  }
  else {
    uVar3 = oserror();
    __s_00 = (char *)__irixerror(uVar3);
  }
  if (__s_00 == (char *)0x0) {
    __s_00 = (char *)atoi(s_uxsyserr_0fbd00a8);
  }
  if ((__s != (char *)0x0) && (sVar5 = strlen(__s), sVar5 != 0)) {
    __s_01 = (char *)atoi(s_uxsyserr_0fbd00a8);
    write(2,__s,sVar5);
    sVar5 = strlen(__s_01);
    write(2,__s_01,sVar5);
  }
  sVar5 = strlen(__s_00);
  write(2,__s_00,sVar5);
  write(2,&DAT_0fbde1a8,1);
  if (lVar1 != 0) {
    ___libc_unlocklocale(lVar1);
  }
  return;
}



// === __libc_lockmisc @ 0fb7d0d4 (60 bytes) ===

undefined8 __libc_lockmisc(void)

{
  longlong lVar1;
  
  if (_us_rsthread_misc != 0) {
    lVar1 = uspsema(__miscsema);
    if (lVar1 == 1) {
      return 1;
    }
  }
  return 0;
}



// === __libc_islockmisc @ 0fb7d110 (52 bytes) ===

bool __libc_islockmisc(void)

{
  longlong lVar1;
  
  if (_us_rsthread_misc != 0) {
    lVar1 = ustestsema(__miscsema);
    return lVar1 < 1;
  }
  return true;
}



// === __libc_unlockmisc @ 0fb7d144 (32 bytes) ===

void __libc_unlockmisc(longlong param_1)

{
  if (param_1 != 0) {
    usvsema(__miscsema);
  }
  return;
}



// === __libc_lockdir @ 0fb7d164 (60 bytes) ===

undefined8 __libc_lockdir(void)

{
  longlong lVar1;
  
  if (_us_rsthread_misc != 0) {
    lVar1 = uspsema(__dirsema);
    if (lVar1 == 1) {
      return 1;
    }
  }
  return 0;
}



// === __libc_islockdir @ 0fb7d1a0 (52 bytes) ===

bool __libc_islockdir(void)

{
  longlong lVar1;
  
  if (_us_rsthread_misc != 0) {
    lVar1 = ustestsema(__dirsema);
    return lVar1 < 1;
  }
  return true;
}



// === __libc_unlockdir @ 0fb7d1d4 (32 bytes) ===

void __libc_unlockdir(longlong param_1)

{
  if (param_1 != 0) {
    usvsema(__dirsema);
  }
  return;
}



// === __libc_lockopen @ 0fb7d1f4 (60 bytes) ===

undefined8 __libc_lockopen(void)

{
  longlong lVar1;
  
  if (_us_rsthread_stdio != 0) {
    lVar1 = uspsema(__opensema);
    if (lVar1 == 1) {
      return 1;
    }
  }
  return 0;
}



// === __libc_islockopen @ 0fb7d230 (52 bytes) ===

bool __libc_islockopen(void)

{
  longlong lVar1;
  
  if (_us_rsthread_stdio != 0) {
    lVar1 = ustestsema(__opensema);
    return lVar1 < 1;
  }
  return true;
}



// === __libc_unlockopen @ 0fb7d264 (32 bytes) ===

void __libc_unlockopen(longlong param_1)

{
  if (param_1 != 0) {
    usvsema(__opensema);
  }
  return;
}



// === ___libc_locklocale @ 0fb7d284 (60 bytes) ===

undefined8 ___libc_locklocale(void)

{
  longlong lVar1;
  
  if (_us_rsthread_misc != 0) {
    lVar1 = uspsema(__localesema);
    if (lVar1 == 1) {
      return 1;
    }
  }
  return 0;
}



// === __libc_islocklocale @ 0fb7d2c0 (52 bytes) ===

bool __libc_islocklocale(void)

{
  longlong lVar1;
  
  if (_us_rsthread_misc != 0) {
    lVar1 = ustestsema(__localesema);
    return lVar1 < 1;
  }
  return true;
}



// === ___libc_unlocklocale @ 0fb7d2f4 (32 bytes) ===

void ___libc_unlocklocale(longlong param_1)

{
  if (param_1 != 0) {
    usvsema(__localesema);
  }
  return;
}



// === __libc_lockfile @ 0fb7d314 (56 bytes) ===

undefined8 __libc_lockfile(FILE *param_1)

{
  if (_us_rsthread_stdio != 0) {
    flockfile(param_1);
    return 1;
  }
  return 0;
}



// === __libc_islockfile @ 0fb7d34c (52 bytes) ===

undefined8 __libc_islockfile(undefined8 param_1)

{
  undefined8 uVar1;
  
  if (_us_rsthread_stdio != 0) {
    uVar1 = ___fislockfile(param_1);
    return uVar1;
  }
  return 1;
}



// === __libc_unlockfile @ 0fb7d380 (36 bytes) ===

void __libc_unlockfile(FILE *param_1,longlong param_2)

{
  if (param_2 != 0) {
    funlockfile(param_1);
  }
  return;
}



// === __libc_lockmalloc @ 0fb7d3a4 (44 bytes) ===

undefined8 __libc_lockmalloc(void)

{
  int unaff_gp_lo;
  
  if (*(int *)(unaff_gp_lo + -0x7714) != 0) {
    (**(code **)(unaff_gp_lo + -0x7754))(*(undefined4 *)(unaff_gp_lo + -0x7704));
  }
  return 1;
}



// === __libc_unlockmalloc @ 0fb7d3d0 (44 bytes) ===

undefined8 __libc_unlockmalloc(void)

{
  if (_us_rsthread_malloc != 0) {
    (*_ulock)(__mmalloclock);
  }
  return 1;
}



// === __libc_lockpmq @ 0fb7d3fc (44 bytes) ===

undefined8 __libc_lockpmq(void)

{
  if (_us_rsthread_pmq != 0) {
    (*_lock)(__pmqlock);
  }
  return 1;
}



// === __libc_unlockpmq @ 0fb7d428 (44 bytes) ===

undefined8 __libc_unlockpmq(void)

{
  if (_us_rsthread_pmq != 0) {
    (*_ulock)(__pmqlock);
  }
  return 1;
}



// === __libc_lockrand @ 0fb7d454 (40 bytes) ===

void __libc_lockrand(void)

{
  if (_us_rsthread_misc != 0) {
    (*_lock)(__randlock);
  }
  return;
}



// === __libc_unlockrand @ 0fb7d47c (40 bytes) ===

void __libc_unlockrand(void)

{
  if (_us_rsthread_misc != 0) {
    (*_ulock)(__randlock);
  }
  return;
}



// === _monlock @ 0fb7d4a4 (40 bytes) ===

void _monlock(void)

{
  if (__monlock != 0) {
    (*_lock)();
  }
  return;
}



// === _monunlock @ 0fb7d4cc (40 bytes) ===

void _monunlock(void)

{
  if (__monlock != 0) {
    (*_ulock)();
  }
  return;
}



// === __libc_threadbind @ 0fb7d4f4 (8 bytes) ===

void __libc_threadbind(void)

{
  return;
}



// === __libc_threadunbind @ 0fb7d4fc (8 bytes) ===

void __libc_threadunbind(void)

{
  return;
}



// === _seminit @ 0fb7d504 (908 bytes) ===

undefined8 _seminit(void)

{
  int *piVar1;
  longlong lVar2;
  void *pvVar8;
  char *pcVar9;
  undefined8 uVar3;
  int iVar10;
  undefined8 uVar4;
  undefined8 uVar5;
  undefined8 uVar6;
  longlong lVar7;
  int iVar11;
  
  if (_sproced != 0) {
    lVar2 = _usany_tids_left(DAT_0fbd3a54[2]);
    if (lVar2 < 0) {
      return 0xffffffffffffffff;
    }
    return 0;
  }
  if (DAT_0fbd3a54 == (int *)0x0) {
    DAT_0fbd3a54 = calloc(1,0xc);
    if (DAT_0fbd3a54 == (int *)0x0) {
      return 0xffffffffffffffff;
    }
  }
  DAT_0fbd3a58 = getdtablesize();
  if (DAT_0fbd3a58 < 100) {
    DAT_0fbd3a58 = 100;
  }
  if (*DAT_0fbd3a54 == 0) {
    pvVar8 = calloc(DAT_0fbd3a58 + 1,4);
    piVar1 = DAT_0fbd3a54;
    *DAT_0fbd3a54 = (int)pvVar8;
    if (pvVar8 == (void *)0x0) {
      free(piVar1);
      DAT_0fbd3a54 = (int *)0x0;
      return 0xffffffffffffffff;
    }
  }
  pcVar9 = getenv(s_USUSEZEROFORLIBC_0fbd00c8);
  if (pcVar9 == (char *)0x0) {
    pcVar9 = tempnam((char *)0x0,"io");
    DAT_0fbd3a54[1] = (int)pcVar9;
  }
  else {
    DAT_0fbd3a54[1] = (int)s__dev_zero_0fbd00e0;
  }
  uVar3 = usconfig(1,8);
  usconfig(1,uVar3);
  iVar10 = _ussemaspace(uVar3);
  uVar3 = usconfig(2,(DAT_0fbd3a58 + 8) * (iVar10 + 0x60) + 0x5000);
  uVar4 = usconfig(0x10,1);
  uVar5 = usconfig(0x14,0);
  uVar6 = usconfig(0x15,0);
  lVar2 = usinit(DAT_0fbd3a54[1],uVar6);
  usconfig(2,uVar3,lVar2);
  usconfig(0x10,uVar4);
  usconfig(0x14,uVar5);
  usconfig(0x15,uVar6);
  if (lVar2 == 0) {
    return 0xffffffffffffffff;
  }
  lVar7 = (*_nlock)(lVar2);
  __mmalloclock = (undefined4)lVar7;
  if (lVar7 != 0) {
    lVar7 = (*_nlock)(lVar2);
    __pmqlock = (undefined4)lVar7;
    if (lVar7 != 0) {
      lVar7 = (*_nlock)(lVar2);
      __monlock = (undefined4)lVar7;
      if (lVar7 != 0) {
        lVar7 = (*_nlock)(lVar2);
        __randlock = (undefined4)lVar7;
        if (lVar7 != 0) {
          lVar7 = usnewsema(lVar2,1);
          __dirsema = (undefined4)lVar7;
          if (lVar7 != 0) {
            usctlsema(lVar7,0x13);
            lVar7 = usnewsema(lVar2,1);
            __localesema = (undefined4)lVar7;
            if (lVar7 != 0) {
              usctlsema(lVar7,0x13);
              lVar7 = usnewsema(lVar2,1);
              __miscsema = (undefined4)lVar7;
              if (lVar7 != 0) {
                lVar7 = usnewsema(lVar2,1);
                __opensema = (undefined4)lVar7;
                if (lVar7 != 0) {
                  usctlsema(lVar7,0x13);
                  iVar10 = 0;
                  if (-1 < DAT_0fbd3a58) {
                    iVar11 = 0;
                    do {
                      lVar7 = usnewsema(lVar2,1);
                      piVar1 = DAT_0fbd3a54;
                      *(int *)(*DAT_0fbd3a54 + iVar11) = (int)lVar7;
                      if (lVar7 == 0) goto LAB_0fb7d874;
                      usctlsema(*(undefined4 *)(*piVar1 + iVar11),0x13);
                      iVar11 = iVar11 + 4;
                      iVar10 = iVar10 + 1;
                    } while (iVar10 <= DAT_0fbd3a58);
                  }
                  DAT_0fbd3a54[2] = (int)lVar2;
                  _sproced = 1;
                  _us_rsthread_malloc = _us_sthread_malloc;
                  _us_rsthread_pmq = _us_sthread_pmq;
                  _us_rsthread_misc = _us_sthread_misc;
                  _us_rsthread_stdio = _us_sthread_stdio;
                  return 0;
                }
              }
            }
          }
        }
      }
    }
  }
LAB_0fb7d874:
  usdetach(lVar2);
  return 0xffffffffffffffff;
}



// === _semadd @ 0fb7d890 (180 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void _semadd(void)

{
  longlong lVar1;
  int __errnum;
  char *pcVar2;
  size_t sVar3;
  char acStack_110 [272];
  
  lVar1 = usadd(*(undefined4 *)(DAT_0fbd3a54 + 8));
  if (lVar1 != 0) {
    __errnum = oserror();
    pcVar2 = strerror(__errnum);
    sprintf(acStack_110,s_New_process_pid__d_could_not_joi_0fbd00f0,_DAT_00200e00,pcVar2);
    sVar3 = strlen(acStack_110);
    write(2,acStack_110,sVar3);
    lVar1 = oserror();
    if (lVar1 == 0x1c) {
      sprintf(acStack_110,s_Need_to_raise_CONF_INITUSERS___u_0fbd0120);
      sVar3 = strlen(acStack_110);
      write(2,acStack_110,sVar3);
    }
                    /* WARNING: Subroutine does not return */
    _exit(-1);
  }
  return;
}



// === flockfile @ 0fb7d948 (68 bytes) ===

void flockfile(FILE *__stream)

{
  if ((DAT_0fbd3a54 != (int *)0x0) &&
     (*(int *)(*DAT_0fbd3a54 + (uint)*(ushort *)((int)&__stream->_IO_read_base + 2) * 4) != 0)) {
    uspsema();
  }
  return;
}



// === ftrylockfile @ 0fb7d98c (92 bytes) ===

int ftrylockfile(FILE *__stream)

{
  longlong lVar1;
  int iVar2;
  
  iVar2 = 0;
  if (((DAT_0fbd3a54 != (int *)0x0) &&
      (iVar2 = 0,
      *(int *)(*DAT_0fbd3a54 + (uint)*(ushort *)((int)&__stream->_IO_read_base + 2) * 4) != 0)) &&
     (lVar1 = uscpsema(), iVar2 = 0, lVar1 == 0)) {
    iVar2 = 1;
  }
  return iVar2;
}



// === funlockfile @ 0fb7d9e8 (68 bytes) ===

void funlockfile(FILE *__stream)

{
  if ((DAT_0fbd3a54 != (int *)0x0) &&
     (*(int *)(*DAT_0fbd3a54 + (uint)*(ushort *)((int)&__stream->_IO_read_base + 2) * 4) != 0)) {
    usvsema();
  }
  return;
}



// === ___fislockfile @ 0fb7da2c (128 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 ___fislockfile(int param_1)

{
  int iVar1;
  longlong lVar2;
  undefined8 uVar3;
  
  if (DAT_0fbd3a54 != (int *)0x0) {
    iVar1 = *(int *)(*DAT_0fbd3a54 + (uint)*(ushort *)(param_1 + 0xe) * 4);
    if (iVar1 != 0) {
      lVar2 = ustestsema();
      if (lVar2 < 1) {
        uVar3 = 0;
        if (_DAT_00200e00 == *(int *)(iVar1 + 4)) {
          uVar3 = 1;
        }
      }
      else {
        uVar3 = 0;
      }
      return uVar3;
    }
  }
  return 1;
}



// === uspsema @ 0fb7daac (1144 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 uspsema(int *param_1)

{
  short sVar1;
  ushort uVar2;
  ushort uVar3;
  int iVar4;
  int iVar5;
  longlong lVar6;
  short sVar7;
  undefined4 in_ra_lo;
  
  uVar2 = *(ushort *)((int)param_1 + 0x16);
  (*_lock)(param_1[9]);
  sVar1 = *(short *)(param_1 + 3);
  sVar7 = (short)(sVar1 + -1);
  *(short *)(param_1 + 3) = sVar7;
  iVar4 = _DAT_00200e00;
  if (-1 < sVar1 + -1) {
    *(undefined2 *)((int)param_1 + 0xe) = 1;
    param_1[1] = iVar4;
    if ((uVar2 & 7) != 0) {
      iVar4 = *param_1;
      (*_lock)(*(undefined4 *)(iVar4 + 0x440));
      if ((uVar2 & 1) != 0) {
        *(int *)(param_1[7] + 4) = *(int *)(param_1[7] + 4) + 1;
        *(int *)param_1[7] = *(int *)param_1[7] + 1;
      }
      if ((uVar2 & 2) != 0) {
        *(int *)param_1[8] = _DAT_00200e00;
        *(undefined4 *)(param_1[8] + 4) = in_ra_lo;
        *(int *)(param_1[8] + 8) = _DAT_00200e00;
        *(undefined4 *)(param_1[8] + 0xc) = in_ra_lo;
      }
      (*_ulock)(*(undefined4 *)(iVar4 + 0x440));
      if (((*(uint *)(iVar4 + 0x428) & 4) != 0) && ((uVar2 & 4) != 0)) {
        FUN_0fb7ec84(param_1,1,_DAT_00200e00,0,in_ra_lo,*(undefined2 *)(param_1 + 3));
      }
    }
    (*_ulock)(param_1[9]);
    return 1;
  }
  iVar4 = *param_1;
  if ((uVar2 & 0x10) == 0) {
    uVar3 = *(ushort *)(param_1 + 4);
  }
  else {
    if (_DAT_00200e00 == param_1[1]) {
      *(short *)(param_1 + 3) = sVar7 + 1;
      *(short *)((int)param_1 + 0xe) = *(short *)((int)param_1 + 0xe) + 1;
      if ((uVar2 & 2) != 0) {
        (*_lock)(*(undefined4 *)(iVar4 + 0x440));
        *(int *)param_1[8] = _DAT_00200e00;
        *(undefined4 *)(param_1[8] + 4) = in_ra_lo;
        *(int *)(param_1[8] + 8) = _DAT_00200e00;
        *(undefined4 *)(param_1[8] + 0xc) = in_ra_lo;
        (*_ulock)(*(undefined4 *)(iVar4 + 0x440));
      }
      (*_ulock)(param_1[9]);
      return 1;
    }
    uVar3 = *(ushort *)(param_1 + 4);
  }
  *(ushort *)(param_1 + 4) = uVar3 + 1;
  if (*(ushort *)(param_1 + 5) == uVar3) {
    *(undefined2 *)(param_1 + 4) = 0;
  }
  if (*(short *)(param_1 + 4) != *(short *)((int)param_1 + 0x12)) {
    param_1[uVar3 + 10] = _DAT_00200e00;
    if ((uVar2 & 7) != 0) {
      (*_lock)(*(undefined4 *)(iVar4 + 0x440));
      if ((uVar2 & 1) != 0) {
        *(int *)(param_1[7] + 4) = *(int *)(param_1[7] + 4) + 1;
        *(int *)(param_1[7] + 0x10) = *(int *)(param_1[7] + 0x10) + 1;
        iVar5 = param_1[7];
        if (*(int *)(iVar5 + 0x14) < *(int *)(iVar5 + 0x10)) {
          *(int *)(iVar5 + 0x14) = *(int *)(iVar5 + 0x10);
        }
      }
      if ((uVar2 & 2) != 0) {
        *(int *)(param_1[8] + 8) = _DAT_00200e00;
        *(undefined4 *)(param_1[8] + 0xc) = in_ra_lo;
      }
      (*_ulock)(*(undefined4 *)(iVar4 + 0x440));
      if (((*(uint *)(iVar4 + 0x428) & 4) != 0) && ((uVar2 & 4) != 0)) {
        FUN_0fb7ec84(param_1,2,_DAT_00200e00,0,in_ra_lo,*(undefined2 *)(param_1 + 3));
      }
    }
    (*_ulock)(param_1[9]);
    lVar6 = _usblock(iVar4,param_1,_DAT_00200e00,"uspsema");
    if (-1 < lVar6) {
      if ((uVar2 & 8) == 0) {
        if ((((uVar2 & 7) != 0) && ((*(uint *)(iVar4 + 0x428) & 4) != 0)) && ((uVar2 & 4) != 0)) {
          FUN_0fb7ec84(param_1,3,_DAT_00200e00,0,in_ra_lo,*(undefined2 *)(param_1 + 3));
        }
        return 1;
      }
      return 0;
    }
    return 0xffffffffffffffff;
  }
  (*_ulock)(param_1[9]);
  if (_uerror != 0) {
    _uprint(0,s_uspsema_ERROR_sp_0x_x_queue_over_0fbd0190,param_1,_DAT_00200e00);
  }
  setoserror(0x22);
  return 0xffffffffffffffff;
}



// === uscpsema @ 0fb7df24 (568 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 uscpsema(int *param_1)

{
  ushort uVar1;
  int iVar2;
  undefined4 in_ra_lo;
  
  uVar1 = *(ushort *)((int)param_1 + 0x16);
  (*_lock)(param_1[9]);
  if (0 < *(short *)(param_1 + 3)) {
    *(short *)(param_1 + 3) = *(short *)(param_1 + 3) + -1;
    iVar2 = _DAT_00200e00;
    *(undefined2 *)((int)param_1 + 0xe) = 1;
    param_1[1] = iVar2;
    if ((uVar1 & 7) != 0) {
      iVar2 = *param_1;
      (*_lock)(*(undefined4 *)(iVar2 + 0x440));
      if ((uVar1 & 1) != 0) {
        *(int *)(param_1[7] + 4) = *(int *)(param_1[7] + 4) + 1;
        *(int *)param_1[7] = *(int *)param_1[7] + 1;
      }
      if ((uVar1 & 2) != 0) {
        *(int *)param_1[8] = _DAT_00200e00;
        *(undefined4 *)(param_1[8] + 4) = in_ra_lo;
        *(int *)(param_1[8] + 8) = _DAT_00200e00;
        *(undefined4 *)(param_1[8] + 0xc) = in_ra_lo;
      }
      (*_ulock)(*(undefined4 *)(iVar2 + 0x440));
      if (((*(uint *)(iVar2 + 0x428) & 4) != 0) && ((uVar1 & 4) != 0)) {
        FUN_0fb7ec84(param_1,1,_DAT_00200e00,0,in_ra_lo,*(undefined2 *)(param_1 + 3));
      }
    }
    (*_ulock)(param_1[9]);
    return 1;
  }
  iVar2 = *param_1;
  if (((uVar1 & 0x10) != 0) && (_DAT_00200e00 == param_1[1])) {
    *(short *)((int)param_1 + 0xe) = *(short *)((int)param_1 + 0xe) + 1;
    if ((uVar1 & 2) != 0) {
      (*_lock)(*(undefined4 *)(iVar2 + 0x440));
      *(int *)param_1[8] = _DAT_00200e00;
      *(undefined4 *)(param_1[8] + 4) = in_ra_lo;
      *(int *)(param_1[8] + 8) = _DAT_00200e00;
      *(undefined4 *)(param_1[8] + 0xc) = in_ra_lo;
      (*_ulock)(*(undefined4 *)(iVar2 + 0x440));
    }
    (*_ulock)(param_1[9]);
    return 1;
  }
  (*_ulock)(param_1[9]);
  return 0;
}



// === usvsema @ 0fb7e15c (836 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 usvsema(int *param_1)

{
  short sVar1;
  ushort uVar2;
  ushort uVar3;
  int iVar4;
  bool bVar5;
  longlong lVar6;
  int iVar7;
  undefined4 in_ra_lo;
  
  uVar2 = *(ushort *)((int)param_1 + 0x16);
  do {
    bVar5 = false;
    (*_lock)(param_1[9]);
    if ((uVar2 & 0x10) == 0) {
      sVar1 = *(short *)(param_1 + 3);
    }
    else {
      iVar7 = *(short *)((int)param_1 + 0xe) + -1;
      *(short *)((int)param_1 + 0xe) = (short)iVar7;
      if (0 < iVar7) {
        (*_ulock)(param_1[9]);
        return 0;
      }
      sVar1 = *(short *)(param_1 + 3);
    }
    param_1[1] = -1;
    *(short *)(param_1 + 3) = (short)(sVar1 + 1);
    if (sVar1 + 1 < 1) {
      uVar3 = *(ushort *)((int)param_1 + 0x12);
      iVar7 = *param_1;
      *(ushort *)((int)param_1 + 0x12) = uVar3 + 1;
      iVar4 = param_1[uVar3 + 10];
      if (*(ushort *)(param_1 + 5) == uVar3) {
        *(undefined2 *)((int)param_1 + 0x12) = 0;
      }
      param_1[1] = iVar4;
      *(undefined2 *)((int)param_1 + 0xe) = 1;
      if ((uVar2 & 7) != 0) {
        (*_lock)(*(undefined4 *)(iVar7 + 0x440));
        if ((uVar2 & 2) != 0) {
          *(int *)param_1[8] = iVar4;
          *(undefined4 *)(param_1[8] + 4) = in_ra_lo;
          *(undefined4 *)(param_1[8] + 8) = _DAT_00200e00;
          *(undefined4 *)(param_1[8] + 0xc) = in_ra_lo;
        }
        if ((uVar2 & 1) != 0) {
          *(int *)(param_1[7] + 0x10) = *(int *)(param_1[7] + 0x10) + -1;
          *(int *)(param_1[7] + 8) = *(int *)(param_1[7] + 8) + 1;
        }
        (*_ulock)(*(undefined4 *)(iVar7 + 0x440));
        if (((*(uint *)(iVar7 + 0x428) & 4) != 0) && ((uVar2 & 4) != 0)) {
          FUN_0fb7ec84(param_1,5,_DAT_00200e00,iVar4,in_ra_lo,*(undefined2 *)(param_1 + 3));
        }
      }
      (*_ulock)(param_1[9]);
      lVar6 = _usunblock(iVar7,param_1,iVar4,"usvsema");
      if (lVar6 < 0) {
        return 0xffffffffffffffff;
      }
      bVar5 = lVar6 == 1;
      if (lVar6 != 1) {
        return 0;
      }
    }
    if (!bVar5) {
      if ((uVar2 & 7) != 0) {
        iVar7 = *param_1;
        (*_lock)(*(undefined4 *)(iVar7 + 0x440));
        if ((uVar2 & 2) != 0) {
          *(undefined4 *)param_1[8] = 0xffffffff;
          *(undefined4 *)(param_1[8] + 4) = 0xffffffff;
          *(undefined4 *)(param_1[8] + 8) = _DAT_00200e00;
          *(undefined4 *)(param_1[8] + 0xc) = in_ra_lo;
        }
        if ((uVar2 & 1) != 0) {
          *(int *)(param_1[7] + 0xc) = *(int *)(param_1[7] + 0xc) + 1;
          *(int *)(param_1[7] + 8) = *(int *)(param_1[7] + 8) + 1;
        }
        (*_ulock)(*(undefined4 *)(iVar7 + 0x440));
        if (((*(uint *)(iVar7 + 0x428) & 4) != 0) && ((uVar2 & 4) != 0)) {
          FUN_0fb7ec84(param_1,4,_DAT_00200e00,0,in_ra_lo,*(undefined2 *)(param_1 + 3));
        }
      }
      (*_ulock)(param_1[9]);
      return 0;
    }
  } while( true );
}



// === ustestsema @ 0fb7e4a0 (8 bytes) ===

undefined2 ustestsema(int param_1)

{
  return *(undefined2 *)(param_1 + 0xc);
}



// === usinitsema @ 0fb7e4a8 (244 bytes) ===

undefined8 usinitsema(int param_1,longlong param_2)

{
  ushort uVar1;
  undefined4 *puVar2;
  uint uVar3;
  
  if ((-1 < param_2) && (puVar2 = (undefined4 *)(param_1 + 0x28), param_2 < 0x7531)) {
    uVar3 = 0;
    *(undefined2 *)(param_1 + 0x10) = 0;
    *(undefined2 *)(param_1 + 0x12) = 0;
    *(undefined2 *)(param_1 + 0xe) = 0;
    *(short *)(param_1 + 0xc) = (short)param_2;
    *(undefined4 *)(param_1 + 4) = 0xffffffff;
    do {
      *puVar2 = 0;
      uVar3 = uVar3 + 1;
      puVar2 = puVar2 + 1;
    } while (uVar3 <= *(ushort *)(param_1 + 0x14));
    uVar1 = *(ushort *)(param_1 + 0x16);
    if (uVar1 != 0) {
      if ((uVar1 & 1) != 0) {
        *(undefined4 *)(*(int *)(param_1 + 0x1c) + 4) = 0;
        **(undefined4 **)(param_1 + 0x1c) = 0;
        *(undefined4 *)(*(int *)(param_1 + 0x1c) + 8) = 0;
        *(undefined4 *)(*(int *)(param_1 + 0x1c) + 0xc) = 0;
        *(undefined4 *)(*(int *)(param_1 + 0x1c) + 0x10) = 0;
        *(undefined4 *)(*(int *)(param_1 + 0x1c) + 0x14) = 0;
        uVar1 = *(ushort *)(param_1 + 0x16);
      }
      if ((uVar1 & 2) != 0) {
        **(undefined4 **)(param_1 + 0x20) = 0xffffffff;
        *(undefined4 *)(*(int *)(param_1 + 0x20) + 4) = 0xffffffff;
        *(undefined4 *)(*(int *)(param_1 + 0x20) + 8) = 0xffffffff;
        *(undefined4 *)(*(int *)(param_1 + 0x20) + 0xc) = 0xffffffff;
      }
    }
    return 0;
  }
  setoserror(0x16);
  return 0xffffffffffffffff;
}



// === usnewsema @ 0fb7e59c (256 bytes) ===

int * usnewsema(int param_1,undefined8 param_2)

{
  int *piVar2;
  longlong lVar1;
  
  piVar2 = (int *)usmalloc(*(int *)(param_1 + 0x424) * 4 + 0x28,param_1);
  if (piVar2 == (int *)0x0) {
    setoserror(0xc);
    return (int *)0x0;
  }
  *(short *)(piVar2 + 5) = (short)*(undefined4 *)(param_1 + 0x424) + -1;
  if ((*(uint *)(param_1 + 0x428) & 4) == 0) {
    *(undefined2 *)((int)piVar2 + 0x16) = 0;
  }
  else {
    *(undefined2 *)((int)piVar2 + 0x16) = 4;
  }
  piVar2[7] = 0;
  piVar2[8] = 0;
  piVar2[2] = 0;
  *piVar2 = param_1;
  lVar1 = (*_nlock)(param_1);
  piVar2[9] = (int)lVar1;
  if (lVar1 == 0) {
    usfreesema(piVar2,param_1);
    return (int *)0x0;
  }
  lVar1 = usinitsema(piVar2,param_2);
  if (lVar1 < 0) {
    usfreesema(piVar2,param_1);
    piVar2 = (int *)0x0;
  }
  return piVar2;
}



// === _ussemaspace @ 0fb7e69c (12 bytes) ===

int _ussemaspace(int param_1)

{
  return param_1 * 8 + 0x7c;
}



// === usfreesema @ 0fb7e6a8 (208 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void usfreesema(int *param_1,int param_2)

{
  int iVar1;
  int iVar2;
  ushort uVar3;
  
  iVar1 = *param_1;
  if (_uerror == 0) {
LAB_0fb7e6f8:
    uVar3 = *(ushort *)((int)param_1 + 0x16);
  }
  else {
    if (iVar1 != param_2) {
      _uprint(0,s_usfreesema_ERROR_sp_0x_x_bad_usp_0fbd01c0,param_1,iVar1,_DAT_00200e00);
      goto LAB_0fb7e6f8;
    }
    uVar3 = *(ushort *)((int)param_1 + 0x16);
  }
  if (uVar3 != 0) {
    if ((uVar3 & 1) != 0) {
      usfree(param_1[7],iVar1);
      uVar3 = *(ushort *)((int)param_1 + 0x16);
    }
    if ((uVar3 & 2) == 0) {
      iVar2 = param_1[9];
      goto LAB_0fb7e738;
    }
    usfree(param_1[8],iVar1);
  }
  iVar2 = param_1[9];
LAB_0fb7e738:
  if (iVar2 == 0) {
    *param_1 = 0;
  }
  else {
    (*_freelock)(iVar2,iVar1);
    *param_1 = 0;
  }
  *(undefined2 *)(param_1 + 4) = 0;
  *(undefined2 *)((int)param_1 + 0x12) = 0;
  usfree(param_1,iVar1);
  return;
}



// === usctlsema @ 0fb7e778 (940 bytes) ===

undefined4 usctlsema(undefined4 *param_1,undefined4 param_2,undefined4 *param_3)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  
  uVar3 = 0;
  uVar2 = uVar3;
  switch(param_2) {
  case 9:
    uVar2 = 0;
    if ((*(ushort *)((int)param_1 + 0x16) & 1) == 0) {
      puVar1 = (undefined4 *)usmalloc(0x18,*param_1);
      param_1[7] = puVar1;
      if (puVar1 == (undefined4 *)0x0) {
        setoserror(0xc);
        return 0xffffffff;
      }
      *(ushort *)((int)param_1 + 0x16) = *(ushort *)((int)param_1 + 0x16) | 1;
      *puVar1 = 0;
      *(undefined4 *)(param_1[7] + 4) = 0;
      *(undefined4 *)(param_1[7] + 8) = 0;
      *(undefined4 *)(param_1[7] + 0xc) = 0;
      *(undefined4 *)(param_1[7] + 0x10) = 0;
      *(undefined4 *)(param_1[7] + 0x14) = 0;
      uVar2 = uVar3;
    }
    break;
  case 10:
    if ((*(ushort *)((int)param_1 + 0x16) & 1) == 0) {
      uVar2 = 0;
    }
    else {
      *(ushort *)((int)param_1 + 0x16) = *(ushort *)((int)param_1 + 0x16) & 0xfffe;
      usfree(param_1[7],*param_1);
    }
    break;
  case 0xb:
    if ((*(ushort *)((int)param_1 + 0x16) & 1) == 0) {
      setoserror(0x16);
      uVar2 = 0xffffffff;
    }
    else {
      param_3[1] = *(undefined4 *)(param_1[7] + 4);
      *param_3 = *(undefined4 *)param_1[7];
      param_3[2] = *(undefined4 *)(param_1[7] + 8);
      param_3[3] = *(undefined4 *)(param_1[7] + 0xc);
      param_3[4] = *(undefined4 *)(param_1[7] + 0x10);
      param_3[5] = *(undefined4 *)(param_1[7] + 0x14);
    }
    break;
  case 0xc:
    uVar2 = 0;
    if ((*(ushort *)((int)param_1 + 0x16) & 1) != 0) {
      *(undefined4 *)(param_1[7] + 4) = 0;
      *(undefined4 *)param_1[7] = 0;
      *(undefined4 *)(param_1[7] + 8) = 0;
      *(undefined4 *)(param_1[7] + 0xc) = 0;
      *(undefined4 *)(param_1[7] + 0x10) = 0;
      *(undefined4 *)(param_1[7] + 0x14) = 0;
      uVar2 = uVar3;
    }
    break;
  case 0xd:
    uVar2 = 0;
    if ((*(ushort *)((int)param_1 + 0x16) & 2) == 0) {
      puVar1 = (undefined4 *)usmalloc(0x10,*param_1);
      param_1[8] = puVar1;
      if (puVar1 == (undefined4 *)0x0) {
        setoserror(0xc);
        return 0xffffffff;
      }
      *(ushort *)((int)param_1 + 0x16) = *(ushort *)((int)param_1 + 0x16) | 2;
      *puVar1 = 0xffffffff;
      *(undefined4 *)(param_1[8] + 4) = 0xffffffff;
      *(undefined4 *)(param_1[8] + 8) = 0xffffffff;
      *(undefined4 *)(param_1[8] + 0xc) = 0xffffffff;
      uVar2 = uVar3;
    }
    break;
  case 0xe:
    if ((*(ushort *)((int)param_1 + 0x16) & 2) == 0) {
      uVar2 = 0;
    }
    else {
      *(ushort *)((int)param_1 + 0x16) = *(ushort *)((int)param_1 + 0x16) & 0xfffd;
      usfree(param_1[8],*param_1);
    }
    break;
  case 0xf:
    if ((*(ushort *)((int)param_1 + 0x16) & 2) == 0) {
      setoserror(0x16);
      uVar2 = 0xffffffff;
    }
    else {
      *param_3 = *(undefined4 *)param_1[8];
      param_3[1] = *(undefined4 *)(param_1[8] + 4);
      param_3[2] = *(undefined4 *)(param_1[8] + 8);
      param_3[3] = *(undefined4 *)(param_1[8] + 0xc);
    }
    break;
  case 0x10:
    uVar2 = 0;
    if ((*(ushort *)((int)param_1 + 0x16) & 2) != 0) {
      *(undefined4 *)param_1[8] = 0xffffffff;
      *(undefined4 *)(param_1[8] + 4) = 0xffffffff;
      *(undefined4 *)(param_1[8] + 8) = 0xffffffff;
      *(undefined4 *)(param_1[8] + 0xc) = 0xffffffff;
      uVar2 = uVar3;
    }
    break;
  case 0x11:
    *(ushort *)((int)param_1 + 0x16) = *(ushort *)((int)param_1 + 0x16) | 4;
    break;
  case 0x12:
    *(ushort *)((int)param_1 + 0x16) = *(ushort *)((int)param_1 + 0x16) & 0xfffb;
    break;
  case 0x13:
    *(ushort *)((int)param_1 + 0x16) = *(ushort *)((int)param_1 + 0x16) | 0x10;
    break;
  case 0x14:
    *(ushort *)((int)param_1 + 0x16) = *(ushort *)((int)param_1 + 0x16) & 0xffef;
    break;
  default:
    setoserror(0x16);
    uVar2 = 0xffffffff;
  }
  return uVar2;
}



// === usdumpsema @ 0fb7eb24 (352 bytes) ===

undefined8 usdumpsema(int *param_1,FILE *param_2,undefined8 param_3)

{
  ushort uVar1;
  int iVar2;
  undefined4 *puVar3;
  undefined4 uVar4;
  undefined8 uVar5;
  
  iVar2 = *param_1;
  uVar5 = ustestsema(param_1);
  (*_lock)(*(undefined4 *)(iVar2 + 0x444));
  fprintf(param_2,s__s__SEMADUMP_of___x_count__d_0fbd0228,param_3,param_1,uVar5);
  if ((*(ushort *)((int)param_1 + 0x16) & 8) == 0) {
    fprintf(param_2,"\n");
    uVar1 = *(ushort *)((int)param_1 + 0x16);
  }
  else {
    fprintf(param_2,s_polling_dev___x_0fbd0248,param_1[6]);
    uVar1 = *(ushort *)((int)param_1 + 0x16);
  }
  if ((uVar1 & 1) != 0) {
    uVar5 = ustestsema(param_1);
    fprintf(param_2,s_semaphore_meter__value__d_nwait___0fbd0260,uVar5,
            *(undefined4 *)(param_1[7] + 0x10),*(undefined4 *)(param_1[7] + 0x14));
    puVar3 = (undefined4 *)param_1[7];
    fprintf(param_2,s_psemas__d_phits__d_vsemas__d_vno_0fbd0298,puVar3[1],*puVar3,puVar3[2],
            puVar3[3]);
    uVar1 = *(ushort *)((int)param_1 + 0x16);
  }
  if ((uVar1 & 2) == 0) {
    uVar4 = *(undefined4 *)(iVar2 + 0x444);
  }
  else {
    fprintf(param_2,s_semaphore_debug__owner_pid__d_ca_0fbd02d8,*(undefined4 *)param_1[8],
            ((undefined4 *)param_1[8])[1]);
    fprintf(param_2,s_last_pid__d_called_from_0x_p_0fbd0310,*(undefined4 *)(param_1[8] + 8),
            *(undefined4 *)(param_1[8] + 0xc));
    uVar4 = *(undefined4 *)(iVar2 + 0x444);
  }
  (*_ulock)(uVar4);
  return 0;
}



// === FUN_0fb7ec84 @ 0fb7ec84 (340 bytes) ===

/* WARNING: Instruction at (ram,0x0fb7eccc) overlaps instruction at (ram,0x0fb7ecc8)
    */

void FUN_0fb7ec84(int *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
                 undefined4 param_5,undefined4 param_6)

{
  int iVar1;
  undefined4 *puVar2;
  
  iVar1 = *param_1;
  if (((*(uint *)(iVar1 + 0x43c) == 0) || (*(uint *)(iVar1 + 0x434) <= *(uint *)(iVar1 + 0x43c))) &&
     (puVar2 = (undefined4 *)usmalloc(0x20,iVar1), puVar2 != (undefined4 *)0x0)) {
    (*_lock)(*(undefined4 *)(iVar1 + 0x440));
    *(int *)(iVar1 + 0x434) = *(int *)(iVar1 + 0x434) + 1;
  }
  else {
    (*_lock)(*(undefined4 *)(iVar1 + 0x440));
    *(int *)(iVar1 + 0x438) = *(int *)(iVar1 + 0x438) + 1;
    if (*(int *)(iVar1 + 0x434) == 0) {
      (*_ulock)(*(undefined4 *)(iVar1 + 0x440));
      return;
    }
    puVar2 = *(undefined4 **)(iVar1 + 0x42c);
    if (puVar2[6] == 0) {
      *(undefined4 *)(iVar1 + 0x430) = 0;
    }
    else {
      *(undefined4 *)(puVar2[6] + 0x1c) = 0;
    }
    *(undefined4 *)(iVar1 + 0x42c) = puVar2[6];
  }
  puVar2[6] = 0;
  puVar2[3] = param_6;
  puVar2[1] = param_2;
  *puVar2 = param_1;
  puVar2[2] = param_3;
  puVar2[5] = param_5;
  puVar2[4] = param_4;
  if (*(int *)(iVar1 + 0x42c) == 0) {
    *(undefined4 **)(iVar1 + 0x42c) = puVar2;
    *(undefined4 **)(iVar1 + 0x430) = puVar2;
    puVar2[7] = 0;
  }
  else {
    *(undefined4 **)(*(int *)(iVar1 + 0x430) + 0x18) = puVar2;
    puVar2[7] = *(undefined4 *)(iVar1 + 0x430);
    *(undefined4 **)(iVar1 + 0x430) = puVar2;
  }
  (*_ulock)(*(undefined4 *)(iVar1 + 0x440));
  return;
}



// === usdumphist @ 0fb7edd8 (352 bytes) ===

/* WARNING: Instruction at (ram,0x0fb7eeb8) overlaps instruction at (ram,0x0fb7eeb4)
    */

longlong usdumphist(undefined4 *param_1,FILE *param_2)

{
  undefined4 uVar1;
  longlong lVar2;
  int iVar3;
  undefined4 *puVar5;
  longlong lVar4;
  undefined4 *puStack_50;
  undefined4 uStack_4c;
  undefined4 uStack_48;
  longlong lStack_40;
  
  if (param_2 == (FILE *)0x0) {
    param_2 = (FILE *)(__iob + 0x20);
  }
  lVar2 = usconfig(7,*param_1,&puStack_50);
  if (lVar2 != 0) {
    return lVar2;
  }
  fprintf(param_2,s_History_current_0x_x_entries__d_e_0fbd0348,puStack_50,uStack_4c,uStack_48);
  fprintf(param_2,s_Backward_in_time_0fbd0378);
  lStack_40 = 0;
  while (lVar2 = (longlong)(int)puStack_50, lVar2 != 0) {
    if (param_1 == (undefined4 *)0x0) {
      uVar1 = puStack_50[3];
LAB_0fb7ee80:
      fprintf(param_2,s__s_sema___0x_p_pid__d_wpid__d_cp_0fbd0390,puStack_50[1] * 8 + 0xfbd0150,
              *puStack_50,puStack_50[2],puStack_50[4],puStack_50[5],uVar1);
      puStack_50 = (undefined4 *)puStack_50[7];
      lStack_40 = lVar2;
    }
    else {
      if ((undefined4 *)*puStack_50 == param_1) {
        uVar1 = puStack_50[3];
        goto LAB_0fb7ee80;
      }
      puStack_50 = (undefined4 *)puStack_50[7];
    }
  }
  fprintf(param_2,s_Forward_in_time_0fbd03c0);
  lVar2 = lStack_40;
  lVar4 = lStack_40;
  do {
    if (lVar4 == 0) {
      return lVar2;
    }
    puVar5 = (undefined4 *)lVar4;
    if (param_1 == (undefined4 *)0x0) {
      uVar1 = puVar5[3];
LAB_0fb7eee8:
      iVar3 = fprintf(param_2,s__s_sema___0x_p_pid__d_wpid__d_cp_0fbd0390,puVar5[1] * 8 + 0xfbd0150,
                      *puVar5,puVar5[2],puVar5[4],puVar5[5],uVar1);
      lVar2 = (longlong)iVar3;
      iVar3 = puVar5[6];
    }
    else {
      if ((undefined4 *)*puVar5 == param_1) {
        uVar1 = puVar5[3];
        goto LAB_0fb7eee8;
      }
      iVar3 = puVar5[6];
    }
    lVar4 = (longlong)iVar3;
  } while( true );
}



// === usnewpollsema @ 0fb7ef38 (216 bytes) ===

int usnewpollsema(int param_1)

{
  int iVar2;
  longlong lVar1;
  int iVar3;
  int iVar4;
  
  iVar2 = usnewsema(param_1);
  if (iVar2 == 0) {
    return 0;
  }
  *(undefined4 *)(iVar2 + 0x18) = 0;
  *(ushort *)(iVar2 + 0x16) = *(ushort *)(iVar2 + 0x16) | 8;
  lVar1 = usmalloc(*(int *)(param_1 + 0x424) * 2,param_1);
  *(int *)(iVar2 + 8) = (int)lVar1;
  if (lVar1 == 0) {
    usfreesema(iVar2,param_1);
    setoserror(0xc);
    return 0;
  }
  iVar3 = 0;
  if (0 < *(int *)(param_1 + 0x424)) {
    iVar4 = 0;
    do {
      *(undefined2 *)(*(int *)(iVar2 + 8) + iVar4) = 0xffff;
      iVar3 = iVar3 + 1;
      iVar4 = iVar4 + 2;
    } while (iVar3 < *(int *)(param_1 + 0x424));
  }
  return iVar2;
}



// === usfreepollsema @ 0fb7f010 (60 bytes) ===

undefined8 usfreepollsema(int param_1,undefined8 param_2)

{
  usfree(*(undefined4 *)(param_1 + 8));
  usfreesema(param_1,param_2);
  return 0;
}



// === usopenpollsema @ 0fb7f04c (1112 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

longlong usopenpollsema(int *param_1,__mode_t param_2)

{
  short sVar1;
  int iVar2;
  bool bVar3;
  bool bVar4;
  bool bVar5;
  bool bVar6;
  bool bVar7;
  longlong lVar8;
  int iVar10;
  longlong lVar9;
  int iVar11;
  stat sStack_150;
  int iStack_b8;
  int iStack_b4;
  int iStack_b0;
  int *piStack_ac;
  longlong lStack_a8;
  longlong lStack_a0;
  longlong lStack_98;
  
  bVar4 = false;
  bVar3 = false;
  bVar5 = false;
  bVar6 = false;
  bVar7 = false;
  iVar2 = *param_1;
  lVar8 = __usfastadd(iVar2);
  lStack_a0 = (longlong)(short)lVar8;
  if (lVar8 == -1) {
    return -1;
  }
  _usclean_tidmap(iVar2);
  lVar8 = uspsema(*(undefined4 *)(iVar2 + 0x458));
  if (lVar8 < 0) {
    return -1;
  }
  iVar10 = param_1[6];
  if ((iVar10 != 0) && (iVar11 = 0, 0 < *(int *)(iVar2 + 0x424))) {
    do {
      lVar8 = _usget_pid(iVar2,(short)iVar11);
      if (lVar8 == -1) {
        iVar10 = *(int *)(iVar2 + 0x424);
      }
      else {
        sVar1 = *(short *)(param_1[2] + iVar11 * 2);
        lVar8 = (longlong)sVar1;
        if (lVar8 == -1) {
          iVar10 = *(int *)(iVar2 + 0x424);
        }
        else {
          iVar10 = fstat((int)sVar1,&sStack_150);
          if (iVar10 == 0) {
            if ((sStack_150.st_uid & 0xf000) == 0x2000) {
              if (param_1[6] == sStack_150._40_4_) goto LAB_0fb7f43c;
              iVar10 = *(int *)(iVar2 + 0x424);
            }
            else {
              iVar10 = *(int *)(iVar2 + 0x424);
            }
          }
          else {
            iVar10 = *(int *)(iVar2 + 0x424);
          }
        }
      }
      iVar11 = iVar11 + 1;
    } while (iVar11 < iVar10);
    iVar10 = param_1[6];
  }
  if (iVar10 != 0) {
    iVar10 = open64(s__dev_usema_0fbd04a8,2);
    if ((longlong)iVar10 < 0) {
      lStack_98 = oserror();
      lVar8 = (longlong)iStack_b4;
      bVar3 = bVar4;
      bVar6 = true;
      goto LAB_0fb7f3f8;
    }
    iStack_b0 = param_1[6];
    piStack_ac = param_1;
    lStack_a8 = (longlong)iVar10;
    iVar10 = ioctl(iVar10,0x757302,&iStack_b0);
    lVar8 = (longlong)iVar10;
    if (-1 < lVar8) {
      if (_utrace != 0) {
        _uprint(0,s_TRACE__usopenpollsema_tid__d_pol_0fbd04f8,lStack_a0,param_1,param_1[6],lVar8);
      }
      close((int)lStack_a8);
      lStack_98 = (longlong)iStack_b8;
      bVar3 = bVar4;
      bVar6 = false;
      bVar7 = false;
      goto LAB_0fb7f3f8;
    }
    lVar9 = oserror();
    lStack_98 = lVar9;
    close((int)lStack_a8);
    if (lVar9 != 0x16) {
      if (_uerror != 0) {
        _uprint(1,s_usopenpollsema_ERROR_cannot_atta_0fbd04b8,param_1[6]);
      }
      bVar3 = bVar4;
      bVar6 = false;
      bVar7 = true;
      goto LAB_0fb7f3f8;
    }
    param_1[6] = 0;
    iStack_b8 = (int)lStack_98;
  }
  lStack_98 = (longlong)iStack_b8;
  iVar10 = open64(s__dev_usemaclone_0fbd03d8,2);
  lVar8 = (longlong)iVar10;
  if (lVar8 < 0) {
    lStack_98 = oserror();
    if (_uerror != 0) {
      _uprint(1,s_usopenpollsema_ERROR_pid__d_cann_0fbd03e8,_DAT_00200e00);
    }
    bVar3 = true;
  }
  else {
    iVar11 = fstat(iVar10,&sStack_150);
    if ((iVar11 == 0) && (iVar11 = fchmod(iVar10,param_2), -1 < iVar11)) {
      if (_utrace != 0) {
        _uprint(0,s_TRACE__usopenpollsema_tid__d_pol_0fbd0460,lStack_a0,param_1,sStack_150._40_4_,
                lVar8);
      }
      param_1[6] = sStack_150._40_4_;
    }
    else {
      lStack_98 = oserror();
      close(iVar10);
      if (_uerror != 0) {
        _uprint(1,s_usopenpollsema_ERROR_pid__d_cann_0fbd0420,_DAT_00200e00);
      }
      bVar5 = true;
    }
  }
LAB_0fb7f3f8:
  if ((((bVar3) || (bVar5)) || (bVar6)) || (bVar7)) {
    usvsema(*(undefined4 *)(iVar2 + 0x458));
    setoserror(lStack_98);
    return -1;
  }
  fcntl((int)lVar8,2,1);
LAB_0fb7f43c:
  lVar9 = lStack_a0;
  usvsema(*(undefined4 *)(iVar2 + 0x458));
  *(short *)(param_1[2] + (int)lVar9 * 2) = (short)lVar8;
  return lVar8;
}



// === usclosepollsema @ 0fb7f4a4 (716 bytes) ===

undefined8 usclosepollsema(int *param_1)

{
  short sVar1;
  int iVar2;
  bool bVar3;
  bool bVar4;
  longlong lVar5;
  int iVar7;
  undefined8 uVar6;
  uint uVar8;
  int iVar9;
  int iVar10;
  stat sStack_f0;
  longlong lStack_58;
  
  bVar4 = false;
  iVar2 = *param_1;
  lVar5 = _usgettid(iVar2);
  lStack_58 = lVar5;
  if (lVar5 == -1) {
    setoserror(0x16);
    return 0xffffffffffffffff;
  }
  _usclean_tidmap(iVar2);
  sVar1 = *(short *)(param_1[2] + (int)lVar5 * 2);
  if (sVar1 != -1) {
    iVar7 = close((int)sVar1);
    if (iVar7 != 0) {
      return 0xffffffffffffffff;
    }
    bVar4 = true;
    *(undefined2 *)(param_1[2] + (int)lStack_58 * 2) = 0xffff;
  }
  iVar7 = prctl(0xd,0);
  if (iVar7 < 0) {
    return 0;
  }
  lVar5 = uspsema(*(undefined4 *)(iVar2 + 0x458));
  if (lVar5 < 0) {
    return 0xffffffffffffffff;
  }
  iVar7 = *(int *)(iVar2 + 0x424);
  iVar10 = 0;
  if (0 < iVar7) {
    iVar9 = 0;
    do {
      if (*(short *)(param_1[2] + iVar9) != -1) {
        uVar6 = _usget_pid(iVar2,(short)iVar10);
        uVar8 = prctl(0xd,uVar6);
        bVar3 = true;
        if ((-1 < (int)uVar8) && (bVar3 = (uVar8 & 2) != 0, (uVar8 & 2) != 0)) {
          if (bVar4) {
            iVar7 = param_1[2];
          }
          else {
            iVar7 = close((int)*(short *)(param_1[2] + iVar9));
            bVar4 = true;
            if (iVar7 != 0) goto LAB_0fb7f73c;
            iVar7 = param_1[2];
          }
          *(undefined2 *)(iVar7 + iVar9) = 0xffff;
        }
        if (bVar3) {
          lVar5 = oserror();
          if (lVar5 == 3) {
            if (bVar4) {
              iVar7 = param_1[2];
            }
            else {
              iVar7 = fstat((int)*(short *)(param_1[2] + iVar9),&sStack_f0);
              if (iVar7 == 0) {
                if ((sStack_f0.st_uid & 0xf000) == 0x2000) {
                  if (param_1[6] == sStack_f0._40_4_) {
                    iVar7 = close((int)*(short *)(param_1[2] + iVar9));
                    bVar4 = true;
                    if (iVar7 != 0) {
LAB_0fb7f73c:
                      usvsema(*(undefined4 *)(iVar2 + 0x458));
                      return 0xffffffffffffffff;
                    }
                    iVar7 = param_1[2];
                  }
                  else {
                    iVar7 = param_1[2];
                  }
                }
                else {
                  iVar7 = param_1[2];
                }
              }
              else {
                iVar7 = param_1[2];
              }
            }
            *(undefined2 *)(iVar7 + iVar9) = 0xffff;
            iVar7 = *(int *)(iVar2 + 0x424);
          }
          else {
            iVar7 = *(int *)(iVar2 + 0x424);
          }
        }
        else {
          iVar7 = *(int *)(iVar2 + 0x424);
        }
      }
      iVar10 = iVar10 + 1;
      iVar9 = iVar9 + 2;
    } while (iVar10 < iVar7);
  }
  usvsema(*(undefined4 *)(iVar2 + 0x458));
  return 0;
}



// === FUN_0fb7f770 @ 0fb7f770 (528 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

short FUN_0fb7f770(int *param_1,short param_2)

{
  short sVar1;
  int iVar2;
  int iVar4;
  undefined8 uVar3;
  uint uVar5;
  int iVar6;
  int iVar7;
  stat sStack_100;
  
  iVar2 = *param_1;
  iVar4 = prctl(0xd,0);
  if (iVar4 < 0) {
    return -1;
  }
  iVar4 = *(int *)(iVar2 + 0x424);
  iVar7 = 0;
  if (0 < iVar4) {
    iVar6 = 0;
    do {
      sVar1 = *(short *)(param_1[2] + iVar6);
      if (sVar1 != -1) {
        uVar3 = _usget_pid(iVar2,(short)iVar7);
        uVar5 = prctl(0xd,uVar3);
        if ((int)uVar5 < 0) {
          iVar4 = *(int *)(iVar2 + 0x424);
        }
        else {
          if ((uVar5 & 2) != 0) {
            *(short *)(param_1[2] + param_2 * 2) = sVar1;
            if ((_utrace != 0) &&
               (((iVar4 = fstat((int)sVar1,&sStack_100), iVar4 != 0 ||
                 ((sStack_100.st_uid & 0xf000) != 0x2000)) || (param_1[6] != sStack_100._40_4_)))) {
              uVar3 = _usget_pid(iVar2,(short)iVar7);
              _uprint(0,s_usfindspfd_ERROR_pid__d_sharing_f_0fbd0540,_DAT_00200e00,uVar3,sVar1);
              return -1;
            }
            if (_utrace != 0) {
              _uprint(0,s_TRACE__giving_auto_open_to_tid___0fbd0590,param_2,param_1,sVar1);
            }
            return sVar1;
          }
          iVar4 = *(int *)(iVar2 + 0x424);
        }
      }
      iVar7 = iVar7 + 1;
      iVar6 = iVar6 + 2;
    } while (iVar7 < iVar4);
  }
  return -1;
}



// === _usblock @ 0fb7f980 (452 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 _usblock(int param_1,int param_2,undefined8 param_3,undefined8 param_4)

{
  bool bVar1;
  longlong lVar2;
  short sVar4;
  int iVar3;
  short sVar5;
  ulong __request;
  
  lVar2 = __usfastadd();
  sVar5 = (short)lVar2;
  if (lVar2 == -1) {
    if (_uerror != 0) {
      _uprint(0,s__s_ERROR_no_tid_for_process__d_0fbd05d0,param_4,_DAT_00200e00);
    }
    return 0xffffffffffffffff;
  }
  if ((param_2 == 0) || ((*(ushort *)(param_2 + 0x16) & 8) == 0)) {
    sVar4 = *(short *)(*(int *)(param_1 + 8) + sVar5 * 8 + 6);
    __request = 0x757303;
  }
  else {
    sVar4 = *(short *)(*(int *)(param_2 + 8) + sVar5 * 2);
    if (sVar4 == -1) {
      sVar4 = FUN_0fb7f770(param_2,sVar5);
    }
    __request = 0x757304;
  }
  do {
    bVar1 = false;
    iVar3 = ioctl(sVar4,__request);
    if (iVar3 != 0) {
      lVar2 = oserror();
      bVar1 = lVar2 == 4;
      if (lVar2 != 4) {
        if (_uerror != 0) {
          _uprint(1,s__s_ERROR_block_pid__d_tid__d_fd___0fbd05f0,param_4,_DAT_00200e00,sVar5,sVar4);
        }
        return 0xffffffffffffffff;
      }
    }
  } while (bVar1);
  return 0;
}



// === _usunblock @ 0fb7fb44 (396 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 _usunblock(int param_1,int param_2,undefined8 param_3,undefined8 param_4)

{
  longlong lVar1;
  short sVar3;
  int iVar2;
  short sVar4;
  ulong __request;
  
  lVar1 = __usfastadd();
  sVar4 = (short)lVar1;
  if (lVar1 == -1) {
    if (_uerror != 0) {
      _uprint(0,s__s_ERROR_no_tid_for_process__d_0fbd05d0,param_4,_DAT_00200e00);
    }
    return 0xffffffffffffffff;
  }
  if (param_2 != 0) {
    if ((*(ushort *)(param_2 + 0x16) & 8) != 0) {
      sVar3 = *(short *)(*(int *)(param_2 + 8) + sVar4 * 2);
      if (sVar3 == -1) {
        sVar3 = FUN_0fb7f770(param_2,sVar4);
      }
      __request = 0x757307;
      goto LAB_0fb7fc48;
    }
  }
  sVar3 = *(short *)(*(int *)(param_1 + 8) + sVar4 * 8 + 6);
  __request = 0x757306;
LAB_0fb7fc48:
  iVar2 = ioctl(sVar3,__request,param_3);
  if (iVar2 == 0) {
    return 0;
  }
  lVar1 = oserror();
  if (lVar1 == 3) {
    return 1;
  }
  if (_uerror != 0) {
    _uprint(1,s__s_ERROR_unblock_pid__d_tid__d_f_0fbd0618,param_4,_DAT_00200e00,sVar4,sVar3);
  }
  return 0xffffffffffffffff;
}



// === FUN_0fb7fcd0 @ 0fb7fcd0 (616 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0fb7fcd0(int param_1,int param_2,undefined8 param_3)

{
  short sVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  longlong lVar6;
  short *psVar7;
  longlong lVar8;
  longlong lVar9;
  int iVar10;
  
  lVar8 = (longlong)*(int *)(param_1 + 0x424);
  if (lVar8 < 1) {
    return;
  }
  lVar9 = 0;
  iVar3 = param_1 + 0x18;
  iVar5 = *(int *)(param_1 + 8);
  iVar4 = _utrace;
  do {
    iVar10 = (int)lVar9 * 8;
    iVar5 = *(int *)(iVar5 + iVar10);
    if ((iVar5 != param_2) && ((iVar4 != 0 || (iVar5 != -1)))) {
      iVar4 = _usfgetlock((int)lVar9 + 1,param_3);
      if (iVar4 == 0) {
        if (iVar5 != -1) {
          psVar7 = (short *)(*(int *)(param_1 + 0xc) +
                            (short)((ushort)*(undefined4 *)(param_1 + 0x10) & (ushort)iVar5) * 2);
          sVar1 = *psVar7;
          iVar4 = (int)sVar1;
          if (lVar9 == sVar1) {
            *psVar7 = *(short *)(*(int *)(param_1 + 8) + iVar4 * 8 + 4);
          }
          else {
            iVar2 = *(int *)(param_1 + 8);
            lVar8 = (longlong)sVar1;
            do {
              sVar1 = *(short *)(iVar4 * 8 + iVar2 + 4);
              lVar6 = (longlong)sVar1;
              iVar4 = (int)sVar1;
              if (lVar9 == lVar6) {
                *(undefined2 *)((short)lVar8 * 8 + iVar2 + 4) =
                     *(undefined2 *)(iVar4 * 8 + iVar2 + 4);
                break;
              }
              lVar8 = lVar6;
            } while (lVar6 != -1);
          }
          if (iVar5 < 0) {
LAB_0fb7fe30:
            iVar5 = *(int *)(param_1 + 8);
          }
          else {
            if (_utrace != 0) {
              _uprint(0,s_TRACE__Process__d_deleted_as_tid_0fbd0640,iVar5,lVar9,iVar3,param_2);
              goto LAB_0fb7fe30;
            }
            iVar5 = *(int *)(param_1 + 8);
          }
          *(undefined4 *)(iVar10 + iVar5) = 0xffffffff;
          *(undefined2 *)(*(int *)(param_1 + 8) + iVar10 + 6) = 0xffff;
          *(undefined2 *)(*(int *)(param_1 + 8) + iVar10 + 4) = 0xffff;
          *(undefined4 *)(param_1 + 0x464) = _DAT_00200e00;
        }
      }
      else {
        if (iVar4 < 0) {
          if (_uerror != 0) {
            _uprint(1,s_ERROR_Attempt_by__d_to_clean_tid_0fbd0698,param_2,iVar3);
          }
          return;
        }
        if ((_uerror != 0) && (iVar4 != iVar5)) {
          _uprint(0,s_ERROR_Process__d_in_slot__d_but_p_0fbd06d0,iVar5,lVar9,iVar4,iVar3);
        }
      }
      lVar8 = (longlong)*(int *)(param_1 + 0x424);
      iVar4 = _utrace;
    }
    lVar9 = (longlong)(short)((short)lVar9 + 1);
    if (lVar8 <= lVar9) {
      return;
    }
    iVar5 = *(int *)(param_1 + 8);
  } while( true );
}



// === FUN_0fb7ff3c @ 0fb7ff3c (132 bytes) ===

longlong FUN_0fb7ff3c(undefined8 param_1,longlong param_2)

{
  longlong lVar1;
  int iVar2;
  
  if (0 < param_2) {
    lVar1 = 0;
    do {
      iVar2 = (int)lVar1 + 1;
      lVar1 = _usfgetlock(iVar2,param_1);
      if (lVar1 != 0) {
        return lVar1;
      }
      lVar1 = (longlong)(short)iVar2;
    } while (lVar1 < param_2);
  }
  return 0;
}



// === _usget_pid @ 0fb7ffc0 (28 bytes) ===

undefined4 _usget_pid(int param_1,short param_2)

{
  return *(undefined4 *)(*(int *)(param_1 + 8) + param_2 * 8);
}



// === _usgettid @ 0fb7ffdc (544 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

short _usgettid(int param_1)

{
  short sVar1;
  int iVar2;
  uint uVar3;
  longlong lVar4;
  int *piVar5;
  uint *puVar6;
  undefined1 auStack_50 [24];
  longlong lStack_38;
  
  uVar3 = _DAT_00200e00;
  sVar1 = *(short *)(*(int *)(param_1 + 0xc) + (*(uint *)(param_1 + 0x10) & _DAT_00200e00) * 2);
  while (sVar1 != -1) {
    puVar6 = (uint *)(sVar1 * 8 + *(int *)(param_1 + 8));
    if (*puVar6 == _DAT_00200e00) {
      return sVar1;
    }
    sVar1 = *(short *)(puVar6 + 1);
  }
  if (DAT_0fbd3a68 == (int *)0x0) {
LAB_0fb80100:
    lStack_38 = -1;
  }
  else {
    iVar2 = DAT_0fbd3a68[1];
    piVar5 = DAT_0fbd3a68;
    while (iVar2 != param_1) {
      piVar5 = (int *)piVar5[2];
      if (piVar5 == (int *)0x0) goto LAB_0fb80100;
      iVar2 = piVar5[1];
    }
    lStack_38 = (longlong)*piVar5;
    if (lStack_38 != -1) {
      __usblockallsigs(auStack_50);
      lVar4 = _usfsplock(0,lStack_38);
      if (lVar4 == -1) {
        __usunblockallsigs(auStack_50);
        if (_uerror != 0) {
          _uprint(1,s_ERROR_Process__d_attempt_to_grab_0fbd0750,uVar3,param_1 + 0x18);
        }
        return -1;
      }
      sVar1 = *(short *)(*(int *)(param_1 + 0xc) + (*(uint *)(param_1 + 0x10) & uVar3) * 2);
      while( true ) {
        if (sVar1 == -1) {
          _usfunsplock(0,lStack_38);
          __usunblockallsigs(auStack_50);
          return -1;
        }
        puVar6 = (uint *)(sVar1 * 8 + *(int *)(param_1 + 8));
        if (*puVar6 == uVar3) break;
        sVar1 = *(short *)(puVar6 + 1);
      }
      _usfunsplock(0,lStack_38);
      __usunblockallsigs(auStack_50);
      return sVar1;
    }
  }
  if (_uerror != 0) {
    _uprint(0,s_ERROR_Process__d_arena_0x_x_neve_0fbd0710,_DAT_00200e00,param_1);
  }
  return -1;
}



// === _usclean_tidmap @ 0fb801fc (424 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 _usclean_tidmap(int param_1)

{
  int iVar1;
  longlong lVar2;
  short sVar3;
  int *piVar4;
  int iVar5;
  undefined1 auStack_50 [16];
  undefined2 uStack_40;
  longlong lStack_30;
  
  lStack_30 = (longlong)_DAT_00200e00;
  if (DAT_0fbd3a68 != (int *)0x0) {
    iVar1 = DAT_0fbd3a68[1];
    piVar4 = DAT_0fbd3a68;
    while (iVar1 != param_1) {
      piVar4 = (int *)piVar4[2];
      if (piVar4 == (int *)0x0) goto LAB_0fb802c0;
      iVar1 = piVar4[1];
    }
    iVar1 = *piVar4;
    if (iVar1 != -1) {
      __usblockallsigs(auStack_50);
      lVar2 = _usfsplock(0,iVar1);
      if (lVar2 == -1) {
        __usunblockallsigs(auStack_50);
        if (_uerror != 0) {
          _uprint(1,s_ERROR_Process__d_attempt_to_chec_0fbd07e0,lStack_30,param_1 + 0x18);
        }
        return 0xffffffffffffffff;
      }
      FUN_0fb7fcd0(param_1,lStack_30,iVar1);
      uStack_40 = 0;
      if (0 < (longlong)*(int *)(param_1 + 0x424)) {
        lVar2 = 0;
        iVar5 = 0;
        do {
          sVar3 = (short)lVar2 + 1;
          if (*(int *)(iVar5 + *(int *)(param_1 + 8)) == -1) {
            _usfunsplock(0,iVar1);
            __usunblockallsigs(auStack_50);
            return 1;
          }
          lVar2 = (longlong)sVar3;
          iVar5 = (int)sVar3 << 3;
        } while (lVar2 < *(int *)(param_1 + 0x424));
      }
      _usfunsplock(0,iVar1);
      __usunblockallsigs(auStack_50);
      setoserror(0x1c);
      return 0xffffffffffffffff;
    }
  }
LAB_0fb802c0:
  if (_uerror != 0) {
    _uprint(0,s_ERROR_Process__d_arena__0x_x_nev_0fbd0798,lStack_30,param_1);
  }
  setoserror(3);
  return 0xffffffffffffffff;
}



// === _usany_tids_left @ 0fb803a4 (224 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

longlong _usany_tids_left(int param_1)

{
  int iVar1;
  longlong lVar2;
  short sVar4;
  longlong lVar3;
  int unaff_gp_lo;
  
  if (0 < (longlong)*(int *)(param_1 + 0x424)) {
    lVar2 = 0;
    iVar1 = 0;
    do {
      sVar4 = (short)lVar2 + 1;
      if (*(int *)(iVar1 + *(int *)(param_1 + 8)) == -1) {
        return 0;
      }
      lVar2 = (longlong)sVar4;
      iVar1 = (int)sVar4 << 3;
    } while (lVar2 < *(int *)(param_1 + 0x424));
  }
  lVar2 = _usclean_tidmap(param_1);
  if (lVar2 == -1) {
    lVar3 = oserror();
    if ((lVar3 == 0x1c) && (*(int *)(unaff_gp_lo + -0x76e4) != 0)) {
      _uprint(0,s_ERROR_Process__d_no_tids_left_in_0fbd0820,_DAT_00200e00,param_1 + 0x18,
              *(undefined4 *)(param_1 + 0x420),*(undefined4 *)(param_1 + 0x424));
    }
  }
  return lVar2;
}



// === FUN_0fb80484 @ 0fb80484 (908 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

longlong FUN_0fb80484(int param_1,undefined8 param_2,int param_3)

{
  bool bVar1;
  undefined4 uVar2;
  int iVar3;
  bool bVar4;
  longlong lVar5;
  longlong lVar6;
  int iVar7;
  int *piVar8;
  undefined4 *puVar9;
  short sVar10;
  longlong lVar11;
  undefined4 uStack_90;
  
  bVar4 = false;
  bVar1 = false;
  if (param_3 < 0) {
    if (DAT_0fbd3a68 != (int *)0x0) {
      iVar7 = DAT_0fbd3a68[1];
      piVar8 = DAT_0fbd3a68;
      while (iVar7 != param_1) {
        piVar8 = (int *)piVar8[2];
        if (piVar8 == (int *)0x0) goto LAB_0fb804d4;
        iVar7 = piVar8[1];
      }
      param_3 = *piVar8;
    }
LAB_0fb804d4:
    if (param_3 == -1) {
      bVar4 = true;
      uStack_90 = 3;
    }
    else {
      lVar5 = _usfsplock(0,param_3);
      bVar1 = lVar5 == -1;
      if (bVar1) {
        uStack_90 = oserror();
      }
      bVar4 = false;
    }
  }
  if ((!bVar4) && (!bVar1)) {
    lVar5 = FUN_0fb817ac(param_1,param_3);
    if (lVar5 == -1) {
      uStack_90 = oserror();
      _usfunsplock(0,param_3);
      if (_uerror != 0) {
        _uprint(1,s_ERROR_Process__d_could_not_get_r_0fbd0870,_DAT_00200e00);
      }
    }
    else {
      FUN_0fb7fcd0(param_1,param_2,param_3);
      if (0 < (longlong)*(int *)(param_1 + 0x424)) {
        lVar11 = 0;
        iVar7 = 0;
        do {
          if (*(int *)(*(int *)(param_1 + 8) + iVar7) == -1) {
            lVar6 = _usfsplock((int)lVar11 + 1,param_3);
            if (lVar6 != -1) {
              uVar2 = *(undefined4 *)(param_1 + 0x10);
              *(int *)(*(int *)(param_1 + 8) + iVar7) = (int)param_2;
              *(short *)(*(int *)(param_1 + 8) + iVar7 + 6) = (short)lVar5;
              iVar3 = (short)((ushort)uVar2 & (ushort)param_2) * 2;
              *(undefined2 *)(*(int *)(param_1 + 8) + iVar7 + 4) =
                   *(undefined2 *)(*(int *)(param_1 + 0xc) + iVar3);
              *(short *)(*(int *)(param_1 + 0xc) + iVar3) = (short)lVar11;
              if (_utrace != 0) {
                puVar9 = (undefined4 *)(*(int *)(param_1 + 8) + iVar7);
                _uprint(0,s_TRACE__Process__d_added_as_tid___0fbd08a8,*puVar9,lVar11,
                        *(undefined2 *)((int)puVar9 + 6),param_1 + 0x18,
                        *(undefined4 *)(param_1 + 0x420));
              }
              _usfunsplock(0,param_3);
              return lVar11;
            }
            uStack_90 = oserror();
            _usfunsplock(0,param_3);
            goto LAB_0fb807d4;
          }
          sVar10 = (short)lVar11 + 1;
          lVar11 = (longlong)sVar10;
          iVar7 = (int)sVar10 << 3;
        } while (lVar11 < *(int *)(param_1 + 0x424));
      }
      _usfunsplock(0,param_3);
      uStack_90 = 0x1c;
      if (_uerror != 0) {
        _uprint(0,s_ERROR_Process__d_no_tids_left_in_0fbd08e8,_DAT_00200e00,param_1 + 0x18,
                *(undefined4 *)(param_1 + 0x424));
      }
    }
  }
LAB_0fb807d4:
  if (_uerror != 0) {
    _uprint(1,s_ERROR_Process__d_attempt_to_add_t_0fbd0930,param_2,param_1 + 0x18,
            *(undefined4 *)(param_1 + 0x420));
  }
  setoserror(uStack_90);
  return -1;
}



// === _usinit_mapfile @ 0fb80810 (864 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 *
_usinit_mapfile(undefined8 param_1,undefined8 param_2,timespec param_3,undefined8 param_4,
               undefined8 param_5,longlong param_6,longlong param_7,undefined4 *param_8)

{
  undefined4 uVar1;
  longlong lVar2;
  int iVar3;
  undefined4 *__addr;
  int __fd;
  size_t sVar4;
  uint __flags;
  size_t sVar5;
  stat sStack_170;
  undefined1 auStack_d8 [2];
  undefined2 uStack_d6;
  ulonglong uStack_d0;
  undefined8 uStack_c8;
  longlong lStack_a8;
  undefined8 uStack_a0;
  longlong lStack_98;
  undefined8 uStack_90;
  undefined8 uStack_88;
  longlong lStack_80;
  ulonglong uStack_78;
  
  lStack_98 = 0;
  uStack_78 = 0;
  __fd = (int)param_1;
  uStack_a0 = param_5;
  uStack_90 = param_4;
  uStack_88 = param_2;
  lStack_80 = param_7;
  fstat(__fd,&sStack_170);
  sVar5 = param_3.tv_nsec;
  if ((sStack_170.st_uid & 0xf000) == 0x8000) {
    sVar4 = sStack_170.st_atim.tv_nsec;
    if ((longlong)param_3 < (longlong)sStack_170.st_atim) {
      ftruncate64(__fd,sVar5);
      sStack_170.st_atim = param_3;
      sVar4 = sVar5;
    }
    lVar2 = FUN_0fb82000(param_1,sVar4);
    if (lVar2 != 0) {
      if (_uerror != 0) {
        _uprint(1,s_usinit_ERROR_unable_to_zero_aren_0fbd0970);
      }
      return (undefined4 *)0x0;
    }
    if (param_6 == 0) {
      uStack_78 = (ulonglong)(int)sVar5;
    }
    else if ((longlong)sStack_170.st_atim < 0x1470) {
      uStack_78 = 0x1470;
    }
    if (uStack_78 != 0) {
      uStack_c8 = 0;
      uStack_d6 = 0;
      uStack_d0 = uStack_78 & 0xffffffff;
      iVar3 = fcntl(__fd,10,auStack_d8);
      if (iVar3 != 0) {
        if (_uerror != 0) {
          _uprint(1,s_usinit_ERROR_unable_to_grow_aren_0fbd0998,uStack_78);
        }
        return (undefined4 *)0x0;
      }
    }
  }
  lStack_a8 = (longlong)(int)sVar5;
  __flags = 1;
  if (param_6 != 0) {
    __flags = 0x41;
  }
  if (lStack_80 != 0) {
    __flags = __flags | 0x100;
  }
  if (param_8 == (undefined4 *)0xffffffff) {
    param_8 = (undefined4 *)0x0;
  }
  else {
    lStack_98 = 1;
  }
  __addr = mmap64(param_8,sVar5,3,__flags,__fd,0);
  if (__addr != (undefined4 *)0xffffffff) {
    sVar5 = (size_t)lStack_a8;
    if ((lStack_98 != 0) && (__addr != param_8)) {
      munmap(__addr,sVar5);
      if (_uerror != 0) {
        _uprint(0,s_usinit_ERROR_Process__d_virtual_a_0fbd0a00,_DAT_00200e00,param_8);
      }
      setoserror(0x10);
      return (undefined4 *)0x0;
    }
    __addr[1] = 5;
    __addr[0x108] = __addr;
    __addr[0x114] = (int)uStack_88;
    __addr[0x113] = (int)uStack_90;
    uVar1 = _DAT_00200e00;
    *(timespec *)(__addr + 0x106) = param_3;
    __addr[0x109] = (int)uStack_a0;
    *__addr = 0xdeadbabe;
    __addr[0x118] = uVar1;
    lVar2 = FUN_0fb819a0(__addr,uStack_a0,lStack_a8,(int)param_6 + (int)lStack_80);
    if (lVar2 != 0) {
      return __addr;
    }
    if (_uerror != 0) {
      _uprint(1,s_usinit_ERROR_failed_to_initializ_0fbd0a40);
    }
    munmap(__addr,sVar5);
    return (undefined4 *)0x0;
  }
  if (_uerror != 0) {
    _uprint(1,s_usinit_ERROR_Process__d_mmap_fai_0fbd09d0,_DAT_00200e00,param_8);
  }
  return (undefined4 *)0x0;
}



// === usinit @ 0fb80b74 (2324 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int * usinit(undefined8 param_1)

{
  bool bVar1;
  size_t __len;
  int *piVar2;
  char *pcVar5;
  int iVar6;
  longlong lVar3;
  int iVar7;
  char *pcVar8;
  int __fd;
  undefined8 uVar4;
  undefined4 uVar9;
  int *piVar10;
  __uid_t _Var11;
  int iVar12;
  undefined4 *puVar13;
  int *piVar14;
  size_t sVar15;
  int iVar16;
  undefined *puVar17;
  int iStack_11e0;
  int iStack_11dc;
  undefined8 uStack_dc8;
  size_t sStack_dc4;
  int *piStack_dc0;
  undefined4 uStack_dbc;
  stat sStack_1e0;
  stat sStack_148;
  undefined1 auStack_b0 [16];
  ulonglong uStack_a0;
  longlong lStack_98;
  longlong lStack_90;
  ulonglong uStack_88;
  
  iVar16 = 0;
  lStack_98 = (longlong)_us_locktype;
  __fd = -1;
  uStack_a0 = (ulonglong)_us_arenatype;
  lStack_90 = (longlong)_DAT_00200e00;
  pcVar5 = getenv("USTRACE");
  if (pcVar5 != (char *)0x0) {
    _uerror = 1;
    _utrace = 1;
    uStack_a0 = uStack_a0 | 2;
  }
  pcVar5 = getenv("USERROR");
  if (pcVar5 != (char *)0x0) {
    _uerror = 1;
  }
  if (DAT_0fbde454 != 0) {
    DAT_0fbde454 = 0;
    pcVar5 = getenv(s_USTRACEFILE_0fbd0a70);
    if ((pcVar5 != (char *)0x0) && (iVar6 = open64(pcVar5,0x10a,0x1b6), -1 < iVar6)) {
      _utracefd = iVar6;
    }
  }
  if (_us_systype == 0) {
    lVar3 = _getsystype();
    _us_systype = (uint)lVar3;
    if (lVar3 == 0) {
      setoserror(0x16);
      return (int *)0x0;
    }
  }
  pcVar5 = (char *)param_1;
  iVar6 = strcmp(pcVar5,s__dev_zero_0fbd0a80);
  if ((iVar6 != 0) && (DAT_0fbd3a68 != (int *)0x0)) {
    pcVar8 = *(char **)((int)DAT_0fbd3a68 + 0xc);
    iVar6 = (int)DAT_0fbd3a68;
    while (iVar7 = strcmp(pcVar8,pcVar5), iVar7 != 0) {
      iVar6 = *(int *)(iVar6 + 8);
      if (iVar6 == 0) goto LAB_0fb80dbc;
      pcVar8 = *(char **)(iVar6 + 0xc);
    }
    piVar10 = *(int **)(iVar6 + 4);
    if ((*piVar10 == -0x21524542) && (piVar10[1] == 5)) {
      if (_utrace != 0) {
        _uprint(0,s_TRACE__Process__d_already_mapped_0fbd0a90,_DAT_00200e00,param_1);
      }
      lVar3 = usadd(piVar10);
      if (lVar3 != 0) {
        return (int *)0x0;
      }
      return piVar10;
    }
  }
LAB_0fb80dbc:
  iVar6 = strcmp(pcVar5,s__dev_zero_0fbd0a80);
  if (iVar6 == 0) {
    pcVar8 = tempnam((char *)0x0,"uslock");
    __fd = open64(pcVar8,0x102,0x180);
    if (__fd == -1) {
      uVar4 = oserror();
      if (_uerror != 0) {
        _uprint(1,s_usinit_ERROR_Process__d_open_on___0fbd0ac0,_DAT_00200e00,pcVar8);
      }
      setoserror(uVar4);
      return (int *)0x0;
    }
    unlink(pcVar8);
    fcntl(__fd,2,1);
  }
  uStack_88 = ZEXT48(s_usinit_ERROR_Process__d_virtual_a_0fbd0a00);
  do {
    bVar1 = false;
    iVar6 = open64(pcVar5,2);
    if (iVar6 == -1) {
      iVar6 = open64(pcVar5,0x102,0x180);
      if (iVar6 == -1) {
        uVar4 = oserror();
        if (_uerror != 0) {
          _uprint(1,s_usinit_ERROR_Process__d_open_on___0fbd0ac0,_DAT_00200e00,param_1);
        }
        setoserror(uVar4);
        return (int *)0x0;
      }
      iVar16 = iVar16 + 1;
    }
    fcntl(iVar6,2,1);
    iVar7 = iVar6;
    if (-1 < __fd) {
      iVar7 = __fd;
    }
    lVar3 = _usfsplock(0,iVar7);
    if (lVar3 == -1) {
      uVar9 = oserror();
      if (_uerror != 0) {
        _uprint(1,s_usinit_ERROR_Process__d_file_loc_0fbd0af0,_DAT_00200e00,param_1);
      }
      goto LAB_0fb80f9c;
    }
    blkclr(&iStack_11e0,0x1000);
    read(iVar6,&iStack_11e0,0x1000);
    if (iStack_11e0 == -0x21524542) {
      lVar3 = FUN_0fb7ff3c(iVar7,uStack_dbc);
      if (lVar3 != 0) {
        if (_utrace != 0) {
          _uprint(0,s_TRACE__Process__d_joining_existi_0fbd0b20,_DAT_00200e00,param_1,piStack_dc0);
        }
        if (iStack_11dc == 5) {
          piVar10 = mmap64(piStack_dc0,sStack_dc4,3,0x41,iVar6,0);
          if (piVar10 == (int *)0xffffffff) {
            uVar9 = oserror();
            if (_uerror != 0) {
              _uprint(1,s_usinit_ERROR_mmap_failed___0x_x_0fbd0b78,piStack_dc0);
            }
            goto LAB_0fb80f9c;
          }
          if (piStack_dc0 == piVar10) {
            _usr4klocks_init(piVar10[0x113],_us_systype);
            __len = sStack_dc4;
            goto LAB_0fb8134c;
          }
          munmap(piVar10,sStack_dc4);
          if (_uerror != 0) {
            _uprint(0,uStack_88,_DAT_00200e00,piStack_dc0);
          }
          uVar9 = 0x10;
        }
        else {
          if (_uerror != 0) {
            _uprint(0,s_usinit_ERROR_Version_Mismatch_0fbd0b58);
          }
          uVar9 = 0x16;
        }
        goto LAB_0fb80f9c;
      }
    }
    if (_utrace != 0) {
      _uprint(0,s_TRACE__Process__d_creating_new_a_0fbd0b98,_DAT_00200e00,param_1,lStack_98,
              _us_maxusers);
    }
    if (((iVar16 == 0) && (fstat(iVar6,&sStack_1e0), (sStack_1e0.st_uid & 0xf000) == 0x8000)) &&
       (_Var11 = geteuid(), sStack_1e0.st_rdev._0_4_ != _Var11)) {
      if (_utrace != 0) {
        _uprint(0,s_TRACE__Process__d_arena_not_owne_0fbd0bd8,_DAT_00200e00,param_1);
      }
      iVar12 = unlink(pcVar5);
      if (iVar12 != 0) {
        if (_uerror != 0) {
          _uprint(1,s_ERROR_usinit_not_owner_and_canno_0fbd0c28,param_1);
        }
        puVar13 = (undefined4 *)__oserror();
        uVar9 = *puVar13;
        goto LAB_0fb80f9c;
      }
      _usfunsplock(0,iVar7);
      close(iVar6);
      bVar1 = true;
    }
  } while (bVar1);
  _usr4klocks_init(lStack_98,_us_systype);
  if (_utrace != 0) {
    puVar17 = &DAT_0fbde1f0;
    if ((_us_systype & 2) != 0) {
      puVar17 = &DAT_0fbde1e8;
    }
    _uprint(0,s_TRACE__Process__d_Initializing_R_0fbd0c58,_DAT_00200e00,puVar17);
  }
  __len = _us_mapsize;
  piVar10 = (int *)_usinit_mapfile(iVar6,uStack_a0,_us_mapsize,lStack_98,_us_maxusers,_us_autogrow,
                                   _us_autoresv,_us_attachaddr);
  if (piVar10 == (int *)0x0) {
    uVar9 = oserror();
    fstat(iVar6,&sStack_148);
    if ((sStack_148.st_uid & 0xf000) == 0x8000) {
      iVar16 = iVar16 + 1;
    }
  }
  else {
    strcpy((char *)(piVar10 + 6),pcVar5);
    lVar3 = FUN_0fb81690(piVar10);
    if (lVar3 < 0) {
      uVar9 = oserror();
      munmap(piVar10,__len);
    }
    else {
      msync(piVar10,0x470,2);
LAB_0fb8134c:
      if (((uStack_a0 & 1) != 0) && (iVar16 != 0)) {
        unlink(pcVar5);
      }
      piVar14 = (int *)malloc(0x10);
      *piVar14 = iVar7;
      piVar14[1] = (int)piVar10;
      sVar15 = strlen(pcVar5);
      pcVar8 = (char *)malloc(sVar15 + 1);
      piVar14[3] = (int)pcVar8;
      strcpy(pcVar8,pcVar5);
      piVar2 = piVar14;
      piVar14[2] = (int)DAT_0fbd3a68;
      DAT_0fbd3a68 = piVar2;
      __usblockallsigs(auStack_b0);
      lVar3 = FUN_0fb80484(piVar10,lStack_90,iVar7);
      if (lVar3 != -1) {
        __usunblockallsigs(auStack_b0);
        if (_utrace != 0) {
          _uprint(0,s_TRACE__Process__d_usinit_succede_0fbd0c88,_DAT_00200e00,param_1,piVar10);
        }
        if (iVar6 != iVar7) {
          close(iVar6);
        }
        return piVar10;
      }
      uVar9 = oserror();
      __usunblockallsigs(auStack_b0);
      DAT_0fbd3a68 = (int *)piVar14[2];
      free(piVar14[3]);
      free(piVar14);
      munmap(piVar10,__len);
    }
  }
LAB_0fb80f9c:
  if (iVar16 != 0) {
    unlink(pcVar5);
  }
  if (-1 < iVar6) {
    close(iVar6);
  }
  if (-1 < __fd) {
    close(__fd);
  }
  setoserror(uVar9);
  return (int *)0x0;
}



// === usadd @ 0fb8148c (208 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 usadd(undefined8 param_1)

{
  undefined4 uVar1;
  longlong lVar2;
  undefined8 uVar3;
  undefined1 auStack_40 [16];
  
  uVar1 = _DAT_00200e00;
  uVar3 = 0;
  __usblockallsigs(auStack_40);
  lVar2 = _usgettid(param_1);
  if (lVar2 == -1) {
    lVar2 = oserror();
    if (lVar2 == 0x2e) {
      uVar3 = 0xffffffffffffffff;
    }
    else {
      lVar2 = FUN_0fb80484(param_1,uVar1,0xffffffffffffffff);
      if (lVar2 == -1) {
        uVar3 = oserror();
        if (_uerror != 0) {
          _uprint(1,s_usadd_ERROR_Process__d_failed_to_0fbd0cc0,uVar1,param_1);
        }
        setoserror(uVar3);
        uVar3 = 0xffffffffffffffff;
      }
    }
  }
  __usunblockallsigs(auStack_40);
  return uVar3;
}



// === __usfastadd @ 0fb8155c (228 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

longlong __usfastadd(undefined8 param_1)

{
  longlong lVar1;
  undefined1 auStack_50 [24];
  longlong lStack_38;
  longlong lStack_30;
  undefined8 uStack_28;
  undefined8 uStack_20;
  
  lStack_30 = (longlong)_DAT_00200e00;
  uStack_28 = param_1;
  lVar1 = _usgettid();
  if (lVar1 != -1) {
    return (longlong)(short)lVar1;
  }
  __usblockallsigs(auStack_50);
  lVar1 = _usgettid(uStack_28);
  lStack_38 = (longlong)(short)lVar1;
  if (lVar1 == -1) {
    lVar1 = oserror();
    if (lVar1 != 0x2e) {
      lVar1 = FUN_0fb80484(uStack_28,lStack_30,0xffffffffffffffff);
      lStack_38 = (longlong)(short)lVar1;
      if (lVar1 == -1) {
        uStack_20 = oserror();
        if (_uerror != 0) {
          _uprint(1,s_usadd_ERROR_Process__d_failed_to_0fbd0cc0,lStack_30,uStack_28);
        }
        setoserror(uStack_20);
      }
    }
  }
  __usunblockallsigs(auStack_50);
  return lStack_38;
}



// === _usretsfd @ 0fb81640 (80 bytes) ===

undefined2 _usretsfd(int param_1)

{
  longlong lVar1;
  
  lVar1 = _usgettid();
  if (lVar1 == -1) {
    return 0xffff;
  }
  return *(undefined2 *)(*(int *)(param_1 + 8) + (short)lVar1 * 8 + 6);
}



// === FUN_0fb81690 @ 0fb81690 (284 bytes) ===

int FUN_0fb81690(int param_1)

{
  int __fd;
  int iVar1;
  stat sStack_d0;
  
  __fd = open64(s__dev_usemaclone_0fbd0cf8,2,param_1);
  if (__fd < 0) {
    if (_uerror != 0) {
      _uprint(1,s_usinit_ERROR_Cannot_open__s_0fbd0d08,s__dev_usemaclone_0fbd0cf8);
    }
    return -1;
  }
  fcntl(__fd,2,1,__fd);
  fchmod(__fd,0x180);
  iVar1 = fstat(__fd,&sStack_d0);
  if (iVar1 != 0) {
    if (_uerror != 0) {
      _uprint(1,s_usinit_ERROR_Cannot_allocate_dev_0fbd0d28);
    }
    close(__fd);
    return -1;
  }
  if (_utrace != 0) {
    _uprint(0,s_TRACE__arena___0x_x_gets_semapho_0fbd0d58,param_1,sStack_d0._40_4_,__fd);
  }
  *(undefined4 *)(param_1 + 0x45c) = sStack_d0._40_4_;
  return __fd;
}



// === FUN_0fb817ac @ 0fb817ac (496 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_0fb817ac(int param_1,undefined8 param_2)

{
  int iVar2;
  int iVar3;
  int iVar4;
  longlong lVar1;
  int unaff_gp_lo;
  stat sStack_e0;
  undefined4 auStack_48 [2];
  
  iVar2 = getdtablehi();
  iVar4 = 0;
  if (0 < iVar2) {
    do {
      iVar3 = fstat(iVar4,&sStack_e0);
      if (((iVar3 == 0) && ((sStack_e0.st_uid & 0xf000) == 0x2000)) &&
         (*(int *)(param_1 + 0x45c) == sStack_e0._40_4_)) {
        return iVar4;
      }
      iVar4 = iVar4 + 1;
    } while (iVar4 < iVar2);
  }
  iVar2 = open64(s__dev_usema_0fbd0d90,2);
  if (iVar2 < 0) {
    if (*(int *)(unaff_gp_lo + -0x76e4) != 0) {
      _uprint(1,s_usinit_ERROR_Cannot_open__s_0fbd0d08,s__dev_usema_0fbd0d90);
    }
    return -1;
  }
  auStack_48[0] = *(undefined4 *)(param_1 + 0x45c);
  iVar4 = ioctl(iVar2,0x757302,auStack_48);
  close(iVar2);
  lVar1 = FUN_0fb7ff3c(param_2,*(undefined4 *)(param_1 + 0x424));
  if (lVar1 != 0) {
    if (iVar4 < 0) {
      if (*(int *)(unaff_gp_lo + -0x76e4) != 0) {
        _uprint(1,s_usinit_ERROR_Cannot_attach_to_se_0fbd0dd8,*(undefined4 *)(param_1 + 0x45c),lVar1
               );
      }
      return -1;
    }
    fcntl(iVar4,2,1);
    return iVar4;
  }
  if (*(int *)(unaff_gp_lo + -0x76e0) != 0) {
    _uprint(0,s_TRACE__Process__d_arena_went_emp_0fbd0da0,_DAT_00200e00);
  }
  if (iVar4 != 0) {
    close(iVar4);
  }
  iVar2 = FUN_0fb81690(param_1);
  return iVar2;
}



// === FUN_0fb819a0 @ 0fb819a0 (980 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 FUN_0fb819a0(int param_1,ulonglong param_2,uint param_3,longlong param_4)

{
  longlong lVar1;
  int iVar2;
  int iVar3;
  longlong lVar4;
  undefined8 uVar5;
  ulonglong uVar6;
  uint uVar7;
  uint uVar8;
  
  if (_utrace != 0) {
    _uprint(0,s_TRACE__Process__d_header_size__d_0fbd0e18,_DAT_00200e00,0x470,param_3 - 0x470);
  }
  if (param_4 == 0) {
    uVar5 = 4;
  }
  else {
    uVar5 = 0;
  }
  lVar1 = acreate(param_1 + 0x470,param_3 - 0x470,uVar5,param_1,0);
  *(int *)(param_1 + 0x14) = (int)lVar1;
  if (lVar1 == 0) {
    if (_uerror != 0) {
      _uprint(1,s_usinit_ERROR_unable_to_create_us_0fbd0e50);
    }
    return 0;
  }
  lVar4 = 0;
  if (param_3 < 0x10000) {
    iVar2 = amallopt(1,0,lVar1);
    iVar3 = amallopt(5,0x200,*(undefined4 *)(param_1 + 0x14));
    if (iVar3 + iVar2 != 0) {
LAB_0fb81b0c:
      if (_uerror != 0) {
        _uprint(0,s_usinit_ERROR_unable_to_set_optio_0fbd0e80);
      }
      setoserror(0x16);
      return 0;
    }
  }
  else {
    if (param_3 < 0x40000) {
      lVar4 = amallopt(5,0x400,lVar1);
    }
    if (lVar4 != 0) goto LAB_0fb81b0c;
  }
  uVar7 = (uint)param_2;
  lVar1 = usmalloc(uVar7 << 3,param_1);
  *(int *)(param_1 + 8) = (int)lVar1;
  if (lVar1 == 0) {
    setoserror(0xc);
    return 0;
  }
  if (((longlong)(int)(uVar7 - 1) & param_2) != 0) {
    uVar6 = (longlong)(int)(uVar7 - 1) | param_2;
    while( true ) {
      uVar8 = (uint)uVar6;
      uVar7 = uVar8 + 1;
      if ((uVar8 & uVar7) == 0) break;
      uVar6 = (ulonglong)(int)(uVar8 | uVar7);
    }
  }
  lVar1 = (longlong)((int)uVar7 >> 3);
  iVar2 = ((int)uVar7 >> 3) * 2;
  if (lVar1 == 0) {
    lVar1 = 1;
    iVar2 = 2;
  }
  *(int *)(param_1 + 0x10) = (int)lVar1 + -1;
  lVar4 = usmalloc(iVar2,param_1);
  *(int *)(param_1 + 0xc) = (int)lVar4;
  if (lVar4 == 0) {
    setoserror(0xc);
    return 0;
  }
  if (0 < (longlong)param_2) {
    lVar4 = 0;
    do {
      iVar2 = (int)lVar4 * 8;
      *(undefined4 *)(*(int *)(param_1 + 8) + iVar2) = 0xffffffff;
      *(undefined2 *)(*(int *)(param_1 + 8) + iVar2 + 4) = 0xffff;
      lVar4 = (longlong)(short)((short)lVar4 + 1);
      *(undefined2 *)(*(int *)(param_1 + 8) + iVar2 + 6) = 0xffff;
    } while (lVar4 < (longlong)param_2);
  }
  if (0 < lVar1) {
    lVar4 = 0;
    do {
      iVar2 = (int)lVar4;
      lVar4 = (longlong)(short)((short)lVar4 + 1);
      *(undefined2 *)(*(int *)(param_1 + 0xc) + iVar2 * 2) = 0xffff;
    } while (lVar4 < lVar1);
  }
  *(undefined4 *)(param_1 + 0x464) = _DAT_00200e00;
  lVar1 = (*_nlock)(param_1);
  *(int *)(param_1 + 0x440) = (int)lVar1;
  if (lVar1 == 0) {
    return 0;
  }
  *(undefined4 *)(param_1 + 0x42c) = 0;
  *(undefined4 *)(param_1 + 0x430) = 0;
  *(undefined4 *)(param_1 + 0x434) = 0;
  *(undefined4 *)(param_1 + 0x438) = 0;
  *(undefined4 *)(param_1 + 0x43c) = 0;
  lVar1 = (*_nlock)(param_1);
  *(int *)(param_1 + 0x444) = (int)lVar1;
  if (lVar1 == 0) {
    return 0;
  }
  lVar1 = usnewsema(param_1,1);
  *(int *)(param_1 + 0x458) = (int)lVar1;
  if (lVar1 == 0) {
    return 0;
  }
  lVar1 = (*_nlock)(param_1);
  *(int *)(param_1 + 0x448) = (int)lVar1;
  if (lVar1 == 0) {
    return 0;
  }
  lVar1 = usmallopt(8,0,param_1);
  if (lVar1 != 0) {
    _uprint(1,s_usinit_ERROR_unable_to_allocate_u_0fbd0eb8);
    setoserror(0x16);
    return 0;
  }
  return 1;
}



// === _uprint @ 0fb81d74 (236 bytes) ===

undefined8
_uprint(longlong param_1,char *param_2,undefined8 param_3,undefined8 param_4,undefined8 param_5,
       undefined8 param_6,undefined8 param_7,undefined8 param_8)

{
  undefined8 uVar1;
  char *__src;
  uint uVar2;
  size_t __n;
  char acStack_260 [512];
  undefined8 uStack_30;
  undefined8 uStack_28;
  undefined8 uStack_20;
  undefined8 uStack_18;
  undefined8 uStack_10;
  undefined8 uStack_8;
  
  uStack_30 = param_3;
  uStack_28 = param_4;
  uStack_20 = param_5;
  uStack_18 = param_6;
  uStack_10 = param_7;
  uStack_8 = param_8;
  uVar1 = oserror();
  _vsprintf(acStack_260,param_2,&uStack_30);
  if (0 < param_1) {
    strcat(acStack_260," : ");
    __src = strerror((int)uVar1);
    strcat(acStack_260,__src);
  }
  strcat(acStack_260,"\n");
  if (DAT_0fbde458 != 0) {
    DAT_0fbde458 = 0;
    uVar2 = fcntl(_utracefd,3);
    if (-1 < (int)uVar2) {
      fcntl(_utracefd,4,uVar2 | 8);
    }
  }
  __n = strlen(acStack_260);
  write(_utracefd,acStack_260,__n);
  setoserror(uVar1);
  return 0;
}



// === usdetach @ 0fb81e60 (288 bytes) ===

int usdetach(void *param_1)

{
  void *pvVar1;
  int *piVar2;
  undefined8 uVar3;
  longlong lVar4;
  int iVar5;
  int *piVar6;
  int *piStack_50;
  int iStack_4c;
  
  uVar3 = *(undefined8 *)((int)param_1 + 0x418);
  if (DAT_0fbd3a68 != (int *)0x0) {
    pvVar1 = (void *)DAT_0fbd3a68[1];
    piVar2 = DAT_0fbd3a68;
    while (piVar6 = piVar2, pvVar1 != param_1) {
      piVar2 = (int *)piVar6[2];
      if (piVar2 == (int *)0x0) goto LAB_0fb81ef0;
      pvVar1 = (void *)piVar2[1];
      piStack_50 = piVar6;
    }
    if (DAT_0fbd3a68 == piVar6) {
      DAT_0fbd3a68 = (int *)piVar6[2];
    }
    else {
      piStack_50[2] = piVar6[2];
    }
    iStack_4c = *piVar6;
    free(piVar6[3]);
    free(piVar6);
  }
LAB_0fb81ef0:
  lVar4 = _usgettid(param_1);
  if (lVar4 != -1) {
    iVar5 = (short)lVar4 * 8;
    close((int)*(short *)(*(int *)((int)param_1 + 8) + iVar5 + 6));
    *(undefined2 *)(*(int *)((int)param_1 + 8) + iVar5 + 6) = 0xffff;
  }
  iVar5 = munmap(param_1,(size_t)uVar3);
  if ((iVar5 == -1) && (_uerror != 0)) {
    _uprint(1,s_usdetach_ERROR_unable_to_unmap_a_0fbd0ee8);
  }
  iVar5 = close(iStack_4c);
  return iVar5;
}



// === __usblockallsigs @ 0fb81f80 (92 bytes) ===

int __usblockallsigs(sigset_t *param_1)

{
  int iVar1;
  
  sigfillset((sigset_t *)&stack0xffffffd0);
  sigdelset((sigset_t *)&stack0xffffffd0,3);
  sigdelset((sigset_t *)&stack0xffffffd0,0xb);
  sigdelset((sigset_t *)&stack0xffffffd0,10);
  iVar1 = sigprocmask(3,(sigset_t *)&stack0xffffffd0,param_1);
  return iVar1;
}



// === __usunblockallsigs @ 0fb81fdc (36 bytes) ===

int __usunblockallsigs(sigset_t *param_1)

{
  int iVar1;
  
  iVar1 = sigprocmask(3,param_1,(sigset_t *)0x0);
  return iVar1;
}



// === FUN_0fb82000 @ 0fb82000 (164 bytes) ===

undefined8 FUN_0fb82000(int param_1,size_t param_2)

{
  ssize_t sVar1;
  size_t __n;
  undefined1 auStack_2030 [8200];
  
  blkclr(auStack_2030,0x2000);
  lseek64(param_1,0,0);
  do {
    if ((int)param_2 < 1) {
      return 0;
    }
    __n = param_2;
    if (0x2000 < (int)param_2) {
      __n = 0x2000;
    }
    sVar1 = write(param_1,auStack_2030,__n);
    param_2 = param_2 - sVar1;
  } while (-1 < sVar1);
  return 0xffffffffffffffff;
}



// === unlink @ 0fb820a4 (44 bytes) ===

int unlink(char *__name)

{
  int iVar1;
  longlong in_a3;
  
  syscall(0);
  if (in_a3 == 0) {
    return 0;
  }
  iVar1 = _cerror(__name);
  return iVar1;
}



// === fchmod @ 0fb820d0 (40 bytes) ===

int fchmod(int __fd,__mode_t __mode)

{
  int iVar1;
  longlong in_a3;
  
  syscall(0);
  if (in_a3 == 0) {
    return 0x481;
  }
  iVar1 = _cerror(__fd,__mode);
  return iVar1;
}



// === msync @ 0fb820f8 (40 bytes) ===

int msync(void *__addr,size_t __len,int __flags)

{
  int iVar1;
  longlong in_a3;
  
  syscall(0);
  if (in_a3 == 0) {
    return 0x471;
  }
  iVar1 = _cerror(__addr,__len,__flags);
  return iVar1;
}



// === _usfreadlock @ 0fb82120 (168 bytes) ===

undefined8 _usfreadlock(undefined8 param_1,int param_2)

{
  int iVar2;
  longlong lVar1;
  undefined2 uStack_70;
  undefined2 uStack_6e;
  undefined8 uStack_68;
  undefined8 uStack_60;
  
  do {
    uStack_6e = 0;
    uStack_60 = 1;
    uStack_70 = 1;
    uStack_68 = param_1;
    iVar2 = fcntl(param_2,7,&uStack_70);
    if (iVar2 != -1) break;
    lVar1 = oserror();
  } while (lVar1 == 4);
  if (iVar2 == -1) {
    return 0xffffffffffffffff;
  }
  return 0;
}



// === _usfsplock @ 0fb821c8 (168 bytes) ===

undefined8 _usfsplock(undefined8 param_1,int param_2)

{
  int iVar2;
  longlong lVar1;
  undefined2 uStack_70;
  undefined2 uStack_6e;
  undefined8 uStack_68;
  undefined8 uStack_60;
  
  do {
    uStack_6e = 0;
    uStack_60 = 1;
    uStack_70 = 2;
    uStack_68 = param_1;
    iVar2 = fcntl(param_2,7,&uStack_70);
    if (iVar2 != -1) break;
    lVar1 = oserror();
  } while (lVar1 == 4);
  if (iVar2 == -1) {
    return 0xffffffffffffffff;
  }
  return 0;
}



// === _usfunsplock @ 0fb82270 (148 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 _usfunsplock(undefined8 param_1,undefined8 param_2)

{
  int iVar1;
  undefined2 uStack_60;
  undefined2 uStack_5e;
  undefined8 uStack_58;
  undefined8 uStack_50;
  undefined8 uStack_28;
  undefined8 uStack_20;
  
  uStack_5e = 0;
  uStack_50 = 1;
  uStack_60 = 3;
  uStack_58 = param_1;
  uStack_28 = param_1;
  uStack_20 = param_2;
  iVar1 = fcntl((int)param_2,6,&uStack_60);
  if (iVar1 == -1) {
    if (_uerror != 0) {
      _uprint(1,s_ERROR__unlock_fd__d_offset__d_pi_0fbd0f10,uStack_20,uStack_28,_DAT_00200e00);
    }
    return 0xffffffffffffffff;
  }
  return 0;
}



// === _usfgetlock @ 0fb82304 (176 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 _usfgetlock(undefined8 param_1,undefined8 param_2)

{
  int iVar1;
  short asStack_60 [4];
  undefined8 uStack_58;
  undefined8 uStack_50;
  undefined4 uStack_44;
  undefined8 uStack_28;
  undefined8 uStack_20;
  
  asStack_60[1] = 0;
  uStack_50 = 1;
  asStack_60[0] = 2;
  uStack_58 = param_1;
  uStack_28 = param_1;
  uStack_20 = param_2;
  iVar1 = fcntl((int)param_2,0xe,asStack_60);
  if (iVar1 == -1) {
    if (_uerror != 0) {
      _uprint(1,s_ERROR__getlock_fd__d_offset__d_p_0fbd0f38,uStack_20,uStack_28,_DAT_00200e00);
    }
    return 0xffffffff;
  }
  if (asStack_60[0] == 3) {
    return 0;
  }
  return uStack_44;
}



// === usconfig @ 0fb823b4 (1124 bytes) ===

/* WARNING: Restarted to delay deadcode elimination for space: stack */

int usconfig(undefined4 param_1,uint param_2,undefined4 *param_3)

{
  int iVar1;
  int iVar3;
  longlong lVar2;
  int iVar4;
  int unaff_gp_lo;
  
  iVar4 = 0;
  switch(param_1) {
  default:
    setoserror(0x16);
    iVar4 = -1;
    break;
  case 1:
    if ((param_2 == 0) || (iVar4 = *(int *)(unaff_gp_lo + 0x3320), 10000 < param_2)) {
      setoserror(0x16);
      return -1;
    }
    *(uint *)(unaff_gp_lo + 0x3320) = param_2;
    break;
  case 2:
    if (param_2 < 0x1000) {
      setoserror(0x16);
      return -1;
    }
    iVar4 = *(int *)(unaff_gp_lo + 0x331c);
    *(uint *)(unaff_gp_lo + 0x331c) = param_2 + 0x470;
    iVar4 = iVar4 + -0x470;
    break;
  case 3:
    iVar4 = *(int *)(param_2 + 0x424);
    break;
  case 4:
    iVar4 = (int)*(undefined8 *)(param_2 + 0x418) + -0x470;
    break;
  case 5:
    *(uint *)(param_2 + 0x428) = *(uint *)(param_2 + 0x428) | 4;
    break;
  case 6:
    *(uint *)(param_2 + 0x428) = *(uint *)(param_2 + 0x428) & 0xfffffffb;
    break;
  case 7:
    if ((*(uint *)(param_2 + 0x428) & 4) == 0) {
      setoserror(0x16);
      iVar4 = -1;
    }
    else {
      *param_3 = *(undefined4 *)(param_2 + 0x430);
      param_3[1] = *(undefined4 *)(param_2 + 0x434);
      param_3[2] = *(undefined4 *)(param_2 + 0x438);
    }
    break;
  case 8:
    iVar3 = *(int *)(param_2 + 0x42c);
    while (iVar3 != 0) {
      iVar1 = *(int *)(iVar3 + 0x18);
      usfree(iVar3,param_2);
      iVar3 = iVar1;
    }
    *(undefined4 *)(param_2 + 0x438) = 0;
    *(undefined4 *)(param_2 + 0x434) = 0;
    *(undefined4 *)(param_2 + 0x430) = 0;
    *(undefined4 *)(param_2 + 0x42c) = 0;
    break;
  case 9:
    iVar4 = *(int *)(unaff_gp_lo + -0x76d4);
    if (2 < param_2) {
      setoserror(0x16);
      return -1;
    }
    *(uint *)(unaff_gp_lo + -0x76d4) = param_2;
    break;
  case 10:
    iVar4 = *(int *)(unaff_gp_lo + 0x3300);
    *(undefined4 *)(unaff_gp_lo + 0x3300) = 1;
    if (*(int *)(unaff_gp_lo + -0x7758) != 0) {
      *(undefined4 *)(unaff_gp_lo + -0x771c) = 1;
    }
    break;
  case 0xb:
    iVar4 = *(int *)(unaff_gp_lo + 0x3300);
    *(undefined4 *)(unaff_gp_lo + 0x3300) = 0;
    if (*(int *)(unaff_gp_lo + -0x7758) != 0) {
      *(undefined4 *)(unaff_gp_lo + -0x771c) = 0;
    }
    break;
  case 0xc:
    iVar4 = *(int *)(unaff_gp_lo + 0x3304);
    *(undefined4 *)(unaff_gp_lo + 0x3304) = 1;
    if (*(int *)(unaff_gp_lo + -0x7758) != 0) {
      *(undefined4 *)(unaff_gp_lo + -0x7718) = 1;
    }
    break;
  case 0xd:
    iVar4 = *(int *)(unaff_gp_lo + 0x3304);
    *(undefined4 *)(unaff_gp_lo + 0x3304) = 0;
    if (*(int *)(unaff_gp_lo + -0x7758) != 0) {
      *(undefined4 *)(unaff_gp_lo + -0x7718) = 0;
    }
    break;
  case 0xe:
    iVar4 = *(int *)(unaff_gp_lo + 0x3308);
    *(undefined4 *)(unaff_gp_lo + 0x3308) = 1;
    if (*(int *)(unaff_gp_lo + -0x7758) != 0) {
      *(undefined4 *)(unaff_gp_lo + -0x7714) = 1;
    }
    break;
  case 0xf:
    iVar4 = *(int *)(unaff_gp_lo + 0x3308);
    *(undefined4 *)(unaff_gp_lo + 0x3308) = 0;
    if (*(int *)(unaff_gp_lo + -0x7758) != 0) {
      *(undefined4 *)(unaff_gp_lo + -0x7714) = 0;
    }
    break;
  case 0x10:
    iVar4 = *(int *)(unaff_gp_lo + -0x76d0);
    if ((param_2 != 1) && (iVar4 = *(int *)(unaff_gp_lo + -0x76d0), param_2 != 0)) {
      setoserror(0x16);
      return -1;
    }
    *(uint *)(unaff_gp_lo + -0x76d0) = param_2;
    break;
  case 0x11:
    iVar3 = chmod((char *)(param_2 + 0x18),(__mode_t)param_3);
    if (iVar3 != 0) {
      return -1;
    }
    lVar2 = _usretsfd(param_2);
    if (lVar2 == -1) {
      setoserror(0x16);
      return -1;
    }
    iVar3 = fchmod((int)lVar2,(__mode_t)param_3);
    if (iVar3 != 0) {
      return -1;
    }
    break;
  case 0x12:
    iVar4 = *(int *)(param_2 + 0x43c);
    *(undefined4 **)(param_2 + 0x43c) = param_3;
    break;
  case 0x13:
    iVar4 = *(int *)(unaff_gp_lo + 0x3324);
    *(uint *)(unaff_gp_lo + 0x3324) = param_2;
    break;
  case 0x14:
    iVar4 = *(int *)(unaff_gp_lo + 0x3328);
    *(uint *)(unaff_gp_lo + 0x3328) = param_2;
    break;
  case 0x15:
    iVar4 = *(int *)(unaff_gp_lo + -0x76cc);
    *(uint *)(unaff_gp_lo + -0x76cc) = param_2;
  }
  return iVar4;
}



// === usputinfo @ 0fb8281c (8 bytes) ===

void usputinfo(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0x454) = param_2;
  return;
}



// === usgetinfo @ 0fb82824 (8 bytes) ===

undefined4 usgetinfo(int param_1)

{
  return *(undefined4 *)(param_1 + 0x454);
}



// === uscasinfo @ 0fb8282c (36 bytes) ===

void uscasinfo(undefined8 param_1,undefined8 param_2,undefined8 param_3)

{
  (*_cas)((int)param_1 + 0x454,param_2,param_3,param_1);
  return;
}



// === chmod @ 0fb82850 (44 bytes) ===

int chmod(char *__file,__mode_t __mode)

{
  int iVar1;
  longlong in_a3;
  
  syscall(0);
  if (in_a3 == 0) {
    return 0;
  }
  iVar1 = _cerror(__file,__mode);
  return iVar1;
}



// === usmalloc @ 0fb8287c (28 bytes) ===

void usmalloc(undefined8 param_1,int param_2)

{
  amalloc(param_1,*(undefined4 *)(param_2 + 0x14));
  return;
}



// === usfree @ 0fb82898 (28 bytes) ===

void usfree(undefined8 param_1,int param_2)

{
  afree(param_1,*(undefined4 *)(param_2 + 0x14));
  return;
}



// === usrealloc @ 0fb828b4 (28 bytes) ===

void usrealloc(undefined8 param_1,undefined8 param_2,int param_3)

{
  arealloc(param_1,param_2,*(undefined4 *)(param_3 + 0x14));
  return;
}



// === uscalloc @ 0fb828d0 (28 bytes) ===

void uscalloc(undefined8 param_1,undefined8 param_2,int param_3)

{
  acalloc(param_1,param_2,*(undefined4 *)(param_3 + 0x14));
  return;
}



// === usmallinfo @ 0fb828ec (128 bytes) ===

void usmallinfo(undefined4 *param_1,int param_2)

{
  undefined4 uStack_50;
  undefined4 uStack_4c;
  undefined4 uStack_48;
  undefined4 uStack_44;
  undefined4 uStack_40;
  undefined4 uStack_3c;
  undefined4 uStack_38;
  undefined4 uStack_34;
  undefined4 uStack_30;
  undefined4 uStack_2c;
  
  amallinfo(&uStack_50,*(undefined4 *)(param_2 + 0x14));
  *param_1 = uStack_50;
  param_1[1] = uStack_4c;
  param_1[2] = uStack_48;
  param_1[3] = uStack_44;
  param_1[4] = uStack_40;
  param_1[5] = uStack_3c;
  param_1[6] = uStack_38;
  param_1[7] = uStack_34;
  param_1[8] = uStack_30;
  param_1[9] = uStack_2c;
  return;
}



// === usmallopt @ 0fb8296c (28 bytes) ===

void usmallopt(undefined8 param_1,undefined8 param_2,int param_3)

{
  amallopt(param_1,param_2,*(undefined4 *)(param_3 + 0x14));
  return;
}



// === usrecalloc @ 0fb82988 (28 bytes) ===

void usrecalloc(void)

{
  arecalloc();
  return;
}



// === usmallocblksize @ 0fb829a4 (28 bytes) ===

void usmallocblksize(undefined8 param_1,int param_2)

{
  amallocblksize(param_1,*(undefined4 *)(param_2 + 0x14));
  return;
}



// === usmemalign @ 0fb829c0 (28 bytes) ===

void usmemalign(undefined8 param_1,undefined8 param_2,int param_3)

{
  amemalign(param_1,param_2,*(undefined4 *)(param_3 + 0x14));
  return;
}



// === amalloc @ 0fb829dc (148 bytes) ===

undefined8 amalloc(undefined8 param_1,int param_2)

{
  longlong lVar1;
  undefined8 uVar2;
  
  if ((*(int *)(param_2 + 0x9c) != 0) && (lVar1 = (*_lock)(*(int *)(param_2 + 0x9c)), lVar1 != 0)) {
    uVar2 = FUN_0fb82e6c(param_1,param_2);
    if (*(int *)(param_2 + 0x9c) != 0) {
      (*_ulock)(*(int *)(param_2 + 0x9c));
    }
    return uVar2;
  }
  uVar2 = FUN_0fb82e6c(param_1,param_2);
  return uVar2;
}



// === afree @ 0fb82a70 (96 bytes) ===

void afree(undefined8 param_1,int param_2)

{
  if (*(int *)(param_2 + 0x9c) != 0) {
    (*_lock)(*(int *)(param_2 + 0x9c));
  }
  FUN_0fb835d8(param_1,param_2);
  if (*(int *)(param_2 + 0x9c) != 0) {
    (*_ulock)(*(int *)(param_2 + 0x9c));
  }
  return;
}



// === FUN_0fb82ad0 @ 0fb82ad0 (204 bytes) ===

uint FUN_0fb82ad0(undefined8 param_1,uint *param_2)

{
  uint uVar1;
  int iVar2;
  uint uVar3;
  
  if ((code *)param_2[0x28] != (code *)0x0) {
    uVar1 = (*(code *)param_2[0x28])(param_1,param_2);
    return uVar1;
  }
  uVar1 = param_2[0x2a];
  if ((uVar1 - (int)param_2) + (int)param_1 < param_2[0x29]) {
    uVar3 = uVar1 + (int)param_1;
    if ((*param_2 & 4) == 0) {
      iVar2 = sigprocmask(0,(sigset_t *)0x0,(sigset_t *)(uVar3 - 0x10 & 0xfffffff8));
      if (iVar2 < 0) {
        return 0xffffffff;
      }
      uVar1 = param_2[0x2a];
    }
    param_2[0x2a] = uVar3;
    return uVar1;
  }
  return 0xffffffff;
}



// === acreate @ 0fb82b9c (500 bytes) ===

uint * acreate(int param_1,ulonglong param_2,uint param_3,longlong param_4,longlong param_5)

{
  longlong lVar1;
  uint uVar2;
  uint *__s;
  
  if ((param_4 == 0) && ((param_3 & 1) != 0)) {
    setoserror(0x16);
    return (uint *)0x0;
  }
  if (param_2 < 0x400) {
    setoserror(0x16);
    return (uint *)0x0;
  }
  __s = (uint *)(param_1 + 7U & 0xfffffff8);
  uVar2 = (int)param_2 - ((int)__s - param_1);
  memset(__s,0,0xd8);
  __s[3] = 0;
  __s[4] = 0;
  __s[0x25] = 0;
  __s[0x29] = uVar2;
  *__s = param_3;
  __s[0x17] = (uint)(__s + 0xb);
  __s[0x10] = (uint)(__s + 0xb);
  __s[0x1a] = (uint)(__s + 0x13);
  __s[0x23] = (uint)(__s + 0xf);
  __s[0x1d] = (uint)(__s + 0x1f);
  __s[9] = 1;
  __s[8] = 100;
  __s[7] = 0x2000;
  __s[1] = 100;
  __s[2] = 0x10;
  __s[10] = 0x10;
  __s[0x22] = (uint)(__s + 0x1b);
  __s[0x15] = (uint)(__s + 0x17);
  __s[5] = 0x1c;
  __s[0xb] = 1;
  __s[0xf] = 1;
  __s[0x2a] = (int)__s + 0xe7U & 0xfffffff0;
  if (param_5 == 0) {
    if (0xffff < uVar2) {
      __s[0x26] = (uint)param_4;
      goto LAB_0fb82d10;
    }
    __s[5] = 0;
    __s[7] = 0x200;
  }
  __s[0x26] = (uint)param_4;
LAB_0fb82d10:
  __s[0x28] = (uint)param_5;
  __s[6] = __s[2] + __s[5] + 0x10 & 0xfffffff0;
  if ((*__s & 1) != 0) {
    lVar1 = (*_nlock)(param_4);
    __s[0x27] = (uint)lVar1;
    if (lVar1 == 0) {
      return (uint *)0x0;
    }
  }
  return __s;
}



// === adelete @ 0fb82d90 (48 bytes) ===

void adelete(int param_1)

{
  if (*(int *)(param_1 + 0x9c) != 0) {
    (*_freelock)(*(int *)(param_1 + 0x9c),*(undefined4 *)(param_1 + 0x98));
  }
  return;
}



// === arecalloc @ 0fb82dc0 (172 bytes) ===

longlong arecalloc(undefined8 param_1,int param_2,int param_3,undefined8 param_4)

{
  ulonglong uVar1;
  longlong lVar2;
  ulonglong uVar3;
  
  param_2 = param_2 * param_3;
  uVar1 = amallocblksize(param_1,param_4);
  lVar2 = arealloc(param_1,param_2,param_4);
  uVar3 = amallocblksize(lVar2,param_4);
  if (lVar2 != 0) {
    if (uVar1 < uVar3) {
      memset((void *)((int)uVar1 + (int)lVar2),0,(int)uVar3 - (int)uVar1);
    }
    else {
      memset((void *)(param_2 + (int)lVar2),0,(int)uVar3 - param_2);
    }
  }
  return lVar2;
}



// === FUN_0fb82e6c @ 0fb82e6c (1900 bytes) ===

uint * FUN_0fb82e6c(uint param_1,int param_2)

{
  undefined4 *puVar1;
  uint *puVar2;
  longlong lVar3;
  int *piVar4;
  uint *puVar5;
  int iVar6;
  int iVar7;
  int *piVar8;
  uint *puVar9;
  uint uVar10;
  uint uVar11;
  int iVar12;
  uint *puVar13;
  uint uVar14;
  
  while( true ) {
    if (param_1 == 0) {
      return (uint *)0x0;
    }
    uVar10 = *(uint *)(param_2 + 0x14);
    if (uVar10 < param_1) break;
    uVar11 = *(uint *)(param_2 + 0xb0);
    iVar7 = *(int *)(param_2 + 0xac);
    if (*(int *)(param_2 + 0xc) != 0) goto LAB_0fb82f4c;
    iVar12 = (int)(uVar10 + *(int *)(param_2 + 0x28) + 3) / *(int *)(param_2 + 0x28);
    *(undefined4 *)(param_2 + 0x14) = 0;
    *(uint *)(param_2 + 0xc) = uVar10;
    *(int *)(param_2 + 0x10) = iVar12;
    lVar3 = FUN_0fb82e6c(iVar12 * 4 + 4,param_2);
    *(int *)(param_2 + 0x90) = (int)lVar3;
    if (lVar3 != 0) {
      iVar12 = 1;
      *(undefined4 *)(param_2 + 0x14) = *(undefined4 *)(param_2 + 0xc);
      if (0 < *(int *)(param_2 + 0x10)) {
        iVar6 = 4;
        do {
          *(undefined4 *)(*(int *)(param_2 + 0x90) + iVar6) = 0;
          iVar12 = iVar12 + 1;
          iVar6 = iVar6 + 4;
        } while (iVar12 <= *(int *)(param_2 + 0x10));
      }
LAB_0fb82f4c:
      if (iVar7 == 0) {
        iVar7 = *(int *)(param_2 + 0x28);
        iVar12 = (int)(iVar7 + param_1 + 3) / iVar7;
        iVar7 = iVar7 * iVar12;
        puVar1 = *(undefined4 **)(*(int *)(param_2 + 0x90) + iVar12 * 4);
      }
      else {
        iVar12 = (int)(iVar7 + param_1 + 4) >> (uVar11 & 0x1f);
        iVar7 = iVar12 << (uVar11 & 0x1f);
        puVar1 = *(undefined4 **)(*(int *)(param_2 + 0x90) + iVar12 * 4);
      }
      if ((puVar1 == (undefined4 *)0x0) || (puVar9 = (uint *)puVar1[2], puVar9 == (uint *)0x0)) {
        puVar5 = (uint *)FUN_0fb82e6c(*(int *)(param_2 + 4) * iVar7 + 0x20,param_2);
        if (puVar5 == (uint *)0x0) {
          return (uint *)0x0;
        }
        if (puVar1 == (undefined4 *)0x0) {
          *puVar5 = (uint)puVar5;
          puVar5[1] = (uint)puVar5;
        }
        else {
          *puVar5 = (uint)puVar1;
          puVar5[1] = puVar1[1];
          puVar1[1] = puVar5;
          *(uint **)puVar5[1] = puVar5;
        }
        uVar10 = (int)puVar5 + iVar7 + 0x1c;
        *(uint **)(*(int *)(param_2 + 0x90) + iVar12 * 4) = puVar5;
        puVar5[4] = iVar7 - 4;
        puVar5[2] = uVar10;
        puVar5[3] = uVar10;
        iVar12 = *(int *)(param_2 + 4);
        puVar9 = puVar5 + 7;
        puVar5[7] = (uint)puVar5 | 3;
        puVar5[5] = iVar12 * iVar7 + 0x20;
      }
      else {
        puVar5 = (uint *)puVar1[3];
        if (puVar9 < puVar5) {
          uVar10 = *puVar9;
          puVar1[2] = uVar10 & 0xfffffffd;
          if ((uVar10 & 0xfffffffd) == 0) {
            *(undefined4 *)(*(int *)(param_2 + 0x90) + iVar12 * 4) = *puVar1;
          }
        }
        else {
          iVar6 = (int)puVar5 + iVar7;
          if (iVar6 + 4U < (uint)(puVar1[5] + (int)puVar1)) {
            puVar1[2] = iVar6;
            puVar1[3] = iVar6;
          }
          else {
            puVar1[2] = 0;
            puVar1[3] = puVar5 + iVar7 * 2;
            *(undefined4 *)(*(int *)(param_2 + 0x90) + iVar12 * 4) = *puVar1;
          }
        }
        *puVar9 = (uint)puVar1 | 3;
      }
      if (*(int *)(param_2 + 0x94) != 0) {
        __acheckq(param_2);
      }
      return puVar9 + 1;
    }
    *(undefined4 *)(param_2 + 0xc) = 0;
  }
  uVar10 = *(int *)(param_2 + 8) + param_1 + 0xf & 0xfffffff0;
  if (uVar10 < 0x11) {
    uVar10 = 0x10;
  }
  if (0x80000000 < param_1) {
    return (uint *)0x0;
  }
  puVar13 = (uint *)(param_2 + 0x5c);
  puVar5 = (uint *)(param_2 + 0x4c);
  iVar7 = 1;
  puVar9 = puVar5;
  do {
    puVar9 = (uint *)puVar9[2];
    if (*(int *)(param_2 + 0x20) < iVar7) {
      if (puVar13 != puVar9) {
        *(undefined4 *)(*(int *)(param_2 + 0x68) + 8) = *(undefined4 *)(param_2 + 0x54);
        *(undefined4 *)(*(int *)(param_2 + 0x54) + 0xc) = *(undefined4 *)(param_2 + 0x68);
        *(uint **)(puVar9[3] + 8) = puVar13;
        *(uint *)(param_2 + 0x68) = puVar9[3];
        puVar9[3] = (uint)puVar5;
        *(uint **)(param_2 + 0x54) = puVar9;
        puVar9 = *(uint **)(*(int *)(param_2 + 0x8c) + 4);
        if (((*puVar9 & 1) != 0) || (*puVar9 - (int)puVar9 < uVar10)) {
          puVar9 = puVar13;
        }
      }
      break;
    }
    iVar7 = iVar7 + 1;
  } while (*puVar9 - (int)puVar9 < uVar10);
  if (puVar13 != puVar9) {
    puVar2 = (uint *)puVar9[2];
    *(uint **)(puVar9[3] + 8) = puVar2;
    *(uint *)(puVar9[2] + 0xc) = puVar9[3];
    puVar9[3] = 0;
    puVar9[2] = 0;
    if (puVar13 != puVar2) {
      *(undefined4 *)(*(int *)(param_2 + 0x68) + 8) = *(undefined4 *)(param_2 + 0x54);
      *(undefined4 *)(*(int *)(param_2 + 0x54) + 0xc) = *(undefined4 *)(param_2 + 0x68);
      *(uint **)(puVar2[3] + 8) = puVar13;
      *(uint *)(param_2 + 0x68) = puVar2[3];
      puVar2[3] = (uint)puVar5;
      *(uint **)(param_2 + 0x54) = puVar2;
    }
    goto LAB_0fb83500;
  }
  puVar9 = *(uint **)(*(int *)(param_2 + 0x8c) + 4);
  if ((*puVar9 & 1) == 0) {
    uVar11 = *(uint *)(param_2 + 0x1c);
    iVar7 = ((((uVar10 - (*(int *)(param_2 + 0x8c) - (int)puVar9)) + uVar11) - 1) / uVar11) * uVar11
    ;
    piVar4 = (int *)FUN_0fb82ad0(iVar7,param_2);
    if (piVar4 == (int *)0xffffffff) {
      return (uint *)0x0;
    }
    iVar12 = *(int *)(param_2 + 0x8c);
    if ((int *)(iVar12 + 0x10U) != piVar4) {
      piVar8 = piVar4;
      if (((uint)piVar4 & 0xf) != 0) {
        piVar8 = (int *)((int)piVar4 + (0x10 - ((uint)piVar4 & 0xf)));
      }
      *(uint *)((int)piVar4 + iVar7 + -0x10) = param_2 + 0x3cU | 1;
      iVar12 = (int)piVar4 + iVar7 + -0x10;
      *piVar8 = iVar12;
      **(uint **)(param_2 + 0x8c) = (uint)piVar8 | 1;
      *(int **)((int)piVar4 + iVar7 + -0xc) = piVar8;
      *(int *)(param_2 + 0x40) = iVar12;
      piVar8[1] = *(int *)(param_2 + 0x8c);
      *(int *)(param_2 + 0x8c) = iVar12;
      if (*(int *)(param_2 + 0x24) == 0) {
        FUN_0fb83af8(puVar5);
        iVar7 = *(int *)(param_2 + 0x8c);
      }
      else {
        FUN_0fb83b14(puVar5);
        iVar7 = *(int *)(param_2 + 0x8c);
      }
      puVar9 = *(uint **)(iVar7 + 4);
      goto LAB_0fb83358;
    }
  }
  else {
LAB_0fb83358:
    uVar11 = *(uint *)(param_2 + 0x1c);
    iVar7 = ((uVar11 + uVar10 + 0x1e) / uVar11) * uVar11;
    puVar13 = (uint *)FUN_0fb82ad0(iVar7,param_2);
    if (puVar13 == (uint *)0xffffffff) {
      return (uint *)0x0;
    }
    iVar12 = *(int *)(param_2 + 0x8c);
    if ((uint *)(iVar12 + 0x10) != puVar13) {
      puVar9 = puVar13;
      if (((uint)puVar13 & 0xf) != 0) {
        puVar9 = (uint *)((int)puVar13 + (0x10 - ((uint)puVar13 & 0xf)));
      }
      *(uint *)((int)puVar13 + iVar7 + -0x10) = param_2 + 0x3cU | 1;
      uVar11 = (int)puVar13 + iVar7 + -0x10;
      *puVar9 = uVar11;
      **(uint **)(param_2 + 0x8c) = (uint)puVar9 | 1;
      *(uint **)((int)puVar13 + iVar7 + -0xc) = puVar9;
      *(uint *)(param_2 + 0x40) = uVar11;
      puVar9[1] = *(uint *)(param_2 + 0x8c);
      *(uint *)(param_2 + 0x8c) = uVar11;
      puVar9[3] = 0;
      goto LAB_0fb83500;
    }
    if ((*puVar9 & 1) != 0) {
      puVar13 = (uint *)(iVar12 + iVar7);
      *puVar13 = param_2 + 0x3cU | 1;
      **(int **)(param_2 + 0x8c) = (int)puVar13;
      puVar13[1] = *(uint *)(param_2 + 0x8c);
      puVar9 = *(uint **)(param_2 + 0x8c);
      *(uint **)(param_2 + 0x40) = puVar13;
      puVar9[3] = 0;
      *(uint **)(param_2 + 0x8c) = puVar13;
      goto LAB_0fb83500;
    }
  }
  puVar13 = (uint *)(iVar12 + iVar7);
  *puVar9 = (uint)puVar13;
  *(uint **)(param_2 + 0x8c) = puVar13;
  puVar13[1] = (uint)puVar9;
  *puVar13 = param_2 + 0x3cU | 1;
  *(uint **)(param_2 + 0x40) = puVar13;
  *(uint *)(puVar9[3] + 8) = puVar9[2];
  *(uint *)(puVar9[2] + 0xc) = puVar9[3];
  puVar9[3] = 0;
  puVar9[2] = 0;
LAB_0fb83500:
  uVar11 = *puVar9;
  uVar14 = (uVar11 - (int)puVar9) - uVar10;
  if (uVar14 < 0x10) {
    *puVar9 = uVar11 | 1;
  }
  else {
    puVar13 = (uint *)(uVar10 + (int)puVar9);
    *puVar13 = uVar11;
    *puVar9 = (uint)puVar13 | 1;
    if (uVar14 < *(uint *)(param_2 + 0x18)) {
      if (*(int *)(param_2 + 0x24) == 0) {
        FUN_0fb83af8(param_2 + 0x6c,puVar13);
        uVar10 = *puVar13;
      }
      else {
        FUN_0fb83b14(param_2 + 0x6c,puVar13);
        uVar10 = *puVar13;
      }
    }
    else if (*(int *)(param_2 + 0x24) == 0) {
      FUN_0fb83af8(puVar5,puVar13);
      uVar10 = *puVar13;
    }
    else {
      FUN_0fb83b14(puVar5,puVar13);
      uVar10 = *puVar13;
    }
    puVar13[1] = (uint)puVar9;
    *(uint **)(uVar10 + 4) = puVar13;
  }
  if (*(int *)(param_2 + 0x94) == 0) {
    iVar7 = *(int *)(param_2 + 8);
  }
  else {
    __acheckq(param_2);
    iVar7 = *(int *)(param_2 + 8);
  }
  return (uint *)(iVar7 + (int)puVar9);
}



// === FUN_0fb835d8 @ 0fb835d8 (528 bytes) ===

void FUN_0fb835d8(void *param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  size_t __n;
  uint uVar3;
  uint *puVar4;
  int *piVar5;
  uint *puVar6;
  undefined4 *puVar7;
  
  if (param_1 == (void *)0x0) {
    return;
  }
  if (*(int *)(param_2 + 0xb4) == 0) {
    uVar3 = *(uint *)((int)param_1 + -4);
  }
  else {
    __n = FUN_0fb8422c(param_1,param_2);
    memset(param_1,*(int *)(param_2 + 0xb8),__n);
    uVar3 = *(uint *)((int)param_1 + -4);
  }
  if ((uVar3 & 2) == 0) {
    puVar4 = (uint *)((int)param_1 - *(int *)(param_2 + 8));
    if ((*puVar4 & 1) == 0) {
      return;
    }
    puVar6 = (uint *)(*puVar4 & 0xfffffffe);
    *puVar4 = (uint)puVar6;
    uVar3 = *puVar6;
    if ((uVar3 & 1) == 0) {
      *(uint *)(puVar6[3] + 8) = puVar6[2];
      *(uint *)(puVar6[2] + 0xc) = puVar6[3];
      puVar6[2] = 0;
      puVar6[3] = 0;
      *puVar4 = uVar3;
      *(uint **)(uVar3 + 4) = puVar4;
    }
    puVar6 = (uint *)puVar4[1];
    if ((*puVar6 & 1) == 0) {
      *(uint *)(puVar6[3] + 8) = puVar6[2];
      *(uint *)(puVar6[2] + 0xc) = puVar6[3];
      puVar6[3] = 0;
      puVar6[2] = 0;
      *puVar6 = *puVar4;
      *(uint **)(*puVar4 + 4) = puVar6;
      puVar4 = puVar6;
    }
    if (*(int *)(param_2 + 0x24) != 0) {
      FUN_0fb83b14(param_2 + 0x4c);
      iVar2 = *(int *)(param_2 + 0x94);
      goto LAB_0fb837c8;
    }
    FUN_0fb83af8(param_2 + 0x4c,puVar4);
  }
  else {
    puVar4 = (uint *)((int)param_1 - 4U & 0xfffffffe);
    if ((*puVar4 & 1) == 0) {
      return;
    }
    piVar5 = (int *)(*puVar4 & 0xfffffffc);
    *puVar4 = piVar5[2] | 2;
    piVar5[2] = (int)puVar4;
    puVar7 = (undefined4 *)
             (*(int *)(param_2 + 0x90) + ((piVar5[4] + 4) / *(int *)(param_2 + 0x28)) * 4);
    piVar1 = (int *)*puVar7;
    if (piVar5 != piVar1) {
      *puVar7 = piVar5;
      *(int *)(*piVar5 + 4) = piVar5[1];
      *(int *)piVar5[1] = *piVar5;
      *piVar5 = (int)piVar1;
      piVar5[1] = piVar1[1];
      piVar1[1] = (int)piVar5;
      *(int **)piVar5[1] = piVar5;
    }
  }
  iVar2 = *(int *)(param_2 + 0x94);
LAB_0fb837c8:
  if (iVar2 != 0) {
    __acheckq(param_2);
  }
  return;
}



// === arealloc @ 0fb837e8 (784 bytes) ===

void * arealloc(void *param_1,uint param_2,int param_3)

{
  int iVar1;
  void *pvVar2;
  uint uVar3;
  uint *puVar4;
  uint uVar5;
  undefined4 *puVar6;
  uint *puVar7;
  
  if (param_1 == (void *)0x0) {
    pvVar2 = (void *)amalloc(param_2,param_3);
    return pvVar2;
  }
  if (param_2 == 0) {
    afree(param_1,param_3);
    return (void *)0x0;
  }
  if (*(int *)(param_3 + 0x9c) == 0) {
    uVar3 = *(uint *)((int)param_1 + -4);
  }
  else {
    (*_lock)();
    uVar3 = *(uint *)((int)param_1 + -4);
  }
  if ((uVar3 & 2) == 0) {
    uVar5 = *(int *)(param_3 + 8) + param_2 + 0xf & 0xfffffff0;
    uVar3 = 0x10;
    if (0xf < uVar5) {
      uVar3 = uVar5;
    }
    puVar7 = (uint *)((int)param_1 - *(int *)(param_3 + 8));
    uVar5 = *puVar7;
    if ((uVar5 & 1) == 0) {
      *(uint *)(puVar7[3] + 8) = puVar7[2];
      *(uint *)(puVar7[2] + 0xc) = puVar7[3];
      *puVar7 = uVar5 | 1;
      puVar7[3] = 0;
      puVar7[2] = 0;
    }
    puVar4 = (uint *)(uVar5 & 0xfffffffe);
    uVar5 = (int)puVar4 - (int)puVar7;
    if (((*puVar4 & 1) == 0) && (uVar5 = (int)puVar4 - (int)puVar7, uVar3 <= *puVar4 - (int)puVar7))
    {
      *(uint *)(puVar4[3] + 8) = puVar4[2];
      *(uint *)(puVar4[2] + 0xc) = puVar4[3];
      puVar4[3] = 0;
      puVar4[2] = 0;
      puVar4 = (uint *)*puVar4;
      puVar4[1] = (uint)puVar7;
      *puVar7 = (uint)puVar4 | 1;
      uVar5 = (int)puVar4 - (int)puVar7;
    }
    if (uVar5 < uVar3) {
      uVar3 = param_2;
      if (uVar5 < param_2) {
        uVar3 = uVar5;
      }
      pvVar2 = (void *)FUN_0fb82e6c(param_2,param_3);
      if (pvVar2 != (void *)0x0) {
        memmove(pvVar2,param_1,uVar3);
        FUN_0fb835d8(param_1,param_3);
      }
    }
    else {
      pvVar2 = param_1;
      if (0xf < uVar5 - uVar3) {
        puVar6 = (undefined4 *)(uVar3 + (int)puVar7);
        *puVar6 = puVar4;
        *puVar7 = (uint)puVar6 | 1;
        puVar4[1] = (uint)puVar6;
        puVar6[1] = puVar7;
        if (*(uint *)(param_3 + 0x18) < uVar5 - uVar3) {
          if (*(int *)(param_3 + 0x24) == 0) {
            FUN_0fb83af8(param_3 + 0x4c);
          }
          else {
            FUN_0fb83b14(param_3 + 0x4c);
          }
        }
        else if (*(int *)(param_3 + 0x24) == 0) {
          FUN_0fb83af8(param_3 + 0x6c);
        }
        else {
          FUN_0fb83b14(param_3 + 0x6c);
        }
      }
    }
  }
  else {
    uVar3 = *(uint *)((int)param_1 - 4U & 0xfffffffe);
    pvVar2 = (void *)FUN_0fb82e6c(param_2,param_3);
    if ((pvVar2 != (void *)0x0) && (pvVar2 != param_1)) {
      uVar3 = *(uint *)((uVar3 & 0xfffffffc) + 0x10);
      if (uVar3 <= param_2) {
        param_2 = uVar3;
      }
      memmove(pvVar2,param_1,param_2);
      FUN_0fb835d8(param_1,param_3);
      iVar1 = *(int *)(param_3 + 0x94);
      goto LAB_0fb83ab4;
    }
  }
  iVar1 = *(int *)(param_3 + 0x94);
LAB_0fb83ab4:
  if (iVar1 != 0) {
    __acheckq(param_3);
  }
  if (*(int *)(param_3 + 0x9c) != 0) {
    (*_ulock)();
  }
  return pvVar2;
}



// === FUN_0fb83af8 @ 0fb83af8 (28 bytes) ===

void FUN_0fb83af8(int param_1,int param_2)

{
  *(int *)(param_2 + 0xc) = param_1;
  *(undefined4 *)(param_2 + 8) = *(undefined4 *)(param_1 + 8);
  *(int *)(*(int *)(param_1 + 8) + 0xc) = param_2;
  *(int *)(param_1 + 8) = param_2;
  return;
}



// === FUN_0fb83b14 @ 0fb83b14 (28 bytes) ===

void FUN_0fb83b14(int param_1,int param_2)

{
  int iVar1;
  
  *(int *)(param_2 + 8) = param_1 + 0x10;
  iVar1 = *(int *)(param_1 + 0x1c);
  *(int *)(param_2 + 0xc) = iVar1;
  *(int *)(iVar1 + 8) = param_2;
  *(int *)(param_1 + 0x1c) = param_2;
  return;
}



// === acalloc @ 0fb83b30 (88 bytes) ===

longlong acalloc(int param_1,int param_2,undefined8 param_3)

{
  longlong lVar1;
  
  lVar1 = amalloc(param_1 * param_2,param_3);
  if (lVar1 == 0) {
    return 0;
  }
  memset((void *)lVar1,0,param_1 * param_2);
  return lVar1;
}



// === FUN_0fb83b88 @ 0fb83b88 (32 bytes) ===

int FUN_0fb83b88(int param_1)

{
  int iVar1;
  
  iVar1 = 0;
  for (; param_1 != 0; param_1 = param_1 >> 1) {
    iVar1 = iVar1 + 1;
  }
  return iVar1 + -1;
}



// === amallopt @ 0fb83ba8 (552 bytes) ===

undefined8 amallopt(longlong param_1,longlong param_2,uint *param_3)

{
  longlong lVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  int unaff_gp_lo;
  
  uVar2 = (uint)param_2;
  if (param_1 == 100) {
    param_3[0x25] = uVar2;
    return 0;
  }
  if (param_1 == 5) {
    if (param_2 < 0x200) {
      return 1;
    }
    param_3[7] = uVar2 + 0xf & 0xfffffff0;
    return 0;
  }
  if (param_1 == 7) {
    param_3[9] = (uint)(param_2 == 0);
    return 0;
  }
  if (param_1 != 6) {
    if (param_1 == 0x68) {
      param_3[0x2e] = uVar2;
      param_3[0x2d] = 1;
      return 0;
    }
    if (param_1 != 8) {
      if (param_3[3] != 0) {
        return 1;
      }
      if (param_1 == 1) {
        if (param_2 < 0) {
          return 1;
        }
        param_3[5] = uVar2;
        param_3[6] = param_3[2] + uVar2 + 0x10 & 0xfffffff0;
      }
      else if (param_1 == 2) {
        if (param_2 < 2) {
          return 1;
        }
        param_3[1] = uVar2;
      }
      else if (param_1 == 3) {
        if (param_2 < 1) {
          return 1;
        }
        param_3[0x2b] = 0;
        uVar3 = uVar2 + 0xf & 0xfffffff0;
        uVar4 = uVar3 - 1;
        param_3[10] = uVar3;
        if ((uVar2 + 0xf & uVar4 & 0xfffffff0) == 0) {
          param_3[0x2b] = uVar4;
          uVar2 = FUN_0fb83b88();
          param_3[0x2c] = uVar2;
        }
      }
      else {
        if (param_1 != 4) {
          return 1;
        }
        uVar2 = param_3[2];
        if ((int)uVar2 < 0x10) {
          uVar2 = 0x10;
        }
        param_3[2] = uVar2;
        param_3[6] = param_3[5] + uVar2 + 0x10 & 0xfffffff0;
      }
      return 0;
    }
    lVar1 = (**(code **)(unaff_gp_lo + -0x774c))(param_3[0x26]);
    param_3[0x27] = (uint)lVar1;
    if (lVar1 == 0) {
      return 1;
    }
    *param_3 = *param_3 | 1;
    return 0;
  }
  param_3[8] = uVar2;
  return 0;
}



// === amallinfo @ 0fb83dd0 (696 bytes) ===

void amallinfo(int *param_1,int param_2)

{
  uint uVar1;
  int iVar2;
  uint *puVar3;
  int iVar4;
  uint *puVar5;
  int *piVar6;
  int *piVar7;
  int iVar8;
  longlong lVar9;
  int iStack_70;
  int iStack_6c;
  int iStack_68;
  int iStack_64;
  int iStack_60;
  int iStack_5c;
  int iStack_58;
  int iStack_54;
  int iStack_50;
  int iStack_4c;
  longlong lStack_40;
  
  memset(&iStack_70,0,0x28);
  if (*(int *)(param_2 + 0x9c) == 0) {
    iVar8 = *(int *)(param_2 + 0x54);
  }
  else {
    (*_lock)();
    iVar8 = *(int *)(param_2 + 0x54);
  }
  if (iVar8 == 0) {
    iVar8 = *(int *)(param_2 + 0x9c);
  }
  else {
    puVar5 = (uint *)(*(uint *)(param_2 + 0x3c) & 0xfffffffe);
    if (puVar5 != (uint *)0x0) {
      iStack_70 = *(int *)(param_2 + 0x8c) - (int)puVar5;
      if (*(int *)(param_2 + 8) < 0x11) {
        lVar9 = (longlong)(0x10 - *(int *)(param_2 + 8));
      }
      else {
        lVar9 = 0;
      }
      uVar1 = *puVar5;
      while (puVar3 = (uint *)(uVar1 & 0xfffffffe), (uint *)(param_2 + 0x3c) != puVar3) {
        iStack_6c = iStack_6c + 1;
        if ((*puVar5 & 1) == 0) {
          iStack_50 = (int)puVar3 + (iStack_50 - (int)puVar5);
        }
        else {
          iStack_4c = (int)lVar9 + iStack_4c;
          iStack_54 = (int)puVar3 + (iStack_54 - (int)puVar5);
        }
        puVar5 = puVar3;
        uVar1 = *puVar3;
      }
      lStack_40 = lVar9;
      if ((*(int *)(param_2 + 0x90) != 0) && (0 < *(int *)(param_2 + 0x10))) {
        iVar8 = *(int *)(param_2 + 0x10) * 4;
        piVar6 = (int *)(*(int *)(param_2 + 0x90) + iVar8);
        do {
          piVar7 = (int *)*piVar6;
          if (piVar7 != (int *)0x0) {
            iVar2 = piVar7[4];
            do {
              iStack_64 = iStack_64 + 1;
              iVar4 = FUN_0fb84088(param_2,piVar7,lVar9);
              iStack_58 = iVar4 + iStack_58;
              iStack_5c = (*(int *)(param_2 + 4) * (iVar2 + 4) - iVar4) + iStack_5c;
              iStack_68 = *(int *)(param_2 + 4) + iStack_68;
              lVar9 = (longlong)iStack_68;
              piVar6 = (int *)(*(int *)(param_2 + 0x90) + iVar8);
              piVar7 = (int *)*piVar7;
            } while ((int *)*piVar6 != piVar7);
          }
          piVar6 = piVar6 + -1;
          iVar8 = iVar8 + -4;
        } while (iVar8 != 0);
      }
      iStack_60 = iStack_64 * 0x20;
      iStack_6c = (iStack_6c - iStack_64) + -1;
      iStack_54 = iStack_54 - (iStack_60 + iStack_5c + iStack_58);
      iStack_4c = iStack_4c - iStack_64 * (int)lStack_40;
    }
    iVar8 = *(int *)(param_2 + 0x9c);
  }
  if (iVar8 != 0) {
    (*_ulock)();
  }
  *param_1 = iStack_70;
  param_1[1] = iStack_6c;
  param_1[2] = iStack_68;
  param_1[3] = iStack_64;
  param_1[4] = iStack_60;
  param_1[5] = iStack_5c;
  param_1[6] = iStack_58;
  param_1[7] = iStack_54;
  param_1[8] = iStack_50;
  param_1[9] = iStack_4c;
  return;
}



// === FUN_0fb84088 @ 0fb84088 (140 bytes) ===

int FUN_0fb84088(undefined8 param_1,int param_2)

{
  bool bVar1;
  int iVar2;
  uint *puVar3;
  uint *puVar4;
  uint uVar5;
  
  puVar3 = (uint *)(*(uint *)(param_2 + 8) & 0xfffffffd);
  iVar2 = 0;
  if ((puVar3 != (uint *)0x0) &&
     (puVar4 = (uint *)(*(uint *)(param_2 + 0xc) & 0xfffffffd), puVar4 != puVar3)) {
    uVar5 = *puVar3;
    while( true ) {
      puVar3 = (uint *)(uVar5 & 0xfffffffd);
      iVar2 = *(int *)(param_2 + 0x10) + iVar2 + 4;
      if (puVar3 == (uint *)0x0) {
        bVar1 = false;
      }
      else {
        bVar1 = false;
        if (puVar4 != puVar3) {
          bVar1 = true;
        }
      }
      if (!bVar1) break;
      uVar5 = *puVar3;
    }
  }
  uVar5 = *(uint *)(param_2 + 0xc) & 0xfffffffd;
  param_2 = *(int *)(param_2 + 0x14) + param_2;
  if ((int)(uVar5 + 4) < param_2) {
    iVar2 = (param_2 - uVar5) + iVar2 + -4;
  }
  return iVar2;
}



// === __acheckq @ 0fb84114 (132 bytes) ===

void __acheckq(int param_1)

{
  int *piVar1;
  int *piVar2;
  int iVar3;
  
  if ((*(int *)(param_1 + 0x90) != 0) &&
     (iVar3 = *(int *)(param_1 + 0x10) * 4, 0 < *(int *)(param_1 + 0x10))) {
    piVar1 = (int *)(*(int *)(param_1 + 0x90) + iVar3);
    do {
      piVar2 = (int *)*piVar1;
      if (piVar2 != (int *)0x0) {
        do {
          FUN_0fb84088(param_1,piVar2);
          piVar1 = (int *)(*(int *)(param_1 + 0x90) + iVar3);
          piVar2 = (int *)*piVar2;
        } while ((int *)*piVar1 != piVar2);
      }
      iVar3 = iVar3 + -4;
      piVar1 = piVar1 + -1;
    } while (iVar3 != 0);
  }
  return;
}



// === amallocblksize @ 0fb84198 (148 bytes) ===

undefined8 amallocblksize(undefined8 param_1,int param_2)

{
  longlong lVar1;
  undefined8 uVar2;
  
  if ((*(int *)(param_2 + 0x9c) != 0) && (lVar1 = (*_lock)(*(int *)(param_2 + 0x9c)), lVar1 != 0)) {
    uVar2 = FUN_0fb8422c(param_1,param_2);
    if (*(int *)(param_2 + 0x9c) != 0) {
      (*_ulock)(*(int *)(param_2 + 0x9c));
    }
    return uVar2;
  }
  uVar2 = FUN_0fb8422c(param_1,param_2);
  return uVar2;
}



// === FUN_0fb8422c @ 0fb8422c (124 bytes) ===

int FUN_0fb8422c(int param_1,int param_2)

{
  uint uVar1;
  
  if (param_1 == 0) {
    return 0;
  }
  if ((*(uint *)(param_1 + -4) & 2) == 0) {
    uVar1 = *(uint *)(param_1 - *(int *)(param_2 + 8));
    if ((uVar1 & 1) == 0) {
      return 0;
    }
    param_1 = (uVar1 & 0xfffffffe) - param_1;
  }
  else {
    uVar1 = *(uint *)(param_1 - 4U & 0xfffffffe);
    if ((uVar1 & 1) == 0) {
      return 0;
    }
    param_1 = *(int *)((uVar1 & 0xfffffffc) + 0x10);
  }
  return param_1;
}



// === amemalign @ 0fb842a8 (172 bytes) ===

undefined8 amemalign(undefined8 param_1,undefined8 param_2,int param_3)

{
  longlong lVar1;
  undefined8 uVar2;
  
  if ((*(int *)(param_3 + 0x9c) != 0) && (lVar1 = (*_lock)(*(int *)(param_3 + 0x9c)), lVar1 != 0)) {
    uVar2 = FUN_0fb84354(param_1,param_2,param_3);
    if (*(int *)(param_3 + 0x9c) != 0) {
      (*_ulock)(*(int *)(param_3 + 0x9c));
    }
    return uVar2;
  }
  uVar2 = FUN_0fb84354(param_1,param_2,param_3);
  return uVar2;
}



// === FUN_0fb84354 @ 0fb84354 (1020 bytes) ===

int FUN_0fb84354(uint param_1,longlong param_2,int param_3)

{
  int iVar1;
  uint *puVar2;
  uint uVar3;
  uint *puVar4;
  int iVar5;
  uint *puVar6;
  uint uVar7;
  uint *puVar8;
  
  if (((param_2 == 0) || ((param_1 & 3) != 0)) || (param_1 == 0)) {
    return 0;
  }
  if (param_1 < 0x11) {
    if (0x10U % param_1 == 0) {
      iVar1 = FUN_0fb82e6c(param_2,param_3);
      return iVar1;
    }
    uVar3 = *(uint *)(param_3 + 0x14);
  }
  else {
    uVar3 = *(uint *)(param_3 + 0x14);
  }
  uVar7 = (int)param_2 + 0xfU & 0xfffffff0;
  if (uVar7 <= uVar3) {
    uVar7 = uVar3 + 4;
  }
  iVar1 = FUN_0fb82e6c(param_1 + uVar7 + 0x20,param_3);
  if (iVar1 != 0) {
    iVar5 = (((iVar1 + param_1) - 1) / param_1) * param_1;
    puVar4 = (uint *)(iVar1 - *(int *)(param_3 + 8));
    puVar6 = (uint *)(*puVar4 & 0xfffffffe);
    if (iVar1 != iVar5) {
      if (iVar5 - iVar1 < 0x10) {
        iVar5 = iVar5 + param_1;
      }
      puVar8 = (uint *)(iVar5 - *(int *)(param_3 + 8));
      puVar2 = (uint *)((iVar5 + 0xfU & 0xfffffff0) + uVar7);
      puVar8[3] = 0;
      if ((uint)((int)puVar6 - (int)puVar2) < 0x10) {
        puVar6[1] = (uint)puVar8;
        *puVar8 = *puVar4;
      }
      else {
        puVar2[1] = (uint)puVar8;
        *puVar2 = (uint)puVar6;
        uVar3 = *puVar6;
        puVar6[1] = (uint)puVar2;
        if ((uVar3 & 1) == 0) {
          *(uint *)(puVar6[3] + 8) = puVar6[2];
          *(uint *)(puVar6[2] + 0xc) = puVar6[3];
          puVar6[2] = 0;
          puVar6[3] = 0;
          *puVar2 = uVar3;
          *(uint **)(uVar3 + 4) = puVar2;
        }
        if ((int)(*puVar2 - (int)puVar2) < *(int *)(param_3 + 0x18)) {
          if (*(int *)(param_3 + 0x24) == 0) {
            FUN_0fb83af8(param_3 + 0x6c,puVar2);
          }
          else {
            FUN_0fb83b14(param_3 + 0x6c,puVar2);
          }
        }
        else if (*(int *)(param_3 + 0x24) == 0) {
          FUN_0fb83af8(param_3 + 0x4c,puVar2);
        }
        else {
          FUN_0fb83b14(param_3 + 0x4c,puVar2);
        }
        *puVar8 = (uint)puVar2 | 1;
      }
      puVar8[1] = (uint)puVar4;
      *puVar4 = (uint)puVar8 & 0xfffffffe;
      if ((int)(((uint)puVar8 & 0xfffffffe) - (int)puVar4) < *(int *)(param_3 + 0x18)) {
        if (*(int *)(param_3 + 0x24) == 0) {
          FUN_0fb83af8(param_3 + 0x6c);
          iVar1 = *(int *)(param_3 + 0x94);
        }
        else {
          FUN_0fb83b14();
          iVar1 = *(int *)(param_3 + 0x94);
        }
      }
      else if (*(int *)(param_3 + 0x24) == 0) {
        FUN_0fb83af8(param_3 + 0x4c);
        iVar1 = *(int *)(param_3 + 0x94);
      }
      else {
        FUN_0fb83b14();
        iVar1 = *(int *)(param_3 + 0x94);
      }
      if (iVar1 != 0) {
        __acheckq(param_3);
      }
      return iVar5;
    }
    puVar2 = (uint *)(iVar5 + uVar7);
    if ((uint)((int)puVar6 - (int)puVar2) < 0x10) {
      iVar1 = *(int *)(param_3 + 0x94);
    }
    else {
      puVar2[1] = (uint)puVar4;
      *puVar2 = (uint)puVar6;
      uVar3 = *puVar6;
      puVar6[1] = (uint)puVar2;
      if ((uVar3 & 1) == 0) {
        *(uint *)(puVar6[3] + 8) = puVar6[2];
        *(uint *)(puVar6[2] + 0xc) = puVar6[3];
        puVar6[2] = 0;
        puVar6[3] = 0;
        *puVar2 = uVar3;
        *(uint **)(uVar3 + 4) = puVar2;
      }
      if ((int)(*puVar2 - (int)puVar2) < *(int *)(param_3 + 0x18)) {
        if (*(int *)(param_3 + 0x24) == 0) {
          FUN_0fb83af8(param_3 + 0x6c,puVar2);
        }
        else {
          FUN_0fb83b14(param_3 + 0x6c,puVar2);
        }
      }
      else if (*(int *)(param_3 + 0x24) == 0) {
        FUN_0fb83af8(param_3 + 0x4c,puVar2);
      }
      else {
        FUN_0fb83b14(param_3 + 0x4c,puVar2);
      }
      *puVar4 = (uint)puVar2 | 1;
      iVar1 = *(int *)(param_3 + 0x94);
    }
    if (iVar1 != 0) {
      __acheckq(param_3);
    }
    return iVar5;
  }
  return 0;
}



// === _getsystype @ 0fb84754 (52 bytes) ===

undefined8 _getsystype(void)

{
  long lVar1;
  
  lVar1 = sysconf(0xe);
  if (lVar1 < 2) {
    return 1;
  }
  return 2;
}



// === sysconf @ 0fb84788 (1244 bytes) ===

long sysconf(int __name)

{
  char *__nptr;
  int iVar1;
  long lVar2;
  
  switch(__name) {
  case 6:
    return 1;
  case 7:
    return 1;
  default:
    lVar2 = syssgi(0x16);
    return lVar2;
  case 9:
    return 0x20;
  case 10:
    return 8;
  case 0xb:
    iVar1 = getpagesize();
    return iVar1;
  case 0xc:
    return 4;
  case 0xd:
    return 0;
  case 0xe:
    lVar2 = sysmp(1);
    return lVar2;
  case 0xf:
    lVar2 = sysmp(2);
    return lVar2;
  case 0x10:
    lVar2 = syssgi(0x16,5);
    return lVar2;
  case 0x11:
    return 8;
  case 0x12:
    return 4;
  case 0x13:
    return 0xff;
  case 0x14:
    return 0x10;
  case 0x15:
    lVar2 = syssgi(0x16,0x15);
    return lVar2;
  case 0x16:
  case 0x40:
  case 0x41:
    return 1;
  case 0x17:
    return 1;
  case 0x18:
    return -1;
  case 0x20:
    return 0x20;
  case 0x21:
    return 0x404;
  case 0x22:
    return 0x20;
  case 0x23:
    return 0x4000;
  case 0x24:
    return 0x7fffffff;
  case 0x25:
    return 0x20;
  case 0x26:
    return 1;
  case 0x27:
    return 1;
  case 0x28:
    return 1;
  case 0x29:
    return 1;
  case 0x2a:
    return 1;
  case 0x2b:
    return 1;
  case 0x2c:
    return -1;
  case 0x2d:
    return 1;
  case 0x2e:
    return 1;
  case 0x2f:
    return 1;
  case 0x30:
    return 1;
  case 0x42:
    return 0x800;
  case 0x43:
    return 0x800;
  case 0x44:
    return 0;
  case 0x4b:
    return 1;
  case 0x4c:
    return 1;
  case 0x4d:
    return 99;
  case 0x4e:
    return 0x800;
  case 0x4f:
    return 99;
  case 0x50:
    return 1000;
  case 0x51:
    return 2;
  case 0x52:
    return 0x20;
  case 0x53:
    return 0x800;
  case 0x54:
    return 0xff;
  case 0x55:
    return 1;
  case 0x56:
    return 1;
  case 0x57:
    return 0x30a29;
  case 0x58:
    return 1;
  case 0x59:
    return 1;
  case 0x5a:
    return 1;
  case 0x5b:
    return 1;
  case 0x5c:
    return 1;
  case 0x5d:
    break;
  case 0x5e:
    return 1;
  case 0x5f:
    return 1;
  case 0x60:
    return 0x10;
  case 0x61:
    return 0x25;
  case 0x62:
    return 1;
  case 99:
    return 4;
  case 100:
  case 0x65:
    return 0x400;
  case 0x66:
    return 9;
  case 0x67:
  case 0x68:
  case 0x69:
  case 0x6a:
  case 0x6c:
  case 0x6d:
  case 0x6e:
  case 0x6f:
  case 0x70:
  case 0x71:
  case 0x72:
    lVar2 = ___pthread_sysconf(__name);
    return lVar2;
  case 0x6b:
    return 0x80;
  case 0x73:
    return 1;
  }
  __nptr = getenv("_XPG");
  if ((__nptr != (char *)0x0) && (iVar1 = atoi(__nptr), 0 < iVar1)) {
    return 0x30a29;
  }
  return -1;
}



// === ftruncate64 @ 0fb84c64 (44 bytes) ===

int ftruncate64(int __fd,__off_t __length)

{
  int iVar1;
  longlong in_a3;
  
  syscall(0);
  if (in_a3 == 0) {
    return 0;
  }
  iVar1 = _cerror(__fd,__length);
  return iVar1;
}



// === getdtablesize @ 0fb84c90 (32 bytes) ===

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

int getdtablesize(void)

{
  long lVar1;
  
  lVar1 = ulimit(4,0);
  return lVar1;
}



// === ulimit @ 0fb84cb0 (40 bytes) ===

long ulimit(int __cmd,...)

{
  long lVar1;
  longlong in_a3;
  
  syscall(0);
  if (in_a3 == 0) {
    return 0x427;
  }
  lVar1 = _cerror(__cmd);
  return lVar1;
}



// === getdtablehi @ 0fb84cd8 (56 bytes) ===

int getdtablehi(void)

{
  int iVar1;
  
  iVar1 = syssgi(0x6d);
  if (iVar1 < 1) {
    iVar1 = getdtablesize();
  }
  return iVar1;
}



// === gmtime @ 0fb84d10 (36 bytes) ===

tm * gmtime(time_t *__timer)

{
  tm *ptVar1;
  
  ptVar1 = (tm *)FUN_0fb84fbc(__timer,0,&DAT_0fbde4e0);
  return ptVar1;
}



// === gmtime_r @ 0fb84d34 (32 bytes) ===

tm * gmtime_r(time_t *__timer,tm *__tp)

{
  tm *ptVar1;
  
  ptVar1 = (tm *)FUN_0fb84fbc(__timer,0,__tp);
  return ptVar1;
}



// === localtime_r @ 0fb84d54 (584 bytes) ===

tm * localtime_r(time_t *__timer,tm *__tp)

{
  int iVar1;
  int iVar2;
  longlong lVar3;
  tm *ptVar4;
  int iVar5;
  int iStack_60;
  int iStack_5c;
  int aiStack_58 [2];
  
  if (_us_rsthread_misc == 0) {
    lVar3 = 0;
  }
  else {
    lVar3 = ___libc_locklocale();
  }
  FUN_0fb85774(*__timer);
  iVar5 = DAT_0fbd3a78;
  iStack_60 = *__timer - timezone;
  ptVar4 = gmtime_r(&iStack_60,__tp);
  if (daylight == 0) {
    if (lVar3 != 0) {
      ___libc_unlocklocale(lVar3);
    }
    return ptVar4;
  }
  if (iVar5 == 0) {
    iVar5 = ptVar4->tm_hour;
  }
  else {
    iVar5 = iStack_60;
    if (DAT_0fbde470 != -1) goto joined_r0x0fb84ee8;
    iVar5 = ptVar4->tm_hour;
  }
  iVar5 = ptVar4->tm_sec + iVar5 * 0xe10 + ptVar4->tm_yday * 0x15180 + ptVar4->tm_min * 0x3c;
joined_r0x0fb84ee8:
  iVar1 = DAT_0fbde474;
  iVar2 = DAT_0fbde470;
  if ((DAT_0fbde470 == -1) && (iVar2 = DAT_0fbde470, DAT_0fbde474 == -1)) {
    FUN_0fb86bf0(&iStack_5c,aiStack_58,ptVar4);
    iVar1 = aiStack_58[0];
    iVar2 = iStack_5c;
  }
  iStack_5c = iVar2;
  aiStack_58[0] = iVar1;
  if (iVar1 < iStack_5c) {
    if ((iVar5 < iVar1) || (iStack_5c <= iVar5)) {
      iStack_60 = *__timer - altzone;
      ptVar4 = gmtime_r(&iStack_60,__tp);
      ptVar4->tm_isdst = 1;
    }
  }
  else if ((iStack_5c <= iVar5) && (iVar5 < iVar1)) {
    iStack_60 = *__timer - altzone;
    ptVar4 = gmtime_r(&iStack_60,__tp);
    ptVar4->tm_isdst = 1;
  }
  if (lVar3 != 0) {
    ___libc_unlocklocale(lVar3);
  }
  return ptVar4;
}



// === localtime @ 0fb84f9c (32 bytes) ===

tm * localtime(time_t *__timer)

{
  tm *ptVar1;
  
  ptVar1 = localtime_r(__timer,(tm *)&DAT_0fbde4e0);
  return ptVar1;
}



// === FUN_0fb84fbc @ 0fb84fbc (684 bytes) ===

int * FUN_0fb84fbc(int *param_1,int param_2,int *param_3)

{
  int iVar1;
  uint uVar2;
  int *piVar3;
  uint uVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int unaff_gp_lo;
  
  iVar7 = *param_1 / 0x15180;
  param_2 = param_2 + *param_1 % 0x15180;
  if (param_2 < 0) {
    uVar2 = (0x1517f - param_2) / 0x15180;
    for (uVar4 = uVar2 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
      iVar7 = iVar7 + -1;
      param_2 = param_2 + 0x15180;
    }
    if ((int)uVar2 >> 2 != 0) {
      do {
        param_2 = param_2 + 0x54600;
        iVar7 = iVar7 + -4;
      } while (param_2 < 0);
    }
  }
  if (0x1517f < param_2) {
    iVar7 = iVar7 + param_2 / 0x15180;
    param_2 = param_2 % 0x15180;
  }
  iVar5 = (iVar7 + 4) % 7;
  param_3[2] = param_2 / 0xe10;
  param_3[1] = (param_2 % 0xe10) / 0x3c;
  *param_3 = (param_2 % 0xe10) % 0x3c;
  param_3[6] = iVar5;
  if (iVar5 < 0) {
    param_3[6] = iVar5 + 7;
  }
  iVar5 = 0x7b2;
  if (iVar7 < 0) {
    iVar5 = 0x7b1;
    while( true ) {
      if (((iVar5 % 4 == 0) && (iVar5 % 100 != 0)) || (iVar6 = 0, iVar5 % 400 == 0)) {
        iVar6 = 1;
      }
      iVar7 = *(int *)(iVar6 * 4 + unaff_gp_lo + 0x30f0) + iVar7;
      if (-1 < iVar7) break;
      iVar5 = iVar5 + -1;
    }
  }
  else {
    while( true ) {
      if (((iVar5 % 4 == 0) && (iVar5 % 100 != 0)) || (iVar6 = 0, iVar5 % 400 == 0)) {
        iVar6 = 1;
      }
      iVar1 = *(int *)(iVar6 * 4 + unaff_gp_lo + 0x30f0);
      if (iVar7 < iVar1) break;
      iVar7 = iVar7 - iVar1;
      iVar5 = iVar5 + 1;
    }
  }
  param_3[5] = iVar5 + -0x76c;
  param_3[7] = iVar7;
  param_3[4] = 0;
  piVar3 = (int *)(__mon_lengths + iVar6 * 0x30);
  if (iVar7 < *piVar3) {
    param_3[8] = 0;
  }
  else {
    iVar5 = param_3[4];
    iVar6 = piVar3[iVar5];
    do {
      iVar5 = iVar5 + 1;
      param_3[4] = iVar5;
      iVar7 = iVar7 - iVar6;
      iVar6 = piVar3[iVar5];
    } while (iVar6 <= iVar7);
    param_3[8] = 0;
  }
  param_3[3] = iVar7 + 1;
  return param_3;
}



// === difftime @ 0fb85268 (16 bytes) ===

double difftime(time_t __time1,time_t __time0)

{
  return (double)(__time1 - __time0);
}



// === mktime @ 0fb85278 (1272 bytes) ===

time_t mktime(tm *__tp)

{
  int iVar1;
  int *piVar2;
  char *pcVar3;
  longlong lVar4;
  int iVar6;
  int aiStack_70 [2];
  tm tStack_68;
  int aiStack_3c [3];
  longlong lStack_30;
  longlong lStack_28;
  tm *ptVar5;
  
  lStack_28 = 0;
  FUN_0fb85774(0);
  aiStack_70[0] =
       __tp->tm_sec + __tp->tm_min * 0x3c + __tp->tm_hour * 0xe10 + __tp->tm_mday * 0x15180 +
       -0x15180;
  if (aiStack_70[0] < 0x1e13381) {
    if (aiStack_70[0] < 0) {
      __tp->tm_year = (__tp->tm_year - -aiStack_70[0] / 0x1e13380) + -1;
      aiStack_70[0] = 0x1e13380 - -aiStack_70[0] % 0x1e13380;
    }
  }
  else {
    __tp->tm_year = __tp->tm_year + aiStack_70[0] / 0x1e13380;
    aiStack_70[0] = aiStack_70[0] % 0x1e13380;
  }
  iVar6 = __tp->tm_mon;
  if (iVar6 < 0xc) {
    if (iVar6 < 0) {
      __tp->tm_year = (__tp->tm_year - -iVar6 / 0xc) + -1;
      __tp->tm_mon = 0xc - -iVar6 % 0xc;
      goto LAB_0fb8540c;
    }
    iVar6 = __tp->tm_year;
  }
  else {
    __tp->tm_year = __tp->tm_year + iVar6 / 0xc;
    __tp->tm_mon = iVar6 % 0xc;
LAB_0fb8540c:
    iVar6 = __tp->tm_year;
  }
  if (0xee < iVar6) {
    return -1;
  }
  if (__tp->tm_year % 4 == 0) {
    iVar1 = *(int *)(__lyday_to_month + __tp->tm_mon * 4);
  }
  else {
    iVar1 = *(int *)(__yday_to_month + __tp->tm_mon * 4);
  }
  aiStack_70[0] =
       iVar1 * 0x15180 +
       ((int)(((uint)(iVar6 + 3 >> 1) >> 0x1e) + iVar6 + 3) >> 2) * 0x15180 + iVar6 * 0x1e13380 +
       aiStack_70[0] + 0x7c543000;
  FUN_0fb85774();
  iVar6 = altzone;
  if ((__tp->tm_isdst < 1) && (iVar6 = timezone, __tp->tm_isdst < 0)) {
    lStack_28 = -1;
  }
  aiStack_70[0] = iVar6 + aiStack_70[0];
  ptVar5 = localtime_r(aiStack_70,&tStack_68);
  lVar4 = (longlong)(int)ptVar5;
  if (lStack_28 == -1) {
    pcVar3 = DAT_0fbde470;
    iVar6 = DAT_0fbde474;
    if ((DAT_0fbde470 == (char *)0xffffffff) &&
       (pcVar3 = DAT_0fbde470, iVar6 = DAT_0fbde474, DAT_0fbde474 == -1)) {
      lStack_30 = lVar4;
      FUN_0fb86bf0(&tStack_68.tm_zone,aiStack_3c,lVar4);
      lVar4 = lStack_30;
      pcVar3 = tStack_68.tm_zone;
      iVar6 = aiStack_3c[0];
    }
    aiStack_3c[0] = iVar6;
    tStack_68.tm_zone = pcVar3;
    piVar2 = (int *)lVar4;
    iVar6 = *piVar2 + piVar2[1] * 0x3c + piVar2[2] * 0xe10 + piVar2[7] * 0x15180;
    if (aiStack_3c[0] < (int)tStack_68.tm_zone) {
      if ((timezone - altzone) + aiStack_3c[0] <= iVar6) {
        iVar6 = ptVar5->tm_sec;
        goto LAB_0fb85700;
      }
      if (iVar6 < (int)tStack_68.tm_zone) {
        iVar6 = ptVar5->tm_sec;
        goto LAB_0fb85700;
      }
      aiStack_70[0] = aiStack_70[0] - (timezone - altzone);
      ptVar5 = localtime_r(aiStack_70,&tStack_68);
    }
    else {
      if (iVar6 < (int)tStack_68.tm_zone) {
        iVar6 = ptVar5->tm_sec;
        goto LAB_0fb85700;
      }
      if ((timezone - altzone) + aiStack_3c[0] <= iVar6) {
        iVar6 = ptVar5->tm_sec;
        goto LAB_0fb85700;
      }
      aiStack_70[0] = aiStack_70[0] - (timezone - altzone);
      ptVar5 = localtime_r(aiStack_70,&tStack_68);
    }
  }
  iVar6 = ptVar5->tm_sec;
LAB_0fb85700:
  __tp->tm_sec = iVar6;
  __tp->tm_min = ptVar5->tm_min;
  __tp->tm_hour = ptVar5->tm_hour;
  __tp->tm_mday = ptVar5->tm_mday;
  __tp->tm_mon = ptVar5->tm_mon;
  __tp->tm_year = ptVar5->tm_year;
  __tp->tm_wday = ptVar5->tm_wday;
  __tp->tm_yday = ptVar5->tm_yday;
  __tp->tm_isdst = ptVar5->tm_isdst;
  if (-1 < aiStack_70[0]) {
    return aiStack_70[0];
  }
  return -1;
}



// === FUN_0fb85774 @ 0fb85774 (1444 bytes) ===

void FUN_0fb85774(int param_1)

{
  bool bVar1;
  undefined1 uVar2;
  undefined1 uVar3;
  undefined1 uVar4;
  undefined *puVar5;
  longlong lVar6;
  char *pcVar8;
  longlong lVar7;
  char cVar9;
  int *piVar10;
  int iVar11;
  int iVar12;
  int *piVar13;
  
  if (_us_rsthread_misc == 0) {
    lVar6 = 0;
  }
  else {
    lVar6 = ___libc_locklocale();
  }
  pcVar8 = getenv("TZ");
  puVar5 = tzname;
  if ((pcVar8 == (char *)0x0) || (*pcVar8 == '\0')) {
    *tzname = DAT_0fbde218;
    uVar3 = DAT_0fbde21b;
    uVar2 = DAT_0fbde21a;
    puVar5[1] = DAT_0fbde219;
    puVar5[2] = uVar2;
    puVar5[3] = uVar3;
    puVar5 = PTR___tzname1_0fbde494;
    *PTR___tzname1_0fbde494 = DAT_0fbde220;
    uVar3 = DAT_0fbde223;
    uVar2 = DAT_0fbde222;
    puVar5[1] = DAT_0fbde221;
    puVar5[2] = uVar2;
    puVar5[3] = uVar3;
    timezone = 0;
    altzone = 0;
    daylight = 0;
    if (lVar6 != 0) {
      ___libc_unlocklocale(lVar6);
    }
    return;
  }
  if (*pcVar8 == ':') {
    lVar7 = FUN_0fb86d80(pcVar8 + 1);
    if (lVar7 == 0) {
      if (param_1 == 0) {
        param_1 = time((time_t *)0x0);
      }
      tzname = (undefined *)FUN_0fb85e40(tzname,DAT_0fbd3a78[6]);
      puVar5 = PTR___tzname1_0fbde494;
      uVar2 = DAT_0fbde221;
      if (tzname != (undefined *)0x0) {
        *PTR___tzname1_0fbde494 = DAT_0fbde220;
        uVar4 = DAT_0fbde223;
        uVar3 = DAT_0fbde222;
        piVar10 = DAT_0fbd3a78;
        puVar5[1] = uVar2;
        puVar5[3] = uVar4;
        puVar5[2] = uVar3;
        daylight = 0;
        timezone = -*(int *)piVar10[5];
        iVar11 = 1;
        if (1 < piVar10[1]) {
          iVar12 = 0xc;
          do {
            piVar13 = (int *)(piVar10[5] + iVar12);
            if (piVar13[1] == 0) {
              tzname = (undefined *)FUN_0fb85e40(tzname,piVar13[2] + piVar10[6]);
              if (tzname == (undefined *)0x0) goto LAB_0fb85cf0;
              timezone = -*piVar13;
            }
            else {
              PTR___tzname1_0fbde494 =
                   (undefined *)FUN_0fb85e40(PTR___tzname1_0fbde494,piVar13[2] + piVar10[6]);
              if (PTR___tzname1_0fbde494 == (undefined *)0x0) goto LAB_0fb85cf0;
              daylight = 1;
            }
            iVar11 = iVar11 + 1;
            iVar12 = iVar12 + 0xc;
            piVar10 = DAT_0fbd3a78;
          } while (iVar11 < DAT_0fbd3a78[1]);
        }
        iVar11 = 0;
        DAT_0fbde470 = 0;
        DAT_0fbde474 = 0;
        if ((0 < *piVar10) && (*(int *)piVar10[3] < param_1)) {
          iVar12 = piVar10[4];
          do {
            piVar13 = (int *)(piVar10[5] + (uint)*(byte *)(iVar12 + iVar11) * 0xc);
            if (piVar13[1] == 0) {
              tzname = (undefined *)FUN_0fb85e40(tzname,piVar13[2] + piVar10[6]);
              if (tzname == (undefined *)0x0) break;
              timezone = -*piVar13;
            }
            else {
              PTR___tzname1_0fbde494 =
                   (undefined *)FUN_0fb85e40(PTR___tzname1_0fbde494,piVar13[2] + piVar10[6]);
              if (PTR___tzname1_0fbde494 == (undefined *)0x0) break;
              daylight = 1;
              altzone = -*piVar13;
              DAT_0fbde470 = *(undefined4 *)(DAT_0fbd3a78[3] + iVar11 * 4);
              if (iVar11 + 1 < *DAT_0fbd3a78) {
                DAT_0fbde474 = *(undefined4 *)(DAT_0fbd3a78[3] + iVar11 * 4 + 4);
              }
              else {
                DAT_0fbde474 = 0x7fffffff;
              }
            }
            iVar11 = iVar11 + 1;
            if (iVar11 < *DAT_0fbd3a78) {
              bVar1 = false;
              if (*(int *)(DAT_0fbd3a78[3] + iVar11 * 4) < param_1) {
                bVar1 = true;
              }
            }
            else {
              bVar1 = false;
            }
            if (!bVar1) break;
            iVar12 = DAT_0fbd3a78[4];
            piVar10 = DAT_0fbd3a78;
          } while( true );
        }
      }
    }
    else {
      DAT_0fbd3a78 = (int *)0x0;
    }
  }
  else {
    lVar7 = FUN_0fb85d7c(pcVar8,&tzname);
    if ((lVar7 == 0) ||
       (lVar7 = FUN_0fb85fcc(lVar7,&timezone,1), puVar5 = PTR___tzname1_0fbde494, lVar7 == 0)) {
      puVar5 = tzname;
      uVar2 = DAT_0fbde219;
      *tzname = DAT_0fbde218;
      uVar4 = DAT_0fbde21b;
      uVar3 = DAT_0fbde21a;
      puVar5[1] = uVar2;
      puVar5[3] = uVar4;
      puVar5[2] = uVar3;
      puVar5 = PTR___tzname1_0fbde494;
      uVar2 = DAT_0fbde221;
      *PTR___tzname1_0fbde494 = DAT_0fbde220;
      uVar4 = DAT_0fbde223;
      uVar3 = DAT_0fbde222;
      puVar5[1] = uVar2;
      puVar5[3] = uVar4;
      puVar5[2] = uVar3;
      timezone = 0;
      altzone = 0;
      daylight = 0;
    }
    else {
      *PTR___tzname1_0fbde494 = DAT_0fbde220;
      uVar3 = DAT_0fbde223;
      uVar2 = DAT_0fbde222;
      puVar5[1] = DAT_0fbde221;
      puVar5[2] = uVar2;
      puVar5[3] = uVar3;
      daylight = 0;
      DAT_0fbde470 = 0;
      DAT_0fbde474 = 0;
      altzone = timezone + -0xe10;
      pcVar8 = (char *)FUN_0fb85d7c(lVar7,&PTR___tzname1_0fbde494);
      puVar5 = PTR___tzname1_0fbde494;
      uVar2 = DAT_0fbde221;
      if (pcVar8 == (char *)0x0) {
        *PTR___tzname1_0fbde494 = DAT_0fbde220;
        uVar4 = DAT_0fbde223;
        uVar3 = DAT_0fbde222;
        puVar5[1] = uVar2;
        puVar5[3] = uVar4;
        puVar5[2] = uVar3;
        altzone = timezone;
      }
      else {
        DAT_0fbde470 = 0xffffffff;
        DAT_0fbde474 = 0xffffffff;
        daylight = 1;
        cVar9 = *pcVar8;
        if (cVar9 != '\0') {
          if ((cVar9 != ';') && (cVar9 != ',')) {
            pcVar8 = (char *)FUN_0fb85fcc(pcVar8,&altzone,1);
            if (pcVar8 == (char *)0x0) goto LAB_0fb85cf0;
            if (*pcVar8 == ';') {
              cVar9 = *pcVar8;
            }
            else {
              if (*pcVar8 != ',') goto LAB_0fb85cf0;
              cVar9 = *pcVar8;
            }
          }
          if (cVar9 == ';') {
            FUN_0fb86150(pcVar8 + 1,&DAT_0fbde470,&DAT_0fbde474);
          }
          else {
            FUN_0fb8630c(pcVar8 + 1,&DAT_0fbde470,&DAT_0fbde474,param_1);
          }
        }
      }
    }
  }
LAB_0fb85cf0:
  if (lVar6 != 0) {
    ___libc_unlocklocale(lVar6);
  }
  return;
}



// === tzset @ 0fb85d18 (28 bytes) ===

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void tzset(void)

{
  FUN_0fb85774(0);
  return;
}



// === FUN_0fb85d34 @ 0fb85d34 (72 bytes) ===

undefined8 FUN_0fb85d34(undefined1 param_1,longlong param_2)

{
  switch(param_1) {
  case 0:
  case 0x2b:
  case 0x2c:
  case 0x2d:
  case 0x30:
  case 0x31:
  case 0x32:
  case 0x33:
  case 0x34:
  case 0x35:
  case 0x36:
  case 0x37:
  case 0x38:
  case 0x39:
    return 0;
  case 0x3a:
    if (param_2 != 0) {
      return 0;
    }
  }
  return 1;
}



// === FUN_0fb85d7c @ 0fb85d7c (196 bytes) ===

undefined1 * FUN_0fb85d7c(undefined1 *param_1,undefined4 *param_2)

{
  undefined1 uVar1;
  longlong lVar2;
  undefined1 *puVar3;
  undefined1 *puVar4;
  
  lVar2 = FUN_0fb85d34(*param_1,1);
  if (lVar2 == 0) {
    return (undefined1 *)0x0;
  }
  puVar3 = param_1 + 1;
  lVar2 = FUN_0fb85d34(param_1[1],0);
  if (lVar2 != 0) {
    uVar1 = param_1[2];
    puVar4 = puVar3;
    while( true ) {
      puVar3 = puVar4 + 1;
      lVar2 = FUN_0fb85d34(uVar1,0);
      if (lVar2 == 0) break;
      uVar1 = puVar4[2];
      puVar4 = puVar3;
    }
  }
  uVar1 = *puVar3;
  *puVar3 = 0;
  lVar2 = FUN_0fb85e40(*param_2,param_1);
  *param_2 = (int)lVar2;
  if (lVar2 != 0) {
    *puVar3 = uVar1;
    return puVar3;
  }
  *puVar3 = uVar1;
  return (undefined1 *)0x0;
}



// === FUN_0fb85e40 @ 0fb85e40 (396 bytes) ===

char * FUN_0fb85e40(char *param_1,char *param_2)

{
  bool bVar1;
  size_t sVar2;
  size_t sVar3;
  uint uVar4;
  char *pcVar5;
  uint uVar6;
  
  sVar2 = strlen(param_1);
  sVar3 = strlen(param_2);
  if (sVar2 < sVar3) {
    sVar2 = 8;
    if (sVar3 < 8) {
      sVar2 = sVar3;
    }
    bVar1 = tzname != param_1;
    param_1 = (char *)realloc(*(undefined4 *)((uint)bVar1 * 4 + 0xfbde478),sVar2 + 1);
    *(char **)((uint)bVar1 * 4 + 0xfbde478) = param_1;
    if (param_1 == (char *)0x0) {
      return (char *)0x0;
    }
    strncpy(param_1,param_2,sVar2);
    param_1[sVar2] = '\0';
  }
  else {
    strcpy(param_1,param_2);
    if (sVar3 < 3) {
      uVar6 = 3 - sVar3;
      param_1 = param_1 + sVar3;
      for (uVar4 = uVar6 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
        sVar3 = sVar3 + 1;
        *param_1 = ' ';
        param_1 = param_1 + 1;
      }
      pcVar5 = param_1;
      if ((int)uVar6 >> 2 != 0) {
        do {
          param_1 = pcVar5 + 4;
          sVar3 = sVar3 + 4;
          builtin_strncpy(pcVar5,"    ",4);
          pcVar5 = param_1;
        } while (sVar3 != 3);
        sVar3 = 3;
      }
      *param_1 = '\0';
      param_1 = param_1 + -sVar3;
    }
  }
  return param_1;
}



// === FUN_0fb85fcc @ 0fb85fcc (276 bytes) ===

char * FUN_0fb85fcc(char *param_1,undefined8 param_2,longlong param_3)

{
  bool bVar1;
  char *pcVar2;
  int iVar3;
  int aiStack_50 [4];
  undefined8 uStack_40;
  
  iVar3 = 0;
  bVar1 = false;
  aiStack_50[0] = 0;
  if (param_3 != 0) {
    bVar1 = *param_1 == '-';
    if ((bVar1) || (*param_1 == '+')) {
      param_1 = param_1 + 1;
    }
  }
  uStack_40 = param_2;
  pcVar2 = (char *)FUN_0fb860e0(param_1,aiStack_50);
  if ((((pcVar2 != (char *)0x0) && (iVar3 = aiStack_50[0] * 0xe10, *pcVar2 == ':')) &&
      (pcVar2 = (char *)FUN_0fb860e0(pcVar2 + 1,aiStack_50), pcVar2 != (char *)0x0)) &&
     ((iVar3 = aiStack_50[0] * 0x3c + iVar3, *pcVar2 == ':' &&
      (pcVar2 = (char *)FUN_0fb860e0(pcVar2 + 1,aiStack_50), pcVar2 != (char *)0x0)))) {
    iVar3 = aiStack_50[0] + iVar3;
  }
  if (bVar1) {
    *(int *)uStack_40 = -iVar3;
  }
  else {
    *(int *)uStack_40 = iVar3;
  }
  return pcVar2;
}



// === FUN_0fb860e0 @ 0fb860e0 (112 bytes) ===

byte * FUN_0fb860e0(byte *param_1,int *param_2)

{
  byte *pbVar1;
  int iVar2;
  
  if ((_ctype[*param_1 + 1] & 4) == 0) {
    return (byte *)0x0;
  }
  *param_2 = 0;
  do {
    iVar2 = *param_2;
    *param_2 = iVar2 * 10;
    *param_2 = (uint)*param_1 + iVar2 * 10 + -0x30;
    pbVar1 = param_1 + 1;
    param_1 = param_1 + 1;
  } while ((_ctype[*pbVar1 + 1] & 4) != 0);
  return param_1;
}



// === FUN_0fb86150 @ 0fb86150 (444 bytes) ===

undefined8 FUN_0fb86150(undefined8 param_1,undefined8 param_2,undefined8 param_3)

{
  char cVar1;
  char *pcVar3;
  longlong lVar2;
  int iStack_50;
  int iStack_4c;
  int iStack_48;
  int iStack_44;
  undefined8 uStack_40;
  undefined8 uStack_38;
  
  iStack_4c = 0;
  iStack_50 = 0;
  uStack_40 = param_3;
  uStack_38 = param_2;
  pcVar3 = (char *)FUN_0fb860e0(param_1,&iStack_48);
  if (pcVar3 == (char *)0x0) {
    return 0;
  }
  iStack_48 = iStack_48 + -1;
  if ((iStack_48 < 0) || (0x16d < iStack_48)) {
    return 0;
  }
  if (*pcVar3 == '/') {
    pcVar3 = (char *)FUN_0fb85fcc(pcVar3 + 1,&iStack_4c,0);
    if (pcVar3 == (char *)0x0) {
      return 0;
    }
    cVar1 = *pcVar3;
  }
  else {
    cVar1 = *pcVar3;
  }
  if (cVar1 == ',') {
    pcVar3 = (char *)FUN_0fb860e0(pcVar3 + 1,&iStack_44);
    if (pcVar3 == (char *)0x0) {
      return 0;
    }
    iStack_44 = iStack_44 + -1;
    if ((iStack_44 < 0) || (0x16d < iStack_44)) {
      return 0;
    }
    if ((*pcVar3 == '/') && (lVar2 = FUN_0fb85fcc(pcVar3 + 1,&iStack_50,0), lVar2 == 0)) {
      return 0;
    }
  }
  *(int *)uStack_38 = iStack_48 * 0x15180 + iStack_4c;
  *(int *)uStack_40 = (iStack_44 * 0x15180 + iStack_50) - (timezone - altzone);
  return 1;
}



// === FUN_0fb8630c @ 0fb8630c (1640 bytes) ===

undefined8 FUN_0fb8630c(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4)

{
  int iVar1;
  int iVar2;
  char *pcVar4;
  longlong lVar3;
  tm *ptVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  int iStack_f0;
  int iStack_ec;
  char acStack_e8 [4];
  int iStack_e4;
  int iStack_e0;
  int iStack_dc;
  int iStack_d8;
  char acStack_d4 [4];
  int iStack_d0;
  int iStack_cc;
  int iStack_c8;
  int iStack_c4;
  int aiStack_c0 [2];
  undefined1 auStack_b8 [48];
  longlong lStack_88;
  undefined8 uStack_80;
  undefined8 uStack_78;
  undefined8 uStack_70;
  longlong lStack_60;
  
  iStack_ec = 0x1c20;
  iStack_f0 = 0x1c20;
  uStack_80 = param_2;
  uStack_78 = param_3;
  uStack_70 = param_4;
  pcVar4 = (char *)FUN_0fb86974(param_1,acStack_e8,&iStack_e4,&iStack_e0,&iStack_dc,&iStack_d8,
                                &iStack_ec);
  if (pcVar4 == (char *)0x0) {
    return 0;
  }
  if (*pcVar4 != ',') {
    return 0;
  }
  lVar3 = FUN_0fb86974(pcVar4 + 1,acStack_d4,&iStack_d0,&iStack_cc,&iStack_c8,&iStack_c4,&iStack_f0)
  ;
  if (lVar3 == 0) {
    return 0;
  }
  aiStack_c0[0] = (int)uStack_70 - timezone;
  ptVar5 = gmtime_r(aiStack_c0,(tm *)auStack_b8);
  lStack_60 = 1;
  do {
    iVar10 = iStack_e4;
    if ((acStack_e8[0] == 'J') &&
       ((((iVar9 = ptVar5->tm_year, iVar9 % 4 == 0 && (iVar9 % 100 != 0)) || (iVar9 % 400 == 0)) &&
        (0x3a < iStack_e4)))) {
      iVar10 = iStack_e4 + 1;
    }
    iVar9 = iStack_d0;
    if ((acStack_d4[0] == 'J') &&
       ((((iVar1 = ptVar5->tm_year, iVar1 % 4 == 0 && (iVar1 % 100 != 0)) || (iVar1 % 400 == 0)) &&
        (0x3a < iStack_d0)))) {
      iVar9 = iStack_d0 + 1;
    }
    if (acStack_e8[0] == 'M') {
      iVar10 = ptVar5->tm_year;
      iVar1 = iVar10 % 4;
      iVar6 = ptVar5->tm_wday - ptVar5->tm_yday % 7;
      iVar11 = 1;
      if (iVar6 < 0) {
        iVar6 = iVar6 + 7;
      }
      if (((iVar1 == 0) && (iVar10 % 100 != 0)) || (iVar10 % 400 == 0)) {
        iVar2 = *(int *)(__yday_to_month + iStack_e0 * 4 + 0x2c);
      }
      else {
        iVar2 = *(int *)(__mon_lengths + iStack_e0 * 4 + 0x5c);
      }
      iVar8 = iStack_e0 * 4;
      auStack_b8._40_8_ = SEXT48(iVar8);
      iVar2 = iVar2 + iVar6;
      lVar3 = 1;
      while( true ) {
        iVar6 = (int)lVar3;
        if (iStack_d8 == iVar2 % 7) break;
        iVar2 = iVar2 % 7 + 1;
        lVar3 = (longlong)(iVar6 + 1);
      }
      iVar2 = iVar6;
      if (iStack_dc != 1) {
        do {
          if (((iVar1 == 0) && (iVar10 % 100 != 0)) || (iVar7 = 0, iVar10 % 400 == 0)) {
            iVar7 = 1;
          }
          iVar11 = iVar11 + 1;
          lStack_88 = lVar3;
          if (*(int *)(__month_size + (iVar7 * 0xc + iStack_e0) * 4 + 0x14) < iVar2 + 7) {
            lVar3 = (longlong)iVar2;
            goto LAB_0fb86650;
          }
          iVar2 = iVar2 + 7;
        } while (iStack_dc != iVar11);
        lVar3 = (longlong)(iStack_dc * 7 + iVar6 + -7);
      }
LAB_0fb86650:
      if (((iVar1 == 0) && (iVar10 % 100 != 0)) || (iVar10 % 400 == 0)) {
        iVar10 = *(int *)(__yday_to_month + iVar8 + 0x2c) + (int)lVar3 + -1;
      }
      else {
        iVar10 = *(int *)(__mon_lengths + iVar8 + 0x5c) + (int)lVar3 + -1;
      }
    }
    if (acStack_d4[0] == 'M') {
      iVar9 = ptVar5->tm_year;
      iVar1 = iVar9 % 4;
      iVar6 = ptVar5->tm_wday - ptVar5->tm_yday % 7;
      iVar11 = 1;
      if (iVar6 < 0) {
        iVar6 = iVar6 + 7;
      }
      if (((iVar1 == 0) && (iVar9 % 100 != 0)) || (iVar9 % 400 == 0)) {
        iVar2 = *(int *)(__yday_to_month + iStack_cc * 4 + 0x2c);
      }
      else {
        iVar2 = *(int *)(__mon_lengths + iStack_cc * 4 + 0x5c);
      }
      iVar8 = 1;
      for (iVar2 = iVar2 + iVar6; iStack_c4 != iVar2 % 7; iVar2 = iVar2 % 7 + 1) {
        iVar8 = iVar8 + 1;
      }
      iVar6 = iVar8;
      if (iStack_c8 != 1) {
        do {
          if (((iVar1 == 0) && (iVar9 % 100 != 0)) || (iVar2 = 0, iVar9 % 400 == 0)) {
            iVar2 = 1;
          }
          iVar11 = iVar11 + 1;
          if (*(int *)(__month_size + (iVar2 * 0xc + iStack_cc) * 4 + 0x14) < iVar6 + 7)
          goto LAB_0fb86818;
          iVar6 = iVar6 + 7;
        } while (iStack_c8 != iVar11);
        iVar6 = iStack_c8 * 7 + iVar8 + -7;
      }
LAB_0fb86818:
      if (((iVar1 == 0) && (iVar9 % 100 != 0)) || (iVar9 % 400 == 0)) {
        iVar9 = *(int *)(__yday_to_month + iStack_cc * 4 + 0x2c);
      }
      else {
        iVar9 = *(int *)(__mon_lengths + iStack_cc * 4 + 0x5c);
      }
      iVar9 = iVar9 + iVar6 + -1;
    }
    if ((iVar10 <= iVar9) || (lStack_60 == 2)) break;
    aiStack_c0[0] = (int)uStack_70 - altzone;
    ptVar5 = gmtime_r(aiStack_c0,(tm *)auStack_b8);
    lStack_60 = (longlong)((int)lStack_60 + 1);
  } while (lStack_60 != 3);
  *(int *)uStack_80 = iVar10 * 0x15180 + iStack_ec;
  *(int *)uStack_78 = (iVar9 * 0x15180 + iStack_f0) - (timezone - altzone);
  return 1;
}



// === FUN_0fb86974 @ 0fb86974 (636 bytes) ===

char * FUN_0fb86974(byte *param_1,undefined1 *param_2,int *param_3,int *param_4,int *param_5,
                   int *param_6,undefined8 param_7)

{
  byte bVar1;
  int iVar2;
  char *pcVar3;
  
  bVar1 = *param_1;
  if ((_ctype[bVar1 + 1] & 4) == 0) {
    if (bVar1 == 0x4a) {
      pcVar3 = (char *)FUN_0fb860e0(param_1 + 1,param_3);
      if (pcVar3 == (char *)0x0) {
        return (char *)0x0;
      }
      iVar2 = *param_3;
      if ((iVar2 < 1) || (0x16d < iVar2)) {
        return (char *)0x0;
      }
      *param_3 = iVar2 + -1;
      *param_2 = 0x4a;
    }
    else {
      if (bVar1 != 0x4d) {
        return (char *)0x0;
      }
      pcVar3 = (char *)FUN_0fb860e0(param_1 + 1,param_4);
      if (pcVar3 == (char *)0x0) {
        return (char *)0x0;
      }
      if ((*param_4 < 1) || (0xc < *param_4)) {
        return (char *)0x0;
      }
      if (*pcVar3 != '.') {
        return (char *)0x0;
      }
      pcVar3 = (char *)FUN_0fb860e0(pcVar3 + 1,param_5);
      if (pcVar3 == (char *)0x0) {
        return (char *)0x0;
      }
      if ((*param_5 < 1) || (5 < *param_5)) {
        return (char *)0x0;
      }
      if (*pcVar3 != '.') {
        return (char *)0x0;
      }
      pcVar3 = (char *)FUN_0fb860e0(pcVar3 + 1,param_6);
      if (pcVar3 == (char *)0x0) {
        return (char *)0x0;
      }
      if ((*param_6 < 0) || (6 < *param_6)) {
        return (char *)0x0;
      }
      *param_2 = 0x4d;
    }
  }
  else {
    pcVar3 = (char *)FUN_0fb860e0(param_1,param_3);
    if (pcVar3 == (char *)0x0) {
      return (char *)0x0;
    }
    if ((*param_3 < 0) || (0x16d < *param_3)) {
      return (char *)0x0;
    }
    *param_2 = 0;
  }
  if ((*pcVar3 == '/') &&
     (pcVar3 = (char *)FUN_0fb85fcc(pcVar3 + 1,param_7,0), pcVar3 == (char *)0x0)) {
    return (char *)0x0;
  }
  return pcVar3;
}



// === FUN_0fb86bf0 @ 0fb86bf0 (268 bytes) ===

void FUN_0fb86bf0(int *param_1,int *param_2,int param_3)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  undefined4 *puVar4;
  
  iVar3 = 0;
  puVar4 = &__daytab;
  iVar2 = __daytab;
  while (*(int *)(param_3 + 0x14) < iVar2) {
    piVar1 = puVar4 + 3;
    puVar4 = puVar4 + 3;
    iVar3 = iVar3 + 1;
    iVar2 = *piVar1;
  }
  *param_1 = *(int *)(iVar3 * 0xc + 0xfbd1364);
  *param_2 = *(int *)(iVar3 * 0xc + 0xfbd1368);
  iVar2 = FUN_0fb86cfc(param_3,*param_1);
  *param_1 = iVar2;
  iVar2 = FUN_0fb86cfc(param_3,*param_2);
  *param_2 = iVar2;
  *param_1 = *param_1 * 0x15180 + 0x1c20;
  *param_2 = *param_2 * 0x15180 + 0xe10;
  return;
}



// === FUN_0fb86cfc @ 0fb86cfc (132 bytes) ===

/* WARNING: Instruction at (ram,0x0fb86d34) overlaps instruction at (ram,0x0fb86d30)
    */

int FUN_0fb86cfc(int param_1,int param_2)

{
  int iVar1;
  int iVar2;
  
  if (0x39 < param_2) {
    iVar1 = *(int *)(param_1 + 0x14);
    if (((iVar1 % 4 == 0) && (iVar1 % 100 != 0)) || (iVar2 = 0x16d, iVar1 % 400 == 0)) {
      iVar2 = 0x16e;
    }
    param_2 = iVar2 + param_2 + -0x16d;
  }
  return param_2 - (*(int *)(param_1 + 0x18) + (param_2 - *(int *)(param_1 + 0x1c)) + 700) % 7;
}



// === FUN_0fb86d80 @ 0fb86d80 (1652 bytes) ===

/* WARNING: Instruction at (ram,0x0fb86f20) overlaps instruction at (ram,0x0fb86f1c)
    */

undefined8 FUN_0fb86d80(char *param_1)

{
  char cVar1;
  bool bVar2;
  undefined1 *puVar3;
  undefined4 *puVar4;
  size_t *psVar6;
  size_t sVar7;
  size_t sVar8;
  int iVar9;
  ssize_t sVar10;
  longlong lVar5;
  void *pvVar11;
  undefined4 uVar12;
  byte *pbVar13;
  int iVar14;
  int iVar15;
  char *__name;
  int iVar16;
  char acStack_2470 [1024];
  undefined1 auStack_2070 [32];
  undefined1 auStack_2050 [4];
  undefined1 auStack_204c [4];
  undefined1 auStack_2048 [4];
  undefined1 auStack_2044 [8156];
  longlong lStack_68;
  longlong lStack_60;
  longlong lStack_58;
  longlong lStack_50;
  longlong lStack_48;
  
  psVar6 = DAT_0fbd3a78;
  if ((DAT_0fbd3a78 == (size_t *)0x0) && (psVar6 = calloc(1,0x1c), psVar6 == (size_t *)0x0)) {
    return 0xffffffffffffffff;
  }
  if (param_1 == (char *)0x0) {
    param_1 = s_localtime_0fbd1260;
  }
  bVar2 = *param_1 == '/';
  __name = param_1;
  DAT_0fbd3a78 = psVar6;
  if (!bVar2) {
    sVar7 = strlen(s__usr_lib_locale_TZ_0fbd1270);
    sVar8 = strlen(param_1);
    if (0x3ff < sVar7 + sVar8 + 1) {
      return 0xffffffffffffffff;
    }
    strcpy(acStack_2470,s__usr_lib_locale_TZ_0fbd1270);
    strcat(acStack_2470,"/");
    strcat(acStack_2470,param_1);
    __name = acStack_2470;
    if (*param_1 != '\0') {
      cVar1 = *param_1;
      while( true ) {
        param_1 = param_1 + 1;
        if (cVar1 == '.') {
          bVar2 = true;
        }
        __name = acStack_2470;
        if (*param_1 == '\0') break;
        cVar1 = *param_1;
      }
    }
  }
  if (bVar2) {
    iVar9 = access(__name,4);
    if (iVar9 != 0) {
      return 0xffffffffffffffff;
    }
  }
  iVar9 = open64(__name,0);
  if (iVar9 == -1) {
    return 0xffffffffffffffff;
  }
  sVar10 = read(iVar9,auStack_2070,0x2000);
  iVar9 = close(iVar9);
  if ((iVar9 != 0) || (sVar10 < 0x2c)) {
    return 0xffffffffffffffff;
  }
  sVar7 = FUN_0fb873f4(auStack_2050);
  *psVar6 = sVar7;
  sVar7 = FUN_0fb873f4(auStack_204c);
  psVar6[1] = sVar7;
  lVar5 = FUN_0fb873f4(auStack_2048);
  psVar6[2] = (size_t)lVar5;
  if ((int)*psVar6 < 0x173) {
    sVar7 = psVar6[1];
    if (((sVar7 != 0) && ((int)sVar7 < 0x101)) && (lVar5 < 0x33)) {
      if (sVar10 < (int)(*psVar6 * 5 + sVar7 * 6 + (size_t)lVar5 + 0x2c)) {
        return 0xffffffffffffffff;
      }
      if (psVar6[3] != 0) {
        free();
      }
      if (psVar6[4] != 0) {
        free();
      }
      if (psVar6[5] != 0) {
        free();
      }
      if (*psVar6 == 0) {
        psVar6[4] = 0;
        psVar6[3] = 0;
      }
      else {
        pvVar11 = calloc(*psVar6,4);
        psVar6[3] = (size_t)pvVar11;
        if (pvVar11 == (void *)0x0) {
          return 0xffffffffffffffff;
        }
        pvVar11 = calloc(*psVar6,1);
        psVar6[4] = (size_t)pvVar11;
        if (pvVar11 == (void *)0x0) {
          free(psVar6[3]);
          return 0xffffffffffffffff;
        }
      }
      pvVar11 = calloc(psVar6[1],0xc);
      psVar6[5] = (size_t)pvVar11;
      if (pvVar11 == (void *)0x0) {
        if (psVar6[3] != 0) {
          free();
        }
        if (psVar6[4] != 0) {
          free();
        }
        return 0xffffffffffffffff;
      }
      pvVar11 = calloc(psVar6[2] + 1,1);
      psVar6[6] = (size_t)pvVar11;
      if (pvVar11 != (void *)0x0) {
        sVar7 = *psVar6;
        iVar9 = 0;
        lVar5 = (longlong)(int)auStack_2044;
        if (0 < (int)sVar7) {
          lStack_58 = 0;
          lVar5 = (longlong)(int)auStack_2044;
          do {
            uVar12 = FUN_0fb873f4(lVar5);
            *(undefined4 *)(psVar6[3] + (int)lStack_58) = uVar12;
            lVar5 = (longlong)((int)lVar5 + 4);
            sVar7 = *psVar6;
            iVar9 = iVar9 + 1;
            lStack_58 = (longlong)((int)lStack_58 + 4);
          } while (iVar9 < (int)sVar7);
        }
        iVar9 = 0;
        if (0 < (int)sVar7) {
          puVar3 = (undefined1 *)lVar5;
          do {
            *(undefined1 *)(psVar6[4] + iVar9) = *puVar3;
            lVar5 = (longlong)((int)lVar5 + 1);
            iVar9 = iVar9 + 1;
            puVar3 = puVar3 + 1;
          } while (iVar9 < (int)*psVar6);
        }
        iVar9 = 0;
        iVar16 = 0;
        if (0 < (int)psVar6[1]) {
          iVar14 = (int)lVar5;
          iVar15 = iVar14 + 5;
          lStack_68 = lVar5;
          do {
            lStack_48 = (longlong)(iVar14 + 6);
            lStack_50 = (longlong)iVar15;
            lStack_60 = (longlong)(int)(psVar6[5] + iVar16);
            uVar12 = FUN_0fb873f4(lStack_68);
            iVar16 = iVar16 + 0xc;
            puVar4 = (undefined4 *)lStack_60;
            *puVar4 = uVar12;
            iVar9 = iVar9 + 1;
            iVar15 = (int)lStack_50 + 6;
            puVar4[1] = (uint)*(byte *)((int)lStack_50 + -1);
            iVar14 = (int)lStack_48;
            puVar4[2] = (uint)*(byte *)(iVar14 + -1);
            lVar5 = (longlong)((int)lStack_68 + 6);
            lStack_68 = lVar5;
          } while (iVar9 < (int)psVar6[1]);
        }
        iVar9 = 0;
        if (0 < (int)psVar6[2]) {
          puVar3 = (undefined1 *)lVar5;
          do {
            *(undefined1 *)(psVar6[6] + iVar9) = *puVar3;
            iVar9 = iVar9 + 1;
            puVar3 = puVar3 + 1;
          } while (iVar9 < (int)psVar6[2]);
        }
        *(undefined1 *)(psVar6[6] + iVar9) = 0;
        iVar9 = 0;
        if (0 < (int)*psVar6) {
          pbVar13 = (byte *)psVar6[4];
          do {
            iVar9 = iVar9 + 1;
            if ((int)psVar6[1] <= (int)(uint)*pbVar13) {
              return 0xffffffffffffffff;
            }
            pbVar13 = pbVar13 + 1;
          } while (iVar9 < (int)*psVar6);
        }
        if (0 < (int)psVar6[1]) {
          sVar7 = psVar6[5];
          iVar9 = 0;
          do {
            iVar9 = iVar9 + 0xc;
            if ((int)psVar6[2] <= *(int *)(sVar7 + 8)) {
              return 0xffffffffffffffff;
            }
            sVar7 = sVar7 + 0xc;
          } while (iVar9 < (int)(psVar6[1] * 0xc));
        }
        return 0;
      }
      if (psVar6[3] != 0) {
        free();
      }
      if (psVar6[4] != 0) {
        free();
      }
      free(psVar6[5]);
      return 0xffffffffffffffff;
    }
  }
  return 0xffffffffffffffff;
}



// === FUN_0fb873f4 @ 0fb873f4 (88 bytes) ===

undefined4 FUN_0fb873f4(undefined4 *param_1)

{
  return *param_1;
}



// === access @ 0fb8744c (40 bytes) ===

int access(char *__name,int __type)

{
  int iVar1;
  longlong in_a3;
  
  syscall(0);
  if (in_a3 == 0) {
    return 0x409;
  }
  iVar1 = _cerror(__name,__type);
  return iVar1;
}



// === strrchr @ 0fb87474 (48 bytes) ===

char * strrchr(char *__s,int __c)

{
  byte bVar1;
  byte *pbVar2;
  
  pbVar2 = (byte *)0x0;
  do {
    if ((uint)(byte)*__s == (__c & 0xffU)) {
      bVar1 = *__s;
      pbVar2 = (byte *)__s;
    }
    else {
      bVar1 = *__s;
    }
    __s = __s + 1;
  } while (bVar1 != 0);
  return (char *)pbVar2;
}



// === strcat @ 0fb874a4 (188 bytes) ===

char * strcat(char *__dest,char *__src)

{
  uint uVar1;
  uint uVar2;
  char *pcVar3;
  int iVar4;
  char *pcVar5;
  char cVar6;
  char cVar7;
  
  pcVar5 = __dest;
  do {
    pcVar3 = pcVar5;
    pcVar5 = pcVar3 + 1;
  } while (*pcVar3 != '\0');
  cVar6 = *__src;
  iVar4 = ((uint)pcVar3 | 3) - (int)pcVar3;
  if (cVar6 != '\0') {
    cVar7 = __src[1];
    if (cVar7 != '\0') {
      uVar1 = *(uint *)__src;
      if (__src[2] != '\0') {
        cVar6 = __src[3];
        uVar2 = (uint)pcVar3 & 3;
        *(uint *)(pcVar3 + -uVar2) =
             *(uint *)(pcVar3 + -uVar2) & -1 << (4 - uVar2) * 8 | uVar1 >> uVar2 * 8;
        if (cVar6 == '\0') {
          pcVar3 = pcVar3 + 3;
          uVar2 = (uint)pcVar3 & 3;
          *(uint *)(pcVar3 + -uVar2) =
               *(uint *)(pcVar3 + -uVar2) & 0xffffffffU >> (uVar2 + 1) * 8 |
               uVar1 << (3 - uVar2) * 8;
          return __dest;
        }
        pcVar3 = (char *)(((uint)pcVar3 | 3) - 3);
        pcVar5 = __src + iVar4;
        while( true ) {
          cVar6 = pcVar5[1];
          pcVar3 = pcVar3 + 4;
          if (cVar6 == '\0') break;
          cVar7 = pcVar5[2];
          if (cVar7 == '\0') goto LAB_0fb87554;
          if (pcVar5[3] == '\0') goto LAB_0fb87550;
          cVar6 = pcVar5[4];
          *(undefined4 *)pcVar3 = *(undefined4 *)(pcVar5 + 1);
          pcVar5 = pcVar5 + 4;
          if (cVar6 == '\0') {
            return __dest;
          }
        }
        goto LAB_0fb87558;
      }
LAB_0fb87550:
      pcVar3[2] = '\0';
    }
LAB_0fb87554:
    pcVar3[1] = cVar7;
  }
LAB_0fb87558:
  *pcVar3 = cVar6;
  return __dest;
}



// === geteuid @ 0fb87560 (44 bytes) ===

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

__uid_t geteuid(void)

{
  __uid_t _Var1;
  __uid_t in_v1_lo;
  longlong in_a3;
  
  syscall(0);
  if (in_a3 == 0) {
    return in_v1_lo;
  }
  _Var1 = _cerror();
  return _Var1;
}



// === fcntl @ 0fb8758c (40 bytes) ===

int fcntl(int __fd,int __cmd,...)

{
  int iVar1;
  longlong in_a3;
  
  syscall(0);
  if (in_a3 == 0) {
    return 0x426;
  }
  iVar1 = _cerror(__fd,__cmd);
  return iVar1;
}



// === _blk_init @ 0fb875b4 (56 bytes) ===

void _blk_init(void)

{
  longlong lVar1;
  
  lVar1 = syssgi(0x81);
  _blk_fp = (undefined4)lVar1;
  if (lVar1 == -1) {
    _blk_fp = 0;
  }
  return;
}



// === syssgi @ 0fb875ec (40 bytes) ===

undefined8 syssgi(void)

{
  undefined8 uVar1;
  longlong in_a3;
  
  syscall(0);
  if (in_a3 == 0) {
    return 0x410;
  }
  uVar1 = _cerror();
  return uVar1;
}



// === oserror @ 0fb87614 (12 bytes) ===

undefined4 oserror(void)

{
  return *(undefined4 *)__errnoaddr;
}



// === setoserror @ 0fb87620 (20 bytes) ===

undefined8 setoserror(undefined8 param_1)

{
  errno = (undefined4)param_1;
  *(undefined4 *)__errnoaddr = errno;
  return param_1;
}



// === goserror @ 0fb87634 (8 bytes) ===

undefined4 goserror(void)

{
  return errno;
}



// === __oserror @ 0fb8763c (8 bytes) ===

undefined * __oserror(void)

{
  return __errnoaddr;
}



// === __initperthread_errno @ 0fb87644 (16 bytes) ===

void __initperthread_errno(void)

{
  __errnoaddr = (undefined *)0x200e84;
  return;
}



// === strchr @ 0fb87654 (44 bytes) ===

char * strchr(char *__s,int __c)

{
  byte bVar1;
  
  do {
    if ((uint)(byte)*__s == (__c & 0xffU)) {
      return (char *)(byte *)__s;
    }
    bVar1 = *__s;
    __s = __s + 1;
  } while (bVar1 != 0);
  return (char *)0x0;
}



// === syslog @ 0fb87680 (64 bytes) ===

void syslog(int __pri,char *__fmt,...)

{
  undefined8 in_a2;
  undefined8 in_a3;
  undefined8 in_t0;
  undefined8 in_t1;
  undefined8 in_t2;
  undefined8 in_t3;
  undefined8 uStack_30;
  undefined8 uStack_28;
  undefined8 uStack_20;
  undefined8 uStack_18;
  undefined8 uStack_10;
  undefined8 uStack_8;
  
  uStack_30 = in_a2;
  uStack_28 = in_a3;
  uStack_20 = in_t0;
  uStack_18 = in_t1;
  uStack_10 = in_t2;
  uStack_8 = in_t3;
  vsyslog(__pri,__fmt,&uStack_30);
  return;
}



// === vsyslog @ 0fb876c0 (1308 bytes) ===

void vsyslog(int __pri,char *__fmt,__gnuc_va_list __ap)

{
  char *pcVar1;
  bool bVar2;
  int *piVar3;
  tm *__tp;
  char *pcVar4;
  __pid_t _Var5;
  char *pcVar6;
  int iVar7;
  size_t __n;
  ssize_t sVar8;
  __pid_t _Var9;
  char cVar10;
  char *pcVar11;
  char *pcVar12;
  time_t atStack_4b0 [2];
  char acStack_4a8 [32];
  char *pcStack_488;
  char acStack_480 [1024];
  char acStack_80 [16];
  int aiStack_70 [2];
  longlong lStack_68;
  
  piVar3 = (int *)__oserror();
  iVar7 = *piVar3;
  if (((0x17 < (uint)((int)(__pri & 0x3f8U) >> 3)) || ((DAT_0fbde4b0 & 1 << (__pri & 7U)) == 0)) ||
     ((__pri & 0xfffffc00U) != 0)) {
    return;
  }
  if (DAT_0fbde4a4 < 0) {
    openlog(PTR_s_syslog_0fbde4a8,DAT_0fbd3a88 | 8,0);
  }
  if (DAT_0fbd3a8c == (char *)0x0) {
    DAT_0fbd3a8c = (char *)malloc(0x1000);
  }
  if ((__pri & 0x3f8U) == 0) {
    __pri = DAT_0fbde4ac | __pri;
  }
  time(atStack_4b0);
  __tp = localtime(atStack_4b0);
  pcVar4 = asctime_r(__tp,acStack_4a8);
  sprintf(DAT_0fbd3a8c,s_<_d>__15s_0fbd2818,__pri,pcVar4 + 4);
  cVar10 = *DAT_0fbd3a8c;
  pcVar4 = DAT_0fbd3a8c;
  while (cVar10 != '\0') {
    pcVar12 = pcVar4 + 1;
    pcVar4 = pcVar4 + 1;
    cVar10 = *pcVar12;
  }
  if ((DAT_0fbd3a88 & 0x20) != 0) {
    pcStack_488 = pcVar4;
  }
  lStack_68 = (longlong)(int)pcStack_488;
  if (PTR_s_syslog_0fbde4a8 != (undefined *)0x0) {
    strcpy(pcVar4,PTR_s_syslog_0fbde4a8);
    cVar10 = *pcVar4;
    while (cVar10 != '\0') {
      pcVar12 = pcVar4 + 1;
      pcVar4 = pcVar4 + 1;
      cVar10 = *pcVar12;
    }
  }
  if ((DAT_0fbd3a88 & 1) != 0) {
    _Var5 = getpid();
    sprintf(pcVar4,"[%d]",_Var5);
    cVar10 = *pcVar4;
    while (cVar10 != '\0') {
      pcVar12 = pcVar4 + 1;
      pcVar4 = pcVar4 + 1;
      cVar10 = *pcVar12;
    }
  }
  if ((PTR_s_syslog_0fbde4a8 != (undefined *)0x0) && (*PTR_s_syslog_0fbde4a8 != '\0')) {
    *pcVar4 = ':';
    pcVar4[1] = ' ';
    pcVar4 = pcVar4 + 2;
  }
  cVar10 = *__fmt;
  pcVar12 = acStack_480;
  if ((cVar10 != '\0') && (cVar10 != '\n')) {
    do {
      if (cVar10 == '%') {
        if (__fmt[1] != 'm') {
          *pcVar12 = '%';
          goto LAB_0fb87948;
        }
        pcVar11 = __fmt + 1;
        pcVar6 = strerror(iVar7);
        if (pcVar6 == (char *)0x0) goto LAB_0fb8794c;
        if (*pcVar6 == '\0') {
          cVar10 = __fmt[2];
        }
        else if (pcVar12 < acStack_80) {
          cVar10 = *pcVar6;
          while( true ) {
            *pcVar12 = cVar10;
            pcVar1 = pcVar6 + 1;
            pcVar12 = pcVar12 + 1;
            pcVar6 = pcVar6 + 1;
            if (*pcVar1 == '\0') {
              bVar2 = false;
            }
            else {
              bVar2 = false;
              if (pcVar12 < acStack_80) {
                bVar2 = true;
              }
            }
            if (!bVar2) break;
            cVar10 = *pcVar6;
          }
          cVar10 = __fmt[2];
        }
        else {
          cVar10 = __fmt[2];
        }
      }
      else {
        *pcVar12 = cVar10;
LAB_0fb87948:
        pcVar12 = pcVar12 + 1;
        pcVar11 = __fmt;
LAB_0fb8794c:
        cVar10 = pcVar11[1];
      }
      __fmt = pcVar11 + 1;
      if (cVar10 == '\0') {
        bVar2 = false;
      }
      else {
        bVar2 = false;
        if ((cVar10 != '\n') && (bVar2 = false, pcVar12 < acStack_80)) {
          bVar2 = true;
        }
      }
    } while (bVar2);
  }
  pcVar12[1] = '\0';
  *pcVar12 = '\n';
  iVar7 = _vsprintf(pcVar4,acStack_480,__ap);
  if ((0x800 < iVar7) && (iVar7 < 4000)) {
    write(DAT_0fbde4a4,s_overly_long_syslog_message_detec_0fbd30d8,0x32);
  }
  if (4000 < iVar7) {
    write(DAT_0fbde4a4,s_overly_long_syslog_message__inte_0fbd3110,0x3d);
                    /* WARNING: Subroutine does not return */
    abort();
  }
  __n = strlen(DAT_0fbd3a8c);
  if ((DAT_0fbd3a88 & 0x20) != 0) {
    write(2,(void *)lStack_68,__n - ((int)(void *)lStack_68 - (int)DAT_0fbd3a8c));
  }
  sVar8 = write(DAT_0fbde4a4,DAT_0fbd3a8c,__n);
  if ((-1 < sVar8) || ((DAT_0fbd3a88 & 2) == 0)) {
    return;
  }
  _Var5 = fork();
  if (_Var5 == -1) {
    return;
  }
  if (_Var5 == 0) {
    BSDsignal(0xe,0);
    sigfillset((sigset_t *)acStack_80);
    sigdelset((sigset_t *)acStack_80,0xe);
    sigprocmask(3,(sigset_t *)acStack_80,(sigset_t *)0x0);
    alarm(5);
    iVar7 = open64(s__dev_console_0fbd2828,1,0);
    if (iVar7 < 0) {
                    /* WARNING: Subroutine does not return */
      _exit(0);
    }
    alarm(0);
    strcat(DAT_0fbd3a8c,"\r");
    pcVar4 = strchr(DAT_0fbd3a8c,0x3e);
    write(iVar7,pcVar4 + 1,__n - ((int)pcVar4 - (int)DAT_0fbd3a8c));
    close(iVar7);
                    /* WARNING: Subroutine does not return */
    _exit(0);
  }
  if ((((DAT_0fbd3a88 & 0x10) == 0) && (_Var9 = waitpid(_Var5,aiStack_70,0), _Var9 < 0)) &&
     (piVar3 = (int *)__oserror(), *piVar3 == 4)) {
    do {
      _Var9 = waitpid(_Var5,aiStack_70,0);
      bVar2 = false;
      if (_Var9 < 0) {
        piVar3 = (int *)__oserror();
        bVar2 = false;
        if (*piVar3 == 4) {
          bVar2 = true;
        }
      }
    } while (bVar2);
  }
  return;
}



// === openlog @ 0fb87be8 (184 bytes) ===

void openlog(char *__ident,int __option,int __facility)

{
  int iVar1;
  
  _using_syslog = 1;
  if (__ident != (char *)0x0) {
    PTR_s_syslog_0fbde4a8 = __ident;
  }
  iVar1 = DAT_0fbde4ac;
  if ((__facility == 0) || (iVar1 = __facility, (__facility & 0xfffffc07U) == 0)) {
    DAT_0fbde4ac = iVar1;
  }
  DAT_0fbd3a88 = __option;
  if ((DAT_0fbde4a4 < 0) && ((__option & 8U) != 0)) {
    DAT_0fbde4a4 = open64(s__dev_log_0fbd2838,5);
    if (DAT_0fbde4a4 < 0) {
      DAT_0fbde4a4 = open64(s__dev_console_0fbd2828,1);
    }
    else {
      fcntl(DAT_0fbde4a4,4,0);
    }
    fcntl(DAT_0fbde4a4,2,1);
  }
  return;
}



// === closelog @ 0fb87ca0 (40 bytes) ===

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void closelog(void)

{
  close(DAT_0fbde4a4);
  DAT_0fbde4a4 = 0xffffffff;
  return;
}



// === setlogmask @ 0fb87cc8 (24 bytes) ===

int setlogmask(int __mask)

{
  int iVar1;
  
  iVar1 = DAT_0fbde4b0;
  if (__mask != 0) {
    DAT_0fbde4b0 = __mask;
  }
  return iVar1;
}



// === alarm @ 0fb87ce0 (40 bytes) ===

uint alarm(uint __seconds)

{
  uint uVar1;
  longlong in_a3;
  
  syscall(0);
  if (in_a3 == 0) {
    return 0x403;
  }
  uVar1 = _cerror(__seconds);
  return uVar1;
}



// === waitpid @ 0fb87d08 (200 bytes) ===

__pid_t waitpid(__pid_t __pid,int *__stat_loc,int __options)

{
  longlong lVar1;
  int iVar2;
  undefined8 uVar3;
  undefined1 auStack_c0 [4];
  undefined4 uStack_bc;
  __pid_t _Stack_b4;
  uint uStack_ac;
  
  if (__pid < 1) {
    iVar2 = -__pid;
    if (__pid < -1) {
      uVar3 = 2;
    }
    else {
      uVar3 = 7;
      if (__pid == -1) {
        iVar2 = 0;
      }
      else {
        uVar3 = 2;
        iVar2 = getpgid(0);
      }
    }
  }
  else {
    uVar3 = 0;
    iVar2 = __pid;
  }
  lVar1 = waitsys(uVar3,iVar2,auStack_c0,__options | 3,0);
  if (lVar1 < 0) {
    return (__pid_t)lVar1;
  }
  if (__stat_loc != (int *)0x0) {
    iVar2 = _wstat(uStack_bc,uStack_ac & 0xff);
    *__stat_loc = iVar2;
  }
  return _Stack_b4;
}



// === waitsys @ 0fb87dd0 (40 bytes) ===

undefined8 waitsys(void)

{
  undefined8 uVar1;
  longlong in_a3;
  
  syscall(0);
  if (in_a3 == 0) {
    return 0x492;
  }
  uVar1 = _cerror();
  return uVar1;
}



// === strncpy @ 0fb87df8 (192 bytes) ===

char * strncpy(char *__dest,char *__src,size_t __n)

{
  char cVar1;
  bool bVar2;
  char *pcVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  char *pcVar7;
  char *pcVar8;
  
  pcVar7 = __dest;
  if (__n != 0) {
    cVar1 = *__src;
    pcVar7 = __dest + 1;
    pcVar3 = __src + 1;
    *__dest = cVar1;
    pcVar8 = pcVar7;
    if (cVar1 != '\0') {
      do {
        __n = __n - 1;
        bVar2 = false;
        pcVar7 = pcVar8;
        if (__n != 0) {
          cVar1 = *pcVar3;
          pcVar7 = pcVar8 + 1;
          pcVar3 = pcVar3 + 1;
          *pcVar8 = cVar1;
          if (cVar1 == '\0') {
            bVar2 = false;
          }
          else {
            bVar2 = true;
          }
        }
        pcVar8 = pcVar7;
      } while (bVar2);
    }
  }
  uVar5 = __n - 1;
  if ((__n != 0) && (uVar4 = uVar5 & 3, uVar6 = uVar5, uVar5 != 0)) {
    for (; uVar4 != 0; uVar4 = uVar4 - 1) {
      *pcVar7 = '\0';
      pcVar7 = pcVar7 + 1;
      uVar6 = uVar6 - 1;
    }
    uVar5 = (int)uVar5 >> 2;
    while (uVar5 != 0) {
      *pcVar7 = '\0';
      pcVar7[1] = '\0';
      pcVar7[2] = '\0';
      pcVar7[3] = '\0';
      uVar6 = uVar6 - 4;
      pcVar7 = pcVar7 + 4;
      uVar5 = uVar6;
    }
  }
  return __dest;
}



// === _memset_fp @ 0fb87ec0 (88 bytes) ===

void _memset_fp(undefined8 *param_1,undefined8 param_2,int param_3)

{
  undefined8 *puVar1;
  undefined8 *puVar2;
  undefined8 uVar3;
  
  *param_1 = param_2;
  uVar3 = *param_1;
  puVar1 = param_1;
  do {
    *puVar1 = uVar3;
    puVar1[1] = uVar3;
    puVar1[2] = uVar3;
    puVar1[3] = uVar3;
    puVar2 = puVar1 + 8;
    if (puVar1 + 4 == (undefined8 *)((int)param_1 + param_3)) {
      return;
    }
    puVar1[4] = uVar3;
    puVar1[5] = uVar3;
    puVar1[6] = uVar3;
    puVar1[7] = uVar3;
    puVar1 = puVar2;
  } while (puVar2 != (undefined8 *)((int)param_1 + param_3));
  return;
}



// === __sigfillset @ 0fb87f18 (24 bytes) ===

void __sigfillset(undefined4 *param_1)

{
  param_1[3] = 0;
  param_1[2] = 0;
  param_1[1] = 0xffffffff;
  *param_1 = 0xffffffff;
  return;
}



// === tempnam @ 0fb87f30 (680 bytes) ===

char * tempnam(char *__dir,char *__pfx)

{
  char cVar1;
  bool bVar2;
  char *pcVar3;
  size_t sVar4;
  size_t sVar5;
  size_t sVar6;
  char *__dest;
  int iVar7;
  longlong lVar8;
  
  sVar4 = 0;
  sVar5 = 0;
  pcVar3 = getenv("TMPDIR");
  if (pcVar3 != (char *)0x0) {
    sVar4 = strlen(pcVar3);
  }
  bVar2 = (int)sVar4 < 0;
  if (__dir != (char *)0x0) {
    sVar5 = strlen(__dir);
    bVar2 = (int)sVar4 < (int)sVar5;
  }
  sVar6 = sVar4;
  if (bVar2) {
    sVar6 = sVar5;
  }
  if ((int)sVar6 < 9) {
    sVar6 = 9;
  }
  else {
    sVar6 = sVar4;
    if ((int)sVar4 < (int)sVar5) {
      sVar6 = sVar5;
    }
  }
  __dest = (char *)malloc(sVar6 + 0x10);
  if (__dest == (char *)0x0) {
    return (char *)0x0;
  }
  lVar8 = 0;
  if (_us_rsthread_stdio != 0) {
    lVar8 = __libc_lockopen();
  }
  if (0 < (int)sVar4) {
    pcVar3 = (char *)FUN_0fb881d8(__dest,pcVar3);
    iVar7 = access(pcVar3,3);
    if (iVar7 == 0) goto LAB_0fb880d8;
  }
  if (0 < (int)sVar5) {
    pcVar3 = (char *)FUN_0fb881d8(__dest,__dir);
    iVar7 = access(pcVar3,3);
    if (iVar7 == 0) goto LAB_0fb880d8;
  }
  pcVar3 = (char *)FUN_0fb881d8(__dest,"/var/tmp/");
  iVar7 = access(pcVar3,3);
  if (iVar7 != 0) {
    pcVar3 = (char *)FUN_0fb881d8(__dest,&DAT_0fbde278);
    iVar7 = access(pcVar3,3);
    if (iVar7 != 0) {
      if (lVar8 != 0) {
        __libc_unlockopen(lVar8);
      }
      free(__dest);
      return (char *)0x0;
    }
  }
LAB_0fb880d8:
  strcat(__dest,"/");
  if (__pfx != (char *)0x0) {
    sVar4 = strlen(__dest);
    (__dest + sVar4)[5] = '\0';
    strncat(__dest,__pfx,5);
  }
  strcat(__dest,&DAT_0fbde4b8);
  strcat(__dest,"XXXXXX");
  pcVar3 = &DAT_0fbde4b8;
  cVar1 = DAT_0fbde4b8;
  while (cVar1 == 'Z') {
    cVar1 = pcVar3[1];
    *pcVar3 = 'A';
    pcVar3 = pcVar3 + 1;
  }
  if (*pcVar3 != '\0') {
    *pcVar3 = *pcVar3 + '\x01';
  }
  pcVar3 = mktemp(__dest);
  if (*pcVar3 == '\0') {
    if (lVar8 != 0) {
      __libc_unlockopen(lVar8);
    }
    free(__dest);
    return (char *)0x0;
  }
  if (lVar8 != 0) {
    __libc_unlockopen(lVar8);
  }
  return __dest;
}



// === FUN_0fb881d8 @ 0fb881d8 (76 bytes) ===

char * FUN_0fb881d8(char *param_1,longlong param_2)

{
  size_t sVar1;
  
  if (param_2 != 0) {
    strcpy(param_1,(char *)param_2);
    sVar1 = strlen(param_1);
    if (param_1[sVar1 - 1] == '/') {
      param_1[sVar1 - 1] = '\0';
    }
  }
  return param_1;
}



// === mktemp @ 0fb88224 (396 bytes) ===

char * mktemp(char *__template)

{
  char cVar1;
  bool bVar2;
  longlong lVar3;
  uint uVar4;
  size_t sVar5;
  uint uVar6;
  int iVar7;
  byte bVar8;
  char *pcVar9;
  uint uVar10;
  byte *pbVar11;
  
  if (_us_rsthread_stdio == 0) {
    lVar3 = 0;
  }
  else {
    lVar3 = __libc_lockopen();
  }
  uVar4 = getpid();
  sVar5 = strlen(__template);
  pcVar9 = __template + (sVar5 - 1);
  if (__template[sVar5 - 1] == 'X') {
    uVar10 = 1;
    do {
      uVar6 = uVar4 & 0x3f;
      cVar1 = (char)uVar6;
      if (uVar6 < 0x24) {
        if (uVar6 < 10) {
          *pcVar9 = cVar1 + '0';
        }
        else {
          *pcVar9 = cVar1 + '7';
        }
      }
      else {
        *pcVar9 = cVar1 + ';';
      }
      if (*pcVar9 == '`') {
        *pcVar9 = '-';
        cVar1 = pcVar9[-1];
      }
      else {
        cVar1 = pcVar9[-1];
      }
      pcVar9 = pcVar9 + -1;
      uVar4 = (int)uVar4 >> 6;
      if (cVar1 == 'X') {
        uVar10 = uVar10 + 1;
        bVar2 = false;
        if (uVar10 < 7) {
          bVar2 = true;
        }
      }
      else {
        bVar2 = false;
      }
    } while (bVar2);
  }
  pbVar11 = (byte *)(pcVar9 + 1);
  if (*pbVar11 == 0) {
    iVar7 = access(__template,0);
    if (iVar7 == 0) {
      *__template = '\0';
    }
  }
  else {
    *pbVar11 = 0x61;
    iVar7 = access(__template,0);
    if (iVar7 == 0) {
      bVar8 = *pbVar11;
      while (*pbVar11 = bVar8 + 1, *pbVar11 < 0x7b) {
        iVar7 = access(__template,0);
        if (iVar7 != 0) goto LAB_0fb88384;
        bVar8 = *pbVar11;
      }
      *__template = '\0';
    }
  }
LAB_0fb88384:
  if (lVar3 != 0) {
    __libc_unlockopen(lVar3);
  }
  return __template;
}



// === strncat @ 0fb883b0 (96 bytes) ===

char * strncat(char *__dest,char *__src,size_t __n)

{
  char cVar1;
  char *pcVar2;
  char *pcVar3;
  int iVar4;
  
  iVar4 = __n + 1;
  cVar1 = *__dest;
  pcVar3 = __dest;
  while (pcVar2 = pcVar3 + 1, cVar1 != '\0') {
    pcVar3 = pcVar2;
    cVar1 = *pcVar2;
  }
  cVar1 = *__src;
  *pcVar3 = cVar1;
  while( true ) {
    if (cVar1 == '\0') {
      return __dest;
    }
    __src = __src + 1;
    iVar4 = iVar4 + -1;
    if (iVar4 == 0) break;
    cVar1 = *__src;
    *pcVar2 = cVar1;
    pcVar2 = pcVar2 + 1;
  }
  pcVar2[-1] = '\0';
  return __dest;
}



// === _sinitlock @ 0fb88410 (140 bytes) ===

undefined8 _sinitlock(int param_1)

{
  ushort uVar1;
  undefined4 *puVar2;
  uint uVar3;
  
  puVar2 = (undefined4 *)(param_1 + 0x1c);
  uVar3 = 0;
  *(undefined4 *)(param_1 + 8) = 0;
  *(undefined4 *)(param_1 + 0xc) = 0;
  *(undefined4 *)(param_1 + 4) = 0;
  do {
    *puVar2 = 0;
    uVar3 = uVar3 + 1;
    puVar2 = puVar2 + 1;
  } while (uVar3 <= *(ushort *)(param_1 + 0x10));
  uVar1 = *(ushort *)(param_1 + 0x12);
  if (uVar1 != 0) {
    if ((uVar1 & 1) != 0) {
      **(undefined4 **)(param_1 + 0x14) = 0;
      *(undefined4 *)(*(int *)(param_1 + 0x14) + 4) = 0;
      *(undefined4 *)(*(int *)(param_1 + 0x14) + 8) = 0;
      uVar1 = *(ushort *)(param_1 + 0x12);
    }
    if ((uVar1 & 2) != 0) {
      **(undefined4 **)(param_1 + 0x18) = 0xffffffff;
    }
  }
  return 0;
}



// === _snewlock @ 0fb8849c (304 bytes) ===

int * _snewlock(int param_1)

{
  int *piVar2;
  longlong lVar1;
  
  piVar2 = (int *)usmalloc(*(int *)(param_1 + 0x424) * 4 + 0x20,param_1);
  if (piVar2 == (int *)0x0) {
    setoserror(0xc);
    return (int *)0x0;
  }
  piVar2[3] = 0;
  piVar2[2] = 0;
  *(short *)(piVar2 + 4) = (short)*(undefined4 *)(param_1 + 0x424);
  *(undefined2 *)((int)piVar2 + 0x12) = 0;
  piVar2[5] = 0;
  piVar2[6] = 0;
  *piVar2 = param_1;
  if (*(int *)(param_1 + 0x44c) != 0) {
    lVar1 = usmalloc(4,param_1);
    piVar2[6] = (int)lVar1;
    if (lVar1 == 0) {
      usfree(piVar2,param_1);
      setoserror(0xc);
      return (int *)0x0;
    }
    lVar1 = usmalloc(0xc,param_1);
    piVar2[5] = (int)lVar1;
    if (lVar1 == 0) {
      usfree(piVar2[6],param_1);
      usfree(piVar2,param_1);
      setoserror(0xc);
      return (int *)0x0;
    }
    *(ushort *)((int)piVar2 + 0x12) = *(ushort *)((int)piVar2 + 0x12) | 3;
  }
  _sinitlock(piVar2);
  return piVar2;
}



// === _sfreelock @ 0fb885cc (104 bytes) ===

void _sfreelock(undefined4 *param_1)

{
  ushort uVar1;
  
  uVar1 = *(ushort *)((int)param_1 + 0x12);
  if (uVar1 != 0) {
    if ((uVar1 & 1) != 0) {
      usfree(param_1[5],*param_1);
      uVar1 = *(ushort *)((int)param_1 + 0x12);
    }
    if ((uVar1 & 2) != 0) {
      usfree(param_1[6],*param_1);
    }
  }
  usfree(param_1,*param_1);
  return;
}



// === _swsetlock @ 0fb88634 (32 bytes) ===

void _swsetlock(void)

{
  (*_lock)();
  return;
}



// === _stestlock @ 0fb88654 (32 bytes) ===

bool _stestlock(int param_1)

{
  return (*(uint *)(param_1 + 0xc) & 0xffff) != *(uint *)(param_1 + 0xc) >> 0x10;
}



// === _sctllock @ 0fb88674 (360 bytes) ===

undefined4 _sctllock(int param_1,longlong param_2,undefined4 *param_3)

{
  undefined4 uVar1;
  
  uVar1 = 0;
  if (param_2 == 3) {
    if ((*(ushort *)(param_1 + 0x12) & 1) == 0) {
      setoserror(0x16);
      uVar1 = 0xffffffff;
    }
    else {
      *param_3 = **(undefined4 **)(param_1 + 0x14);
      param_3[1] = *(undefined4 *)(*(int *)(param_1 + 0x14) + 4);
      param_3[2] = *(undefined4 *)(*(int *)(param_1 + 0x14) + 8);
    }
  }
  else if (param_2 == 4) {
    if ((*(ushort *)(param_1 + 0x12) & 1) != 0) {
      **(undefined4 **)(param_1 + 0x14) = 0;
      *(undefined4 *)(*(int *)(param_1 + 0x14) + 4) = 0;
      *(undefined4 *)(*(int *)(param_1 + 0x14) + 8) = 0;
    }
  }
  else if (param_2 == 7) {
    if ((*(ushort *)(param_1 + 0x12) & 2) == 0) {
      setoserror(0x16);
      uVar1 = 0xffffffff;
    }
    else {
      *param_3 = **(undefined4 **)(param_1 + 0x18);
    }
  }
  else if (param_2 == 8) {
    if ((*(ushort *)(param_1 + 0x12) & 2) != 0) {
      **(undefined4 **)(param_1 + 0x18) = 0xffffffff;
    }
  }
  else {
    setoserror(0x16);
    uVar1 = 0xffffffff;
  }
  return uVar1;
}



// === _sdumplock @ 0fb887dc (264 bytes) ===

undefined8 _sdumplock(int *param_1,FILE *param_2,undefined8 param_3)

{
  ushort uVar1;
  int iVar2;
  undefined4 *puVar3;
  undefined4 uVar4;
  longlong lVar5;
  
  iVar2 = *param_1;
  (*_lock)(*(undefined4 *)(iVar2 + 0x448));
  fprintf(param_2,s__s__LOCKDUMP_at___x__last_owner___0fbd2848,param_3,param_1,param_1[1]);
  lVar5 = (*_tlock)(param_1);
  if (lVar5 == 0) {
    fprintf(param_2,s_lock_free_0fbd2880);
    uVar1 = *(ushort *)((int)param_1 + 0x12);
  }
  else {
    fprintf(param_2,s_lock_held_0fbd2870);
    uVar1 = *(ushort *)((int)param_1 + 0x12);
  }
  if ((uVar1 & 1) != 0) {
    puVar3 = (undefined4 *)param_1[5];
    fprintf(param_2,s_lock_meter__tries__d_spins__d_hi_0fbd2890,puVar3[1],*puVar3,puVar3[2]);
    uVar1 = *(ushort *)((int)param_1 + 0x12);
  }
  if ((uVar1 & 2) == 0) {
    uVar4 = *(undefined4 *)(iVar2 + 0x448);
  }
  else {
    fprintf(param_2,s_lock_debug__owner_pid__d_0fbd28c0,*(undefined4 *)param_1[6]);
    uVar4 = *(undefined4 *)(iVar2 + 0x448);
  }
  (*_ulock)(uVar4);
  return 0;
}



// === _dssetlock @ 0fb888e4 (524 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 _dssetlock(int *param_1,undefined8 param_2)

{
  ushort uVar1;
  ushort uVar2;
  int iVar3;
  uint uVar4;
  int iVar5;
  int iVar6;
  longlong lVar7;
  uint uVar8;
  uint uVar9;
  int iStack_5c;
  
  iVar6 = _DAT_00200e00;
  iStack_5c = 0;
  uVar1 = *(ushort *)(param_1 + 4);
  iVar3 = *param_1;
  uVar2 = *(ushort *)((int)param_1 + 0x12);
  if ((param_1[1] == _DAT_00200e00) && (*(int *)(iVar3 + 0x44c) == 2)) {
    _uprint(0,s_Double_tripped_on_lock___0x_x_by_0fbd28e0,param_1,_DAT_00200e00,param_2);
  }
  uVar4 = param_1[3];
  while( true ) {
    uVar8 = uVar4 >> 0x10;
    if (uVar8 == uVar1) {
      uVar9 = uVar4 & 0xffff;
    }
    else {
      uVar9 = (uVar8 + 1) * 0x10000 | uVar4 & 0xffff;
    }
    lVar7 = (*_cas32)(param_1 + 3,uVar4,uVar9,iVar3);
    if (lVar7 != 0) break;
    uVar4 = param_1[3];
  }
  if (uVar8 == (uVar4 & 0xffff)) {
    param_1[1] = iVar6;
    if ((uVar2 & 2) != 0) {
      *(int *)param_1[6] = iVar6;
    }
  }
  else {
    do {
      iVar5 = param_1[uVar8 + 7];
      lVar7 = (*_cas32)(param_1 + uVar8 + 7,iVar5,iVar6,iVar3);
    } while (lVar7 == 0);
    if (iVar5 == 1) {
      param_1[1] = iVar6;
      if ((uVar2 & 2) != 0) {
        *(int *)param_1[6] = iVar6;
      }
    }
    else {
      iStack_5c = 1;
      lVar7 = _usblock(iVar3,0,iVar6,s_ussetlock_0fbd2920);
      if (lVar7 < 0) {
        return 0xffffffffffffffff;
      }
    }
  }
  if ((uVar2 & 1) != 0) {
    *(int *)(param_1[5] + 4) = *(int *)(param_1[5] + 4) + 1;
    *(int *)(param_1[5] + 8) = *(int *)(param_1[5] + 8) + 1;
    *(int *)param_1[5] = *(int *)param_1[5] + iStack_5c;
  }
  return 1;
}



// === _dsunsetlock @ 0fb88af0 (596 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 _dsunsetlock(int *param_1,undefined8 param_2)

{
  ushort uVar1;
  ushort uVar2;
  int iVar3;
  bool bVar4;
  longlong lVar5;
  uint uVar6;
  uint uVar7;
  
  uVar1 = *(ushort *)((int)param_1 + 0x12);
  iVar3 = *param_1;
  uVar2 = *(ushort *)(param_1 + 4);
  if (_DAT_00200e00 != param_1[1]) {
    if (param_1[1] == 0) {
      if (*(int *)(iVar3 + 0x44c) == 2) {
        _uprint(0,s_Unset_lock__but_lock_not_locked__0fbd2930,param_1,_DAT_00200e00,param_2);
      }
      return 0;
    }
    if (*(int *)(iVar3 + 0x44c) == 2) {
      _uprint(0,s_Unlocking_lock_that_other_proces_0fbd2978,param_1,_DAT_00200e00,param_2);
    }
  }
  if ((uVar1 & 2) != 0) {
    *(undefined4 *)param_1[6] = 0xffffffff;
  }
  param_1[1] = 0;
  uVar6 = param_1[3];
  do {
    bVar4 = false;
    param_1[(uVar6 & 0xffff) + 7] = 0;
    uVar6 = param_1[3];
    while( true ) {
      if ((uVar6 & 0xffff) == (uint)uVar2) {
        uVar7 = uVar6 & 0xffff0000;
      }
      else {
        uVar7 = (uVar6 & 0xffff) + 1 | uVar6 & 0xffff0000;
      }
      lVar5 = (*_cas32)(param_1 + 3,uVar6,uVar7,iVar3);
      if (lVar5 != 0) break;
      uVar6 = param_1[3];
    }
    uVar6 = uVar7 & 0xffff;
    if (uVar7 >> 0x10 != uVar6) {
      do {
        uVar7 = param_1[uVar6 + 7];
        lVar5 = (*_cas32)(param_1 + uVar6 + 7,uVar7,1,iVar3);
      } while (lVar5 == 0);
      bVar4 = false;
      if (1 < uVar7) {
        if ((uVar1 & 2) != 0) {
          *(uint *)param_1[6] = uVar7;
        }
        param_1[1] = uVar7;
        lVar5 = _usunblock(iVar3,0,uVar7,s_usunsetlock_0fbd29d0);
        if (lVar5 < 0) {
          return 0xffffffffffffffff;
        }
        bVar4 = lVar5 == 1;
      }
    }
    if (!bVar4) {
      return 0;
    }
    uVar6 = param_1[3];
  } while( true );
}



// === _dscsetlock @ 0fb88d44 (196 bytes) ===

longlong _dscsetlock(int param_1,int param_2)

{
  longlong lVar1;
  int iVar2;
  
  lVar1 = 0;
  iVar2 = 0;
  while ((param_2 = param_2 + -1, param_2 != -1 && (lVar1 = FUN_0fb88e08(param_1), lVar1 != 1))) {
    iVar2 = 1;
  }
  if ((*(ushort *)(param_1 + 0x12) & 1) != 0) {
    *(int *)(*(int *)(param_1 + 0x14) + 4) = *(int *)(*(int *)(param_1 + 0x14) + 4) + 1;
    if (lVar1 == 1) {
      *(int *)(*(int *)(param_1 + 0x14) + 8) = *(int *)(*(int *)(param_1 + 0x14) + 8) + 1;
    }
    **(int **)(param_1 + 0x14) = **(int **)(param_1 + 0x14) + iVar2;
  }
  return lVar1;
}



// === FUN_0fb88e08 @ 0fb88e08 (224 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 FUN_0fb88e08(undefined4 *param_1)

{
  ushort uVar1;
  ushort uVar2;
  undefined4 uVar3;
  uint uVar4;
  undefined4 uVar5;
  longlong lVar6;
  uint uVar7;
  uint uVar8;
  
  uVar5 = _DAT_00200e00;
  uVar3 = *param_1;
  uVar1 = *(ushort *)((int)param_1 + 0x12);
  uVar2 = *(ushort *)(param_1 + 4);
  uVar4 = param_1[3];
  while( true ) {
    uVar8 = uVar4 & 0xffff;
    uVar7 = uVar4 >> 0x10;
    if (uVar8 != uVar7) {
      return 0;
    }
    if (uVar7 != uVar2) {
      uVar8 = (uVar7 + 1) * 0x10000 | uVar8;
    }
    lVar6 = (*_cas32)(param_1 + 3,uVar4,uVar8,uVar3);
    if (lVar6 != 0) break;
    uVar4 = param_1[3];
  }
  param_1[1] = uVar5;
  if ((uVar1 & 2) != 0) {
    *(undefined4 *)param_1[6] = uVar5;
  }
  return 1;
}



// === _dhwsetlock @ 0fb88ee8 (180 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 _dhwsetlock(int *param_1,undefined8 param_2,undefined8 param_3)

{
  longlong lVar1;
  int iVar2;
  
  iVar2 = 0;
  if ((param_1[1] == _DAT_00200e00) && (*(int *)(*param_1 + 0x44c) == 2)) {
    _uprint(0,s_Double_tripped_on_lock___0x_x_by_0fbd28e0,param_1,_DAT_00200e00,param_3);
  }
  lVar1 = FUN_0fb88f9c(param_1,param_2,0);
  while (lVar1 == 0) {
    sginap(0);
    iVar2 = iVar2 + 1;
    lVar1 = FUN_0fb88f9c(param_1,param_2,iVar2);
  }
  return 1;
}



// === FUN_0fb88f9c @ 0fb88f9c (240 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 FUN_0fb88f9c(int param_1,int param_2,int param_3)

{
  longlong lVar1;
  ushort uVar2;
  
  do {
    param_2 = param_2 + -1;
    if (param_2 == -1) {
      return 0;
    }
    lVar1 = (*_usgrablock)(param_1);
  } while (lVar1 != 0);
  uVar2 = *(ushort *)(param_1 + 0x12);
  *(undefined4 *)(param_1 + 4) = _DAT_00200e00;
  if (uVar2 != 0) {
    if ((uVar2 & 2) != 0) {
      **(undefined4 **)(param_1 + 0x18) = _DAT_00200e00;
      uVar2 = *(ushort *)(param_1 + 0x12);
    }
    if ((uVar2 & 1) != 0) {
      *(int *)(*(int *)(param_1 + 0x14) + 4) = *(int *)(*(int *)(param_1 + 0x14) + 4) + 1;
      *(int *)(*(int *)(param_1 + 0x14) + 8) = *(int *)(*(int *)(param_1 + 0x14) + 8) + 1;
      **(int **)(param_1 + 0x14) = **(int **)(param_1 + 0x14) + param_3;
    }
  }
  return 1;
}



// === _dhcsetlock @ 0fb8908c (116 bytes) ===

undefined8 _dhcsetlock(int param_1,undefined8 param_2)

{
  longlong lVar1;
  
  lVar1 = FUN_0fb88f9c(param_1,param_2,0);
  if (lVar1 == 0) {
    if ((*(ushort *)(param_1 + 0x12) & 1) != 0) {
      *(int *)(*(int *)(param_1 + 0x14) + 4) = *(int *)(*(int *)(param_1 + 0x14) + 4) + 1;
      **(int **)(param_1 + 0x14) = **(int **)(param_1 + 0x14) + 1;
    }
    return 0;
  }
  return 1;
}



// === _dhsetlock @ 0fb89100 (220 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 _dhsetlock(int *param_1,undefined8 param_2)

{
  undefined4 uVar1;
  longlong lVar2;
  int iVar3;
  int iVar4;
  
  iVar3 = _usyieldcnt;
  uVar1 = _ushlockdefspin;
  iVar4 = 0;
  if ((param_1[1] == _DAT_00200e00) && (*(int *)(*param_1 + 0x44c) == 2)) {
    _uprint(0,s_Double_tripped_on_lock___0x_x_by_0fbd28e0,param_1,_DAT_00200e00,param_2);
  }
  lVar2 = FUN_0fb88f9c(param_1,uVar1,0);
  while (lVar2 == 0) {
    iVar3 = iVar3 + -1;
    if (iVar3 == 0) {
      nanosleep((timespec *)&__usnano,(timespec *)0x0);
      iVar3 = _usyieldcnt;
    }
    else {
      sginap(0);
    }
    iVar4 = iVar4 + 1;
    lVar2 = FUN_0fb88f9c(param_1,uVar1,iVar4);
  }
  return 1;
}



// === _dhunsetlock @ 0fb891dc (176 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 _dhunsetlock(int *param_1,undefined8 param_2)

{
  ushort uVar1;
  
  if (_DAT_00200e00 != param_1[1]) {
    if (*(int *)(*param_1 + 0x44c) != 2) {
      uVar1 = *(ushort *)((int)param_1 + 0x12);
      goto LAB_0fb89250;
    }
    if (param_1[1] == 0) {
      _uprint(0,s_Unset_lock__but_lock_not_locked__0fbd2930,param_1,_DAT_00200e00,param_2);
      uVar1 = *(ushort *)((int)param_1 + 0x12);
      goto LAB_0fb89250;
    }
    _uprint(0,s_Unlocking_lock_that_other_proces_0fbd2978,param_1,_DAT_00200e00,param_2);
  }
  uVar1 = *(ushort *)((int)param_1 + 0x12);
LAB_0fb89250:
  if ((uVar1 & 2) == 0) {
    param_1[1] = 0;
  }
  else {
    *(undefined4 *)param_1[6] = 0xffffffff;
    param_1[1] = 0;
  }
  (*_usrellock)(param_1);
  return 0;
}



// === strncmp @ 0fb8928c (144 bytes) ===

int strncmp(char *__s1,char *__s2,size_t __n)

{
  char cVar1;
  byte bVar2;
  bool bVar3;
  int iVar4;
  
  if (__s1 == __s2) {
    return 0;
  }
  if (__n != 0) {
    cVar1 = *__s2;
    __s2 = __s2 + 1;
    if (cVar1 == *__s1) {
      bVar2 = *__s1;
      while( true ) {
        __s1 = __s1 + 1;
        if (bVar2 == 0) {
          return 0;
        }
        __n = __n - 1;
        bVar3 = false;
        if (__n != 0) {
          bVar2 = *__s2;
          __s2 = __s2 + 1;
          if (bVar2 == *__s1) {
            bVar3 = true;
          }
          else {
            bVar3 = false;
          }
        }
        if (!bVar3) break;
        bVar2 = *__s1;
      }
    }
  }
  if (__n == 0) {
    iVar4 = 0;
  }
  else {
    iVar4 = (uint)(byte)*__s1 - (uint)(byte)__s2[-1];
  }
  return iVar4;
}



// === strcmp @ 0fb8931c (68 bytes) ===

int strcmp(char *__s1,char *__s2)

{
  byte bVar1;
  byte bVar2;
  byte bVar3;
  
  bVar1 = *__s1;
  while( true ) {
    bVar2 = *__s2;
    if (bVar1 == 0) break;
    bVar3 = __s1[1];
    if (bVar1 != bVar2) {
      return (uint)bVar1 - (uint)bVar2;
    }
    bVar2 = __s2[1];
    __s2 = __s2 + 2;
    if (bVar3 == 0) break;
    bVar1 = __s1[2];
    __s1 = __s1 + 2;
    if (bVar3 != bVar2) {
      return (uint)bVar3 - (uint)bVar2;
    }
  }
  return -(uint)bVar2;
}



// === sysmp @ 0fb89360 (64 bytes) ===

long sysmp(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,
          undefined8 param_5)

{
  long lVar1;
  
  lVar1 = syscall(0x420,param_1,param_2,param_3,param_4,param_5);
  return lVar1;
}



// === _usr4klocks_init @ 0fb893a8 (244 bytes) ===

void _usr4klocks_init(undefined8 param_1,ulonglong param_2)

{
  int unaff_gp_lo;
  
  *(code **)(unaff_gp_lo + -0x7748) = _sinitlock;
  *(code **)(unaff_gp_lo + -0x774c) = _snewlock;
  *(code **)(unaff_gp_lo + -0x7730) = _sdumplock;
  *(code **)(unaff_gp_lo + -0x7744) = _sfreelock;
  *(code **)(unaff_gp_lo + -0x7734) = _sctllock;
  if ((param_2 & 2) == 0) {
    *(code **)(unaff_gp_lo + -0x7738) = _stestlock;
    *(code **)(unaff_gp_lo + -0x7740) = _swsetlock;
    *(code **)(unaff_gp_lo + -0x773c) = _r4kup_csetlock;
    *(code **)(unaff_gp_lo + -0x7750) = _r4kup_unsetlock;
    *(code **)(unaff_gp_lo + -0x7754) = _r4kup_setlock;
  }
  else {
    *(code **)(unaff_gp_lo + -0x7720) = _r4kmp_rellock;
    *(code **)(unaff_gp_lo + -0x7724) = _r4kmp_grablock;
    *(undefined1 **)(unaff_gp_lo + -0x7738) = &LAB_0fb893a0;
    *(code **)(unaff_gp_lo + -0x7740) = _r4kmp_wsetlock;
    *(code **)(unaff_gp_lo + -0x773c) = _r4kmp_csetlock;
    *(code **)(unaff_gp_lo + -0x7750) = _r4kmp_unsetlock;
    *(code **)(unaff_gp_lo + -0x7754) = _r4kmp_setlock;
  }
  *(code **)(unaff_gp_lo + -0x7728) = _r4k_cas32;
  *(code **)(unaff_gp_lo + -0x772c) = _r4k_cas;
  return;
}



// === _r4kup_setlock @ 0fb8949c (200 bytes) ===

/* WARNING: Removing unreachable block (ram,0x0fb894d0) */
/* WARNING: Removing unreachable block (ram,0x0fb89504) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 _r4kup_setlock(int *param_1)

{
  ushort uVar1;
  ushort uVar2;
  int iVar3;
  uint uVar4;
  int iVar5;
  longlong lVar6;
  int iVar7;
  uint uVar8;
  uint uVar9;
  undefined8 in_ra;
  int iStack_5c;
  
  iVar7 = _DAT_00200e00;
  if (*(short *)((int)param_1 + 0x12) != 0) {
    iStack_5c = 0;
    uVar1 = *(ushort *)(param_1 + 4);
    iVar5 = *param_1;
    uVar2 = *(ushort *)((int)param_1 + 0x12);
    if ((param_1[1] == _DAT_00200e00) && (*(int *)(iVar5 + 0x44c) == 2)) {
      _uprint(0,s_Double_tripped_on_lock___0x_x_by_0fbd28e0,param_1,_DAT_00200e00,in_ra);
    }
    uVar4 = param_1[3];
    while( true ) {
      uVar8 = uVar4 >> 0x10;
      if (uVar8 == uVar1) {
        uVar9 = uVar4 & 0xffff;
      }
      else {
        uVar9 = (uVar8 + 1) * 0x10000 | uVar4 & 0xffff;
      }
      lVar6 = (*_cas32)(param_1 + 3,uVar4,uVar9,iVar5);
      if (lVar6 != 0) break;
      uVar4 = param_1[3];
    }
    if (uVar8 == (uVar4 & 0xffff)) {
      param_1[1] = iVar7;
      if ((uVar2 & 2) != 0) {
        *(int *)param_1[6] = iVar7;
      }
    }
    else {
      do {
        iVar3 = param_1[uVar8 + 7];
        lVar6 = (*_cas32)(param_1 + uVar8 + 7,iVar3,iVar7,iVar5);
      } while (lVar6 == 0);
      if (iVar3 == 1) {
        param_1[1] = iVar7;
        if ((uVar2 & 2) != 0) {
          *(int *)param_1[6] = iVar7;
        }
      }
      else {
        iStack_5c = 1;
        lVar6 = _usblock(iVar5,0,iVar7,s_ussetlock_0fbd2920);
        if (lVar6 < 0) {
          return 0xffffffffffffffff;
        }
      }
    }
    if ((uVar2 & 1) != 0) {
      *(int *)(param_1[5] + 4) = *(int *)(param_1[5] + 4) + 1;
      *(int *)(param_1[5] + 8) = *(int *)(param_1[5] + 8) + 1;
      *(int *)param_1[5] = *(int *)param_1[5] + iStack_5c;
    }
    return 1;
  }
  uVar4 = param_1[3];
  uVar8 = uVar4 >> 0x10;
  if ((longlong)(int)uVar8 == (longlong)*(short *)(param_1 + 4)) {
    iVar7 = (int)((longlong)(int)uVar4 & 0xffffU);
  }
  else {
    iVar7 = uVar4 + 0x10000;
  }
  param_1[3] = iVar7;
  iVar7 = _DAT_00200e00;
  if ((longlong)(int)uVar8 == ((longlong)(int)uVar4 & 0xffffU)) {
    return 1;
  }
  iVar5 = param_1[uVar8 + 7];
  param_1[uVar8 + 7] = _DAT_00200e00;
  if ((iVar5 != 1) && (lVar6 = _usblock(*param_1,0,iVar7,s_ussetlock_0fbd29e0), lVar6 != 0)) {
    return 0xffffffffffffffff;
  }
  return 1;
}



// === _r4kup_csetlock @ 0fb89564 (124 bytes) ===

/* WARNING: Removing unreachable block (ram,0x0fb895ac) */

longlong _r4kup_csetlock(int param_1,int param_2)

{
  uint uVar1;
  int iVar2;
  longlong lVar3;
  
  if (*(short *)(param_1 + 0x12) != 0) {
    lVar3 = 0;
    iVar2 = 0;
    while ((param_2 = param_2 + -1, param_2 != -1 && (lVar3 = FUN_0fb88e08(param_1), lVar3 != 1))) {
      iVar2 = 1;
    }
    if ((*(ushort *)(param_1 + 0x12) & 1) != 0) {
      *(int *)(*(int *)(param_1 + 0x14) + 4) = *(int *)(*(int *)(param_1 + 0x14) + 4) + 1;
      if (lVar3 == 1) {
        *(int *)(*(int *)(param_1 + 0x14) + 8) = *(int *)(*(int *)(param_1 + 0x14) + 8) + 1;
      }
      **(int **)(param_1 + 0x14) = **(int **)(param_1 + 0x14) + iVar2;
    }
    return lVar3;
  }
  while( true ) {
    if (param_2 == 0) {
      return 0;
    }
    uVar1 = *(uint *)(param_1 + 0xc);
    if ((longlong)(int)(uVar1 >> 0x10) == ((longlong)(int)uVar1 & 0xffffU)) break;
    param_2 = param_2 + -1;
  }
  if ((longlong)(int)(uVar1 >> 0x10) == (longlong)*(short *)(param_1 + 0x10)) {
    iVar2 = (int)((longlong)(int)uVar1 & 0xffffU);
  }
  else {
    iVar2 = uVar1 + 0x10000;
  }
  *(int *)(param_1 + 0xc) = iVar2;
  return 1;
}



// === _r4kup_unsetlock @ 0fb895e0 (252 bytes) ===

/* WARNING: Removing unreachable block (ram,0x0fb89638) */
/* WARNING: Removing unreachable block (ram,0x0fb89668) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

longlong _r4kup_unsetlock(int *param_1)

{
  short sVar1;
  ushort uVar2;
  ushort uVar3;
  int iVar4;
  bool bVar5;
  uint uVar7;
  longlong lVar6;
  uint uVar8;
  ulonglong uVar9;
  ulonglong uVar10;
  undefined8 in_ra;
  
  sVar1 = *(short *)(param_1 + 4);
  if (*(short *)((int)param_1 + 0x12) == 0) {
    param_1[(param_1[3] & 0xffffU) + 7] = 0;
    while( true ) {
      uVar7 = param_1[3];
      uVar9 = (longlong)(int)uVar7 & 0xffff;
      if (uVar9 == (longlong)(int)(uVar7 >> 0x10)) {
        return 0;
      }
      uVar10 = (ulonglong)((int)uVar9 + 1);
      if (uVar9 == (longlong)sVar1) {
        uVar10 = 0;
        uVar8 = uVar7 & 0xffff0000;
      }
      else {
        uVar8 = uVar7 + 1;
      }
      param_1[3] = uVar8;
      if ((longlong)(int)(uVar7 >> 0x10) == uVar10) break;
      uVar7 = param_1[(int)uVar10 + 7];
      param_1[(int)uVar10 + 7] = 1;
      if ((uVar7 < 2) || (lVar6 = _usunblock(*param_1,0,uVar7,s_usunsetlock_0fbd29ea), lVar6 == 0))
      {
        return 0;
      }
      sVar1 = *(short *)(param_1 + 4);
      if (lVar6 != 1) {
        return lVar6;
      }
    }
    return 0;
  }
  uVar2 = *(ushort *)((int)param_1 + 0x12);
  iVar4 = *param_1;
  uVar3 = *(ushort *)(param_1 + 4);
  if (_DAT_00200e00 != param_1[1]) {
    if (param_1[1] == 0) {
      if (*(int *)(iVar4 + 0x44c) == 2) {
        _uprint(0,s_Unset_lock__but_lock_not_locked__0fbd2930,param_1,_DAT_00200e00,in_ra);
      }
      return 0;
    }
    if (*(int *)(iVar4 + 0x44c) == 2) {
      _uprint(0,s_Unlocking_lock_that_other_proces_0fbd2978,param_1,_DAT_00200e00,in_ra);
    }
  }
  if ((uVar2 & 2) != 0) {
    *(undefined4 *)param_1[6] = 0xffffffff;
  }
  param_1[1] = 0;
  uVar7 = param_1[3];
  do {
    bVar5 = false;
    param_1[(uVar7 & 0xffff) + 7] = 0;
    uVar7 = param_1[3];
    while( true ) {
      if ((uVar7 & 0xffff) == (uint)uVar3) {
        uVar8 = uVar7 & 0xffff0000;
      }
      else {
        uVar8 = (uVar7 & 0xffff) + 1 | uVar7 & 0xffff0000;
      }
      lVar6 = (*_cas32)(param_1 + 3,uVar7,uVar8,iVar4);
      if (lVar6 != 0) break;
      uVar7 = param_1[3];
    }
    uVar7 = uVar8 & 0xffff;
    if (uVar8 >> 0x10 != uVar7) {
      do {
        uVar8 = param_1[uVar7 + 7];
        lVar6 = (*_cas32)(param_1 + uVar7 + 7,uVar8,1,iVar4);
      } while (lVar6 == 0);
      bVar5 = false;
      if (1 < uVar8) {
        if ((uVar2 & 2) != 0) {
          *(uint *)param_1[6] = uVar8;
        }
        param_1[1] = uVar8;
        lVar6 = _usunblock(iVar4,0,uVar8,s_usunsetlock_0fbd29d0);
        if (lVar6 < 0) {
          return -1;
        }
        bVar5 = lVar6 == 1;
      }
    }
    if (!bVar5) {
      return 0;
    }
    uVar7 = param_1[3];
  } while( true );
}



// === _r4k_cas @ 0fb896dc (96 bytes) ===

undefined8 _r4k_cas(int *param_1,int param_2,int param_3)

{
  if (*param_1 == param_2) {
    *param_1 = param_3;
    return 1;
  }
  return 0;
}



// === _r4k_cas32 @ 0fb89764 (96 bytes) ===

undefined8 _r4k_cas32(int *param_1,int param_2,int param_3)

{
  if (*param_1 == param_2) {
    *param_1 = param_3;
    return 1;
  }
  return 0;
}



// === _r4kmp_grablock @ 0fb897ec (36 bytes) ===

/* WARNING: Removing unreachable block (ram,0x0fb897fc) */

int _r4kmp_grablock(int param_1)

{
  int iVar1;
  
  iVar1 = *(int *)(param_1 + 8);
  if (iVar1 == 0) {
    *(undefined4 *)(param_1 + 8) = 1;
    iVar1 = 0;
  }
  return iVar1;
}



// === _r4kmp_rellock @ 0fb89810 (8 bytes) ===

void _r4kmp_rellock(int param_1)

{
  *(undefined4 *)(param_1 + 8) = 0;
  return;
}



// === _r4kmp_wsetlock @ 0fb89818 (124 bytes) ===

/* WARNING: Removing unreachable block (ram,0x0fb8983c) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 _r4kmp_wsetlock(int *param_1,int param_2)

{
  longlong lVar1;
  int iVar2;
  undefined8 in_ra;
  
  iVar2 = param_2;
  if (*(short *)((int)param_1 + 0x12) != 0) {
    iVar2 = 0;
    if ((param_1[1] == _DAT_00200e00) && (*(int *)(*param_1 + 0x44c) == 2)) {
      _uprint(0,s_Double_tripped_on_lock___0x_x_by_0fbd28e0,param_1,_DAT_00200e00,in_ra);
    }
    lVar1 = FUN_0fb88f9c(param_1,param_2,0);
    while (lVar1 == 0) {
      sginap(0);
      iVar2 = iVar2 + 1;
      lVar1 = FUN_0fb88f9c(param_1,param_2,iVar2);
    }
    return 1;
  }
  while (param_1[2] != 0) {
    iVar2 = iVar2 + -1;
    if (iVar2 < 1) {
      sginap(0);
      iVar2 = param_2;
    }
  }
  param_1[2] = 1;
  return 1;
}



// === _r4kmp_setlock @ 0fb89894 (200 bytes) ===

/* WARNING: Removing unreachable block (ram,0x0fb898cc) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 _r4kmp_setlock(int *param_1)

{
  longlong lVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  undefined8 in_ra;
  
  iVar3 = _usyieldcnt;
  iVar2 = _ushlockdefspin;
  if (*(short *)((int)param_1 + 0x12) != 0) {
    iVar4 = 0;
    if ((param_1[1] == _DAT_00200e00) && (*(int *)(*param_1 + 0x44c) == 2)) {
      _uprint(0,s_Double_tripped_on_lock___0x_x_by_0fbd28e0,param_1,_DAT_00200e00,in_ra);
    }
    lVar1 = FUN_0fb88f9c(param_1,iVar2,0);
    while (lVar1 == 0) {
      iVar3 = iVar3 + -1;
      if (iVar3 == 0) {
        nanosleep((timespec *)&__usnano,(timespec *)0x0);
        iVar3 = _usyieldcnt;
      }
      else {
        sginap(0);
      }
      iVar4 = iVar4 + 1;
      lVar1 = FUN_0fb88f9c(param_1,iVar2,iVar4);
    }
    return 1;
  }
  while (iVar4 = _usyieldcnt, param_1[2] != 0) {
    iVar2 = iVar2 + -1;
    if (iVar2 < 1) {
      iVar3 = iVar3 + -1;
      if (iVar3 < 1) {
        nanosleep((timespec *)&__usnano,(timespec *)0x0);
        iVar2 = _ushlockdefspin;
        iVar3 = iVar4;
      }
      else {
        sginap(0);
        iVar2 = _ushlockdefspin;
      }
    }
  }
  param_1[2] = 1;
  return 1;
}



// === _r4kmp_csetlock @ 0fb8995c (84 bytes) ===

/* WARNING: Removing unreachable block (ram,0x0fb89980) */

undefined8 _r4kmp_csetlock(int param_1,int param_2)

{
  longlong lVar1;
  
  if (*(short *)(param_1 + 0x12) == 0) {
    do {
      if (*(int *)(param_1 + 8) == 0) {
        *(undefined4 *)(param_1 + 8) = 1;
        return 1;
      }
      param_2 = param_2 + -1;
    } while (0 < param_2);
    return 0;
  }
  lVar1 = FUN_0fb88f9c(param_1,param_2,0);
  if (lVar1 == 0) {
    if ((*(ushort *)(param_1 + 0x12) & 1) != 0) {
      *(int *)(*(int *)(param_1 + 0x14) + 4) = *(int *)(*(int *)(param_1 + 0x14) + 4) + 1;
      **(int **)(param_1 + 0x14) = **(int **)(param_1 + 0x14) + 1;
    }
    return 0;
  }
  return 1;
}



// === _r4kmp_unsetlock @ 0fb899b0 (40 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 _r4kmp_unsetlock(int *param_1)

{
  ushort uVar1;
  undefined8 in_ra;
  
  if (*(short *)((int)param_1 + 0x12) == 0) {
    param_1[2] = 0;
    return 0;
  }
  if (_DAT_00200e00 != param_1[1]) {
    if (*(int *)(*param_1 + 0x44c) != 2) {
      uVar1 = *(ushort *)((int)param_1 + 0x12);
      goto LAB_0fb89250;
    }
    if (param_1[1] == 0) {
      _uprint(0,s_Unset_lock__but_lock_not_locked__0fbd2930,param_1,_DAT_00200e00,in_ra);
      uVar1 = *(ushort *)((int)param_1 + 0x12);
      goto LAB_0fb89250;
    }
    _uprint(0,s_Unlocking_lock_that_other_proces_0fbd2978,param_1,_DAT_00200e00,in_ra);
  }
  uVar1 = *(ushort *)((int)param_1 + 0x12);
LAB_0fb89250:
  if ((uVar1 & 2) == 0) {
    param_1[1] = 0;
  }
  else {
    *(undefined4 *)param_1[6] = 0xffffffff;
    param_1[1] = 0;
  }
  (*_usrellock)(param_1);
  return 0;
}



// === syscall @ 0fb899d8 (40 bytes) ===

long syscall(long __sysno,...)

{
  long lVar1;
  longlong in_a3;
  
  syscall(0);
  if (in_a3 == 0) {
    return 1000;
  }
  lVar1 = _cerror(__sysno);
  return lVar1;
}



// === bcopy @ 0fb89a00 (12 bytes) ===

/* WARNING: Instruction at (ram,0x0fb89b2c) overlaps instruction at (ram,0x0fb89b28)
    */

void bcopy(void *__src,void *__dest,size_t __n)

{
  uint uVar1;
  ulonglong uVar2;
  ulonglong uVar3;
  ulonglong uVar4;
  ulonglong uVar5;
  ulonglong uVar6;
  ulonglong uVar7;
  ulonglong uVar8;
  ulonglong uVar9;
  ulonglong uVar10;
  ulonglong uVar11;
  ulonglong uVar12;
  ulonglong uVar13;
  ulonglong uVar14;
  ulonglong uVar15;
  ulonglong uVar16;
  ulonglong uVar17;
  longlong lVar18;
  longlong lVar19;
  longlong lVar20;
  longlong lVar21;
  longlong lVar22;
  longlong lVar23;
  longlong lVar24;
  uint uVar26;
  ulonglong uVar25;
  ulonglong uVar27;
  ulonglong uVar28;
  uint uVar29;
  longlong *plVar30;
  undefined8 uVar31;
  undefined8 uVar32;
  ulonglong in_v1;
  undefined8 uVar33;
  ulonglong uVar34;
  undefined8 *puVar35;
  ulonglong *puVar36;
  int iVar37;
  undefined8 *puVar38;
  undefined8 *puVar40;
  uint uVar41;
  ulonglong *puVar42;
  undefined8 *extraout_a0_lo;
  ulonglong uVar43;
  undefined8 *puVar44;
  ulonglong *puVar45;
  undefined8 *extraout_a1_lo;
  ulonglong uVar46;
  ulonglong extraout_a2;
  int iVar48;
  ulonglong uVar47;
  ulonglong in_t0;
  undefined8 uVar49;
  ulonglong in_t1;
  undefined8 uVar50;
  ulonglong in_t2;
  undefined8 uVar51;
  ulonglong in_t3;
  undefined8 uVar52;
  ulonglong in_t4;
  ulonglong in_t5;
  undefined8 uVar53;
  ulonglong in_t6;
  undefined8 uVar54;
  ulonglong in_t7;
  undefined8 uVar55;
  ulonglong unaff_s0;
  undefined8 uVar56;
  ulonglong unaff_s1;
  undefined8 uVar57;
  ulonglong unaff_s2;
  undefined8 uVar58;
  ulonglong uVar59;
  ulonglong uVar60;
  undefined8 uVar61;
  ulonglong uVar62;
  undefined8 uVar63;
  ulonglong uVar64;
  int iVar39;
  
  uVar46 = (ulonglong)(int)__n;
  uVar34 = (ulonglong)(int)__dest;
  uVar43 = (ulonglong)(int)__src;
  if (uVar43 != uVar34) {
    if ((uVar34 < uVar43) || ((ulonglong)(longlong)(int)((int)__src + __n) <= uVar34)) {
      if (0x1f < uVar46) {
        uVar59 = uVar43 & 7;
        uVar62 = uVar34 & 7;
        if (uVar59 != uVar62) {
          uVar59 = uVar34;
          if (uVar62 != 0) {
            iVar48 = 8 - (int)uVar62;
            uVar43 = (ulonglong)((int)__src + iVar48);
            uVar46 = uVar34 & 7;
            puVar42 = (ulonglong *)((int)__dest - (int)uVar46);
            *puVar42 = *puVar42 & -1L << (8 - uVar46) * 8 | *(ulonglong *)__src >> uVar46 * 8;
            uVar59 = (ulonglong)((int)__dest + iVar48);
            uVar46 = (ulonglong)(int)(__n - iVar48);
          }
          puVar42 = (ulonglong *)uVar59;
          uVar62 = 0xffffffffffffff80;
          uVar60 = uVar46 & 0x40;
          if ((uVar46 & 0xffffffffffffff80) != 0) {
            uVar64 = (ulonglong)((int)uVar46 + -0x1000);
            uVar47 = (ulonglong)((int)(uVar46 & 0xffffffffffffff80) + (int)puVar42);
            if ((longlong)uVar64 < 0) {
              do {
                uVar34 = uVar43 & 7;
                iVar48 = (int)uVar43;
                uVar2 = uVar43 + 8 & 7;
                lVar18 = *(longlong *)((int)(uVar43 + 8) - (int)uVar2);
                uVar3 = uVar43 + 0x10 & 7;
                lVar19 = *(longlong *)((int)(uVar43 + 0x10) - (int)uVar3);
                uVar4 = uVar43 + 0x18 & 7;
                lVar20 = *(longlong *)((int)(uVar43 + 0x18) - (int)uVar4);
                uVar5 = uVar43 + 0x20 & 7;
                uVar6 = uVar43 + 0x28 & 7;
                uVar7 = uVar43 + 0x30 & 7;
                uVar8 = uVar43 + 0x38 & 7;
                uVar9 = uVar43 + 0x40 & 7;
                uVar10 = uVar43 + 0x48 & 7;
                uVar11 = uVar43 + 0x50 & 7;
                uVar12 = uVar43 + 0x58 & 7;
                uVar13 = uVar43 + 7 & 7;
                uVar14 = uVar43 + 0xf & 7;
                uVar25 = *(ulonglong *)((int)(uVar43 + 0xf) - (int)uVar14);
                uVar15 = uVar43 + 0x17 & 7;
                uVar27 = *(ulonglong *)((int)(uVar43 + 0x17) - (int)uVar15);
                uVar16 = uVar43 + 0x1f & 7;
                uVar28 = *(ulonglong *)((int)(uVar43 + 0x1f) - (int)uVar16);
                uVar17 = uVar43 + 0x27 & 7;
                in_t0 = (*(longlong *)((int)(uVar43 + 0x20) - (int)uVar5) << uVar5 * 8 |
                        in_t0 & 0xffffffffffffffffU >> (8 - uVar5) * 8) & -1L << (uVar17 + 1) * 8 |
                        *(ulonglong *)((int)(uVar43 + 0x27) - (int)uVar17) >> (7 - uVar17) * 8;
                uVar5 = uVar43 + 0x2f & 7;
                in_t1 = (*(longlong *)((int)(uVar43 + 0x28) - (int)uVar6) << uVar6 * 8 |
                        in_t1 & 0xffffffffffffffffU >> (8 - uVar6) * 8) & -1L << (uVar5 + 1) * 8 |
                        *(ulonglong *)((int)(uVar43 + 0x2f) - (int)uVar5) >> (7 - uVar5) * 8;
                uVar5 = uVar43 + 0x37 & 7;
                in_t2 = (*(longlong *)((int)(uVar43 + 0x30) - (int)uVar7) << uVar7 * 8 |
                        in_t2 & 0xffffffffffffffffU >> (8 - uVar7) * 8) & -1L << (uVar5 + 1) * 8 |
                        *(ulonglong *)((int)(uVar43 + 0x37) - (int)uVar5) >> (7 - uVar5) * 8;
                uVar5 = uVar43 + 0x3f & 7;
                in_t3 = (*(longlong *)((int)(uVar43 + 0x38) - (int)uVar8) << uVar8 * 8 |
                        in_t3 & 0xffffffffffffffffU >> (8 - uVar8) * 8) & -1L << (uVar5 + 1) * 8 |
                        *(ulonglong *)((int)(uVar43 + 0x3f) - (int)uVar5) >> (7 - uVar5) * 8;
                uVar5 = uVar43 + 0x47 & 7;
                uVar60 = (*(longlong *)((int)(uVar43 + 0x40) - (int)uVar9) << uVar9 * 8 |
                         uVar60 & 0xffffffffffffffffU >> (8 - uVar9) * 8) & -1L << (uVar5 + 1) * 8 |
                         *(ulonglong *)((int)(uVar43 + 0x47) - (int)uVar5) >> (7 - uVar5) * 8;
                uVar5 = uVar43 + 0x4f & 7;
                uVar64 = (*(longlong *)((int)(uVar43 + 0x48) - (int)uVar10) << uVar10 * 8 |
                         uVar64 & 0xffffffffffffffffU >> (8 - uVar10) * 8) & -1L << (uVar5 + 1) * 8
                         | *(ulonglong *)((int)(uVar43 + 0x4f) - (int)uVar5) >> (7 - uVar5) * 8;
                uVar5 = uVar43 + 0x57 & 7;
                in_v1 = (*(longlong *)((int)(uVar43 + 0x50) - (int)uVar11) << uVar11 * 8 |
                        in_v1 & 0xffffffffffffffffU >> (8 - uVar11) * 8) & -1L << (uVar5 + 1) * 8 |
                        *(ulonglong *)((int)(uVar43 + 0x57) - (int)uVar5) >> (7 - uVar5) * 8;
                uVar5 = uVar43 + 0x5f & 7;
                uVar62 = (*(longlong *)((int)(uVar43 + 0x58) - (int)uVar12) << uVar12 * 8 |
                         uVar62 & 0xffffffffffffffffU >> (8 - uVar12) * 8) & -1L << (uVar5 + 1) * 8
                         | *(ulonglong *)((int)(uVar43 + 0x5f) - (int)uVar5) >> (7 - uVar5) * 8;
                puVar36 = (ulonglong *)uVar59;
                *puVar36 = (*(longlong *)(iVar48 - (int)uVar34) << uVar34 * 8 |
                           in_t4 & 0xffffffffffffffffU >> (8 - uVar34) * 8) &
                           -1L << (uVar13 + 1) * 8 |
                           *(ulonglong *)((int)(uVar43 + 7) - (int)uVar13) >> (7 - uVar13) * 8;
                puVar36[1] = (lVar18 << uVar2 * 8 | in_t5 & 0xffffffffffffffffU >> (8 - uVar2) * 8)
                             & -1L << (uVar14 + 1) * 8 | uVar25 >> (7 - uVar14) * 8;
                puVar36[2] = (lVar19 << uVar3 * 8 | in_t6 & 0xffffffffffffffffU >> (8 - uVar3) * 8)
                             & -1L << (uVar15 + 1) * 8 | uVar27 >> (7 - uVar15) * 8;
                puVar36[3] = (lVar20 << uVar4 * 8 | in_t7 & 0xffffffffffffffffU >> (8 - uVar4) * 8)
                             & -1L << (uVar16 + 1) * 8 | uVar28 >> (7 - uVar16) * 8;
                puVar36[4] = in_t0;
                puVar36[5] = in_t1;
                puVar36[6] = in_t2;
                puVar36[7] = in_t3;
                puVar36[8] = uVar60;
                puVar36[9] = uVar64;
                puVar36[10] = in_v1;
                puVar36[0xb] = uVar62;
                in_t4 = *(ulonglong *)(iVar48 + 0x60);
                in_t5 = *(ulonglong *)(iVar48 + 0x68);
                in_t6 = *(ulonglong *)(iVar48 + 0x70);
                in_t7 = *(ulonglong *)(iVar48 + 0x78);
                puVar42 = puVar36 + 0x10;
                uVar59 = (ulonglong)(int)puVar42;
                uVar43 = (ulonglong)(iVar48 + 0x80);
                puVar36[0xc] = in_t4;
                puVar36[0xd] = in_t5;
                puVar36[0xe] = in_t6;
                puVar36[0xf] = in_t7;
              } while (uVar47 != uVar59);
            }
            else {
              do {
                uVar2 = uVar43 & 7;
                uVar3 = uVar43 + 8 & 7;
                uVar4 = uVar43 + 0x10 & 7;
                uVar5 = uVar43 + 0x18 & 7;
                uVar6 = uVar43 + 0x20 & 7;
                uVar7 = uVar43 + 0x28 & 7;
                uVar8 = uVar43 + 0x30 & 7;
                uVar9 = uVar43 + 0x38 & 7;
                uVar10 = uVar43 + 0x40 & 7;
                uVar11 = uVar43 + 0x48 & 7;
                uVar12 = uVar43 + 0x50 & 7;
                uVar13 = uVar43 + 0x58 & 7;
                uVar14 = uVar43 + 0x60 & 7;
                uVar15 = uVar43 + 0x68 & 7;
                uVar16 = uVar43 + 0x70 & 7;
                uVar17 = uVar43 + 0x78 & 7;
                uVar25 = uVar43 + 7 & 7;
                in_t4 = (*(longlong *)((int)uVar43 - (int)uVar2) << uVar2 * 8 |
                        in_t4 & 0xffffffffffffffffU >> (8 - uVar2) * 8) & -1L << (uVar25 + 1) * 8 |
                        *(ulonglong *)((int)(uVar43 + 7) - (int)uVar25) >> (7 - uVar25) * 8;
                uVar2 = uVar43 + 0xf & 7;
                in_t5 = (*(longlong *)((int)(uVar43 + 8) - (int)uVar3) << uVar3 * 8 |
                        in_t5 & 0xffffffffffffffffU >> (8 - uVar3) * 8) & -1L << (uVar2 + 1) * 8 |
                        *(ulonglong *)((int)(uVar43 + 0xf) - (int)uVar2) >> (7 - uVar2) * 8;
                uVar2 = uVar43 + 0x17 & 7;
                in_t6 = (*(longlong *)((int)(uVar43 + 0x10) - (int)uVar4) << uVar4 * 8 |
                        in_t6 & 0xffffffffffffffffU >> (8 - uVar4) * 8) & -1L << (uVar2 + 1) * 8 |
                        *(ulonglong *)((int)(uVar43 + 0x17) - (int)uVar2) >> (7 - uVar2) * 8;
                uVar2 = uVar43 + 0x1f & 7;
                in_t7 = (*(longlong *)((int)(uVar43 + 0x18) - (int)uVar5) << uVar5 * 8 |
                        in_t7 & 0xffffffffffffffffU >> (8 - uVar5) * 8) & -1L << (uVar2 + 1) * 8 |
                        *(ulonglong *)((int)(uVar43 + 0x1f) - (int)uVar2) >> (7 - uVar2) * 8;
                uVar2 = uVar43 + 0x27 & 7;
                in_t0 = (*(longlong *)((int)(uVar43 + 0x20) - (int)uVar6) << uVar6 * 8 |
                        in_t0 & 0xffffffffffffffffU >> (8 - uVar6) * 8) & -1L << (uVar2 + 1) * 8 |
                        *(ulonglong *)((int)(uVar43 + 0x27) - (int)uVar2) >> (7 - uVar2) * 8;
                uVar2 = uVar43 + 0x2f & 7;
                in_t1 = (*(longlong *)((int)(uVar43 + 0x28) - (int)uVar7) << uVar7 * 8 |
                        in_t1 & 0xffffffffffffffffU >> (8 - uVar7) * 8) & -1L << (uVar2 + 1) * 8 |
                        *(ulonglong *)((int)(uVar43 + 0x2f) - (int)uVar2) >> (7 - uVar2) * 8;
                uVar2 = uVar43 + 0x37 & 7;
                in_t2 = (*(longlong *)((int)(uVar43 + 0x30) - (int)uVar8) << uVar8 * 8 |
                        in_t2 & 0xffffffffffffffffU >> (8 - uVar8) * 8) & -1L << (uVar2 + 1) * 8 |
                        *(ulonglong *)((int)(uVar43 + 0x37) - (int)uVar2) >> (7 - uVar2) * 8;
                uVar2 = uVar43 + 0x3f & 7;
                in_t3 = (*(longlong *)((int)(uVar43 + 0x38) - (int)uVar9) << uVar9 * 8 |
                        in_t3 & 0xffffffffffffffffU >> (8 - uVar9) * 8) & -1L << (uVar2 + 1) * 8 |
                        *(ulonglong *)((int)(uVar43 + 0x3f) - (int)uVar2) >> (7 - uVar2) * 8;
                uVar2 = uVar43 + 0x47 & 7;
                uVar60 = (*(longlong *)((int)(uVar43 + 0x40) - (int)uVar10) << uVar10 * 8 |
                         uVar60 & 0xffffffffffffffffU >> (8 - uVar10) * 8) & -1L << (uVar2 + 1) * 8
                         | *(ulonglong *)((int)(uVar43 + 0x47) - (int)uVar2) >> (7 - uVar2) * 8;
                uVar2 = uVar43 + 0x4f & 7;
                uVar64 = (*(longlong *)((int)(uVar43 + 0x48) - (int)uVar11) << uVar11 * 8 |
                         uVar64 & 0xffffffffffffffffU >> (8 - uVar11) * 8) & -1L << (uVar2 + 1) * 8
                         | *(ulonglong *)((int)(uVar43 + 0x4f) - (int)uVar2) >> (7 - uVar2) * 8;
                uVar2 = uVar43 + 0x57 & 7;
                uVar34 = (*(longlong *)((int)(uVar43 + 0x50) - (int)uVar12) << uVar12 * 8 |
                         uVar34 & 0xffffffffffffffffU >> (8 - uVar12) * 8) & -1L << (uVar2 + 1) * 8
                         | *(ulonglong *)((int)(uVar43 + 0x57) - (int)uVar2) >> (7 - uVar2) * 8;
                uVar2 = uVar43 + 0x5f & 7;
                in_v1 = (*(longlong *)((int)(uVar43 + 0x58) - (int)uVar13) << uVar13 * 8 |
                        in_v1 & 0xffffffffffffffffU >> (8 - uVar13) * 8) & -1L << (uVar2 + 1) * 8 |
                        *(ulonglong *)((int)(uVar43 + 0x5f) - (int)uVar2) >> (7 - uVar2) * 8;
                uVar2 = uVar43 + 0x67 & 7;
                unaff_s0 = (*(longlong *)((int)(uVar43 + 0x60) - (int)uVar14) << uVar14 * 8 |
                           unaff_s0 & 0xffffffffffffffffU >> (8 - uVar14) * 8) &
                           -1L << (uVar2 + 1) * 8 |
                           *(ulonglong *)((int)(uVar43 + 0x67) - (int)uVar2) >> (7 - uVar2) * 8;
                uVar2 = uVar43 + 0x6f & 7;
                unaff_s1 = (*(longlong *)((int)(uVar43 + 0x68) - (int)uVar15) << uVar15 * 8 |
                           unaff_s1 & 0xffffffffffffffffU >> (8 - uVar15) * 8) &
                           -1L << (uVar2 + 1) * 8 |
                           *(ulonglong *)((int)(uVar43 + 0x6f) - (int)uVar2) >> (7 - uVar2) * 8;
                uVar2 = uVar43 + 0x77 & 7;
                unaff_s2 = (*(longlong *)((int)(uVar43 + 0x70) - (int)uVar16) << uVar16 * 8 |
                           unaff_s2 & 0xffffffffffffffffU >> (8 - uVar16) * 8) &
                           -1L << (uVar2 + 1) * 8 |
                           *(ulonglong *)((int)(uVar43 + 0x77) - (int)uVar2) >> (7 - uVar2) * 8;
                uVar2 = uVar43 + 0x7f & 7;
                uVar62 = (*(longlong *)((int)(uVar43 + 0x78) - (int)uVar17) << uVar17 * 8 |
                         uVar62 & 0xffffffffffffffffU >> (8 - uVar17) * 8) & -1L << (uVar2 + 1) * 8
                         | *(ulonglong *)((int)(uVar43 + 0x7f) - (int)uVar2) >> (7 - uVar2) * 8;
                puVar36 = (ulonglong *)uVar59;
                puVar42 = puVar36 + 0x10;
                uVar59 = (ulonglong)(int)puVar42;
                uVar43 = (ulonglong)((int)uVar43 + 0x80);
                *puVar36 = in_t4;
                puVar36[1] = in_t5;
                puVar36[2] = in_t6;
                puVar36[3] = in_t7;
                puVar36[4] = in_t0;
                puVar36[5] = in_t1;
                puVar36[6] = in_t2;
                puVar36[7] = in_t3;
                puVar36[8] = uVar60;
                puVar36[9] = uVar64;
                puVar36[10] = uVar34;
                puVar36[0xb] = in_v1;
                puVar36[0xc] = unaff_s0;
                puVar36[0xd] = unaff_s1;
                puVar36[0xe] = unaff_s2;
                puVar36[0xf] = uVar62;
              } while (uVar47 != uVar59);
            }
          }
          puVar45 = (ulonglong *)uVar43;
          puVar36 = puVar42;
          if ((uVar46 & 0x40) != 0) {
            uVar34 = uVar43 & 7;
            plVar30 = (longlong *)((int)puVar45 - (int)uVar34);
            uVar59 = uVar43 + 8 & 7;
            lVar18 = *(longlong *)((int)(uVar43 + 8) - (int)uVar59);
            uVar62 = uVar43 + 0x10 & 7;
            lVar19 = *(longlong *)((int)(uVar43 + 0x10) - (int)uVar62);
            uVar60 = uVar43 + 0x18 & 7;
            lVar20 = *(longlong *)((int)(uVar43 + 0x18) - (int)uVar60);
            uVar47 = uVar43 + 0x20 & 7;
            lVar21 = *(longlong *)((int)(uVar43 + 0x20) - (int)uVar47);
            uVar64 = uVar43 + 0x28 & 7;
            lVar22 = *(longlong *)((int)(uVar43 + 0x28) - (int)uVar64);
            uVar2 = uVar43 + 0x30 & 7;
            lVar23 = *(longlong *)((int)(uVar43 + 0x30) - (int)uVar2);
            uVar3 = uVar43 + 0x38 & 7;
            lVar24 = *(longlong *)((int)(uVar43 + 0x38) - (int)uVar3);
            uVar4 = uVar43 + 7 & 7;
            uVar5 = uVar43 + 0xf & 7;
            uVar12 = *(ulonglong *)((int)(uVar43 + 0xf) - (int)uVar5);
            uVar6 = uVar43 + 0x17 & 7;
            uVar13 = *(ulonglong *)((int)(uVar43 + 0x17) - (int)uVar6);
            uVar7 = uVar43 + 0x1f & 7;
            uVar14 = *(ulonglong *)((int)(uVar43 + 0x1f) - (int)uVar7);
            uVar8 = uVar43 + 0x27 & 7;
            uVar15 = *(ulonglong *)((int)(uVar43 + 0x27) - (int)uVar8);
            uVar9 = uVar43 + 0x2f & 7;
            uVar16 = *(ulonglong *)((int)(uVar43 + 0x2f) - (int)uVar9);
            uVar10 = uVar43 + 0x37 & 7;
            uVar17 = *(ulonglong *)((int)(uVar43 + 0x37) - (int)uVar10);
            uVar11 = uVar43 + 0x3f & 7;
            uVar25 = *(ulonglong *)((int)(uVar43 + 0x3f) - (int)uVar11);
            puVar45 = puVar45 + 8;
            puVar36 = puVar42 + 8;
            *puVar42 = (*plVar30 << uVar34 * 8 | in_t4 & 0xffffffffffffffffU >> (8 - uVar34) * 8) &
                       -1L << (uVar4 + 1) * 8 |
                       *(ulonglong *)((int)(uVar43 + 7) - (int)uVar4) >> (7 - uVar4) * 8;
            puVar42[1] = (lVar18 << uVar59 * 8 | in_t5 & 0xffffffffffffffffU >> (8 - uVar59) * 8) &
                         -1L << (uVar5 + 1) * 8 | uVar12 >> (7 - uVar5) * 8;
            puVar42[2] = (lVar19 << uVar62 * 8 | in_t6 & 0xffffffffffffffffU >> (8 - uVar62) * 8) &
                         -1L << (uVar6 + 1) * 8 | uVar13 >> (7 - uVar6) * 8;
            puVar42[3] = (lVar20 << uVar60 * 8 | in_t7 & 0xffffffffffffffffU >> (8 - uVar60) * 8) &
                         -1L << (uVar7 + 1) * 8 | uVar14 >> (7 - uVar7) * 8;
            puVar42[4] = (lVar21 << uVar47 * 8 | in_t0 & 0xffffffffffffffffU >> (8 - uVar47) * 8) &
                         -1L << (uVar8 + 1) * 8 | uVar15 >> (7 - uVar8) * 8;
            puVar42[5] = (lVar22 << uVar64 * 8 | in_t1 & 0xffffffffffffffffU >> (8 - uVar64) * 8) &
                         -1L << (uVar9 + 1) * 8 | uVar16 >> (7 - uVar9) * 8;
            puVar42[6] = (lVar23 << uVar2 * 8 | in_t2 & 0xffffffffffffffffU >> (8 - uVar2) * 8) &
                         -1L << (uVar10 + 1) * 8 | uVar17 >> (7 - uVar10) * 8;
            puVar42[7] = (lVar24 << uVar3 * 8 | in_t3 & 0xffffffffffffffffU >> (8 - uVar3) * 8) &
                         -1L << (uVar11 + 1) * 8 | uVar25 >> (7 - uVar11) * 8;
          }
          puVar42 = puVar36;
          if ((uVar46 & 0x20) != 0) {
            uVar43 = *puVar45;
            uVar34 = puVar45[1];
            uVar59 = puVar45[2];
            uVar62 = puVar45[3];
            puVar45 = puVar45 + 4;
            puVar42 = puVar36 + 4;
            *puVar36 = uVar43;
            puVar36[1] = uVar34;
            puVar36[2] = uVar59;
            puVar36[3] = uVar62;
          }
          puVar36 = puVar42;
          if ((uVar46 & 0x10) != 0) {
            uVar43 = *puVar45;
            uVar34 = puVar45[1];
            puVar45 = puVar45 + 2;
            puVar36 = puVar42 + 2;
            *puVar42 = uVar43;
            puVar42[1] = uVar34;
          }
          puVar42 = puVar36;
          if ((uVar46 & 8) != 0) {
            uVar43 = *puVar45;
            puVar45 = puVar45 + 1;
            puVar42 = puVar36 + 1;
            *puVar36 = uVar43;
          }
          iVar48 = (int)(uVar46 & 7);
          if ((uVar46 & 7) != 0) {
            uVar1 = (int)puVar42 + iVar48 + -1;
            uVar41 = uVar1 & 7;
            puVar42 = (ulonglong *)(uVar1 - uVar41);
            *puVar42 = *(longlong *)((int)puVar45 + iVar48 + -8) << (7 - uVar41) * 8 |
                       *puVar42 & 0xffffffffffffffffU >> (uVar41 + 1) * 8;
          }
          return;
        }
        uVar60 = 0;
        if (uVar59 != 0) {
          uVar43 = uVar43 & 7;
          uVar60 = *(longlong *)((int)__src - (int)uVar43) << uVar43 * 8 |
                   uVar59 & 0xffffffffffffffffU >> (8 - uVar43) * 8;
          iVar48 = 8 - (int)uVar62;
          uVar43 = (ulonglong)((int)__src + iVar48);
          uVar34 = uVar34 & 7;
          puVar42 = (ulonglong *)((int)__dest - (int)uVar34);
          *puVar42 = *puVar42 & -1L << (8 - uVar34) * 8 | uVar60 >> uVar34 * 8;
          uVar34 = (ulonglong)((int)__dest + iVar48);
          uVar46 = (ulonglong)(int)(__n - iVar48);
        }
        puVar38 = (undefined8 *)uVar43;
        if (((uVar46 < 0x100) || (_blk_fp == 0)) || ((_blk_fp < 1 && (_blk_init(), _blk_fp == 0))))
        {
          puVar40 = (undefined8 *)uVar34;
          if ((uVar46 & 0xffffffffffffff80) != 0) {
            uVar59 = (ulonglong)((int)(uVar46 & 0xffffffffffffff80) + (int)puVar40);
            if ((int)uVar46 + -0x1000 < 0) {
              do {
                puVar44 = (undefined8 *)uVar43;
                uVar52 = puVar44[1];
                uVar53 = puVar44[2];
                uVar54 = puVar44[3];
                uVar33 = puVar44[4];
                uVar49 = puVar44[5];
                uVar50 = puVar44[6];
                uVar51 = puVar44[7];
                uVar55 = puVar44[8];
                uVar56 = puVar44[9];
                uVar32 = puVar44[10];
                uVar31 = puVar44[0xb];
                puVar35 = (undefined8 *)uVar34;
                *puVar35 = *puVar44;
                puVar35[1] = uVar52;
                puVar35[2] = uVar53;
                puVar35[3] = uVar54;
                puVar35[4] = uVar33;
                puVar35[5] = uVar49;
                puVar35[6] = uVar50;
                puVar35[7] = uVar51;
                puVar35[8] = uVar55;
                puVar35[9] = uVar56;
                puVar35[10] = uVar32;
                puVar35[0xb] = uVar31;
                uVar33 = puVar44[0xe];
                uVar32 = puVar44[0xd];
                uVar31 = puVar44[0xc];
                puVar40 = puVar35 + 0x10;
                uVar34 = (ulonglong)(int)puVar40;
                puVar38 = puVar44 + 0x10;
                uVar43 = (ulonglong)(int)puVar38;
                puVar35[0xf] = puVar44[0xf];
                puVar35[0xe] = uVar33;
                puVar35[0xd] = uVar32;
                puVar35[0xc] = uVar31;
              } while (uVar59 != uVar34);
            }
            else {
              do {
                puVar44 = (undefined8 *)uVar43;
                uVar53 = puVar44[1];
                uVar54 = puVar44[2];
                uVar55 = puVar44[3];
                uVar49 = puVar44[4];
                uVar50 = puVar44[5];
                uVar51 = puVar44[6];
                uVar52 = puVar44[7];
                uVar61 = puVar44[8];
                uVar63 = puVar44[9];
                uVar32 = puVar44[10];
                uVar33 = puVar44[0xb];
                uVar56 = puVar44[0xc];
                uVar57 = puVar44[0xd];
                uVar58 = puVar44[0xe];
                uVar31 = puVar44[0xf];
                puVar35 = (undefined8 *)uVar34;
                puVar40 = puVar35 + 0x10;
                uVar34 = (ulonglong)(int)puVar40;
                puVar38 = puVar44 + 0x10;
                uVar43 = (ulonglong)(int)puVar38;
                *puVar35 = *puVar44;
                puVar35[1] = uVar53;
                puVar35[2] = uVar54;
                puVar35[3] = uVar55;
                puVar35[4] = uVar49;
                puVar35[5] = uVar50;
                puVar35[6] = uVar51;
                puVar35[7] = uVar52;
                puVar35[8] = uVar61;
                puVar35[9] = uVar63;
                puVar35[10] = uVar32;
                puVar35[0xb] = uVar33;
                puVar35[0xc] = uVar56;
                puVar35[0xd] = uVar57;
                puVar35[0xe] = uVar58;
                puVar35[0xf] = uVar31;
              } while (uVar59 != uVar34);
            }
          }
          puVar35 = puVar40;
          if ((uVar46 & 0x40) != 0) {
            uVar50 = *puVar38;
            uVar51 = puVar38[1];
            uVar52 = puVar38[2];
            uVar53 = puVar38[3];
            uVar31 = puVar38[4];
            uVar32 = puVar38[5];
            uVar33 = puVar38[6];
            uVar49 = puVar38[7];
            puVar38 = puVar38 + 8;
            puVar35 = puVar40 + 8;
            *puVar40 = uVar50;
            puVar40[1] = uVar51;
            puVar40[2] = uVar52;
            puVar40[3] = uVar53;
            puVar40[4] = uVar31;
            puVar40[5] = uVar32;
            puVar40[6] = uVar33;
            puVar40[7] = uVar49;
          }
          uVar60 = uVar46 & 0x10;
          puVar44 = puVar35;
          if ((uVar46 & 0x20) != 0) {
            uVar31 = *puVar38;
            uVar32 = puVar38[1];
            uVar33 = puVar38[2];
            uVar49 = puVar38[3];
            puVar38 = puVar38 + 4;
            puVar44 = puVar35 + 4;
            *puVar35 = uVar31;
            puVar35[1] = uVar32;
            puVar35[2] = uVar33;
            puVar35[3] = uVar49;
          }
          uVar43 = uVar46 & 8;
          puVar40 = puVar44;
          if (uVar60 != 0) {
            uVar31 = *puVar38;
            uVar32 = puVar38[1];
            puVar38 = puVar38 + 2;
            puVar40 = puVar44 + 2;
            *puVar44 = uVar31;
            puVar44[1] = uVar32;
          }
        }
        else {
          _memcpy_fp();
          uVar43 = extraout_a2 & 8;
          uVar46 = extraout_a2;
          puVar40 = extraout_a0_lo;
          puVar38 = extraout_a1_lo;
          if (extraout_a2 == 0) {
            return;
          }
        }
        puVar35 = puVar40;
        if (uVar43 != 0) {
          uVar31 = *puVar38;
          puVar38 = puVar38 + 1;
          puVar35 = puVar40 + 1;
          *puVar40 = uVar31;
        }
        iVar48 = (int)(uVar46 & 7);
        if ((uVar46 & 7) != 0) {
          uVar1 = (int)puVar38 + iVar48 + -1;
          uVar26 = uVar1 & 7;
          uVar41 = (int)puVar35 + iVar48 + -1;
          uVar29 = uVar41 & 7;
          puVar42 = (ulonglong *)(uVar41 - uVar29);
          *puVar42 = (uVar60 & -1L << (uVar26 + 1) * 8 |
                     *(ulonglong *)(uVar1 - uVar26) >> (7 - uVar26) * 8) << (7 - uVar29) * 8 |
                     *puVar42 & 0xffffffffffffffffU >> (uVar29 + 1) * 8;
        }
        return;
      }
      while (iVar48 = (int)uVar46, 7 < uVar46) {
        uVar59 = *(ulonglong *)uVar43;
        uVar46 = uVar34 & 7;
        puVar42 = (ulonglong *)((int)uVar34 - (int)uVar46);
        *puVar42 = *puVar42 & -1L << (8 - uVar46) * 8 | uVar59 >> uVar46 * 8;
        __src = (ulonglong *)uVar43 + 1;
        uVar43 = (ulonglong)(int)__src;
        __dest = (void *)((int)uVar34 + 8);
        uVar34 = (ulonglong)(int)__dest;
        uVar46 = uVar34 - 1 & 7;
        puVar42 = (ulonglong *)((int)(uVar34 - 1) - (int)uVar46);
        *puVar42 = uVar59 << (7 - uVar46) * 8 | *puVar42 & 0xffffffffffffffffU >> (uVar46 + 1) * 8;
        uVar46 = (ulonglong)(iVar48 + -8);
      }
      if (((uVar46 != 0) && (*(undefined1 *)__dest = *(undefined1 *)__src, iVar48 != 1)) &&
         ((*(undefined1 *)((int)__dest + 1) = *(undefined1 *)((int)__src + 1), iVar48 != 2 &&
          ((((*(undefined1 *)((int)__dest + 2) = *(undefined1 *)((int)__src + 2), iVar48 != 3 &&
             (*(undefined1 *)((int)__dest + 3) = *(undefined1 *)((int)__src + 3), iVar48 != 4)) &&
            (*(undefined1 *)((int)__dest + 4) = *(undefined1 *)((int)__src + 4), iVar48 != 5)) &&
           (*(undefined1 *)((int)__dest + 5) = *(undefined1 *)((int)__src + 5), iVar48 != 6)))))) {
        *(undefined1 *)((int)__dest + 6) = *(undefined1 *)((int)__src + 6);
      }
    }
    else {
      iVar48 = (int)__src + __n;
      uVar43 = (ulonglong)iVar48;
      iVar37 = (int)__dest + __n;
      uVar59 = (ulonglong)iVar37;
      if (0x1f < uVar46) {
        uVar62 = uVar43 & 7;
        uVar60 = uVar59 & 7;
        iVar39 = (int)uVar60;
        if (uVar62 == uVar60) {
          if (uVar62 != 0) {
            uVar34 = uVar43 - 1;
            uVar60 = uVar34 & 7;
            uVar43 = (ulonglong)(iVar48 - iVar39);
            uVar46 = (ulonglong)(int)(__n - iVar39);
            uVar47 = uVar59 - 1 & 7;
            puVar42 = (ulonglong *)((int)(uVar59 - 1) - (int)uVar47);
            *puVar42 = (uVar62 & -1L << (uVar60 + 1) * 8 |
                       *(ulonglong *)((int)uVar34 - (int)uVar60) >> (7 - uVar60) * 8) <<
                       (7 - uVar47) * 8 | *puVar42 & 0xffffffffffffffffU >> (uVar47 + 1) * 8;
            uVar59 = (ulonglong)(iVar37 - iVar39);
          }
          iVar48 = (int)uVar43;
          puVar38 = (undefined8 *)uVar59;
          if ((uVar46 & 0xffffffffffffff80) != 0) {
            uVar34 = (ulonglong)((int)puVar38 - (int)(uVar46 & 0xffffffffffffff80));
            if ((int)uVar46 + -0x1000 < 0) {
              do {
                iVar37 = (int)uVar43;
                uVar52 = *(undefined8 *)(iVar37 + -0x10);
                uVar53 = *(undefined8 *)(iVar37 + -0x18);
                uVar54 = *(undefined8 *)(iVar37 + -0x20);
                uVar33 = *(undefined8 *)(iVar37 + -0x28);
                uVar49 = *(undefined8 *)(iVar37 + -0x30);
                uVar50 = *(undefined8 *)(iVar37 + -0x38);
                uVar51 = *(undefined8 *)(iVar37 + -0x40);
                uVar55 = *(undefined8 *)(iVar37 + -0x48);
                uVar56 = *(undefined8 *)(iVar37 + -0x50);
                uVar32 = *(undefined8 *)(iVar37 + -0x58);
                uVar31 = *(undefined8 *)(iVar37 + -0x60);
                iVar39 = (int)uVar59;
                *(undefined8 *)(iVar39 + -8) = *(undefined8 *)(iVar37 + -8);
                *(undefined8 *)(iVar39 + -0x10) = uVar52;
                *(undefined8 *)(iVar39 + -0x18) = uVar53;
                *(undefined8 *)(iVar39 + -0x20) = uVar54;
                *(undefined8 *)(iVar39 + -0x28) = uVar33;
                *(undefined8 *)(iVar39 + -0x30) = uVar49;
                *(undefined8 *)(iVar39 + -0x38) = uVar50;
                *(undefined8 *)(iVar39 + -0x40) = uVar51;
                *(undefined8 *)(iVar39 + -0x48) = uVar55;
                *(undefined8 *)(iVar39 + -0x50) = uVar56;
                *(undefined8 *)(iVar39 + -0x58) = uVar32;
                *(undefined8 *)(iVar39 + -0x60) = uVar31;
                uVar33 = *(undefined8 *)(iVar37 + -0x78);
                uVar32 = *(undefined8 *)(iVar37 + -0x70);
                uVar31 = *(undefined8 *)(iVar37 + -0x68);
                puVar38 = (undefined8 *)(iVar39 + -0x80);
                uVar59 = (ulonglong)(int)puVar38;
                iVar48 = iVar37 + -0x80;
                uVar43 = (ulonglong)iVar48;
                *puVar38 = *(undefined8 *)(iVar37 + -0x80);
                *(undefined8 *)(iVar39 + -0x78) = uVar33;
                *(undefined8 *)(iVar39 + -0x70) = uVar32;
                *(undefined8 *)(iVar39 + -0x68) = uVar31;
              } while (uVar34 != uVar59);
            }
            else {
              do {
                iVar37 = (int)uVar43;
                uVar53 = *(undefined8 *)(iVar37 + -0x10);
                uVar54 = *(undefined8 *)(iVar37 + -0x18);
                uVar55 = *(undefined8 *)(iVar37 + -0x20);
                uVar49 = *(undefined8 *)(iVar37 + -0x28);
                uVar50 = *(undefined8 *)(iVar37 + -0x30);
                uVar51 = *(undefined8 *)(iVar37 + -0x38);
                uVar52 = *(undefined8 *)(iVar37 + -0x40);
                uVar61 = *(undefined8 *)(iVar37 + -0x48);
                uVar63 = *(undefined8 *)(iVar37 + -0x50);
                uVar32 = *(undefined8 *)(iVar37 + -0x58);
                uVar33 = *(undefined8 *)(iVar37 + -0x60);
                uVar56 = *(undefined8 *)(iVar37 + -0x68);
                uVar57 = *(undefined8 *)(iVar37 + -0x70);
                uVar58 = *(undefined8 *)(iVar37 + -0x78);
                uVar31 = *(undefined8 *)(iVar37 + -0x80);
                iVar39 = (int)uVar59;
                puVar38 = (undefined8 *)(iVar39 + -0x80);
                uVar59 = (ulonglong)(int)puVar38;
                iVar48 = iVar37 + -0x80;
                uVar43 = (ulonglong)iVar48;
                *(undefined8 *)(iVar39 + -8) = *(undefined8 *)(iVar37 + -8);
                *(undefined8 *)(iVar39 + -0x10) = uVar53;
                *(undefined8 *)(iVar39 + -0x18) = uVar54;
                *(undefined8 *)(iVar39 + -0x20) = uVar55;
                *(undefined8 *)(iVar39 + -0x28) = uVar49;
                *(undefined8 *)(iVar39 + -0x30) = uVar50;
                *(undefined8 *)(iVar39 + -0x38) = uVar51;
                *(undefined8 *)(iVar39 + -0x40) = uVar52;
                *(undefined8 *)(iVar39 + -0x48) = uVar61;
                *(undefined8 *)(iVar39 + -0x50) = uVar63;
                *(undefined8 *)(iVar39 + -0x58) = uVar32;
                *(undefined8 *)(iVar39 + -0x60) = uVar33;
                *(undefined8 *)(iVar39 + -0x68) = uVar56;
                *(undefined8 *)(iVar39 + -0x70) = uVar57;
                *(undefined8 *)(iVar39 + -0x78) = uVar58;
                *puVar38 = uVar31;
              } while (uVar34 != uVar59);
            }
          }
          puVar40 = puVar38;
          if ((uVar46 & 0x40) != 0) {
            puVar35 = (undefined8 *)(iVar48 + -8);
            uVar50 = *(undefined8 *)(iVar48 + -0x10);
            uVar51 = *(undefined8 *)(iVar48 + -0x18);
            uVar52 = *(undefined8 *)(iVar48 + -0x20);
            uVar31 = *(undefined8 *)(iVar48 + -0x28);
            uVar32 = *(undefined8 *)(iVar48 + -0x30);
            uVar33 = *(undefined8 *)(iVar48 + -0x38);
            uVar49 = *(undefined8 *)(iVar48 + -0x40);
            iVar48 = iVar48 + -0x40;
            puVar40 = puVar38 + -8;
            puVar38[-1] = *puVar35;
            puVar38[-2] = uVar50;
            puVar38[-3] = uVar51;
            puVar38[-4] = uVar52;
            puVar38[-5] = uVar31;
            puVar38[-6] = uVar32;
            puVar38[-7] = uVar33;
            *puVar40 = uVar49;
          }
          puVar38 = puVar40;
          if ((uVar46 & 0x20) != 0) {
            puVar35 = (undefined8 *)(iVar48 + -8);
            uVar31 = *(undefined8 *)(iVar48 + -0x10);
            uVar32 = *(undefined8 *)(iVar48 + -0x18);
            uVar33 = *(undefined8 *)(iVar48 + -0x20);
            iVar48 = iVar48 + -0x20;
            puVar38 = puVar40 + -4;
            puVar40[-1] = *puVar35;
            puVar40[-2] = uVar31;
            puVar40[-3] = uVar32;
            *puVar38 = uVar33;
          }
          puVar40 = puVar38;
          if ((uVar46 & 0x10) != 0) {
            puVar35 = (undefined8 *)(iVar48 + -8);
            uVar31 = *(undefined8 *)(iVar48 + -0x10);
            iVar48 = iVar48 + -0x10;
            puVar40 = puVar38 + -2;
            puVar38[-1] = *puVar35;
            *puVar40 = uVar31;
          }
          if ((uVar46 & 8) != 0) {
            puVar38 = (undefined8 *)(iVar48 + -8);
            iVar48 = iVar48 + -8;
            puVar40 = puVar40 + -1;
            *puVar40 = *puVar38;
          }
          iVar37 = (int)(uVar46 & 7);
          iVar48 = iVar48 - iVar37;
          if ((uVar46 & 7) != 0) {
            uVar43 = (longlong)iVar48 & 7;
            uVar41 = (int)puVar40 - iVar37;
            uVar1 = uVar41 & 7;
            puVar42 = (ulonglong *)(uVar41 - uVar1);
            *puVar42 = *puVar42 & -1L << (8 - uVar1) * 8 |
                       (*(longlong *)(iVar48 - (int)uVar43) << uVar43 * 8 |
                       uVar46 & 0x10 & 0xffffffffffffffffU >> (8 - uVar43) * 8) >> uVar1 * 8;
          }
          return;
        }
        if (uVar60 != 0) {
          uVar43 = (ulonglong)(iVar48 - iVar39);
          uVar46 = (ulonglong)(int)(__n - iVar39);
          uVar62 = uVar59 - 1 & 7;
          puVar42 = (ulonglong *)((int)(uVar59 - 1) - (int)uVar62);
          *puVar42 = *(longlong *)(iVar48 + -8) << (7 - uVar62) * 8 |
                     *puVar42 & 0xffffffffffffffffU >> (uVar62 + 1) * 8;
          uVar59 = (ulonglong)(iVar37 - iVar39);
        }
        puVar42 = (ulonglong *)uVar59;
        uVar62 = 0xffffffffffffff80;
        uVar60 = uVar46 & 0x40;
        if ((uVar46 & 0xffffffffffffff80) != 0) {
          uVar64 = (ulonglong)((int)uVar46 + -0x1000);
          uVar47 = (ulonglong)((int)puVar42 - (int)(uVar46 & 0xffffffffffffff80));
          if ((longlong)uVar64 < 0) {
            do {
              uVar34 = uVar43 - 8 & 7;
              uVar2 = uVar43 - 0x10 & 7;
              lVar18 = *(longlong *)((int)(uVar43 - 0x10) - (int)uVar2);
              uVar3 = uVar43 - 0x18 & 7;
              lVar19 = *(longlong *)((int)(uVar43 - 0x18) - (int)uVar3);
              uVar4 = uVar43 - 0x20 & 7;
              lVar20 = *(longlong *)((int)(uVar43 - 0x20) - (int)uVar4);
              uVar5 = uVar43 - 0x28 & 7;
              uVar6 = uVar43 - 0x30 & 7;
              uVar7 = uVar43 - 0x38 & 7;
              uVar8 = uVar43 - 0x40 & 7;
              uVar9 = uVar43 - 0x48 & 7;
              uVar10 = uVar43 - 0x50 & 7;
              uVar11 = uVar43 - 0x58 & 7;
              uVar12 = uVar43 - 0x60 & 7;
              uVar13 = uVar43 - 1 & 7;
              uVar14 = uVar43 - 9 & 7;
              uVar25 = *(ulonglong *)((int)(uVar43 - 9) - (int)uVar14);
              uVar15 = uVar43 - 0x11 & 7;
              uVar27 = *(ulonglong *)((int)(uVar43 - 0x11) - (int)uVar15);
              uVar16 = uVar43 - 0x19 & 7;
              uVar28 = *(ulonglong *)((int)(uVar43 - 0x19) - (int)uVar16);
              uVar17 = uVar43 - 0x21 & 7;
              in_t0 = (*(longlong *)((int)(uVar43 - 0x28) - (int)uVar5) << uVar5 * 8 |
                      in_t0 & 0xffffffffffffffffU >> (8 - uVar5) * 8) & -1L << (uVar17 + 1) * 8 |
                      *(ulonglong *)((int)(uVar43 - 0x21) - (int)uVar17) >> (7 - uVar17) * 8;
              uVar5 = uVar43 - 0x29 & 7;
              in_t1 = (*(longlong *)((int)(uVar43 - 0x30) - (int)uVar6) << uVar6 * 8 |
                      in_t1 & 0xffffffffffffffffU >> (8 - uVar6) * 8) & -1L << (uVar5 + 1) * 8 |
                      *(ulonglong *)((int)(uVar43 - 0x29) - (int)uVar5) >> (7 - uVar5) * 8;
              uVar5 = uVar43 - 0x31 & 7;
              in_t2 = (*(longlong *)((int)(uVar43 - 0x38) - (int)uVar7) << uVar7 * 8 |
                      in_t2 & 0xffffffffffffffffU >> (8 - uVar7) * 8) & -1L << (uVar5 + 1) * 8 |
                      *(ulonglong *)((int)(uVar43 - 0x31) - (int)uVar5) >> (7 - uVar5) * 8;
              uVar5 = uVar43 - 0x39 & 7;
              in_t3 = (*(longlong *)((int)(uVar43 - 0x40) - (int)uVar8) << uVar8 * 8 |
                      in_t3 & 0xffffffffffffffffU >> (8 - uVar8) * 8) & -1L << (uVar5 + 1) * 8 |
                      *(ulonglong *)((int)(uVar43 - 0x39) - (int)uVar5) >> (7 - uVar5) * 8;
              uVar5 = uVar43 - 0x41 & 7;
              uVar60 = (*(longlong *)((int)(uVar43 - 0x48) - (int)uVar9) << uVar9 * 8 |
                       uVar60 & 0xffffffffffffffffU >> (8 - uVar9) * 8) & -1L << (uVar5 + 1) * 8 |
                       *(ulonglong *)((int)(uVar43 - 0x41) - (int)uVar5) >> (7 - uVar5) * 8;
              uVar5 = uVar43 - 0x49 & 7;
              uVar64 = (*(longlong *)((int)(uVar43 - 0x50) - (int)uVar10) << uVar10 * 8 |
                       uVar64 & 0xffffffffffffffffU >> (8 - uVar10) * 8) & -1L << (uVar5 + 1) * 8 |
                       *(ulonglong *)((int)(uVar43 - 0x49) - (int)uVar5) >> (7 - uVar5) * 8;
              uVar5 = uVar43 - 0x51 & 7;
              in_v1 = (*(longlong *)((int)(uVar43 - 0x58) - (int)uVar11) << uVar11 * 8 |
                      in_v1 & 0xffffffffffffffffU >> (8 - uVar11) * 8) & -1L << (uVar5 + 1) * 8 |
                      *(ulonglong *)((int)(uVar43 - 0x51) - (int)uVar5) >> (7 - uVar5) * 8;
              uVar5 = uVar43 - 0x59 & 7;
              uVar62 = (*(longlong *)((int)(uVar43 - 0x60) - (int)uVar12) << uVar12 * 8 |
                       uVar62 & 0xffffffffffffffffU >> (8 - uVar12) * 8) & -1L << (uVar5 + 1) * 8 |
                       *(ulonglong *)((int)(uVar43 - 0x59) - (int)uVar5) >> (7 - uVar5) * 8;
              iVar48 = (int)uVar59;
              *(ulonglong *)(iVar48 + -8) =
                   (*(longlong *)((int)(uVar43 - 8) - (int)uVar34) << uVar34 * 8 |
                   in_t4 & 0xffffffffffffffffU >> (8 - uVar34) * 8) & -1L << (uVar13 + 1) * 8 |
                   *(ulonglong *)((int)(uVar43 - 1) - (int)uVar13) >> (7 - uVar13) * 8;
              *(ulonglong *)(iVar48 + -0x10) =
                   (lVar18 << uVar2 * 8 | in_t5 & 0xffffffffffffffffU >> (8 - uVar2) * 8) &
                   -1L << (uVar14 + 1) * 8 | uVar25 >> (7 - uVar14) * 8;
              *(ulonglong *)(iVar48 + -0x18) =
                   (lVar19 << uVar3 * 8 | in_t6 & 0xffffffffffffffffU >> (8 - uVar3) * 8) &
                   -1L << (uVar15 + 1) * 8 | uVar27 >> (7 - uVar15) * 8;
              *(ulonglong *)(iVar48 + -0x20) =
                   (lVar20 << uVar4 * 8 | in_t7 & 0xffffffffffffffffU >> (8 - uVar4) * 8) &
                   -1L << (uVar16 + 1) * 8 | uVar28 >> (7 - uVar16) * 8;
              *(ulonglong *)(iVar48 + -0x28) = in_t0;
              *(ulonglong *)(iVar48 + -0x30) = in_t1;
              *(ulonglong *)(iVar48 + -0x38) = in_t2;
              *(ulonglong *)(iVar48 + -0x40) = in_t3;
              *(ulonglong *)(iVar48 + -0x48) = uVar60;
              *(ulonglong *)(iVar48 + -0x50) = uVar64;
              *(ulonglong *)(iVar48 + -0x58) = in_v1;
              *(ulonglong *)(iVar48 + -0x60) = uVar62;
              iVar37 = (int)uVar43;
              in_t4 = *(ulonglong *)(iVar37 + -0x68);
              in_t5 = *(ulonglong *)(iVar37 + -0x70);
              in_t6 = *(ulonglong *)(iVar37 + -0x78);
              in_t7 = *(ulonglong *)(iVar37 + -0x80);
              puVar42 = (ulonglong *)(iVar48 + -0x80);
              uVar59 = (ulonglong)(int)puVar42;
              uVar43 = (ulonglong)(iVar37 + -0x80);
              *(ulonglong *)(iVar48 + -0x68) = in_t4;
              *(ulonglong *)(iVar48 + -0x70) = in_t5;
              *(ulonglong *)(iVar48 + -0x78) = in_t6;
              *puVar42 = in_t7;
            } while (uVar47 != uVar59);
          }
          else {
            do {
              uVar2 = uVar43 - 8 & 7;
              uVar3 = uVar43 - 0x10 & 7;
              uVar4 = uVar43 - 0x18 & 7;
              uVar5 = uVar43 - 0x20 & 7;
              uVar6 = uVar43 - 0x28 & 7;
              uVar7 = uVar43 - 0x30 & 7;
              uVar8 = uVar43 - 0x38 & 7;
              uVar9 = uVar43 - 0x40 & 7;
              uVar10 = uVar43 - 0x48 & 7;
              uVar11 = uVar43 - 0x50 & 7;
              uVar12 = uVar43 - 0x58 & 7;
              uVar13 = uVar43 - 0x60 & 7;
              uVar14 = uVar43 - 0x68 & 7;
              uVar15 = uVar43 - 0x70 & 7;
              uVar16 = uVar43 - 0x78 & 7;
              uVar17 = uVar43 - 0x80 & 7;
              uVar25 = uVar43 - 1 & 7;
              in_t4 = (*(longlong *)((int)(uVar43 - 8) - (int)uVar2) << uVar2 * 8 |
                      in_t4 & 0xffffffffffffffffU >> (8 - uVar2) * 8) & -1L << (uVar25 + 1) * 8 |
                      *(ulonglong *)((int)(uVar43 - 1) - (int)uVar25) >> (7 - uVar25) * 8;
              uVar2 = uVar43 - 9 & 7;
              in_t5 = (*(longlong *)((int)(uVar43 - 0x10) - (int)uVar3) << uVar3 * 8 |
                      in_t5 & 0xffffffffffffffffU >> (8 - uVar3) * 8) & -1L << (uVar2 + 1) * 8 |
                      *(ulonglong *)((int)(uVar43 - 9) - (int)uVar2) >> (7 - uVar2) * 8;
              uVar2 = uVar43 - 0x11 & 7;
              in_t6 = (*(longlong *)((int)(uVar43 - 0x18) - (int)uVar4) << uVar4 * 8 |
                      in_t6 & 0xffffffffffffffffU >> (8 - uVar4) * 8) & -1L << (uVar2 + 1) * 8 |
                      *(ulonglong *)((int)(uVar43 - 0x11) - (int)uVar2) >> (7 - uVar2) * 8;
              uVar2 = uVar43 - 0x19 & 7;
              in_t7 = (*(longlong *)((int)(uVar43 - 0x20) - (int)uVar5) << uVar5 * 8 |
                      in_t7 & 0xffffffffffffffffU >> (8 - uVar5) * 8) & -1L << (uVar2 + 1) * 8 |
                      *(ulonglong *)((int)(uVar43 - 0x19) - (int)uVar2) >> (7 - uVar2) * 8;
              uVar2 = uVar43 - 0x21 & 7;
              in_t0 = (*(longlong *)((int)(uVar43 - 0x28) - (int)uVar6) << uVar6 * 8 |
                      in_t0 & 0xffffffffffffffffU >> (8 - uVar6) * 8) & -1L << (uVar2 + 1) * 8 |
                      *(ulonglong *)((int)(uVar43 - 0x21) - (int)uVar2) >> (7 - uVar2) * 8;
              uVar2 = uVar43 - 0x29 & 7;
              in_t1 = (*(longlong *)((int)(uVar43 - 0x30) - (int)uVar7) << uVar7 * 8 |
                      in_t1 & 0xffffffffffffffffU >> (8 - uVar7) * 8) & -1L << (uVar2 + 1) * 8 |
                      *(ulonglong *)((int)(uVar43 - 0x29) - (int)uVar2) >> (7 - uVar2) * 8;
              uVar2 = uVar43 - 0x31 & 7;
              in_t2 = (*(longlong *)((int)(uVar43 - 0x38) - (int)uVar8) << uVar8 * 8 |
                      in_t2 & 0xffffffffffffffffU >> (8 - uVar8) * 8) & -1L << (uVar2 + 1) * 8 |
                      *(ulonglong *)((int)(uVar43 - 0x31) - (int)uVar2) >> (7 - uVar2) * 8;
              uVar2 = uVar43 - 0x39 & 7;
              in_t3 = (*(longlong *)((int)(uVar43 - 0x40) - (int)uVar9) << uVar9 * 8 |
                      in_t3 & 0xffffffffffffffffU >> (8 - uVar9) * 8) & -1L << (uVar2 + 1) * 8 |
                      *(ulonglong *)((int)(uVar43 - 0x39) - (int)uVar2) >> (7 - uVar2) * 8;
              uVar2 = uVar43 - 0x41 & 7;
              uVar60 = (*(longlong *)((int)(uVar43 - 0x48) - (int)uVar10) << uVar10 * 8 |
                       uVar60 & 0xffffffffffffffffU >> (8 - uVar10) * 8) & -1L << (uVar2 + 1) * 8 |
                       *(ulonglong *)((int)(uVar43 - 0x41) - (int)uVar2) >> (7 - uVar2) * 8;
              uVar2 = uVar43 - 0x49 & 7;
              uVar64 = (*(longlong *)((int)(uVar43 - 0x50) - (int)uVar11) << uVar11 * 8 |
                       uVar64 & 0xffffffffffffffffU >> (8 - uVar11) * 8) & -1L << (uVar2 + 1) * 8 |
                       *(ulonglong *)((int)(uVar43 - 0x49) - (int)uVar2) >> (7 - uVar2) * 8;
              uVar2 = uVar43 - 0x51 & 7;
              uVar34 = (*(longlong *)((int)(uVar43 - 0x58) - (int)uVar12) << uVar12 * 8 |
                       uVar34 & 0xffffffffffffffffU >> (8 - uVar12) * 8) & -1L << (uVar2 + 1) * 8 |
                       *(ulonglong *)((int)(uVar43 - 0x51) - (int)uVar2) >> (7 - uVar2) * 8;
              uVar2 = uVar43 - 0x59 & 7;
              in_v1 = (*(longlong *)((int)(uVar43 - 0x60) - (int)uVar13) << uVar13 * 8 |
                      in_v1 & 0xffffffffffffffffU >> (8 - uVar13) * 8) & -1L << (uVar2 + 1) * 8 |
                      *(ulonglong *)((int)(uVar43 - 0x59) - (int)uVar2) >> (7 - uVar2) * 8;
              uVar2 = uVar43 - 0x61 & 7;
              unaff_s0 = (*(longlong *)((int)(uVar43 - 0x68) - (int)uVar14) << uVar14 * 8 |
                         unaff_s0 & 0xffffffffffffffffU >> (8 - uVar14) * 8) &
                         -1L << (uVar2 + 1) * 8 |
                         *(ulonglong *)((int)(uVar43 - 0x61) - (int)uVar2) >> (7 - uVar2) * 8;
              uVar2 = uVar43 - 0x69 & 7;
              unaff_s1 = (*(longlong *)((int)(uVar43 - 0x70) - (int)uVar15) << uVar15 * 8 |
                         unaff_s1 & 0xffffffffffffffffU >> (8 - uVar15) * 8) &
                         -1L << (uVar2 + 1) * 8 |
                         *(ulonglong *)((int)(uVar43 - 0x69) - (int)uVar2) >> (7 - uVar2) * 8;
              uVar2 = uVar43 - 0x71 & 7;
              unaff_s2 = (*(longlong *)((int)(uVar43 - 0x78) - (int)uVar16) << uVar16 * 8 |
                         unaff_s2 & 0xffffffffffffffffU >> (8 - uVar16) * 8) &
                         -1L << (uVar2 + 1) * 8 |
                         *(ulonglong *)((int)(uVar43 - 0x71) - (int)uVar2) >> (7 - uVar2) * 8;
              uVar2 = uVar43 - 0x79 & 7;
              uVar62 = (*(longlong *)((int)(uVar43 - 0x80) - (int)uVar17) << uVar17 * 8 |
                       uVar62 & 0xffffffffffffffffU >> (8 - uVar17) * 8) & -1L << (uVar2 + 1) * 8 |
                       *(ulonglong *)((int)(uVar43 - 0x79) - (int)uVar2) >> (7 - uVar2) * 8;
              iVar48 = (int)uVar59;
              puVar42 = (ulonglong *)(iVar48 + -0x80);
              uVar59 = (ulonglong)(int)puVar42;
              uVar43 = (ulonglong)((int)uVar43 + -0x80);
              *(ulonglong *)(iVar48 + -8) = in_t4;
              *(ulonglong *)(iVar48 + -0x10) = in_t5;
              *(ulonglong *)(iVar48 + -0x18) = in_t6;
              *(ulonglong *)(iVar48 + -0x20) = in_t7;
              *(ulonglong *)(iVar48 + -0x28) = in_t0;
              *(ulonglong *)(iVar48 + -0x30) = in_t1;
              *(ulonglong *)(iVar48 + -0x38) = in_t2;
              *(ulonglong *)(iVar48 + -0x40) = in_t3;
              *(ulonglong *)(iVar48 + -0x48) = uVar60;
              *(ulonglong *)(iVar48 + -0x50) = uVar64;
              *(ulonglong *)(iVar48 + -0x58) = uVar34;
              *(ulonglong *)(iVar48 + -0x60) = in_v1;
              *(ulonglong *)(iVar48 + -0x68) = unaff_s0;
              *(ulonglong *)(iVar48 + -0x70) = unaff_s1;
              *(ulonglong *)(iVar48 + -0x78) = unaff_s2;
              *puVar42 = uVar62;
            } while (uVar47 != uVar59);
          }
        }
        iVar48 = (int)uVar43;
        puVar36 = puVar42;
        if ((uVar46 & 0x40) != 0) {
          uVar34 = uVar43 - 8 & 7;
          uVar59 = uVar43 - 0x10 & 7;
          lVar18 = *(longlong *)((int)(uVar43 - 0x10) - (int)uVar59);
          uVar62 = uVar43 - 0x18 & 7;
          lVar19 = *(longlong *)((int)(uVar43 - 0x18) - (int)uVar62);
          uVar60 = uVar43 - 0x20 & 7;
          lVar20 = *(longlong *)((int)(uVar43 - 0x20) - (int)uVar60);
          uVar47 = uVar43 - 0x28 & 7;
          lVar21 = *(longlong *)((int)(uVar43 - 0x28) - (int)uVar47);
          uVar64 = uVar43 - 0x30 & 7;
          lVar22 = *(longlong *)((int)(uVar43 - 0x30) - (int)uVar64);
          uVar2 = uVar43 - 0x38 & 7;
          lVar23 = *(longlong *)((int)(uVar43 - 0x38) - (int)uVar2);
          uVar3 = uVar43 - 0x40 & 7;
          lVar24 = *(longlong *)((int)(uVar43 - 0x40) - (int)uVar3);
          uVar4 = uVar43 - 1 & 7;
          uVar5 = uVar43 - 9 & 7;
          uVar12 = *(ulonglong *)((int)(uVar43 - 9) - (int)uVar5);
          uVar6 = uVar43 - 0x11 & 7;
          uVar13 = *(ulonglong *)((int)(uVar43 - 0x11) - (int)uVar6);
          uVar7 = uVar43 - 0x19 & 7;
          uVar14 = *(ulonglong *)((int)(uVar43 - 0x19) - (int)uVar7);
          uVar8 = uVar43 - 0x21 & 7;
          uVar15 = *(ulonglong *)((int)(uVar43 - 0x21) - (int)uVar8);
          uVar9 = uVar43 - 0x29 & 7;
          uVar16 = *(ulonglong *)((int)(uVar43 - 0x29) - (int)uVar9);
          uVar10 = uVar43 - 0x31 & 7;
          uVar17 = *(ulonglong *)((int)(uVar43 - 0x31) - (int)uVar10);
          uVar11 = uVar43 - 0x39 & 7;
          uVar25 = *(ulonglong *)((int)(uVar43 - 0x39) - (int)uVar11);
          iVar48 = iVar48 + -0x40;
          puVar36 = puVar42 + -8;
          puVar42[-1] = (*(longlong *)((int)(uVar43 - 8) - (int)uVar34) << uVar34 * 8 |
                        in_t4 & 0xffffffffffffffffU >> (8 - uVar34) * 8) & -1L << (uVar4 + 1) * 8 |
                        *(ulonglong *)((int)(uVar43 - 1) - (int)uVar4) >> (7 - uVar4) * 8;
          puVar42[-2] = (lVar18 << uVar59 * 8 | in_t5 & 0xffffffffffffffffU >> (8 - uVar59) * 8) &
                        -1L << (uVar5 + 1) * 8 | uVar12 >> (7 - uVar5) * 8;
          puVar42[-3] = (lVar19 << uVar62 * 8 | in_t6 & 0xffffffffffffffffU >> (8 - uVar62) * 8) &
                        -1L << (uVar6 + 1) * 8 | uVar13 >> (7 - uVar6) * 8;
          puVar42[-4] = (lVar20 << uVar60 * 8 | in_t7 & 0xffffffffffffffffU >> (8 - uVar60) * 8) &
                        -1L << (uVar7 + 1) * 8 | uVar14 >> (7 - uVar7) * 8;
          puVar42[-5] = (lVar21 << uVar47 * 8 | in_t0 & 0xffffffffffffffffU >> (8 - uVar47) * 8) &
                        -1L << (uVar8 + 1) * 8 | uVar15 >> (7 - uVar8) * 8;
          puVar42[-6] = (lVar22 << uVar64 * 8 | in_t1 & 0xffffffffffffffffU >> (8 - uVar64) * 8) &
                        -1L << (uVar9 + 1) * 8 | uVar16 >> (7 - uVar9) * 8;
          puVar42[-7] = (lVar23 << uVar2 * 8 | in_t2 & 0xffffffffffffffffU >> (8 - uVar2) * 8) &
                        -1L << (uVar10 + 1) * 8 | uVar17 >> (7 - uVar10) * 8;
          *puVar36 = (lVar24 << uVar3 * 8 | in_t3 & 0xffffffffffffffffU >> (8 - uVar3) * 8) &
                     -1L << (uVar11 + 1) * 8 | uVar25 >> (7 - uVar11) * 8;
        }
        puVar42 = puVar36;
        if ((uVar46 & 0x20) != 0) {
          puVar45 = (ulonglong *)(iVar48 + -8);
          uVar43 = *(ulonglong *)(iVar48 + -0x10);
          uVar34 = *(ulonglong *)(iVar48 + -0x18);
          uVar59 = *(ulonglong *)(iVar48 + -0x20);
          iVar48 = iVar48 + -0x20;
          puVar42 = puVar36 + -4;
          puVar36[-1] = *puVar45;
          puVar36[-2] = uVar43;
          puVar36[-3] = uVar34;
          *puVar42 = uVar59;
        }
        puVar36 = puVar42;
        if ((uVar46 & 0x10) != 0) {
          puVar45 = (ulonglong *)(iVar48 + -8);
          uVar43 = *(ulonglong *)(iVar48 + -0x10);
          iVar48 = iVar48 + -0x10;
          puVar36 = puVar42 + -2;
          puVar42[-1] = *puVar45;
          *puVar36 = uVar43;
        }
        if ((uVar46 & 8) != 0) {
          puVar42 = (ulonglong *)(iVar48 + -8);
          iVar48 = iVar48 + -8;
          puVar36 = puVar36 + -1;
          *puVar36 = *puVar42;
        }
        iVar37 = (int)(uVar46 & 7);
        if ((uVar46 & 7) != 0) {
          uVar41 = (int)puVar36 - iVar37;
          uVar1 = uVar41 & 7;
          puVar42 = (ulonglong *)(uVar41 - uVar1);
          *puVar42 = *puVar42 & -1L << (8 - uVar1) * 8 |
                     *(ulonglong *)(iVar48 - iVar37) >> uVar1 * 8;
        }
        return;
      }
      while (iVar39 = (int)uVar46, 7 < uVar46) {
        uVar46 = *(ulonglong *)((int)uVar43 + -8);
        uVar34 = uVar59 - 8 & 7;
        puVar42 = (ulonglong *)((int)(uVar59 - 8) - (int)uVar34);
        *puVar42 = *puVar42 & -1L << (8 - uVar34) * 8 | uVar46 >> uVar34 * 8;
        iVar48 = (int)uVar43 + -8;
        uVar43 = (ulonglong)iVar48;
        iVar37 = (int)uVar59 + -8;
        uVar59 = (ulonglong)iVar37;
        uVar34 = uVar59 + 7 & 7;
        puVar42 = (ulonglong *)((int)(uVar59 + 7) - (int)uVar34);
        *puVar42 = uVar46 << (7 - uVar34) * 8 | *puVar42 & 0xffffffffffffffffU >> (uVar34 + 1) * 8;
        uVar46 = (ulonglong)(iVar39 + -8);
      }
      if ((((uVar46 != 0) &&
           (*(undefined1 *)(iVar37 + -1) = *(undefined1 *)(iVar48 + -1), iVar39 != 1)) &&
          (*(undefined1 *)(iVar37 + -2) = *(undefined1 *)(iVar48 + -2), iVar39 != 2)) &&
         (((*(undefined1 *)(iVar37 + -3) = *(undefined1 *)(iVar48 + -3), iVar39 != 3 &&
           (*(undefined1 *)(iVar37 + -4) = *(undefined1 *)(iVar48 + -4), iVar39 != 4)) &&
          ((*(undefined1 *)(iVar37 + -5) = *(undefined1 *)(iVar48 + -5), iVar39 != 5 &&
           (*(undefined1 *)(iVar37 + -6) = *(undefined1 *)(iVar48 + -6), iVar39 != 6)))))) {
        *(undefined1 *)(iVar37 + -7) = *(undefined1 *)(iVar48 + -7);
        return;
      }
    }
  }
  return;
}



// === memmove @ 0fb89a0c (3360 bytes) ===

/* WARNING: Instruction at (ram,0x0fb89b2c) overlaps instruction at (ram,0x0fb89b28)
    */

void * memmove(void *__dest,void *__src,size_t __n)

{
  uint uVar1;
  ulonglong uVar2;
  ulonglong uVar3;
  ulonglong uVar4;
  ulonglong uVar5;
  ulonglong uVar6;
  ulonglong uVar7;
  ulonglong uVar8;
  ulonglong uVar9;
  ulonglong uVar10;
  ulonglong uVar11;
  ulonglong uVar12;
  ulonglong uVar13;
  ulonglong uVar14;
  ulonglong uVar15;
  ulonglong uVar16;
  ulonglong uVar17;
  longlong lVar18;
  longlong lVar19;
  longlong lVar20;
  longlong lVar21;
  longlong lVar22;
  longlong lVar23;
  longlong lVar24;
  uint uVar26;
  ulonglong uVar25;
  ulonglong uVar27;
  ulonglong uVar28;
  uint uVar29;
  longlong *plVar30;
  undefined8 uVar31;
  void *pvVar33;
  undefined8 uVar32;
  ulonglong in_v1;
  undefined8 uVar34;
  ulonglong uVar35;
  undefined8 *puVar36;
  ulonglong *puVar37;
  int iVar38;
  undefined8 *puVar39;
  undefined8 *puVar41;
  uint uVar42;
  ulonglong *puVar43;
  undefined8 *extraout_a0_lo;
  ulonglong uVar44;
  undefined8 *puVar45;
  ulonglong *puVar46;
  undefined8 *extraout_a1_lo;
  ulonglong uVar47;
  ulonglong extraout_a2;
  int iVar49;
  ulonglong uVar48;
  ulonglong in_t0;
  undefined8 uVar50;
  ulonglong in_t1;
  undefined8 uVar51;
  ulonglong in_t2;
  undefined8 uVar52;
  ulonglong in_t3;
  undefined8 uVar53;
  ulonglong in_t4;
  ulonglong in_t5;
  undefined8 uVar54;
  ulonglong in_t6;
  undefined8 uVar55;
  ulonglong in_t7;
  undefined8 uVar56;
  ulonglong unaff_s0;
  undefined8 uVar57;
  ulonglong unaff_s1;
  undefined8 uVar58;
  ulonglong unaff_s2;
  undefined8 uVar59;
  ulonglong uVar60;
  ulonglong uVar61;
  undefined8 uVar62;
  ulonglong uVar63;
  undefined8 uVar64;
  ulonglong uVar65;
  int unaff_gp_lo;
  int iVar40;
  
  uVar47 = (ulonglong)(int)__n;
  uVar44 = (ulonglong)(int)__src;
  uVar35 = (ulonglong)(int)__dest;
  pvVar33 = __dest;
  if (uVar44 != uVar35) {
    if ((uVar35 < uVar44) || ((ulonglong)(longlong)(int)((int)__src + __n) <= uVar35)) {
      if (0x1f < uVar47) {
        uVar60 = uVar44 & 7;
        uVar63 = uVar35 & 7;
        if (uVar60 != uVar63) {
          uVar60 = uVar35;
          if (uVar63 != 0) {
            iVar49 = 8 - (int)uVar63;
            uVar44 = (ulonglong)((int)__src + iVar49);
            uVar47 = uVar35 & 7;
            puVar43 = (ulonglong *)((int)__dest - (int)uVar47);
            *puVar43 = *puVar43 & -1L << (8 - uVar47) * 8 | *(ulonglong *)__src >> uVar47 * 8;
            uVar60 = (ulonglong)((int)__dest + iVar49);
            uVar47 = (ulonglong)(int)(__n - iVar49);
          }
          puVar43 = (ulonglong *)uVar60;
          uVar63 = 0xffffffffffffff80;
          uVar61 = uVar47 & 0x40;
          if ((uVar47 & 0xffffffffffffff80) != 0) {
            uVar65 = (ulonglong)((int)uVar47 + -0x1000);
            uVar48 = (ulonglong)((int)(uVar47 & 0xffffffffffffff80) + (int)puVar43);
            if ((longlong)uVar65 < 0) {
              do {
                uVar35 = uVar44 & 7;
                iVar49 = (int)uVar44;
                uVar2 = uVar44 + 8 & 7;
                lVar18 = *(longlong *)((int)(uVar44 + 8) - (int)uVar2);
                uVar3 = uVar44 + 0x10 & 7;
                lVar19 = *(longlong *)((int)(uVar44 + 0x10) - (int)uVar3);
                uVar4 = uVar44 + 0x18 & 7;
                lVar20 = *(longlong *)((int)(uVar44 + 0x18) - (int)uVar4);
                uVar5 = uVar44 + 0x20 & 7;
                uVar6 = uVar44 + 0x28 & 7;
                uVar7 = uVar44 + 0x30 & 7;
                uVar8 = uVar44 + 0x38 & 7;
                uVar9 = uVar44 + 0x40 & 7;
                uVar10 = uVar44 + 0x48 & 7;
                uVar11 = uVar44 + 0x50 & 7;
                uVar12 = uVar44 + 0x58 & 7;
                uVar13 = uVar44 + 7 & 7;
                uVar14 = uVar44 + 0xf & 7;
                uVar25 = *(ulonglong *)((int)(uVar44 + 0xf) - (int)uVar14);
                uVar15 = uVar44 + 0x17 & 7;
                uVar27 = *(ulonglong *)((int)(uVar44 + 0x17) - (int)uVar15);
                uVar16 = uVar44 + 0x1f & 7;
                uVar28 = *(ulonglong *)((int)(uVar44 + 0x1f) - (int)uVar16);
                uVar17 = uVar44 + 0x27 & 7;
                in_t0 = (*(longlong *)((int)(uVar44 + 0x20) - (int)uVar5) << uVar5 * 8 |
                        in_t0 & 0xffffffffffffffffU >> (8 - uVar5) * 8) & -1L << (uVar17 + 1) * 8 |
                        *(ulonglong *)((int)(uVar44 + 0x27) - (int)uVar17) >> (7 - uVar17) * 8;
                uVar5 = uVar44 + 0x2f & 7;
                in_t1 = (*(longlong *)((int)(uVar44 + 0x28) - (int)uVar6) << uVar6 * 8 |
                        in_t1 & 0xffffffffffffffffU >> (8 - uVar6) * 8) & -1L << (uVar5 + 1) * 8 |
                        *(ulonglong *)((int)(uVar44 + 0x2f) - (int)uVar5) >> (7 - uVar5) * 8;
                uVar5 = uVar44 + 0x37 & 7;
                in_t2 = (*(longlong *)((int)(uVar44 + 0x30) - (int)uVar7) << uVar7 * 8 |
                        in_t2 & 0xffffffffffffffffU >> (8 - uVar7) * 8) & -1L << (uVar5 + 1) * 8 |
                        *(ulonglong *)((int)(uVar44 + 0x37) - (int)uVar5) >> (7 - uVar5) * 8;
                uVar5 = uVar44 + 0x3f & 7;
                in_t3 = (*(longlong *)((int)(uVar44 + 0x38) - (int)uVar8) << uVar8 * 8 |
                        in_t3 & 0xffffffffffffffffU >> (8 - uVar8) * 8) & -1L << (uVar5 + 1) * 8 |
                        *(ulonglong *)((int)(uVar44 + 0x3f) - (int)uVar5) >> (7 - uVar5) * 8;
                uVar5 = uVar44 + 0x47 & 7;
                uVar61 = (*(longlong *)((int)(uVar44 + 0x40) - (int)uVar9) << uVar9 * 8 |
                         uVar61 & 0xffffffffffffffffU >> (8 - uVar9) * 8) & -1L << (uVar5 + 1) * 8 |
                         *(ulonglong *)((int)(uVar44 + 0x47) - (int)uVar5) >> (7 - uVar5) * 8;
                uVar5 = uVar44 + 0x4f & 7;
                uVar65 = (*(longlong *)((int)(uVar44 + 0x48) - (int)uVar10) << uVar10 * 8 |
                         uVar65 & 0xffffffffffffffffU >> (8 - uVar10) * 8) & -1L << (uVar5 + 1) * 8
                         | *(ulonglong *)((int)(uVar44 + 0x4f) - (int)uVar5) >> (7 - uVar5) * 8;
                uVar5 = uVar44 + 0x57 & 7;
                in_v1 = (*(longlong *)((int)(uVar44 + 0x50) - (int)uVar11) << uVar11 * 8 |
                        in_v1 & 0xffffffffffffffffU >> (8 - uVar11) * 8) & -1L << (uVar5 + 1) * 8 |
                        *(ulonglong *)((int)(uVar44 + 0x57) - (int)uVar5) >> (7 - uVar5) * 8;
                uVar5 = uVar44 + 0x5f & 7;
                uVar63 = (*(longlong *)((int)(uVar44 + 0x58) - (int)uVar12) << uVar12 * 8 |
                         uVar63 & 0xffffffffffffffffU >> (8 - uVar12) * 8) & -1L << (uVar5 + 1) * 8
                         | *(ulonglong *)((int)(uVar44 + 0x5f) - (int)uVar5) >> (7 - uVar5) * 8;
                puVar37 = (ulonglong *)uVar60;
                *puVar37 = (*(longlong *)(iVar49 - (int)uVar35) << uVar35 * 8 |
                           in_t4 & 0xffffffffffffffffU >> (8 - uVar35) * 8) &
                           -1L << (uVar13 + 1) * 8 |
                           *(ulonglong *)((int)(uVar44 + 7) - (int)uVar13) >> (7 - uVar13) * 8;
                puVar37[1] = (lVar18 << uVar2 * 8 | in_t5 & 0xffffffffffffffffU >> (8 - uVar2) * 8)
                             & -1L << (uVar14 + 1) * 8 | uVar25 >> (7 - uVar14) * 8;
                puVar37[2] = (lVar19 << uVar3 * 8 | in_t6 & 0xffffffffffffffffU >> (8 - uVar3) * 8)
                             & -1L << (uVar15 + 1) * 8 | uVar27 >> (7 - uVar15) * 8;
                puVar37[3] = (lVar20 << uVar4 * 8 | in_t7 & 0xffffffffffffffffU >> (8 - uVar4) * 8)
                             & -1L << (uVar16 + 1) * 8 | uVar28 >> (7 - uVar16) * 8;
                puVar37[4] = in_t0;
                puVar37[5] = in_t1;
                puVar37[6] = in_t2;
                puVar37[7] = in_t3;
                puVar37[8] = uVar61;
                puVar37[9] = uVar65;
                puVar37[10] = in_v1;
                puVar37[0xb] = uVar63;
                in_t4 = *(ulonglong *)(iVar49 + 0x60);
                in_t5 = *(ulonglong *)(iVar49 + 0x68);
                in_t6 = *(ulonglong *)(iVar49 + 0x70);
                in_t7 = *(ulonglong *)(iVar49 + 0x78);
                puVar43 = puVar37 + 0x10;
                uVar60 = (ulonglong)(int)puVar43;
                uVar44 = (ulonglong)(iVar49 + 0x80);
                puVar37[0xc] = in_t4;
                puVar37[0xd] = in_t5;
                puVar37[0xe] = in_t6;
                puVar37[0xf] = in_t7;
              } while (uVar48 != uVar60);
            }
            else {
              do {
                uVar2 = uVar44 & 7;
                uVar3 = uVar44 + 8 & 7;
                uVar4 = uVar44 + 0x10 & 7;
                uVar5 = uVar44 + 0x18 & 7;
                uVar6 = uVar44 + 0x20 & 7;
                uVar7 = uVar44 + 0x28 & 7;
                uVar8 = uVar44 + 0x30 & 7;
                uVar9 = uVar44 + 0x38 & 7;
                uVar10 = uVar44 + 0x40 & 7;
                uVar11 = uVar44 + 0x48 & 7;
                uVar12 = uVar44 + 0x50 & 7;
                uVar13 = uVar44 + 0x58 & 7;
                uVar14 = uVar44 + 0x60 & 7;
                uVar15 = uVar44 + 0x68 & 7;
                uVar16 = uVar44 + 0x70 & 7;
                uVar17 = uVar44 + 0x78 & 7;
                uVar25 = uVar44 + 7 & 7;
                in_t4 = (*(longlong *)((int)uVar44 - (int)uVar2) << uVar2 * 8 |
                        in_t4 & 0xffffffffffffffffU >> (8 - uVar2) * 8) & -1L << (uVar25 + 1) * 8 |
                        *(ulonglong *)((int)(uVar44 + 7) - (int)uVar25) >> (7 - uVar25) * 8;
                uVar2 = uVar44 + 0xf & 7;
                in_t5 = (*(longlong *)((int)(uVar44 + 8) - (int)uVar3) << uVar3 * 8 |
                        in_t5 & 0xffffffffffffffffU >> (8 - uVar3) * 8) & -1L << (uVar2 + 1) * 8 |
                        *(ulonglong *)((int)(uVar44 + 0xf) - (int)uVar2) >> (7 - uVar2) * 8;
                uVar2 = uVar44 + 0x17 & 7;
                in_t6 = (*(longlong *)((int)(uVar44 + 0x10) - (int)uVar4) << uVar4 * 8 |
                        in_t6 & 0xffffffffffffffffU >> (8 - uVar4) * 8) & -1L << (uVar2 + 1) * 8 |
                        *(ulonglong *)((int)(uVar44 + 0x17) - (int)uVar2) >> (7 - uVar2) * 8;
                uVar2 = uVar44 + 0x1f & 7;
                in_t7 = (*(longlong *)((int)(uVar44 + 0x18) - (int)uVar5) << uVar5 * 8 |
                        in_t7 & 0xffffffffffffffffU >> (8 - uVar5) * 8) & -1L << (uVar2 + 1) * 8 |
                        *(ulonglong *)((int)(uVar44 + 0x1f) - (int)uVar2) >> (7 - uVar2) * 8;
                uVar2 = uVar44 + 0x27 & 7;
                in_t0 = (*(longlong *)((int)(uVar44 + 0x20) - (int)uVar6) << uVar6 * 8 |
                        in_t0 & 0xffffffffffffffffU >> (8 - uVar6) * 8) & -1L << (uVar2 + 1) * 8 |
                        *(ulonglong *)((int)(uVar44 + 0x27) - (int)uVar2) >> (7 - uVar2) * 8;
                uVar2 = uVar44 + 0x2f & 7;
                in_t1 = (*(longlong *)((int)(uVar44 + 0x28) - (int)uVar7) << uVar7 * 8 |
                        in_t1 & 0xffffffffffffffffU >> (8 - uVar7) * 8) & -1L << (uVar2 + 1) * 8 |
                        *(ulonglong *)((int)(uVar44 + 0x2f) - (int)uVar2) >> (7 - uVar2) * 8;
                uVar2 = uVar44 + 0x37 & 7;
                in_t2 = (*(longlong *)((int)(uVar44 + 0x30) - (int)uVar8) << uVar8 * 8 |
                        in_t2 & 0xffffffffffffffffU >> (8 - uVar8) * 8) & -1L << (uVar2 + 1) * 8 |
                        *(ulonglong *)((int)(uVar44 + 0x37) - (int)uVar2) >> (7 - uVar2) * 8;
                uVar2 = uVar44 + 0x3f & 7;
                in_t3 = (*(longlong *)((int)(uVar44 + 0x38) - (int)uVar9) << uVar9 * 8 |
                        in_t3 & 0xffffffffffffffffU >> (8 - uVar9) * 8) & -1L << (uVar2 + 1) * 8 |
                        *(ulonglong *)((int)(uVar44 + 0x3f) - (int)uVar2) >> (7 - uVar2) * 8;
                uVar2 = uVar44 + 0x47 & 7;
                uVar61 = (*(longlong *)((int)(uVar44 + 0x40) - (int)uVar10) << uVar10 * 8 |
                         uVar61 & 0xffffffffffffffffU >> (8 - uVar10) * 8) & -1L << (uVar2 + 1) * 8
                         | *(ulonglong *)((int)(uVar44 + 0x47) - (int)uVar2) >> (7 - uVar2) * 8;
                uVar2 = uVar44 + 0x4f & 7;
                uVar65 = (*(longlong *)((int)(uVar44 + 0x48) - (int)uVar11) << uVar11 * 8 |
                         uVar65 & 0xffffffffffffffffU >> (8 - uVar11) * 8) & -1L << (uVar2 + 1) * 8
                         | *(ulonglong *)((int)(uVar44 + 0x4f) - (int)uVar2) >> (7 - uVar2) * 8;
                uVar2 = uVar44 + 0x57 & 7;
                uVar35 = (*(longlong *)((int)(uVar44 + 0x50) - (int)uVar12) << uVar12 * 8 |
                         uVar35 & 0xffffffffffffffffU >> (8 - uVar12) * 8) & -1L << (uVar2 + 1) * 8
                         | *(ulonglong *)((int)(uVar44 + 0x57) - (int)uVar2) >> (7 - uVar2) * 8;
                uVar2 = uVar44 + 0x5f & 7;
                in_v1 = (*(longlong *)((int)(uVar44 + 0x58) - (int)uVar13) << uVar13 * 8 |
                        in_v1 & 0xffffffffffffffffU >> (8 - uVar13) * 8) & -1L << (uVar2 + 1) * 8 |
                        *(ulonglong *)((int)(uVar44 + 0x5f) - (int)uVar2) >> (7 - uVar2) * 8;
                uVar2 = uVar44 + 0x67 & 7;
                unaff_s0 = (*(longlong *)((int)(uVar44 + 0x60) - (int)uVar14) << uVar14 * 8 |
                           unaff_s0 & 0xffffffffffffffffU >> (8 - uVar14) * 8) &
                           -1L << (uVar2 + 1) * 8 |
                           *(ulonglong *)((int)(uVar44 + 0x67) - (int)uVar2) >> (7 - uVar2) * 8;
                uVar2 = uVar44 + 0x6f & 7;
                unaff_s1 = (*(longlong *)((int)(uVar44 + 0x68) - (int)uVar15) << uVar15 * 8 |
                           unaff_s1 & 0xffffffffffffffffU >> (8 - uVar15) * 8) &
                           -1L << (uVar2 + 1) * 8 |
                           *(ulonglong *)((int)(uVar44 + 0x6f) - (int)uVar2) >> (7 - uVar2) * 8;
                uVar2 = uVar44 + 0x77 & 7;
                unaff_s2 = (*(longlong *)((int)(uVar44 + 0x70) - (int)uVar16) << uVar16 * 8 |
                           unaff_s2 & 0xffffffffffffffffU >> (8 - uVar16) * 8) &
                           -1L << (uVar2 + 1) * 8 |
                           *(ulonglong *)((int)(uVar44 + 0x77) - (int)uVar2) >> (7 - uVar2) * 8;
                uVar2 = uVar44 + 0x7f & 7;
                uVar63 = (*(longlong *)((int)(uVar44 + 0x78) - (int)uVar17) << uVar17 * 8 |
                         uVar63 & 0xffffffffffffffffU >> (8 - uVar17) * 8) & -1L << (uVar2 + 1) * 8
                         | *(ulonglong *)((int)(uVar44 + 0x7f) - (int)uVar2) >> (7 - uVar2) * 8;
                puVar37 = (ulonglong *)uVar60;
                puVar43 = puVar37 + 0x10;
                uVar60 = (ulonglong)(int)puVar43;
                uVar44 = (ulonglong)((int)uVar44 + 0x80);
                *puVar37 = in_t4;
                puVar37[1] = in_t5;
                puVar37[2] = in_t6;
                puVar37[3] = in_t7;
                puVar37[4] = in_t0;
                puVar37[5] = in_t1;
                puVar37[6] = in_t2;
                puVar37[7] = in_t3;
                puVar37[8] = uVar61;
                puVar37[9] = uVar65;
                puVar37[10] = uVar35;
                puVar37[0xb] = in_v1;
                puVar37[0xc] = unaff_s0;
                puVar37[0xd] = unaff_s1;
                puVar37[0xe] = unaff_s2;
                puVar37[0xf] = uVar63;
              } while (uVar48 != uVar60);
            }
          }
          puVar46 = (ulonglong *)uVar44;
          puVar37 = puVar43;
          if ((uVar47 & 0x40) != 0) {
            uVar35 = uVar44 & 7;
            plVar30 = (longlong *)((int)puVar46 - (int)uVar35);
            uVar60 = uVar44 + 8 & 7;
            lVar18 = *(longlong *)((int)(uVar44 + 8) - (int)uVar60);
            uVar63 = uVar44 + 0x10 & 7;
            lVar19 = *(longlong *)((int)(uVar44 + 0x10) - (int)uVar63);
            uVar61 = uVar44 + 0x18 & 7;
            lVar20 = *(longlong *)((int)(uVar44 + 0x18) - (int)uVar61);
            uVar48 = uVar44 + 0x20 & 7;
            lVar21 = *(longlong *)((int)(uVar44 + 0x20) - (int)uVar48);
            uVar65 = uVar44 + 0x28 & 7;
            lVar22 = *(longlong *)((int)(uVar44 + 0x28) - (int)uVar65);
            uVar2 = uVar44 + 0x30 & 7;
            lVar23 = *(longlong *)((int)(uVar44 + 0x30) - (int)uVar2);
            uVar3 = uVar44 + 0x38 & 7;
            lVar24 = *(longlong *)((int)(uVar44 + 0x38) - (int)uVar3);
            uVar4 = uVar44 + 7 & 7;
            uVar5 = uVar44 + 0xf & 7;
            uVar12 = *(ulonglong *)((int)(uVar44 + 0xf) - (int)uVar5);
            uVar6 = uVar44 + 0x17 & 7;
            uVar13 = *(ulonglong *)((int)(uVar44 + 0x17) - (int)uVar6);
            uVar7 = uVar44 + 0x1f & 7;
            uVar14 = *(ulonglong *)((int)(uVar44 + 0x1f) - (int)uVar7);
            uVar8 = uVar44 + 0x27 & 7;
            uVar15 = *(ulonglong *)((int)(uVar44 + 0x27) - (int)uVar8);
            uVar9 = uVar44 + 0x2f & 7;
            uVar16 = *(ulonglong *)((int)(uVar44 + 0x2f) - (int)uVar9);
            uVar10 = uVar44 + 0x37 & 7;
            uVar17 = *(ulonglong *)((int)(uVar44 + 0x37) - (int)uVar10);
            uVar11 = uVar44 + 0x3f & 7;
            uVar25 = *(ulonglong *)((int)(uVar44 + 0x3f) - (int)uVar11);
            puVar46 = puVar46 + 8;
            puVar37 = puVar43 + 8;
            *puVar43 = (*plVar30 << uVar35 * 8 | in_t4 & 0xffffffffffffffffU >> (8 - uVar35) * 8) &
                       -1L << (uVar4 + 1) * 8 |
                       *(ulonglong *)((int)(uVar44 + 7) - (int)uVar4) >> (7 - uVar4) * 8;
            puVar43[1] = (lVar18 << uVar60 * 8 | in_t5 & 0xffffffffffffffffU >> (8 - uVar60) * 8) &
                         -1L << (uVar5 + 1) * 8 | uVar12 >> (7 - uVar5) * 8;
            puVar43[2] = (lVar19 << uVar63 * 8 | in_t6 & 0xffffffffffffffffU >> (8 - uVar63) * 8) &
                         -1L << (uVar6 + 1) * 8 | uVar13 >> (7 - uVar6) * 8;
            puVar43[3] = (lVar20 << uVar61 * 8 | in_t7 & 0xffffffffffffffffU >> (8 - uVar61) * 8) &
                         -1L << (uVar7 + 1) * 8 | uVar14 >> (7 - uVar7) * 8;
            puVar43[4] = (lVar21 << uVar48 * 8 | in_t0 & 0xffffffffffffffffU >> (8 - uVar48) * 8) &
                         -1L << (uVar8 + 1) * 8 | uVar15 >> (7 - uVar8) * 8;
            puVar43[5] = (lVar22 << uVar65 * 8 | in_t1 & 0xffffffffffffffffU >> (8 - uVar65) * 8) &
                         -1L << (uVar9 + 1) * 8 | uVar16 >> (7 - uVar9) * 8;
            puVar43[6] = (lVar23 << uVar2 * 8 | in_t2 & 0xffffffffffffffffU >> (8 - uVar2) * 8) &
                         -1L << (uVar10 + 1) * 8 | uVar17 >> (7 - uVar10) * 8;
            puVar43[7] = (lVar24 << uVar3 * 8 | in_t3 & 0xffffffffffffffffU >> (8 - uVar3) * 8) &
                         -1L << (uVar11 + 1) * 8 | uVar25 >> (7 - uVar11) * 8;
          }
          puVar43 = puVar37;
          if ((uVar47 & 0x20) != 0) {
            uVar44 = *puVar46;
            uVar35 = puVar46[1];
            uVar60 = puVar46[2];
            uVar63 = puVar46[3];
            puVar46 = puVar46 + 4;
            puVar43 = puVar37 + 4;
            *puVar37 = uVar44;
            puVar37[1] = uVar35;
            puVar37[2] = uVar60;
            puVar37[3] = uVar63;
          }
          puVar37 = puVar43;
          if ((uVar47 & 0x10) != 0) {
            uVar44 = *puVar46;
            uVar35 = puVar46[1];
            puVar46 = puVar46 + 2;
            puVar37 = puVar43 + 2;
            *puVar43 = uVar44;
            puVar43[1] = uVar35;
          }
          puVar43 = puVar37;
          if ((uVar47 & 8) != 0) {
            uVar44 = *puVar46;
            puVar46 = puVar46 + 1;
            puVar43 = puVar37 + 1;
            *puVar37 = uVar44;
          }
          iVar49 = (int)(uVar47 & 7);
          if ((uVar47 & 7) != 0) {
            uVar1 = (int)puVar43 + iVar49 + -1;
            uVar42 = uVar1 & 7;
            puVar43 = (ulonglong *)(uVar1 - uVar42);
            *puVar43 = *(longlong *)((int)puVar46 + iVar49 + -8) << (7 - uVar42) * 8 |
                       *puVar43 & 0xffffffffffffffffU >> (uVar42 + 1) * 8;
          }
          return __dest;
        }
        uVar61 = 0;
        if (uVar60 != 0) {
          uVar44 = uVar44 & 7;
          uVar61 = *(longlong *)((int)__src - (int)uVar44) << uVar44 * 8 |
                   uVar60 & 0xffffffffffffffffU >> (8 - uVar44) * 8;
          iVar49 = 8 - (int)uVar63;
          uVar44 = (ulonglong)((int)__src + iVar49);
          uVar35 = uVar35 & 7;
          puVar43 = (ulonglong *)((int)__dest - (int)uVar35);
          *puVar43 = *puVar43 & -1L << (8 - uVar35) * 8 | uVar61 >> uVar35 * 8;
          uVar35 = (ulonglong)((int)__dest + iVar49);
          uVar47 = (ulonglong)(int)(__n - iVar49);
        }
        puVar39 = (undefined8 *)uVar44;
        if (((uVar47 < 0x100) || (*(int *)(unaff_gp_lo + 0x3358) == 0)) ||
           ((*(int *)(unaff_gp_lo + 0x3358) < 1 &&
            (_blk_init(), *(int *)(unaff_gp_lo + 0x3358) == 0)))) {
          puVar41 = (undefined8 *)uVar35;
          if ((uVar47 & 0xffffffffffffff80) != 0) {
            uVar60 = (ulonglong)((int)(uVar47 & 0xffffffffffffff80) + (int)puVar41);
            if ((int)uVar47 + -0x1000 < 0) {
              do {
                puVar45 = (undefined8 *)uVar44;
                uVar53 = puVar45[1];
                uVar54 = puVar45[2];
                uVar55 = puVar45[3];
                uVar34 = puVar45[4];
                uVar50 = puVar45[5];
                uVar51 = puVar45[6];
                uVar52 = puVar45[7];
                uVar56 = puVar45[8];
                uVar57 = puVar45[9];
                uVar32 = puVar45[10];
                uVar31 = puVar45[0xb];
                puVar36 = (undefined8 *)uVar35;
                *puVar36 = *puVar45;
                puVar36[1] = uVar53;
                puVar36[2] = uVar54;
                puVar36[3] = uVar55;
                puVar36[4] = uVar34;
                puVar36[5] = uVar50;
                puVar36[6] = uVar51;
                puVar36[7] = uVar52;
                puVar36[8] = uVar56;
                puVar36[9] = uVar57;
                puVar36[10] = uVar32;
                puVar36[0xb] = uVar31;
                uVar34 = puVar45[0xe];
                uVar32 = puVar45[0xd];
                uVar31 = puVar45[0xc];
                puVar41 = puVar36 + 0x10;
                uVar35 = (ulonglong)(int)puVar41;
                puVar39 = puVar45 + 0x10;
                uVar44 = (ulonglong)(int)puVar39;
                puVar36[0xf] = puVar45[0xf];
                puVar36[0xe] = uVar34;
                puVar36[0xd] = uVar32;
                puVar36[0xc] = uVar31;
              } while (uVar60 != uVar35);
            }
            else {
              do {
                puVar45 = (undefined8 *)uVar44;
                uVar54 = puVar45[1];
                uVar55 = puVar45[2];
                uVar56 = puVar45[3];
                uVar50 = puVar45[4];
                uVar51 = puVar45[5];
                uVar52 = puVar45[6];
                uVar53 = puVar45[7];
                uVar62 = puVar45[8];
                uVar64 = puVar45[9];
                uVar32 = puVar45[10];
                uVar34 = puVar45[0xb];
                uVar57 = puVar45[0xc];
                uVar58 = puVar45[0xd];
                uVar59 = puVar45[0xe];
                uVar31 = puVar45[0xf];
                puVar36 = (undefined8 *)uVar35;
                puVar41 = puVar36 + 0x10;
                uVar35 = (ulonglong)(int)puVar41;
                puVar39 = puVar45 + 0x10;
                uVar44 = (ulonglong)(int)puVar39;
                *puVar36 = *puVar45;
                puVar36[1] = uVar54;
                puVar36[2] = uVar55;
                puVar36[3] = uVar56;
                puVar36[4] = uVar50;
                puVar36[5] = uVar51;
                puVar36[6] = uVar52;
                puVar36[7] = uVar53;
                puVar36[8] = uVar62;
                puVar36[9] = uVar64;
                puVar36[10] = uVar32;
                puVar36[0xb] = uVar34;
                puVar36[0xc] = uVar57;
                puVar36[0xd] = uVar58;
                puVar36[0xe] = uVar59;
                puVar36[0xf] = uVar31;
              } while (uVar60 != uVar35);
            }
          }
          puVar36 = puVar41;
          if ((uVar47 & 0x40) != 0) {
            uVar51 = *puVar39;
            uVar52 = puVar39[1];
            uVar53 = puVar39[2];
            uVar54 = puVar39[3];
            uVar31 = puVar39[4];
            uVar32 = puVar39[5];
            uVar34 = puVar39[6];
            uVar50 = puVar39[7];
            puVar39 = puVar39 + 8;
            puVar36 = puVar41 + 8;
            *puVar41 = uVar51;
            puVar41[1] = uVar52;
            puVar41[2] = uVar53;
            puVar41[3] = uVar54;
            puVar41[4] = uVar31;
            puVar41[5] = uVar32;
            puVar41[6] = uVar34;
            puVar41[7] = uVar50;
          }
          uVar61 = uVar47 & 0x10;
          puVar45 = puVar36;
          if ((uVar47 & 0x20) != 0) {
            uVar31 = *puVar39;
            uVar32 = puVar39[1];
            uVar34 = puVar39[2];
            uVar50 = puVar39[3];
            puVar39 = puVar39 + 4;
            puVar45 = puVar36 + 4;
            *puVar36 = uVar31;
            puVar36[1] = uVar32;
            puVar36[2] = uVar34;
            puVar36[3] = uVar50;
          }
          uVar44 = uVar47 & 8;
          puVar41 = puVar45;
          if (uVar61 != 0) {
            uVar31 = *puVar39;
            uVar32 = puVar39[1];
            puVar39 = puVar39 + 2;
            puVar41 = puVar45 + 2;
            *puVar45 = uVar31;
            puVar45[1] = uVar32;
          }
        }
        else {
          __dest = (void *)_memcpy_fp();
          uVar44 = extraout_a2 & 8;
          uVar47 = extraout_a2;
          puVar41 = extraout_a0_lo;
          puVar39 = extraout_a1_lo;
          if (extraout_a2 == 0) {
            return __dest;
          }
        }
        puVar36 = puVar41;
        if (uVar44 != 0) {
          uVar31 = *puVar39;
          puVar39 = puVar39 + 1;
          puVar36 = puVar41 + 1;
          *puVar41 = uVar31;
        }
        iVar49 = (int)(uVar47 & 7);
        if ((uVar47 & 7) != 0) {
          uVar1 = (int)puVar39 + iVar49 + -1;
          uVar26 = uVar1 & 7;
          uVar42 = (int)puVar36 + iVar49 + -1;
          uVar29 = uVar42 & 7;
          puVar43 = (ulonglong *)(uVar42 - uVar29);
          *puVar43 = (uVar61 & -1L << (uVar26 + 1) * 8 |
                     *(ulonglong *)(uVar1 - uVar26) >> (7 - uVar26) * 8) << (7 - uVar29) * 8 |
                     *puVar43 & 0xffffffffffffffffU >> (uVar29 + 1) * 8;
        }
        return __dest;
      }
      while (iVar49 = (int)uVar47, 7 < uVar47) {
        uVar60 = *(ulonglong *)uVar44;
        uVar47 = uVar35 & 7;
        puVar43 = (ulonglong *)((int)uVar35 - (int)uVar47);
        *puVar43 = *puVar43 & -1L << (8 - uVar47) * 8 | uVar60 >> uVar47 * 8;
        __src = (ulonglong *)uVar44 + 1;
        uVar44 = (ulonglong)(int)__src;
        __dest = (void *)((int)uVar35 + 8);
        uVar35 = (ulonglong)(int)__dest;
        uVar47 = uVar35 - 1 & 7;
        puVar43 = (ulonglong *)((int)(uVar35 - 1) - (int)uVar47);
        *puVar43 = uVar60 << (7 - uVar47) * 8 | *puVar43 & 0xffffffffffffffffU >> (uVar47 + 1) * 8;
        uVar47 = (ulonglong)(iVar49 + -8);
      }
      if (((uVar47 != 0) && (*(undefined1 *)__dest = *(undefined1 *)__src, iVar49 != 1)) &&
         ((*(undefined1 *)((int)__dest + 1) = *(undefined1 *)((int)__src + 1), iVar49 != 2 &&
          ((((*(undefined1 *)((int)__dest + 2) = *(undefined1 *)((int)__src + 2), iVar49 != 3 &&
             (*(undefined1 *)((int)__dest + 3) = *(undefined1 *)((int)__src + 3), iVar49 != 4)) &&
            (*(undefined1 *)((int)__dest + 4) = *(undefined1 *)((int)__src + 4), iVar49 != 5)) &&
           (*(undefined1 *)((int)__dest + 5) = *(undefined1 *)((int)__src + 5), iVar49 != 6)))))) {
        *(undefined1 *)((int)__dest + 6) = *(undefined1 *)((int)__src + 6);
      }
    }
    else {
      iVar49 = (int)__src + __n;
      uVar44 = (ulonglong)iVar49;
      iVar38 = (int)__dest + __n;
      uVar60 = (ulonglong)iVar38;
      if (0x1f < uVar47) {
        uVar63 = uVar44 & 7;
        uVar61 = uVar60 & 7;
        iVar40 = (int)uVar61;
        if (uVar63 == uVar61) {
          if (uVar63 != 0) {
            uVar35 = uVar44 - 1;
            uVar61 = uVar35 & 7;
            uVar44 = (ulonglong)(iVar49 - iVar40);
            uVar47 = (ulonglong)(int)(__n - iVar40);
            uVar48 = uVar60 - 1 & 7;
            puVar43 = (ulonglong *)((int)(uVar60 - 1) - (int)uVar48);
            *puVar43 = (uVar63 & -1L << (uVar61 + 1) * 8 |
                       *(ulonglong *)((int)uVar35 - (int)uVar61) >> (7 - uVar61) * 8) <<
                       (7 - uVar48) * 8 | *puVar43 & 0xffffffffffffffffU >> (uVar48 + 1) * 8;
            uVar60 = (ulonglong)(iVar38 - iVar40);
          }
          iVar49 = (int)uVar44;
          puVar39 = (undefined8 *)uVar60;
          if ((uVar47 & 0xffffffffffffff80) != 0) {
            uVar35 = (ulonglong)((int)puVar39 - (int)(uVar47 & 0xffffffffffffff80));
            if ((int)uVar47 + -0x1000 < 0) {
              do {
                iVar38 = (int)uVar44;
                uVar53 = *(undefined8 *)(iVar38 + -0x10);
                uVar54 = *(undefined8 *)(iVar38 + -0x18);
                uVar55 = *(undefined8 *)(iVar38 + -0x20);
                uVar34 = *(undefined8 *)(iVar38 + -0x28);
                uVar50 = *(undefined8 *)(iVar38 + -0x30);
                uVar51 = *(undefined8 *)(iVar38 + -0x38);
                uVar52 = *(undefined8 *)(iVar38 + -0x40);
                uVar56 = *(undefined8 *)(iVar38 + -0x48);
                uVar57 = *(undefined8 *)(iVar38 + -0x50);
                uVar32 = *(undefined8 *)(iVar38 + -0x58);
                uVar31 = *(undefined8 *)(iVar38 + -0x60);
                iVar40 = (int)uVar60;
                *(undefined8 *)(iVar40 + -8) = *(undefined8 *)(iVar38 + -8);
                *(undefined8 *)(iVar40 + -0x10) = uVar53;
                *(undefined8 *)(iVar40 + -0x18) = uVar54;
                *(undefined8 *)(iVar40 + -0x20) = uVar55;
                *(undefined8 *)(iVar40 + -0x28) = uVar34;
                *(undefined8 *)(iVar40 + -0x30) = uVar50;
                *(undefined8 *)(iVar40 + -0x38) = uVar51;
                *(undefined8 *)(iVar40 + -0x40) = uVar52;
                *(undefined8 *)(iVar40 + -0x48) = uVar56;
                *(undefined8 *)(iVar40 + -0x50) = uVar57;
                *(undefined8 *)(iVar40 + -0x58) = uVar32;
                *(undefined8 *)(iVar40 + -0x60) = uVar31;
                uVar34 = *(undefined8 *)(iVar38 + -0x78);
                uVar32 = *(undefined8 *)(iVar38 + -0x70);
                uVar31 = *(undefined8 *)(iVar38 + -0x68);
                puVar39 = (undefined8 *)(iVar40 + -0x80);
                uVar60 = (ulonglong)(int)puVar39;
                iVar49 = iVar38 + -0x80;
                uVar44 = (ulonglong)iVar49;
                *puVar39 = *(undefined8 *)(iVar38 + -0x80);
                *(undefined8 *)(iVar40 + -0x78) = uVar34;
                *(undefined8 *)(iVar40 + -0x70) = uVar32;
                *(undefined8 *)(iVar40 + -0x68) = uVar31;
              } while (uVar35 != uVar60);
            }
            else {
              do {
                iVar38 = (int)uVar44;
                uVar54 = *(undefined8 *)(iVar38 + -0x10);
                uVar55 = *(undefined8 *)(iVar38 + -0x18);
                uVar56 = *(undefined8 *)(iVar38 + -0x20);
                uVar50 = *(undefined8 *)(iVar38 + -0x28);
                uVar51 = *(undefined8 *)(iVar38 + -0x30);
                uVar52 = *(undefined8 *)(iVar38 + -0x38);
                uVar53 = *(undefined8 *)(iVar38 + -0x40);
                uVar62 = *(undefined8 *)(iVar38 + -0x48);
                uVar64 = *(undefined8 *)(iVar38 + -0x50);
                uVar32 = *(undefined8 *)(iVar38 + -0x58);
                uVar34 = *(undefined8 *)(iVar38 + -0x60);
                uVar57 = *(undefined8 *)(iVar38 + -0x68);
                uVar58 = *(undefined8 *)(iVar38 + -0x70);
                uVar59 = *(undefined8 *)(iVar38 + -0x78);
                uVar31 = *(undefined8 *)(iVar38 + -0x80);
                iVar40 = (int)uVar60;
                puVar39 = (undefined8 *)(iVar40 + -0x80);
                uVar60 = (ulonglong)(int)puVar39;
                iVar49 = iVar38 + -0x80;
                uVar44 = (ulonglong)iVar49;
                *(undefined8 *)(iVar40 + -8) = *(undefined8 *)(iVar38 + -8);
                *(undefined8 *)(iVar40 + -0x10) = uVar54;
                *(undefined8 *)(iVar40 + -0x18) = uVar55;
                *(undefined8 *)(iVar40 + -0x20) = uVar56;
                *(undefined8 *)(iVar40 + -0x28) = uVar50;
                *(undefined8 *)(iVar40 + -0x30) = uVar51;
                *(undefined8 *)(iVar40 + -0x38) = uVar52;
                *(undefined8 *)(iVar40 + -0x40) = uVar53;
                *(undefined8 *)(iVar40 + -0x48) = uVar62;
                *(undefined8 *)(iVar40 + -0x50) = uVar64;
                *(undefined8 *)(iVar40 + -0x58) = uVar32;
                *(undefined8 *)(iVar40 + -0x60) = uVar34;
                *(undefined8 *)(iVar40 + -0x68) = uVar57;
                *(undefined8 *)(iVar40 + -0x70) = uVar58;
                *(undefined8 *)(iVar40 + -0x78) = uVar59;
                *puVar39 = uVar31;
              } while (uVar35 != uVar60);
            }
          }
          puVar41 = puVar39;
          if ((uVar47 & 0x40) != 0) {
            puVar36 = (undefined8 *)(iVar49 + -8);
            uVar51 = *(undefined8 *)(iVar49 + -0x10);
            uVar52 = *(undefined8 *)(iVar49 + -0x18);
            uVar53 = *(undefined8 *)(iVar49 + -0x20);
            uVar31 = *(undefined8 *)(iVar49 + -0x28);
            uVar32 = *(undefined8 *)(iVar49 + -0x30);
            uVar34 = *(undefined8 *)(iVar49 + -0x38);
            uVar50 = *(undefined8 *)(iVar49 + -0x40);
            iVar49 = iVar49 + -0x40;
            puVar41 = puVar39 + -8;
            puVar39[-1] = *puVar36;
            puVar39[-2] = uVar51;
            puVar39[-3] = uVar52;
            puVar39[-4] = uVar53;
            puVar39[-5] = uVar31;
            puVar39[-6] = uVar32;
            puVar39[-7] = uVar34;
            *puVar41 = uVar50;
          }
          puVar39 = puVar41;
          if ((uVar47 & 0x20) != 0) {
            puVar36 = (undefined8 *)(iVar49 + -8);
            uVar31 = *(undefined8 *)(iVar49 + -0x10);
            uVar32 = *(undefined8 *)(iVar49 + -0x18);
            uVar34 = *(undefined8 *)(iVar49 + -0x20);
            iVar49 = iVar49 + -0x20;
            puVar39 = puVar41 + -4;
            puVar41[-1] = *puVar36;
            puVar41[-2] = uVar31;
            puVar41[-3] = uVar32;
            *puVar39 = uVar34;
          }
          puVar41 = puVar39;
          if ((uVar47 & 0x10) != 0) {
            puVar36 = (undefined8 *)(iVar49 + -8);
            uVar31 = *(undefined8 *)(iVar49 + -0x10);
            iVar49 = iVar49 + -0x10;
            puVar41 = puVar39 + -2;
            puVar39[-1] = *puVar36;
            *puVar41 = uVar31;
          }
          if ((uVar47 & 8) != 0) {
            puVar39 = (undefined8 *)(iVar49 + -8);
            iVar49 = iVar49 + -8;
            puVar41 = puVar41 + -1;
            *puVar41 = *puVar39;
          }
          iVar38 = (int)(uVar47 & 7);
          iVar49 = iVar49 - iVar38;
          if ((uVar47 & 7) != 0) {
            uVar44 = (longlong)iVar49 & 7;
            uVar42 = (int)puVar41 - iVar38;
            uVar1 = uVar42 & 7;
            puVar43 = (ulonglong *)(uVar42 - uVar1);
            *puVar43 = *puVar43 & -1L << (8 - uVar1) * 8 |
                       (*(longlong *)(iVar49 - (int)uVar44) << uVar44 * 8 |
                       uVar47 & 0x10 & 0xffffffffffffffffU >> (8 - uVar44) * 8) >> uVar1 * 8;
          }
          return __dest;
        }
        if (uVar61 != 0) {
          uVar44 = (ulonglong)(iVar49 - iVar40);
          uVar47 = (ulonglong)(int)(__n - iVar40);
          uVar63 = uVar60 - 1 & 7;
          puVar43 = (ulonglong *)((int)(uVar60 - 1) - (int)uVar63);
          *puVar43 = *(longlong *)(iVar49 + -8) << (7 - uVar63) * 8 |
                     *puVar43 & 0xffffffffffffffffU >> (uVar63 + 1) * 8;
          uVar60 = (ulonglong)(iVar38 - iVar40);
        }
        puVar43 = (ulonglong *)uVar60;
        uVar63 = 0xffffffffffffff80;
        uVar61 = uVar47 & 0x40;
        if ((uVar47 & 0xffffffffffffff80) != 0) {
          uVar65 = (ulonglong)((int)uVar47 + -0x1000);
          uVar48 = (ulonglong)((int)puVar43 - (int)(uVar47 & 0xffffffffffffff80));
          if ((longlong)uVar65 < 0) {
            do {
              uVar35 = uVar44 - 8 & 7;
              uVar2 = uVar44 - 0x10 & 7;
              lVar18 = *(longlong *)((int)(uVar44 - 0x10) - (int)uVar2);
              uVar3 = uVar44 - 0x18 & 7;
              lVar19 = *(longlong *)((int)(uVar44 - 0x18) - (int)uVar3);
              uVar4 = uVar44 - 0x20 & 7;
              lVar20 = *(longlong *)((int)(uVar44 - 0x20) - (int)uVar4);
              uVar5 = uVar44 - 0x28 & 7;
              uVar6 = uVar44 - 0x30 & 7;
              uVar7 = uVar44 - 0x38 & 7;
              uVar8 = uVar44 - 0x40 & 7;
              uVar9 = uVar44 - 0x48 & 7;
              uVar10 = uVar44 - 0x50 & 7;
              uVar11 = uVar44 - 0x58 & 7;
              uVar12 = uVar44 - 0x60 & 7;
              uVar13 = uVar44 - 1 & 7;
              uVar14 = uVar44 - 9 & 7;
              uVar25 = *(ulonglong *)((int)(uVar44 - 9) - (int)uVar14);
              uVar15 = uVar44 - 0x11 & 7;
              uVar27 = *(ulonglong *)((int)(uVar44 - 0x11) - (int)uVar15);
              uVar16 = uVar44 - 0x19 & 7;
              uVar28 = *(ulonglong *)((int)(uVar44 - 0x19) - (int)uVar16);
              uVar17 = uVar44 - 0x21 & 7;
              in_t0 = (*(longlong *)((int)(uVar44 - 0x28) - (int)uVar5) << uVar5 * 8 |
                      in_t0 & 0xffffffffffffffffU >> (8 - uVar5) * 8) & -1L << (uVar17 + 1) * 8 |
                      *(ulonglong *)((int)(uVar44 - 0x21) - (int)uVar17) >> (7 - uVar17) * 8;
              uVar5 = uVar44 - 0x29 & 7;
              in_t1 = (*(longlong *)((int)(uVar44 - 0x30) - (int)uVar6) << uVar6 * 8 |
                      in_t1 & 0xffffffffffffffffU >> (8 - uVar6) * 8) & -1L << (uVar5 + 1) * 8 |
                      *(ulonglong *)((int)(uVar44 - 0x29) - (int)uVar5) >> (7 - uVar5) * 8;
              uVar5 = uVar44 - 0x31 & 7;
              in_t2 = (*(longlong *)((int)(uVar44 - 0x38) - (int)uVar7) << uVar7 * 8 |
                      in_t2 & 0xffffffffffffffffU >> (8 - uVar7) * 8) & -1L << (uVar5 + 1) * 8 |
                      *(ulonglong *)((int)(uVar44 - 0x31) - (int)uVar5) >> (7 - uVar5) * 8;
              uVar5 = uVar44 - 0x39 & 7;
              in_t3 = (*(longlong *)((int)(uVar44 - 0x40) - (int)uVar8) << uVar8 * 8 |
                      in_t3 & 0xffffffffffffffffU >> (8 - uVar8) * 8) & -1L << (uVar5 + 1) * 8 |
                      *(ulonglong *)((int)(uVar44 - 0x39) - (int)uVar5) >> (7 - uVar5) * 8;
              uVar5 = uVar44 - 0x41 & 7;
              uVar61 = (*(longlong *)((int)(uVar44 - 0x48) - (int)uVar9) << uVar9 * 8 |
                       uVar61 & 0xffffffffffffffffU >> (8 - uVar9) * 8) & -1L << (uVar5 + 1) * 8 |
                       *(ulonglong *)((int)(uVar44 - 0x41) - (int)uVar5) >> (7 - uVar5) * 8;
              uVar5 = uVar44 - 0x49 & 7;
              uVar65 = (*(longlong *)((int)(uVar44 - 0x50) - (int)uVar10) << uVar10 * 8 |
                       uVar65 & 0xffffffffffffffffU >> (8 - uVar10) * 8) & -1L << (uVar5 + 1) * 8 |
                       *(ulonglong *)((int)(uVar44 - 0x49) - (int)uVar5) >> (7 - uVar5) * 8;
              uVar5 = uVar44 - 0x51 & 7;
              in_v1 = (*(longlong *)((int)(uVar44 - 0x58) - (int)uVar11) << uVar11 * 8 |
                      in_v1 & 0xffffffffffffffffU >> (8 - uVar11) * 8) & -1L << (uVar5 + 1) * 8 |
                      *(ulonglong *)((int)(uVar44 - 0x51) - (int)uVar5) >> (7 - uVar5) * 8;
              uVar5 = uVar44 - 0x59 & 7;
              uVar63 = (*(longlong *)((int)(uVar44 - 0x60) - (int)uVar12) << uVar12 * 8 |
                       uVar63 & 0xffffffffffffffffU >> (8 - uVar12) * 8) & -1L << (uVar5 + 1) * 8 |
                       *(ulonglong *)((int)(uVar44 - 0x59) - (int)uVar5) >> (7 - uVar5) * 8;
              iVar49 = (int)uVar60;
              *(ulonglong *)(iVar49 + -8) =
                   (*(longlong *)((int)(uVar44 - 8) - (int)uVar35) << uVar35 * 8 |
                   in_t4 & 0xffffffffffffffffU >> (8 - uVar35) * 8) & -1L << (uVar13 + 1) * 8 |
                   *(ulonglong *)((int)(uVar44 - 1) - (int)uVar13) >> (7 - uVar13) * 8;
              *(ulonglong *)(iVar49 + -0x10) =
                   (lVar18 << uVar2 * 8 | in_t5 & 0xffffffffffffffffU >> (8 - uVar2) * 8) &
                   -1L << (uVar14 + 1) * 8 | uVar25 >> (7 - uVar14) * 8;
              *(ulonglong *)(iVar49 + -0x18) =
                   (lVar19 << uVar3 * 8 | in_t6 & 0xffffffffffffffffU >> (8 - uVar3) * 8) &
                   -1L << (uVar15 + 1) * 8 | uVar27 >> (7 - uVar15) * 8;
              *(ulonglong *)(iVar49 + -0x20) =
                   (lVar20 << uVar4 * 8 | in_t7 & 0xffffffffffffffffU >> (8 - uVar4) * 8) &
                   -1L << (uVar16 + 1) * 8 | uVar28 >> (7 - uVar16) * 8;
              *(ulonglong *)(iVar49 + -0x28) = in_t0;
              *(ulonglong *)(iVar49 + -0x30) = in_t1;
              *(ulonglong *)(iVar49 + -0x38) = in_t2;
              *(ulonglong *)(iVar49 + -0x40) = in_t3;
              *(ulonglong *)(iVar49 + -0x48) = uVar61;
              *(ulonglong *)(iVar49 + -0x50) = uVar65;
              *(ulonglong *)(iVar49 + -0x58) = in_v1;
              *(ulonglong *)(iVar49 + -0x60) = uVar63;
              iVar38 = (int)uVar44;
              in_t4 = *(ulonglong *)(iVar38 + -0x68);
              in_t5 = *(ulonglong *)(iVar38 + -0x70);
              in_t6 = *(ulonglong *)(iVar38 + -0x78);
              in_t7 = *(ulonglong *)(iVar38 + -0x80);
              puVar43 = (ulonglong *)(iVar49 + -0x80);
              uVar60 = (ulonglong)(int)puVar43;
              uVar44 = (ulonglong)(iVar38 + -0x80);
              *(ulonglong *)(iVar49 + -0x68) = in_t4;
              *(ulonglong *)(iVar49 + -0x70) = in_t5;
              *(ulonglong *)(iVar49 + -0x78) = in_t6;
              *puVar43 = in_t7;
            } while (uVar48 != uVar60);
          }
          else {
            do {
              uVar2 = uVar44 - 8 & 7;
              uVar3 = uVar44 - 0x10 & 7;
              uVar4 = uVar44 - 0x18 & 7;
              uVar5 = uVar44 - 0x20 & 7;
              uVar6 = uVar44 - 0x28 & 7;
              uVar7 = uVar44 - 0x30 & 7;
              uVar8 = uVar44 - 0x38 & 7;
              uVar9 = uVar44 - 0x40 & 7;
              uVar10 = uVar44 - 0x48 & 7;
              uVar11 = uVar44 - 0x50 & 7;
              uVar12 = uVar44 - 0x58 & 7;
              uVar13 = uVar44 - 0x60 & 7;
              uVar14 = uVar44 - 0x68 & 7;
              uVar15 = uVar44 - 0x70 & 7;
              uVar16 = uVar44 - 0x78 & 7;
              uVar17 = uVar44 - 0x80 & 7;
              uVar25 = uVar44 - 1 & 7;
              in_t4 = (*(longlong *)((int)(uVar44 - 8) - (int)uVar2) << uVar2 * 8 |
                      in_t4 & 0xffffffffffffffffU >> (8 - uVar2) * 8) & -1L << (uVar25 + 1) * 8 |
                      *(ulonglong *)((int)(uVar44 - 1) - (int)uVar25) >> (7 - uVar25) * 8;
              uVar2 = uVar44 - 9 & 7;
              in_t5 = (*(longlong *)((int)(uVar44 - 0x10) - (int)uVar3) << uVar3 * 8 |
                      in_t5 & 0xffffffffffffffffU >> (8 - uVar3) * 8) & -1L << (uVar2 + 1) * 8 |
                      *(ulonglong *)((int)(uVar44 - 9) - (int)uVar2) >> (7 - uVar2) * 8;
              uVar2 = uVar44 - 0x11 & 7;
              in_t6 = (*(longlong *)((int)(uVar44 - 0x18) - (int)uVar4) << uVar4 * 8 |
                      in_t6 & 0xffffffffffffffffU >> (8 - uVar4) * 8) & -1L << (uVar2 + 1) * 8 |
                      *(ulonglong *)((int)(uVar44 - 0x11) - (int)uVar2) >> (7 - uVar2) * 8;
              uVar2 = uVar44 - 0x19 & 7;
              in_t7 = (*(longlong *)((int)(uVar44 - 0x20) - (int)uVar5) << uVar5 * 8 |
                      in_t7 & 0xffffffffffffffffU >> (8 - uVar5) * 8) & -1L << (uVar2 + 1) * 8 |
                      *(ulonglong *)((int)(uVar44 - 0x19) - (int)uVar2) >> (7 - uVar2) * 8;
              uVar2 = uVar44 - 0x21 & 7;
              in_t0 = (*(longlong *)((int)(uVar44 - 0x28) - (int)uVar6) << uVar6 * 8 |
                      in_t0 & 0xffffffffffffffffU >> (8 - uVar6) * 8) & -1L << (uVar2 + 1) * 8 |
                      *(ulonglong *)((int)(uVar44 - 0x21) - (int)uVar2) >> (7 - uVar2) * 8;
              uVar2 = uVar44 - 0x29 & 7;
              in_t1 = (*(longlong *)((int)(uVar44 - 0x30) - (int)uVar7) << uVar7 * 8 |
                      in_t1 & 0xffffffffffffffffU >> (8 - uVar7) * 8) & -1L << (uVar2 + 1) * 8 |
                      *(ulonglong *)((int)(uVar44 - 0x29) - (int)uVar2) >> (7 - uVar2) * 8;
              uVar2 = uVar44 - 0x31 & 7;
              in_t2 = (*(longlong *)((int)(uVar44 - 0x38) - (int)uVar8) << uVar8 * 8 |
                      in_t2 & 0xffffffffffffffffU >> (8 - uVar8) * 8) & -1L << (uVar2 + 1) * 8 |
                      *(ulonglong *)((int)(uVar44 - 0x31) - (int)uVar2) >> (7 - uVar2) * 8;
              uVar2 = uVar44 - 0x39 & 7;
              in_t3 = (*(longlong *)((int)(uVar44 - 0x40) - (int)uVar9) << uVar9 * 8 |
                      in_t3 & 0xffffffffffffffffU >> (8 - uVar9) * 8) & -1L << (uVar2 + 1) * 8 |
                      *(ulonglong *)((int)(uVar44 - 0x39) - (int)uVar2) >> (7 - uVar2) * 8;
              uVar2 = uVar44 - 0x41 & 7;
              uVar61 = (*(longlong *)((int)(uVar44 - 0x48) - (int)uVar10) << uVar10 * 8 |
                       uVar61 & 0xffffffffffffffffU >> (8 - uVar10) * 8) & -1L << (uVar2 + 1) * 8 |
                       *(ulonglong *)((int)(uVar44 - 0x41) - (int)uVar2) >> (7 - uVar2) * 8;
              uVar2 = uVar44 - 0x49 & 7;
              uVar65 = (*(longlong *)((int)(uVar44 - 0x50) - (int)uVar11) << uVar11 * 8 |
                       uVar65 & 0xffffffffffffffffU >> (8 - uVar11) * 8) & -1L << (uVar2 + 1) * 8 |
                       *(ulonglong *)((int)(uVar44 - 0x49) - (int)uVar2) >> (7 - uVar2) * 8;
              uVar2 = uVar44 - 0x51 & 7;
              uVar35 = (*(longlong *)((int)(uVar44 - 0x58) - (int)uVar12) << uVar12 * 8 |
                       uVar35 & 0xffffffffffffffffU >> (8 - uVar12) * 8) & -1L << (uVar2 + 1) * 8 |
                       *(ulonglong *)((int)(uVar44 - 0x51) - (int)uVar2) >> (7 - uVar2) * 8;
              uVar2 = uVar44 - 0x59 & 7;
              in_v1 = (*(longlong *)((int)(uVar44 - 0x60) - (int)uVar13) << uVar13 * 8 |
                      in_v1 & 0xffffffffffffffffU >> (8 - uVar13) * 8) & -1L << (uVar2 + 1) * 8 |
                      *(ulonglong *)((int)(uVar44 - 0x59) - (int)uVar2) >> (7 - uVar2) * 8;
              uVar2 = uVar44 - 0x61 & 7;
              unaff_s0 = (*(longlong *)((int)(uVar44 - 0x68) - (int)uVar14) << uVar14 * 8 |
                         unaff_s0 & 0xffffffffffffffffU >> (8 - uVar14) * 8) &
                         -1L << (uVar2 + 1) * 8 |
                         *(ulonglong *)((int)(uVar44 - 0x61) - (int)uVar2) >> (7 - uVar2) * 8;
              uVar2 = uVar44 - 0x69 & 7;
              unaff_s1 = (*(longlong *)((int)(uVar44 - 0x70) - (int)uVar15) << uVar15 * 8 |
                         unaff_s1 & 0xffffffffffffffffU >> (8 - uVar15) * 8) &
                         -1L << (uVar2 + 1) * 8 |
                         *(ulonglong *)((int)(uVar44 - 0x69) - (int)uVar2) >> (7 - uVar2) * 8;
              uVar2 = uVar44 - 0x71 & 7;
              unaff_s2 = (*(longlong *)((int)(uVar44 - 0x78) - (int)uVar16) << uVar16 * 8 |
                         unaff_s2 & 0xffffffffffffffffU >> (8 - uVar16) * 8) &
                         -1L << (uVar2 + 1) * 8 |
                         *(ulonglong *)((int)(uVar44 - 0x71) - (int)uVar2) >> (7 - uVar2) * 8;
              uVar2 = uVar44 - 0x79 & 7;
              uVar63 = (*(longlong *)((int)(uVar44 - 0x80) - (int)uVar17) << uVar17 * 8 |
                       uVar63 & 0xffffffffffffffffU >> (8 - uVar17) * 8) & -1L << (uVar2 + 1) * 8 |
                       *(ulonglong *)((int)(uVar44 - 0x79) - (int)uVar2) >> (7 - uVar2) * 8;
              iVar49 = (int)uVar60;
              puVar43 = (ulonglong *)(iVar49 + -0x80);
              uVar60 = (ulonglong)(int)puVar43;
              uVar44 = (ulonglong)((int)uVar44 + -0x80);
              *(ulonglong *)(iVar49 + -8) = in_t4;
              *(ulonglong *)(iVar49 + -0x10) = in_t5;
              *(ulonglong *)(iVar49 + -0x18) = in_t6;
              *(ulonglong *)(iVar49 + -0x20) = in_t7;
              *(ulonglong *)(iVar49 + -0x28) = in_t0;
              *(ulonglong *)(iVar49 + -0x30) = in_t1;
              *(ulonglong *)(iVar49 + -0x38) = in_t2;
              *(ulonglong *)(iVar49 + -0x40) = in_t3;
              *(ulonglong *)(iVar49 + -0x48) = uVar61;
              *(ulonglong *)(iVar49 + -0x50) = uVar65;
              *(ulonglong *)(iVar49 + -0x58) = uVar35;
              *(ulonglong *)(iVar49 + -0x60) = in_v1;
              *(ulonglong *)(iVar49 + -0x68) = unaff_s0;
              *(ulonglong *)(iVar49 + -0x70) = unaff_s1;
              *(ulonglong *)(iVar49 + -0x78) = unaff_s2;
              *puVar43 = uVar63;
            } while (uVar48 != uVar60);
          }
        }
        iVar49 = (int)uVar44;
        puVar37 = puVar43;
        if ((uVar47 & 0x40) != 0) {
          uVar35 = uVar44 - 8 & 7;
          uVar60 = uVar44 - 0x10 & 7;
          lVar18 = *(longlong *)((int)(uVar44 - 0x10) - (int)uVar60);
          uVar63 = uVar44 - 0x18 & 7;
          lVar19 = *(longlong *)((int)(uVar44 - 0x18) - (int)uVar63);
          uVar61 = uVar44 - 0x20 & 7;
          lVar20 = *(longlong *)((int)(uVar44 - 0x20) - (int)uVar61);
          uVar48 = uVar44 - 0x28 & 7;
          lVar21 = *(longlong *)((int)(uVar44 - 0x28) - (int)uVar48);
          uVar65 = uVar44 - 0x30 & 7;
          lVar22 = *(longlong *)((int)(uVar44 - 0x30) - (int)uVar65);
          uVar2 = uVar44 - 0x38 & 7;
          lVar23 = *(longlong *)((int)(uVar44 - 0x38) - (int)uVar2);
          uVar3 = uVar44 - 0x40 & 7;
          lVar24 = *(longlong *)((int)(uVar44 - 0x40) - (int)uVar3);
          uVar4 = uVar44 - 1 & 7;
          uVar5 = uVar44 - 9 & 7;
          uVar12 = *(ulonglong *)((int)(uVar44 - 9) - (int)uVar5);
          uVar6 = uVar44 - 0x11 & 7;
          uVar13 = *(ulonglong *)((int)(uVar44 - 0x11) - (int)uVar6);
          uVar7 = uVar44 - 0x19 & 7;
          uVar14 = *(ulonglong *)((int)(uVar44 - 0x19) - (int)uVar7);
          uVar8 = uVar44 - 0x21 & 7;
          uVar15 = *(ulonglong *)((int)(uVar44 - 0x21) - (int)uVar8);
          uVar9 = uVar44 - 0x29 & 7;
          uVar16 = *(ulonglong *)((int)(uVar44 - 0x29) - (int)uVar9);
          uVar10 = uVar44 - 0x31 & 7;
          uVar17 = *(ulonglong *)((int)(uVar44 - 0x31) - (int)uVar10);
          uVar11 = uVar44 - 0x39 & 7;
          uVar25 = *(ulonglong *)((int)(uVar44 - 0x39) - (int)uVar11);
          iVar49 = iVar49 + -0x40;
          puVar37 = puVar43 + -8;
          puVar43[-1] = (*(longlong *)((int)(uVar44 - 8) - (int)uVar35) << uVar35 * 8 |
                        in_t4 & 0xffffffffffffffffU >> (8 - uVar35) * 8) & -1L << (uVar4 + 1) * 8 |
                        *(ulonglong *)((int)(uVar44 - 1) - (int)uVar4) >> (7 - uVar4) * 8;
          puVar43[-2] = (lVar18 << uVar60 * 8 | in_t5 & 0xffffffffffffffffU >> (8 - uVar60) * 8) &
                        -1L << (uVar5 + 1) * 8 | uVar12 >> (7 - uVar5) * 8;
          puVar43[-3] = (lVar19 << uVar63 * 8 | in_t6 & 0xffffffffffffffffU >> (8 - uVar63) * 8) &
                        -1L << (uVar6 + 1) * 8 | uVar13 >> (7 - uVar6) * 8;
          puVar43[-4] = (lVar20 << uVar61 * 8 | in_t7 & 0xffffffffffffffffU >> (8 - uVar61) * 8) &
                        -1L << (uVar7 + 1) * 8 | uVar14 >> (7 - uVar7) * 8;
          puVar43[-5] = (lVar21 << uVar48 * 8 | in_t0 & 0xffffffffffffffffU >> (8 - uVar48) * 8) &
                        -1L << (uVar8 + 1) * 8 | uVar15 >> (7 - uVar8) * 8;
          puVar43[-6] = (lVar22 << uVar65 * 8 | in_t1 & 0xffffffffffffffffU >> (8 - uVar65) * 8) &
                        -1L << (uVar9 + 1) * 8 | uVar16 >> (7 - uVar9) * 8;
          puVar43[-7] = (lVar23 << uVar2 * 8 | in_t2 & 0xffffffffffffffffU >> (8 - uVar2) * 8) &
                        -1L << (uVar10 + 1) * 8 | uVar17 >> (7 - uVar10) * 8;
          *puVar37 = (lVar24 << uVar3 * 8 | in_t3 & 0xffffffffffffffffU >> (8 - uVar3) * 8) &
                     -1L << (uVar11 + 1) * 8 | uVar25 >> (7 - uVar11) * 8;
        }
        puVar43 = puVar37;
        if ((uVar47 & 0x20) != 0) {
          puVar46 = (ulonglong *)(iVar49 + -8);
          uVar44 = *(ulonglong *)(iVar49 + -0x10);
          uVar35 = *(ulonglong *)(iVar49 + -0x18);
          uVar60 = *(ulonglong *)(iVar49 + -0x20);
          iVar49 = iVar49 + -0x20;
          puVar43 = puVar37 + -4;
          puVar37[-1] = *puVar46;
          puVar37[-2] = uVar44;
          puVar37[-3] = uVar35;
          *puVar43 = uVar60;
        }
        puVar37 = puVar43;
        if ((uVar47 & 0x10) != 0) {
          puVar46 = (ulonglong *)(iVar49 + -8);
          uVar44 = *(ulonglong *)(iVar49 + -0x10);
          iVar49 = iVar49 + -0x10;
          puVar37 = puVar43 + -2;
          puVar43[-1] = *puVar46;
          *puVar37 = uVar44;
        }
        if ((uVar47 & 8) != 0) {
          puVar43 = (ulonglong *)(iVar49 + -8);
          iVar49 = iVar49 + -8;
          puVar37 = puVar37 + -1;
          *puVar37 = *puVar43;
        }
        iVar38 = (int)(uVar47 & 7);
        if ((uVar47 & 7) != 0) {
          uVar42 = (int)puVar37 - iVar38;
          uVar1 = uVar42 & 7;
          puVar43 = (ulonglong *)(uVar42 - uVar1);
          *puVar43 = *puVar43 & -1L << (8 - uVar1) * 8 |
                     *(ulonglong *)(iVar49 - iVar38) >> uVar1 * 8;
        }
        return __dest;
      }
      while (iVar40 = (int)uVar47, 7 < uVar47) {
        uVar47 = *(ulonglong *)((int)uVar44 + -8);
        uVar35 = uVar60 - 8 & 7;
        puVar43 = (ulonglong *)((int)(uVar60 - 8) - (int)uVar35);
        *puVar43 = *puVar43 & -1L << (8 - uVar35) * 8 | uVar47 >> uVar35 * 8;
        iVar49 = (int)uVar44 + -8;
        uVar44 = (ulonglong)iVar49;
        iVar38 = (int)uVar60 + -8;
        uVar60 = (ulonglong)iVar38;
        uVar35 = uVar60 + 7 & 7;
        puVar43 = (ulonglong *)((int)(uVar60 + 7) - (int)uVar35);
        *puVar43 = uVar47 << (7 - uVar35) * 8 | *puVar43 & 0xffffffffffffffffU >> (uVar35 + 1) * 8;
        uVar47 = (ulonglong)(iVar40 + -8);
      }
      if ((((uVar47 != 0) &&
           (*(undefined1 *)(iVar38 + -1) = *(undefined1 *)(iVar49 + -1), iVar40 != 1)) &&
          (*(undefined1 *)(iVar38 + -2) = *(undefined1 *)(iVar49 + -2), iVar40 != 2)) &&
         (((*(undefined1 *)(iVar38 + -3) = *(undefined1 *)(iVar49 + -3), iVar40 != 3 &&
           (*(undefined1 *)(iVar38 + -4) = *(undefined1 *)(iVar49 + -4), iVar40 != 4)) &&
          ((*(undefined1 *)(iVar38 + -5) = *(undefined1 *)(iVar49 + -5), iVar40 != 5 &&
           (*(undefined1 *)(iVar38 + -6) = *(undefined1 *)(iVar49 + -6), iVar40 != 6)))))) {
        *(undefined1 *)(iVar38 + -7) = *(undefined1 *)(iVar49 + -7);
        return pvVar33;
      }
    }
  }
  return pvVar33;
}



// === _memcpy_fp @ 0fb8a730 (40 bytes) ===

void _memcpy_fp(undefined8 *param_1,undefined8 *param_2,int param_3,int param_4)

{
  undefined8 uVar1;
  undefined8 uVar2;
  
  do {
    uVar1 = *param_2;
    uVar2 = param_2[1];
    param_2 = param_2 + 2;
    param_3 = param_3 + -0x10;
    *param_1 = uVar1;
    param_1[1] = uVar2;
    param_1 = param_1 + 2;
  } while (param_3 != param_4);
  return;
}



// === strcpy @ 0fb8a758 (168 bytes) ===

char * strcpy(char *__dest,char *__src)

{
  uint uVar1;
  uint uVar2;
  char *pcVar3;
  int iVar4;
  char *pcVar5;
  char cVar6;
  char cVar7;
  
  cVar6 = *__src;
  iVar4 = ((uint)__dest | 3) - (int)__dest;
  pcVar3 = __dest;
  if (cVar6 != '\0') {
    cVar7 = __src[1];
    if (cVar7 != '\0') {
      uVar1 = *(uint *)__src;
      if (__src[2] != '\0') {
        cVar6 = __src[3];
        uVar2 = (uint)__dest & 3;
        *(uint *)(__dest + -uVar2) =
             *(uint *)(__dest + -uVar2) & -1 << (4 - uVar2) * 8 | uVar1 >> uVar2 * 8;
        if (cVar6 == '\0') {
          pcVar5 = __dest + 3;
          uVar2 = (uint)pcVar5 & 3;
          *(uint *)(pcVar5 + -uVar2) =
               *(uint *)(pcVar5 + -uVar2) & 0xffffffffU >> (uVar2 + 1) * 8 |
               uVar1 << (3 - uVar2) * 8;
          return pcVar3;
        }
        __dest = (char *)(((uint)__dest | 3) - 3);
        pcVar5 = __src + iVar4;
        while( true ) {
          cVar6 = pcVar5[1];
          __dest = __dest + 4;
          if (cVar6 == '\0') break;
          cVar7 = pcVar5[2];
          if (cVar7 == '\0') goto LAB_0fb8a7f4;
          if (pcVar5[3] == '\0') goto LAB_0fb8a7f0;
          cVar6 = pcVar5[4];
          *(undefined4 *)__dest = *(undefined4 *)(pcVar5 + 1);
          pcVar5 = pcVar5 + 4;
          if (cVar6 == '\0') {
            return pcVar3;
          }
        }
        goto LAB_0fb8a7f8;
      }
LAB_0fb8a7f0:
      __dest[2] = '\0';
    }
LAB_0fb8a7f4:
    __dest[1] = cVar7;
  }
LAB_0fb8a7f8:
  *__dest = cVar6;
  return pcVar3;
}



// === fstat @ 0fb8a800 (40 bytes) ===

int fstat(int __fd,stat *__buf)

{
  int iVar1;
  
  iVar1 = fxstat(2,__fd,__buf);
  return iVar1;
}



// === fstat64 @ 0fb8a828 (40 bytes) ===

int fstat64(int __fd,stat64 *__buf)

{
  int iVar1;
  
  iVar1 = fxstat(2,__fd,__buf);
  return iVar1;
}



// === _wstat @ 0fb8a850 (100 bytes) ===

uint _wstat(longlong param_1,uint param_2)

{
  param_2 = param_2 & 0xff;
  if (param_1 == 1) {
    param_2 = param_2 << 8;
  }
  else if (param_1 == 3) {
    param_2 = param_2 | 0x80;
  }
  else if (param_1 != 2) {
    if ((param_1 == 4) || (param_1 == 5)) {
      param_2 = param_2 << 8 | 0x7f;
    }
    else if (param_1 == 6) {
      param_2 = 0xffff;
    }
  }
  return param_2;
}



// === abort @ 0fb8a8b4 (104 bytes) ===

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void abort(void)

{
  EVP_PKEY_CTX *extraout_a0_lo;
  EVP_PKEY_CTX *extraout_a0_lo_00;
  EVP_PKEY_CTX *ctx;
  ulong uStack_2c;
  
  sigaction(6,(sigaction *)0x0,(sigaction *)&stack0xffffffd0);
  ctx = extraout_a0_lo;
  if ((uStack_2c != 0) && (uStack_2c != 1)) {
    sigrelse(6);
    raise(6);
    ctx = extraout_a0_lo_00;
  }
  _cleanup(ctx);
  signal(6,(__sighandler_t)0x0);
  sigrelse(6);
  raise(6);
                    /* WARNING: Subroutine does not return */
  _exit(1);
}



// === signal @ 0fb8a928 (180 bytes) ===

__sighandler_t signal(int __sig,__sighandler_t __handler)

{
  int iVar2;
  longlong lVar1;
  ulong auStack_80 [2];
  ulong auStack_78 [2];
  ulong uStack_70;
  ulong uStack_6c;
  undefined1 auStack_50 [4];
  __sighandler_t p_Stack_4c;
  longlong lStack_28;
  
  auStack_78 = (ulong  [2])DAT_0fbd2a00;
  auStack_80 = (ulong  [2])DAT_0fbd29f8;
  if ((0 < __sig) && (lStack_28 = (longlong)(int)__handler, __sig < 0x41)) {
    iVar2 = sigismember((sigset_t *)auStack_80,__sig);
    if (iVar2 == 0) {
      uStack_70 = 0x12;
      uStack_6c = (ulong)lStack_28;
      lVar1 = _ssig(__sig,&uStack_70,auStack_50);
      if (lVar1 != 0) {
        return (__sighandler_t)0xffffffff;
      }
      return p_Stack_4c;
    }
  }
  setoserror(0x16);
  return (__sighandler_t)0xffffffff;
}



// === raise @ 0fb8a9dc (48 bytes) ===

int raise(int __sig)

{
  __pid_t __pid;
  int iVar1;
  
  __pid = getpid();
  iVar1 = kill(__pid,__sig);
  return iVar1;
}



// === kill @ 0fb8aa0c (44 bytes) ===

int kill(__pid_t __pid,int __sig)

{
  int iVar1;
  longlong in_a3;
  
  syscall(0);
  if (in_a3 == 0) {
    return 0;
  }
  iVar1 = _cerror(__pid,__sig);
  return iVar1;
}



// === sigrelse @ 0fb8aa38 (152 bytes) ===

/* WARNING: Instruction at (ram,0x0fb8aa80) overlaps instruction at (ram,0x0fb8aa7c)
    */

int sigrelse(int __sig)

{
  int iVar1;
  ulong auStack_40 [2];
  ulong auStack_38 [8];
  
  auStack_38._0_8_ = DAT_0fbd2a10;
  auStack_40 = (ulong  [2])DAT_0fbd2a08;
  if (((0 < __sig) && (__sig < 0x41)) &&
     (iVar1 = sigismember((sigset_t *)auStack_40,__sig), iVar1 == 0)) {
    sigemptyset((sigset_t *)(auStack_38 + 2));
    sigaddset((sigset_t *)(auStack_38 + 2),__sig);
    iVar1 = sigprocmask(2,(sigset_t *)(auStack_38 + 2),(sigset_t *)0x0);
    return iVar1;
  }
  setoserror(0x16);
  return -1;
}



// === _ssig @ 0fb8aad0 (136 bytes) ===

int _ssig(longlong param_1,sigaction *param_2,sigaction *param_3)

{
  int iVar1;
  
  sigemptyset((sigset_t *)((param_2->sa_mask).__val + 1));
  if ((param_1 == 0x12) &&
     ((param_2->__sigaction_handler).sa_handler =
           (__sighandler_t)((uint)(param_2->__sigaction_handler).sa_handler | 0x20000),
     (param_2->sa_mask).__val[0] == 1)) {
    (param_2->__sigaction_handler).sa_handler =
         (__sighandler_t)((uint)(param_2->__sigaction_handler).sa_handler | 0x10000);
  }
  iVar1 = sigaction((int)param_1,param_2,param_3);
  return iVar1;
}



// === getpid @ 0fb8ab58 (40 bytes) ===

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

__pid_t getpid(void)

{
  __pid_t _Var1;
  longlong in_a3;
  
  syscall(0);
  if (in_a3 == 0) {
    return 0x3fc;
  }
  _Var1 = _cerror();
  return _Var1;
}



// === qsort @ 0fb8ab80 (628 bytes) ===

void qsort(void *__base,size_t __nmemb,size_t __size,__compar_fn_t __compar)

{
  undefined1 uVar1;
  undefined1 *puVar2;
  int iVar3;
  undefined1 uVar4;
  undefined1 *puVar5;
  undefined1 *puVar6;
  undefined1 *puVar7;
  undefined1 *puVar8;
  undefined1 *puVar9;
  undefined8 in_t2;
  undefined1 *puVar10;
  undefined1 *puVar11;
  undefined1 *puVar12;
  
  if (1 < __nmemb) {
    if (__nmemb < 4) {
      puVar11 = (undefined1 *)(__nmemb * __size + (int)__base);
      puVar5 = puVar11;
    }
    else {
      puVar11 = (undefined1 *)(__nmemb * __size + (int)__base);
      FUN_0fb8adf4(__base,puVar11,__compar,__size,__size * 4,__size * 6,in_t2,__nmemb,
                   __nmemb * __size);
      puVar5 = (undefined1 *)(__size * 4 + (int)__base);
    }
    puVar12 = (undefined1 *)((int)__base + __size);
    puVar7 = __base;
    for (puVar10 = puVar12; puVar10 < puVar5; puVar10 = puVar10 + __size) {
      iVar3 = (*__compar)(puVar7,puVar10);
      puVar9 = puVar10;
      if (iVar3 < 1) {
        puVar9 = puVar7;
      }
      puVar7 = puVar9;
    }
    if ((__base != puVar7) && (__base < puVar12)) {
      puVar5 = __base;
      puVar10 = puVar7;
      if ((__size & 1) != 0) {
        puVar5 = (undefined1 *)((int)__base + 1);
        uVar1 = *puVar7;
        puVar10 = puVar7 + 1;
        *puVar7 = *(undefined1 *)__base;
        *(undefined1 *)__base = uVar1;
      }
      if ((int)__size >> 1 != 0) {
        do {
          uVar1 = *puVar10;
          *puVar10 = *puVar5;
          *puVar5 = uVar1;
          puVar7 = puVar5 + 2;
          uVar1 = puVar10[1];
          puVar10[1] = puVar5[1];
          puVar5[1] = uVar1;
          puVar5 = puVar7;
          puVar10 = puVar10 + 2;
        } while (puVar12 != puVar7);
      }
    }
    if (puVar12 < puVar11) {
      puVar5 = puVar12 + -__size;
      do {
        iVar3 = (*__compar)(puVar5,puVar12);
        puVar10 = puVar5;
        while ((0 < iVar3 && (__base != puVar10))) {
          puVar10 = puVar10 + -__size;
          iVar3 = (*__compar)(puVar10,puVar12);
        }
        puVar10 = puVar10 + __size;
        if (puVar10 != puVar12) {
          puVar7 = puVar12 + (__size - 1);
          if (puVar12 <= puVar7) {
            puVar9 = puVar7 + -__size;
            do {
              uVar1 = *puVar7;
              puVar6 = puVar7;
              if (puVar10 <= puVar9) {
                uVar4 = *puVar9;
                puVar8 = puVar7;
                puVar2 = puVar9;
                while( true ) {
                  *puVar8 = uVar4;
                  puVar6 = puVar2 + -__size;
                  if (puVar6 < puVar10) break;
                  uVar4 = *puVar6;
                  puVar8 = puVar2;
                  puVar2 = puVar6;
                }
                puVar6 = puVar6 + __size;
              }
              puVar9 = puVar9 + -1;
              puVar7 = puVar7 + -1;
              *puVar6 = uVar1;
            } while (puVar12 + -1 != puVar7);
          }
        }
        puVar12 = puVar12 + __size;
        puVar5 = puVar5 + __size;
      } while (puVar12 < puVar11);
    }
    return;
  }
  return;
}



// === FUN_0fb8adf4 @ 0fb8adf4 (876 bytes) ===

void FUN_0fb8adf4(undefined1 *param_1,undefined1 *param_2,code *param_3,uint param_4,uint param_5,
                 uint param_6)

{
  undefined1 uVar1;
  bool bVar2;
  uint uVar3;
  longlong lVar4;
  uint uVar5;
  undefined1 *puVar6;
  uint uVar7;
  undefined1 *puVar8;
  undefined1 *puVar9;
  undefined1 *puVar10;
  undefined1 *puVar11;
  uint uVar12;
  undefined1 *puVar13;
  undefined1 *puVar14;
  
  uVar7 = (int)param_2 - (int)param_1;
  uVar12 = 1;
  if (param_4 != 0) {
    uVar12 = param_4;
  }
  do {
    puVar13 = param_2 + -param_4;
    puVar9 = param_1 + (uVar7 / param_4 >> 1) * param_4;
    uVar5 = (int)uVar12 >> 1;
    puVar10 = param_1;
    if (param_6 <= uVar7) {
      lVar4 = (*param_3)(param_1,puVar9);
      puVar8 = puVar9;
      if (0 < lVar4) {
        puVar8 = param_1;
      }
      lVar4 = (*param_3)(puVar8,puVar13);
      if (lVar4 < 1) {
LAB_0fb8aedc:
        puVar14 = puVar8;
      }
      else {
        puVar14 = param_1;
        if (param_1 == puVar8) {
          puVar14 = puVar9;
        }
        lVar4 = (*param_3)(puVar14,puVar13);
        puVar8 = puVar13;
        if (lVar4 < 0) goto LAB_0fb8aedc;
      }
      if (puVar9 != puVar14) {
        uVar7 = param_4;
        puVar8 = puVar14;
        puVar6 = puVar9;
        uVar3 = uVar5;
        if ((uVar12 & 1) != 0) {
          uVar7 = param_4 - 1;
          puVar8 = puVar14 + 1;
          uVar1 = *puVar9;
          puVar6 = puVar9 + 1;
          *puVar9 = *puVar14;
          *puVar14 = uVar1;
        }
        while (uVar3 != 0) {
          uVar1 = *puVar6;
          *puVar6 = *puVar8;
          *puVar8 = uVar1;
          uVar1 = puVar6[1];
          puVar6[1] = puVar8[1];
          puVar8[1] = uVar1;
          uVar7 = uVar7 - 2;
          puVar8 = puVar8 + 2;
          puVar6 = puVar6 + 2;
          uVar3 = uVar7;
        }
      }
    }
    while( true ) {
      if ((puVar10 < puVar9) && (lVar4 = (*param_3)(puVar10,puVar9), lVar4 < 0)) {
        do {
          puVar10 = puVar10 + param_4;
          bVar2 = false;
          if (puVar10 < puVar9) {
            lVar4 = (*param_3)(puVar10,puVar9);
            bVar2 = false;
            if (lVar4 < 0) {
              bVar2 = true;
            }
          }
        } while (bVar2);
      }
      puVar14 = puVar10 + param_4;
      for (puVar8 = puVar13; puVar9 < puVar8; puVar8 = puVar8 + -param_4) {
        lVar4 = (*param_3)(puVar9,puVar8);
        if (-1 < lVar4) {
          puVar13 = puVar8;
          puVar6 = puVar8;
          if (puVar10 != puVar9) {
            puVar13 = puVar8 + -param_4;
            puVar6 = puVar9;
          }
          goto LAB_0fb8b000;
        }
      }
      if (puVar10 == puVar9) break;
      puVar13 = puVar8 + -param_4;
      puVar8 = puVar9;
      puVar14 = puVar10;
      puVar6 = puVar10;
LAB_0fb8b000:
      puVar9 = puVar6;
      uVar7 = param_4;
      puVar6 = puVar8;
      puVar11 = puVar10;
      uVar3 = uVar5;
      if ((uVar12 & 1) != 0) {
        uVar7 = param_4 - 1;
        puVar6 = puVar8 + 1;
        uVar1 = *puVar10;
        puVar11 = puVar10 + 1;
        *puVar10 = *puVar8;
        *puVar8 = uVar1;
      }
      while (puVar10 = puVar14, uVar3 != 0) {
        uVar1 = *puVar11;
        *puVar11 = *puVar6;
        *puVar6 = uVar1;
        uVar1 = puVar11[1];
        puVar11[1] = puVar6[1];
        puVar6[1] = uVar1;
        uVar7 = uVar7 - 2;
        puVar6 = puVar6 + 2;
        puVar11 = puVar11 + 2;
        uVar3 = uVar7;
      }
    }
    puVar13 = puVar9 + param_4;
    uVar7 = (int)param_2 - (int)puVar13;
    uVar5 = (int)puVar9 - (int)param_1;
    if (uVar7 < uVar5) {
      bVar2 = param_5 <= uVar7;
      uVar7 = uVar5;
      puVar10 = puVar9;
      puVar8 = param_1;
      if (bVar2) {
        FUN_0fb8adf4(puVar13,param_2,param_3,param_4,param_5,param_6);
      }
    }
    else {
      puVar10 = param_2;
      puVar8 = puVar13;
      if (param_5 <= uVar5) {
        FUN_0fb8adf4(param_1,puVar9,param_3,param_4,param_5,param_6);
      }
    }
    param_1 = puVar8;
    param_2 = puVar10;
    if (uVar7 < param_5) {
      return;
    }
  } while( true );
}



// === close @ 0fb8b160 (56 bytes) ===

int close(int __fd)

{
  int iVar1;
  
  if (_aioinfo != 0) {
    _aqueue(__fd);
  }
  iVar1 = __close(__fd);
  return iVar1;
}



// === ioctl @ 0fb8b198 (40 bytes) ===

int ioctl(int __fd,ulong __request,...)

{
  int iVar1;
  longlong in_a3;
  
  syscall(0);
  if (in_a3 == 0) {
    return 0x41e;
  }
  iVar1 = _cerror(__fd,__request);
  return iVar1;
}



// === prctl @ 0fb8b1c0 (40 bytes) ===

int prctl(int __option,...)

{
  int iVar1;
  longlong in_a3;
  
  syscall(0);
  if (in_a3 == 0) {
    return 0x46a;
  }
  iVar1 = _cerror(__option);
  return iVar1;
}



// === open64 @ 0fb8b1e8 (40 bytes) ===

int open64(char *__file,int __oflag,...)

{
  int iVar1;
  longlong in_a3;
  
  syscall(0);
  if (in_a3 == 0) {
    return 0x3ed;
  }
  iVar1 = _cerror(__file,__oflag);
  return iVar1;
}



// === strlen @ 0fb8b210 (176 bytes) ===

size_t strlen(char *__s)

{
  char *pcVar1;
  char cVar2;
  char *pcVar3;
  char *pcVar4;
  char *pcVar5;
  char *pcVar6;
  char *pcVar7;
  char *pcVar8;
  
  pcVar5 = __s;
  if (*__s == '\0') {
LAB_0fb8b2a8:
    return (int)pcVar5 - (int)__s;
  }
  pcVar6 = __s + 1;
  if (__s[1] == '\0') {
LAB_0fb8b2b0:
    return (int)pcVar6 - (int)__s;
  }
  pcVar7 = __s + 2;
  if (__s[2] == '\0') {
LAB_0fb8b2b8:
    return (int)pcVar7 - (int)__s;
  }
  pcVar8 = __s + 3;
  cVar2 = __s[3];
  pcVar1 = pcVar6;
  pcVar3 = pcVar7;
  pcVar4 = __s;
  while (pcVar5 = pcVar8, cVar2 != '\0') {
    pcVar5 = pcVar4 + 4;
    if (pcVar4[4] == '\0') goto LAB_0fb8b2a8;
    pcVar6 = pcVar1 + 4;
    if (pcVar1[4] == '\0') goto LAB_0fb8b2b0;
    pcVar7 = pcVar3 + 4;
    if (pcVar3[4] == '\0') goto LAB_0fb8b2b8;
    pcVar5 = pcVar8 + 4;
    if (pcVar8[4] == '\0') break;
    pcVar5 = pcVar4 + 8;
    if (pcVar4[8] == '\0') goto LAB_0fb8b2a8;
    pcVar6 = pcVar1 + 8;
    if (pcVar1[8] == '\0') goto LAB_0fb8b2b0;
    pcVar7 = pcVar3 + 8;
    if (pcVar3[8] == '\0') goto LAB_0fb8b2b8;
    pcVar1 = pcVar8 + 8;
    pcVar8 = pcVar8 + 8;
    cVar2 = *pcVar1;
    pcVar1 = pcVar6;
    pcVar3 = pcVar7;
    pcVar4 = pcVar5;
  }
  return (int)pcVar5 - (int)__s;
}



// === fxstat @ 0fb8b2c0 (44 bytes) ===

undefined8 fxstat(void)

{
  undefined8 uVar1;
  longlong in_a3;
  
  syscall(0);
  if (in_a3 == 0) {
    return 0;
  }
  uVar1 = _cerror();
  return uVar1;
}



// === __sgi_prda_procmask @ 0fb8b2ec (76 bytes) ===

undefined8 __sgi_prda_procmask(longlong param_1)

{
  longlong lVar1;
  
  lVar1 = syssgi(0xb0,param_1);
  if (lVar1 == 0) {
    if (param_1 == 1) {
      DAT_0fbd3a94 = 1;
    }
    return 0;
  }
  return 0xffffffffffffffff;
}



// === sigprocmask @ 0fb8b338 (528 bytes) ===

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int sigprocmask(int __how,sigset_t *__set,sigset_t *__oset)

{
  ulonglong uVar1;
  int iVar2;
  ulonglong auStack_30 [6];
  
  if ((DAT_0fbd3a94 == 0) || ((_DAT_00200e6c & 1) == 0)) {
    iVar2 = ksigprocmask(__how);
    return iVar2;
  }
  if (((((__set != (sigset_t *)0x0) && (__how != 0)) && (__how != 1)) &&
      ((__how != 2 && (__how != 3)))) && (__how != 0x100)) {
    setoserror(0x16);
    return -1;
  }
  if (__oset != (sigset_t *)0x0) {
    if ((_DAT_00200e6c & 2) == 0) {
      __oset->__val[0] = (ulong)_DAT_00200e70;
      __oset->__val[1] = (ulong)(_DAT_00200e70 >> 0x20);
    }
    else {
      __oset->__val[0] = (ulong)(_DAT_00200e70 >> 0x20);
      __oset->__val[1] = (ulong)_DAT_00200e70;
    }
    __oset->__val[3] = 0;
    __oset->__val[2] = 0;
  }
  if (__set != (sigset_t *)0x0) {
    if ((_DAT_00200e6c & 2) == 0) {
      auStack_30[0] = CONCAT44(__set->__val[1],__set->__val[0]) & 0xffffffffffbffeff;
    }
    else {
      auStack_30[0] =
           (ulonglong)__set->__val[1] |
           ((longlong)(int)__set->__val[0] & 0xffffffffffbffeffU) << 0x20;
    }
    if (__how == 1) {
      __ksigorset(&DAT_00200e70,auStack_30);
      uVar1 = _DAT_00200e70;
    }
    else if (__how == 2) {
      __ksigdiffset(&DAT_00200e70,auStack_30);
      uVar1 = _DAT_00200e70;
    }
    else {
      uVar1 = auStack_30[0];
      if (((__how != 3) && (uVar1 = _DAT_00200e70, __how == 0x100)) &&
         (uVar1 = auStack_30[0], __set->__val[1] != 0xffffffff)) {
        if ((_DAT_00200e6c & 2) == 0) {
          _DAT_00200e70 = CONCAT44(_DAT_00200e70,(int)auStack_30[0]);
          uVar1 = _DAT_00200e70;
        }
        else {
          _DAT_00200e70 = CONCAT44((int)(auStack_30[0] >> 0x20),_DAT_00200e74);
          uVar1 = _DAT_00200e70;
        }
      }
    }
    _DAT_00200e70 = uVar1;
    return 0;
  }
  return 0;
}



// === __ksigorset @ 0fb8b548 (32 bytes) ===

/* WARNING: Removing unreachable block (ram,0x0fb8b558) */

void __ksigorset(ulonglong *param_1,ulonglong *param_2)

{
  *param_1 = *param_1 | *param_2;
  return;
}



// === __ksigdiffset @ 0fb8b568 (36 bytes) ===

/* WARNING: Removing unreachable block (ram,0x0fb8b57c) */

void __ksigdiffset(ulonglong *param_1,ulonglong *param_2)

{
  *param_1 = *param_1 & ~*param_2;
  return;
}



// === ksigprocmask @ 0fb8b58c (40 bytes) ===

undefined8 ksigprocmask(void)

{
  undefined8 uVar1;
  longlong in_a3;
  
  syscall(0);
  if (in_a3 == 0) {
    return 0x48c;
  }
  uVar1 = _cerror();
  return uVar1;
}



// === __close @ 0fb8b5b4 (44 bytes) ===

undefined8 __close(void)

{
  undefined8 uVar1;
  longlong in_a3;
  
  syscall(0);
  if (in_a3 == 0) {
    return 0;
  }
  uVar1 = _cerror();
  return uVar1;
}



// === exit @ 0fb8b5e0 (112 bytes) ===

/* WARNING: Removing unreachable block (ram,0x0fb8b5fc) */
/* WARNING: Removing unreachable block (ram,0x0fb8b638) */

void exit(int __status)

{
  int iVar1;
  EVP_PKEY_CTX *extraout_a0_lo;
  EVP_PKEY_CTX *extraout_a0_lo_00;
  EVP_PKEY_CTX *ctx;
  
  __eachexithandle();
  iVar1 = prctl(0xe);
  ctx = extraout_a0_lo;
  if (iVar1 < 2) {
    _exithandle();
    ctx = extraout_a0_lo_00;
  }
  _cleanup(ctx);
                    /* WARNING: Subroutine does not return */
  _exit(__status);
}



// === _exit @ 0fb8b65c (32 bytes) ===

void _exit(int __status)

{
  ___tp_exit();
  syscall(0);
  trap(0);
  return;
}



// === ___tp_exit @ 0fb8b67c (8 bytes) ===

void ___tp_exit(void)

{
  return;
}



// === bsd_signal @ 0fb8b684 (28 bytes) ===

__sighandler_t bsd_signal(int __sig,__sighandler_t __handler)

{
  __sighandler_t p_Var1;
  
  p_Var1 = (__sighandler_t)BSDsignal(__sig,__handler);
  return p_Var1;
}



// === BSDsignal @ 0fb8b6a0 (268 bytes) ===

longlong BSDsignal(longlong param_1,longlong param_2)

{
  int iVar1;
  _union_1051 _Stack_50;
  int iStack_4c;
  longlong lStack_30;
  longlong lStack_20;
  
  if ((param_1 < 1) || (0x20 < param_1)) {
    setoserror(0x16);
    return -1;
  }
  lStack_20 = param_2;
  if ((param_1 == 0x19) && (param_2 == 1)) {
    setoserror(0x16);
    return -1;
  }
  iVar1 = sigaction((int)param_1,(sigaction *)0x0,(sigaction *)&_Stack_50);
  if (iVar1 != 0) {
    return -1;
  }
  lStack_30 = (longlong)iStack_4c;
  iStack_4c = (int)lStack_20;
  sigdelset((sigset_t *)&stack0xffffffb8,0x19);
  _Stack_50.sa_handler = (__sighandler_t)((uint)_Stack_50.sa_handler & 0xfffdffff | 0x10000000);
  iVar1 = sigaction((int)param_1,(sigaction *)&_Stack_50,(sigaction *)0x0);
  if (iVar1 != 0) {
    return -1;
  }
  return lStack_30;
}



// === nanosleep @ 0fb8b7ac (40 bytes) ===

int nanosleep(timespec *__requested_time,timespec *__remaining)

{
  int iVar1;
  longlong in_a3;
  
  syscall(0);
  if (in_a3 == 0) {
    return 0x435;
  }
  iVar1 = _cerror(__requested_time,__remaining);
  return iVar1;
}



// === calloc @ 0fb8b7d4 (132 bytes) ===

/* WARNING: Instruction at (ram,0x0fb8b7ec) overlaps instruction at (ram,0x0fb8b7e8)
    */

void * calloc(size_t __nmemb,size_t __size)

{
  longlong lVar1;
  uint __n;
  
  if (__nmemb == 0) {
    __n = 0;
  }
  else {
    __n = 0;
    if ((__size != 0) && (__n = __nmemb * __size, __n / __nmemb != __size)) {
      return (void *)0x0;
    }
  }
  lVar1 = malloc(__n);
  if (lVar1 != 0) {
    memset((void *)lVar1,0,__n);
  }
  return (void *)lVar1;
}



// === strtok @ 0fb8b858 (160 bytes) ===

char * strtok(char *__s,char *__delim)

{
  size_t sVar1;
  char *pcVar2;
  char *__s_00;
  
  if (__s == (char *)0x0) {
    __s = DAT_0fbd3a9c;
  }
  if (__s == (char *)0x0) {
    return (char *)0x0;
  }
  sVar1 = strspn(__s,__delim);
  __s_00 = __s + sVar1;
  if (*__s_00 == '\0') {
    DAT_0fbd3a9c = (char *)0x0;
    return (char *)0x0;
  }
  pcVar2 = strpbrk(__s_00,__delim);
  if (pcVar2 == (char *)0x0) {
    DAT_0fbd3a9c = (char *)0x0;
  }
  else {
    *pcVar2 = '\0';
    DAT_0fbd3a9c = pcVar2 + 1;
  }
  return __s_00;
}



// === strtok_r @ 0fb8b8f8 (176 bytes) ===

char * strtok_r(char *__s,char *__delim,char **__save_ptr)

{
  size_t sVar1;
  char *pcVar2;
  char *__s_00;
  
  if (__s == (char *)0x0) {
    __s = *__save_ptr;
  }
  if (__s == (char *)0x0) {
    return (char *)0x0;
  }
  sVar1 = strspn(__s,__delim);
  __s_00 = __s + sVar1;
  if (*__s_00 == '\0') {
    *__save_ptr = (char *)0x0;
    return (char *)0x0;
  }
  pcVar2 = strpbrk(__s_00,__delim);
  if (pcVar2 == (char *)0x0) {
    *__save_ptr = (char *)0x0;
  }
  else {
    *pcVar2 = '\0';
    *__save_ptr = pcVar2 + 1;
  }
  return __s_00;
}



// === strpbrk @ 0fb8b9a8 (108 bytes) ===

char * strpbrk(char *__s,char *__accept)

{
  char cVar1;
  bool bVar2;
  char *pcVar3;
  char *pcVar4;
  
  do {
    pcVar3 = __accept;
    if (*__accept == '\0') {
LAB_0fb8b9f0:
      cVar1 = *pcVar3;
    }
    else {
      if (*__accept != *__s) {
        cVar1 = __accept[1];
        pcVar4 = __accept;
        while( true ) {
          pcVar3 = pcVar4 + 1;
          if (cVar1 == '\0') {
            bVar2 = false;
          }
          else {
            bVar2 = false;
            if (*__s != cVar1) {
              bVar2 = true;
            }
          }
          if (!bVar2) break;
          cVar1 = pcVar4[2];
          pcVar4 = pcVar3;
        }
        goto LAB_0fb8b9f0;
      }
      cVar1 = *__accept;
    }
    if (cVar1 != '\0') {
      return __s;
    }
    cVar1 = *__s;
    __s = __s + 1;
    if (cVar1 == '\0') {
      return (char *)0x0;
    }
  } while( true );
}



// === strspn @ 0fb8ba14 (116 bytes) ===

size_t strspn(char *__s,char *__accept)

{
  char cVar1;
  bool bVar2;
  char *pcVar3;
  char *pcVar5;
  char *pcVar4;
  
  pcVar5 = __s;
  if (*__s != '\0') {
    do {
      pcVar3 = __accept;
      if (*__accept == '\0') {
LAB_0fb8ba68:
        cVar1 = *pcVar3;
      }
      else {
        if (*__accept != *pcVar5) {
          cVar1 = __accept[1];
          pcVar4 = __accept;
          while( true ) {
            pcVar3 = pcVar4 + 1;
            if (cVar1 == '\0') {
              bVar2 = false;
            }
            else {
              bVar2 = false;
              if (*pcVar5 != cVar1) {
                bVar2 = true;
              }
            }
            if (!bVar2) break;
            cVar1 = pcVar4[2];
            pcVar4 = pcVar3;
          }
          goto LAB_0fb8ba68;
        }
        cVar1 = *__accept;
      }
    } while ((cVar1 != '\0') && (pcVar3 = pcVar5 + 1, pcVar5 = pcVar5 + 1, *pcVar3 != '\0'));
  }
  return (int)pcVar5 - (int)__s;
}



// === read @ 0fb8ba88 (40 bytes) ===

ssize_t read(int __fd,void *__buf,size_t __nbytes)

{
  ssize_t sVar1;
  longlong in_a3;
  
  syscall(0);
  if (in_a3 == 0) {
    return 0x3eb;
  }
  sVar1 = _cerror(__fd,__buf,__nbytes);
  return sVar1;
}



// === munmap @ 0fb8bab0 (40 bytes) ===

int munmap(void *__addr,size_t __len)

{
  int iVar1;
  longlong in_a3;
  
  syscall(0);
  if (in_a3 == 0) {
    return 0x46f;
  }
  iVar1 = _cerror(__addr,__len);
  return iVar1;
}



// === mmap64 @ 0fb8bad8 (40 bytes) ===

void * mmap64(void *__addr,size_t __len,int __prot,int __flags,int __fd,__off64_t __offset)

{
  void *pvVar1;
  
  syscall(0);
  if (__flags == 0) {
    return (void *)0x46e;
  }
  pvVar1 = (void *)_cerror(__addr,__len,__prot,__flags,__fd);
  return pvVar1;
}



// === ___pthread_sysconf @ 0fb8bb00 (136 bytes) ===

undefined8 ___pthread_sysconf(undefined4 param_1)

{
  switch(param_1) {
  case 0x67:
    return 4;
  case 0x68:
    return 0x80;
  case 0x69:
    return 0;
  case 0x6a:
    return 0x40;
  default:
    return 0x16;
  case 0x6c:
    return 1;
  case 0x6d:
    return 1;
  case 0x6e:
    return 1;
  case 0x6f:
    return 1;
  case 0x70:
    return 1;
  case 0x71:
    return 1;
  case 0x72:
    return 0xffffffffffffffff;
  }
}



// === endinvent_r @ 0fb8bb88 (72 bytes) ===

void endinvent_r(int param_1)

{
  if (param_1 == 0) {
    return;
  }
  if (*(int *)(param_1 + 4) != 0) {
    free();
  }
  free(param_1);
  return;
}



// === endinvent @ 0fb8bbd0 (32 bytes) ===

void endinvent(void)

{
  endinvent_r(DAT_0fbd3aa0);
  DAT_0fbd3aa0 = 0;
  return;
}



// === setinvent_r @ 0fb8bbf0 (504 bytes) ===

undefined8 setinvent_r(int *param_1)

{
  size_t __nmemb;
  size_t *psVar3;
  void *__src;
  longlong lVar1;
  longlong lVar2;
  void *pvVar4;
  size_t __n;
  int iVar5;
  
  if (*param_1 != 0) {
    *(undefined4 *)(*param_1 + 8) = 0;
    return 0;
  }
  psVar3 = (size_t *)malloc(0xc);
  __src = (void *)malloc(24000);
  lVar1 = syssgi(5,1,__src);
  if ((((psVar3 != (size_t *)0x0) && (__src != (void *)0x0)) && (lVar1 != -1)) &&
     (lVar2 = syssgi(5,2,__src,24000), lVar2 != -1)) {
    __n = (size_t)lVar1;
    __nmemb = (int)lVar2 / (int)__n;
    pvVar4 = calloc(__nmemb,0x18);
    psVar3[1] = (size_t)pvVar4;
    if (pvVar4 != (void *)0x0) {
      if (lVar1 < 0x19) {
        if (lVar1 < 0x18) {
          if (0 < (int)__nmemb) {
            iVar5 = 0;
            pvVar4 = __src;
            do {
              memmove((void *)(psVar3[1] + iVar5),pvVar4,__n);
              iVar5 = iVar5 + 0x18;
              pvVar4 = (void *)(__n + (int)pvVar4);
            } while (iVar5 != __nmemb * 0x18);
          }
        }
        else {
          memmove(pvVar4,__src,__nmemb * 0x18);
        }
      }
      else if (0 < (int)__nmemb) {
        iVar5 = 0;
        pvVar4 = __src;
        do {
          memmove((void *)(psVar3[1] + iVar5),pvVar4,0x18);
          iVar5 = iVar5 + 0x18;
          pvVar4 = (void *)(__n + (int)pvVar4);
        } while (iVar5 != __nmemb * 0x18);
      }
      psVar3[2] = 0;
      *psVar3 = __nmemb;
      free(__src);
      *param_1 = (int)psVar3;
      return 0;
    }
  }
  free(__src);
  free(psVar3);
  return 0xffffffffffffffff;
}



// === setinvent @ 0fb8bde8 (28 bytes) ===

void setinvent(void)

{
  setinvent_r(&DAT_0fbd3aa0);
  return;
}



// === getinvent_r @ 0fb8be04 (60 bytes) ===

int getinvent_r(int *param_1)

{
  int iVar1;
  
  iVar1 = param_1[2];
  if (iVar1 < *param_1) {
    param_1[2] = iVar1 + 1;
    return param_1[1] + iVar1 * 0x18;
  }
  return 0;
}



// === getinvent @ 0fb8be40 (68 bytes) ===

undefined8 getinvent(void)

{
  longlong lVar1;
  undefined8 uVar2;
  
  if (DAT_0fbd3aa0 == 0) {
    lVar1 = setinvent_r(&DAT_0fbd3aa0);
    if (lVar1 != 0) {
      return 0;
    }
  }
  uVar2 = getinvent_r(DAT_0fbd3aa0);
  return uVar2;
}



// === getpgid @ 0fb8be84 (32 bytes) ===

__pid_t getpgid(__pid_t __pid)

{
  __pid_t _Var1;
  
  _Var1 = syssgi(0x40,__pid);
  return _Var1;
}



// === sigaction @ 0fb8bea4 (32 bytes) ===

int sigaction(int __sig,sigaction *__act,sigaction *__oact)

{
  int iVar1;
  
  iVar1 = ksigaction(__sig,__act,__oact,_sigtramp);
  return iVar1;
}



// === _sigtramp @ 0fb8bec4 (76 bytes) ===

undefined8 _sigtramp(ulonglong param_1,undefined8 param_2,undefined8 param_3,code *param_4)

{
  undefined8 uVar1;
  ulonglong uVar2;
  longlong extraout_a3;
  undefined8 uStack_10;
  
  uVar2 = param_1;
  uStack_10 = param_3;
  if ((param_1 & 0xffffffff80000000) != 0) {
    uStack_10 = 0;
    uVar2 = param_1 & ~(param_1 & 0xffffffff80000000);
  }
  (*param_4)(uVar2);
  syscall(0);
  syscall(0);
  if (extraout_a3 != 0) {
    uVar1 = _cerror(uStack_10,param_3,param_1);
    return uVar1;
  }
  return 0x48a;
}



// === ksigaction @ 0fb8bf10 (40 bytes) ===

undefined8 ksigaction(void)

{
  undefined8 uVar1;
  longlong in_a3;
  
  syscall(0);
  if (in_a3 == 0) {
    return 0x48a;
  }
  uVar1 = _cerror();
  return uVar1;
}



// === lseek64 @ 0fb8bf38 (40 bytes) ===

__off64_t lseek64(int __fd,__off64_t __offset,int __whence)

{
  __off64_t _Var1;
  longlong in_a3;
  
  syscall(0);
  if (in_a3 == 0) {
    return 0x3fb;
  }
  _Var1 = _cerror(__fd,__offset,__whence);
  return _Var1;
}



// === ctime @ 0fb8bf60 (36 bytes) ===

char * ctime(time_t *__timer)

{
  tm *__tp;
  char *pcVar1;
  
  __tp = localtime(__timer);
  pcVar1 = asctime(__tp);
  return pcVar1;
}



// === ctime_r @ 0fb8bf84 (52 bytes) ===

char * ctime_r(time_t *__timer,char *__buf)

{
  tm *__tp;
  char *pcVar1;
  tm tStack_50;
  
  __tp = localtime_r(__timer,&tStack_50);
  pcVar1 = asctime_r(__tp,__buf);
  return pcVar1;
}



// === asctime_r @ 0fb8bfb8 (280 bytes) ===

char * asctime_r(tm *__tp,char *__buf)

{
  char cVar1;
  char cVar2;
  char *pcVar4;
  undefined8 uVar3;
  int iVar5;
  char *pcVar6;
  
  cVar1 = DAT_0fbd2a48;
  pcVar6 = &DAT_0fbd2a49;
  *__buf = DAT_0fbd2a48;
  pcVar4 = __buf;
  while (cVar1 != '\0') {
    pcVar4 = pcVar4 + 1;
    cVar1 = *pcVar6;
    pcVar6 = pcVar6 + 1;
    *pcVar4 = cVar1;
  }
  iVar5 = __tp->tm_wday * 3;
  cVar1 = s_SunMonTueWedThuFriSat_0fbd2a68[iVar5 + 1];
  cVar2 = s_SunMonTueWedThuFriSat_0fbd2a68[iVar5 + 2];
  *__buf = s_SunMonTueWedThuFriSat_0fbd2a68[iVar5];
  __buf[1] = cVar1;
  __buf[2] = cVar2;
  iVar5 = __tp->tm_mon * 3;
  cVar1 = s_JanFebMarAprMayJunJulAugSepOctNo_0fbd2a80[iVar5 + 1];
  cVar2 = s_JanFebMarAprMayJunJulAugSepOctNo_0fbd2a80[iVar5 + 2];
  __buf[4] = s_JanFebMarAprMayJunJulAugSepOctNo_0fbd2a80[iVar5];
  __buf[5] = cVar1;
  __buf[6] = cVar2;
  uVar3 = FUN_0fb8c0f0(__buf + 7,__tp->tm_mday);
  uVar3 = FUN_0fb8c0f0(uVar3,__tp->tm_hour + 100);
  uVar3 = FUN_0fb8c0f0(uVar3,__tp->tm_min + 100);
  uVar3 = FUN_0fb8c0f0(uVar3,__tp->tm_sec + 100);
  iVar5 = FUN_0fb8c0f0(uVar3,(__tp->tm_year + 0x76c) / 100);
  FUN_0fb8c0f0(iVar5 + -1,__tp->tm_year + 100);
  return __buf;
}



// === asctime @ 0fb8c0d0 (32 bytes) ===

char * asctime(tm *__tp)

{
  char *pcVar1;
  
  pcVar1 = asctime_r(__tp,&DAT_0fbde508);
  return pcVar1;
}



// === FUN_0fb8c0f0 @ 0fb8c0f0 (96 bytes) ===

void FUN_0fb8c0f0(int param_1,longlong param_2)

{
  if (param_2 < 10) {
    *(undefined1 *)(param_1 + 1) = 0x20;
  }
  else {
    *(char *)(param_1 + 1) = (char)(((int)param_2 / 10) % 10) + '0';
  }
  *(char *)(param_1 + 2) = (char)((int)param_2 % 10) + '0';
  return;
}



// === getuid @ 0fb8c150 (40 bytes) ===

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

__uid_t getuid(void)

{
  __uid_t _Var1;
  longlong in_a3;
  
  syscall(0);
  if (in_a3 == 0) {
    return 0x400;
  }
  _Var1 = _cerror();
  return _Var1;
}



// Total: 664 decompiled, 0 failed
