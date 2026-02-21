// === CALLERS of resolve_relocations ===

// Callers: [lazy_text_resolve, FUN_0fb68ef0, FUN_0fb663a0, FUN_0fb69380, FUN_0fb66100]

// Skipping lazy_text_resolve (already decompiled)

// === FUN_0fb68ef0 (addr: 0fb68ef0, size: 1132 bytes) ===

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



// === FUN_0fb663a0 (addr: 0fb663a0, size: 2016 bytes) ===

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



// === FUN_0fb69380 (addr: 0fb69380, size: 472 bytes) ===

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



// === FUN_0fb66100 (addr: 0fb66100, size: 648 bytes) ===

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




// === LOAD-TIME RELOCATION FUNCTIONS ===


// === ALL FUNCTIONS matching reloc/resolve/init/load ===

// _rld_text_resolve @ 0fb602ac (256 bytes)
// init_bridge @ 0fb60454 (160 bytes)
// obj_init @ 0fb627a0 (1408 bytes)
// get_dynsym_hash_value @ 0fb64310 (128 bytes)
// obj_dynsym_got @ 0fb65550 (304 bytes)
// obj_set_dynsym_got @ 0fb65680 (48 bytes)
// lazy_text_resolve @ 0fb659b0 (1628 bytes)
// resolve_symbol @ 0fb66b90 (1788 bytes)
// is_symbol_in_got @ 0fb67890 (64 bytes)
// resolve_relocations @ 0fb67de0 (1188 bytes)
// ascending_init_order @ 0fb68920 (60 bytes)
// call_pixie_init @ 0fb68cf0 (156 bytes)
// execute_all_init_sections @ 0fb68d90 (244 bytes)
// load_dependent_libs @ 0fb6a3a0 (740 bytes)
// free_up_msym_gotinfo_llmap @ 0fb6a690 (152 bytes)
// unset_undefineds_and_reresolve @ 0fb6a7a0 (72 bytes)
// unset_undefineds_are_resolved_if_has_unres_undefs @ 0fb6b260 (116 bytes)
// unset_undefineds_are_resolved @ 0fb6b2e0 (108 bytes)
// unset_undef_are_resolved_for_dlopen_obj @ 0fb6b350 (112 bytes)
// set_obj_initial_bit @ 0fb6b4a0 (68 bytes)
// find_reloc @ 0fb6b6c0 (708 bytes)
// map_object_into_mem_and_init_object_info @ 0fb6d470 (2464 bytes)
// load_single_object @ 0fb6e310 (704 bytes)
// init_signal_mask @ 0fb6f220 (60 bytes)
// hashinit @ 0fb75010 (196 bytes)
// __rld_page_load_object @ 0fb78710 (1028 bytes)
// init_lock @ 0fb7c00c (12 bytes)
// migr_policy_args_init @ 0fb7ca80 (152 bytes)
// _seminit @ 0fb7d504 (908 bytes)
// usinitsema @ 0fb7e4a8 (244 bytes)
// _usinit_mapfile @ 0fb80810 (864 bytes)
// usinit @ 0fb80b74 (2324 bytes)
// _blk_init @ 0fb875b4 (56 bytes)
// __initperthread_errno @ 0fb87644 (16 bytes)
// _sinitlock @ 0fb88410 (140 bytes)
// _usr4klocks_init @ 0fb893a8 (244 bytes)
