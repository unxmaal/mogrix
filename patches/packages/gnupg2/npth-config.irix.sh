#!/bin/sh
prefix=/opt/mogrix
exec_prefix=/opt/mogrix
libdir=/opt/mogrix/lib32
includedir=/opt/mogrix/include
case "$1" in
  --version) echo "1.7" ;;
  --cflags) echo "-I${includedir}" ;;
  --libs) echo "-L${libdir} -lnpth -lpthread" ;;
  --api-version) echo "1" ;;
  --host) echo "mips-unknown-irix6.5" ;;
  *) echo "Usage: npth-config [--version|--cflags|--libs|--api-version|--host]" >&2 ;;
esac
