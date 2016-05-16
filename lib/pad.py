#!/usr/bin/env python

import sys
import os.path


fns = sys.argv[1:]
if not fns:
    print "pad.py -- pad image file to multiple of 8K"
    print "usage: %s <file> ..." % sys.argv[0]
    sys.exit(0)

for fn in fns:
    # read input file and pad to multiple of 8K
    try:
        with open(fn, "rb") as f:
            size = len(f.read())
        with open(fn, "ab") as f:
            f.write("\xff" * (-size % 8192))
    except:
        pass
