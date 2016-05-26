#!/usr/bin/env python
# -*- coding: utf-8 -*-

from random import randint


def TestInt():
    test_int = open("test_int.txt", "w")

    for i in range(0, 10000+1):
        test_int.write(str(randint(-2147483648, 2147483647)) + '\n')
        
    test_int.close()

def TestStr():
    chars = ["a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", 
            "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", 
            "y", "z", "ę", "ó", "ą", "ś", "ł", "ż", "ź", "ć", "ń"]
    test_str = open("test_str.txt", "w")
    
    for i in range(0, 10000+1):
        length = randint(3, 30)
        s = ""
        for j in range(0, length+1):
            s += chars[randint(0, len(chars)-1)]
        test_str.write(s + '\n')
    test_str.close()

TestInt()
TestStr()
