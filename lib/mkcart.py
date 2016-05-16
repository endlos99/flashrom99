#!/usr/bin/env python

import sys
import os.path
import zipfile


fns = sys.argv[1:]
if not fns:
    print "mkcart.py -- MESS .rpk creator for 8K images"
    print "usage: %s <file> ..." % sys.argv[0]
    sys.exit(0)

    
for fn in fns:
    try:
        with open(fn, "rb") as fin:
            data = fin.read()
    except IOError as e:
        print "Error: %s, skipping" % e
        continue

    layout="""<?xml version="1.0" encoding="utf-8"?>
<romset version="0.07">
   <resources>
      <rom id="romimage" file="%s"/>
   </resources>
   <configuration>
      <pcb type="standard">
         <socket id="rom_socket" uses="romimage"/>
      </pcb>
   </configuration>
</romset>
""" % fn

    basename = os.path.basename(fn)
    name, ext = os.path.splitext(basename)

    try:
        with zipfile.ZipFile(name + ".rpk", "w") as archive:
            archive.writestr(fn, data[:8192])
            archive.writestr("layout.xml", layout)
    except IOError as e:
        print "Error: %s, skipping" % e
