#!/usr/bin/env python3

import sys
import os
import getopt
import string
import pprint
import yaml
import subprocess
import re
import collections

def processTodo(filename):

    try:
        f = open(filename, "r")

        data = getValidData(f)
        show(data)

        f.close()
    except Exception as e:
        print ("Fatal in processTodo: " + str(e))
        raise


# Only process recent 100 records
def getValidData(f):
    res = ""
    lineNo = 1

    data = collections.defaultdict(list)

    for i in range(100):
        line = f.readline()
        if not line:
            break

        ok = re.match("^todo:", line)
        if ok is not None:
            data['todo'].append(line)

        ok = re.match("^done:", line)
        if ok is not None:
            data['done'].append(line)

    return data

def show(data):
    todo = len(data['todo'])
    done = len(data['done'])
    total = todo + done

    res = "{}/{}: ".format(done, total)
    res += putInto1Line(data['todo'])
    res += putInto1Line(data['done'], True)

    print(res)

def putInto1Line(data, strike = False):
    res = ""
    id = 1

    for content in data:
        content = content[5:].strip('\n').strip(' ')

        content = "({}) {}".format(id, content)
        id += 1

        res += content + " "

    if strike:
        res = _strike(res)

    return res

def _strike(text):
    result = ''

    for c in text:
        result += c + '\u0336'

    return result

def usage():
    print ("usage:")
    print ("  todo_show.py")

if __name__ == "__main__":
    try:
        # 1. parse the args
        opts, args = getopt.getopt(sys.argv[1:], 'hf:')

        filename = "/home/final/TODO"

        for op, value in opts:
            print("op: {}, val: {}".format(op, value))
            if op == "-h":
                usage()
            elif opt == "-f":
                filename = value

        processTodo(filename)

    except Exception as e:
        print ("Fatal: " + str(e))
        raise
