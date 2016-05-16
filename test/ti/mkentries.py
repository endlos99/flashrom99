#!/usr/bin/env python

import random


def genImage(filename, progname):
    return """
       text '%-8s'
       data 6, sample_browser
       text '%-20s'
    """ % (filename, progname)


def test1():
    """menu and browser: sorting many different images"""
        
    with open("sample_b.a99", "w") as f:
        l = range(1, 171 + 1) # max. (1..171)
        random.shuffle(l)
    
        for i in l:
            f.write(genImage("PROG" + str(i),
                             "SAMPLE PROGRAM #" + str(i)))
        f.write("data 0, 0, 0, 0, 0, 0, 0\n")  # required for sort to detect end of images

def test2():
    """menu and browser: sorting equal images"""
        
    with open("sample_b.a99", "w") as f:
        l = range(1, 16)
        random.shuffle(l)
    
        for i in l:
            f.write(genImage("FOO" + str(i), "PROGRAM " + str(i)))
            f.write(genImage("BAR" + str(i), "PROGRAM " + str(i)))  # dup name
        f.write("data 0, 0, 0, 0, 0, 0, 0\n")  # required for sort to detect end of images


### select test: #########################################################

test1()
