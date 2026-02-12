#!/bin/sh
prefix=/usr/sgug
exec_prefix=/usr/sgug
libdir=/usr/sgug/lib32
includedir=/usr/sgug/include
case "$1" in
  --version) echo "1.7" ;;
  --cflags) echo "-I${includedir}" ;;
  --libs) echo "-L${libdir} -lnpth -lpthread" ;;
  --api-version) echo "1" ;;
  --host) echo "mips-unknown-irix6.5" ;;
  *) echo "Usage: npth-config [--version|--cflags|--libs|--api-version|--host]" >&2 ;;
esac
