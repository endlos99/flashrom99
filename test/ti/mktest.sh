#!/bin/sh

xas99.py -R -b ../../ti/menu.a99 -I .,../../ti,../../../xdt99/lib -D TEST -o test -L test.lst || exit
mv test_6000 test.bin
../../lib/mkcart.py test.bin
