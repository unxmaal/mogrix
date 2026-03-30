#!/bin/ksh
# Test script with rld verbose
export _RLD_VERBOSE=1
export LD_LIBRARYN32_PATH=/opt/popt-irix-test/opt/mogrix/lib32:/opt/mogrix/lib32
/opt/popt-irix-test/test-popt --help
