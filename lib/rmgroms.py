#!/usr/bin/env python

import sys
import os


fns = sys.argv[1:]
if not fns or fns[0] != "--confirm":
    print "rmgrom.s -- remove images with GROMs in current directory"
    print "usage: %s --confirm" % sys.argv[0]
    sys.exit(0)

for fg in os.listdir('.'):
    if fg[-5:].upper() != "G.BIN":
        continue

    fc = fg[:-5] + (
        "c.bin" if fg[-1] < 'a' else "C.BIN"
    )
    fd = fg[:-5] + (
        "d.bin" if fg[-1] < 'a' else "D.BIN"
    )
    
    try:
        os.remove(fc);
    except:
        pass

    try:
        os.remove(fd);
    except:
        pass

    os.remove(fg);
