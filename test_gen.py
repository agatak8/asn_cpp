#!/usr/bin/env python
# -*- coding: utf-8 -*-

from random import randint


def TestInt():
    test_int = open("test_int.txt", "w")

    for i in range(0, 10000+1):
        test_int.write(str(randint(-2147483648, 2147483647)) + '\n')
        
    test_int.close()

def TestStr():
    chars = ["a", "b", "c", "d", "e", "f"ghijklmnopqrstuvwwxyzęóąśłżźćń"]
    test_str = open("test_str.txt", "w")
    
    for i in range(0, 10000+1):
        length = randint(3, 128)
        s = ""
        for j in range(0, length+1):
            s += chars[randint(0, len(chars)-1)]
        test_str.write(s + '\n')
    test_str.close()

TestInt()
TestStr()
