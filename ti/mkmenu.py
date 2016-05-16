#!/usr/bin/env python

with open("menu_6000.bin", "rb") as f:
    data = f.read()
    
with open("menu.c", "w") as f:
    f.write("const uint8_t menu[] PROGMEM = {\n");
    for b in data[:-1]:
        f.write(hex(ord(b)) + ",\n");
    f.write(hex(ord(data[-1])) + "};\n");
