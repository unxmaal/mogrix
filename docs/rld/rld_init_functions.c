// === map_object_into_mem_and_init_object_info (addr: 0fb6d470, size: 2464 bytes) ===

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



// === execute_all_init_sections (addr: 0fb68d90, size: 244 bytes) ===

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



// === obj_init (addr: 0fb627a0, size: 1408 bytes) ===

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



