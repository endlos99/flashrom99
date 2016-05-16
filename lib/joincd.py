#!/usr/bin/env python

import sys
import os

fns = sys.argv[1:]
if not fns or fns[0] != "--confirm":
    print "joincd.py -- replace split multi-file images in current dir by single image"
    print "usage: %s --confirm" % sys.argv[0]
    sys.exit(0)

for fnd in os.listdir(fns[0]):
    if fnd[-5:].upper() != "D.BIN":
        continue
    fnc = fnd[:-5] + (
        "C.BIN" if fnd[-1] < 'a' else "c.bin"
    )
    fn = fnd[:-5] + (
        ".BIN" if fnd[-1] < 'a' else ".bin"
    )

    try:
        with open(fnc, "rb") as f:
            datac = f.read()
    except:
        continue
    try:
        with open(fnd, "rb") as f:
            datad = f.read()
    except:
        continue

    with open(fn, "wb") as f:
        f.write(datac)
        f.write(datad)
    
    os.remove(fnc);
    os.remove(fnd);
