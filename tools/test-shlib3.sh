#!/bin/ksh
export LD_LIBRARYN32_PATH=/tmp
export _RLD_VERBOSE=1
/tmp/test-shlib 2>&1
echo "exit code: $?"
