#!/bin/ksh
# Test script with rld tracing
export _RLD_ARGS="-trace"
export LD_LIBRARYN32_PATH=/opt/popt-irix-test/usr/sgug/lib32:/usr/sgug/lib32
/opt/popt-irix-test/test-popt --help
