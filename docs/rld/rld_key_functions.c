// === resolve_relocations (addr: 0fb67de0, size: 1188 bytes) ===

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



// === find_reloc (addr: 0fb6b6c0, size: 708 bytes) ===

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



// === resolve_symbol (addr: 0fb66b90, size: 1788 bytes) ===

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



// === obj_dynsym_got (addr: 0fb65550, size: 304 bytes) ===

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



// === is_symbol_in_got (addr: 0fb67890, size: 64 bytes) ===

undefined8 is_symbol_in_got(int param_1,uint param_2)

{
  if ((*(byte *)(*(int *)(param_1 + 100) + param_2 * 0x10 + 0xc) >> 4 == 1) &&
     (*(uint *)(param_1 + 0xf4) <= param_2)) {
    return 1;
  }
  return 0;
}



// === lazy_text_resolve (addr: 0fb659b0, size: 1628 bytes) ===

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



// === get_dynsym_hash_value (addr: 0fb64310, size: 128 bytes) ===

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



// Function process_relocation not found

// Function relocate_objs not found

// Function do_reloc not found

