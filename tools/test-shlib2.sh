#!/bin/ksh
export LD_LIBRARYN32_PATH=/tmp
/tmp/test-shlib
RC=$?
echo "exit code: $RC"
if [ $RC -eq 139 ]; then
    echo "SEGFAULT"
fi
