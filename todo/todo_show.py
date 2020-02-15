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
import datetime

def processTodo(filename):

    try:
        f = open(filename, "r")

        today, older, later = getValidData(f)
        res = show(today, older, later)
        print(res)

        f.close()
    except Exception as e:
        print ("Fatal in processTodo: " + str(e))
        raise


def getValidData(f):
    """
    Only process recent 100 records:
     - For todo list, only collect today's items
     - For overdue, it will consider all items older than today in the records
    """

    RECORDS = 100
    res = ""
    lineNo = 1

    today = collections.defaultdict(list)
    later = collections.defaultdict(list)
    older = collections.defaultdict(list)
    td = datetime.date.today()
    data = None

    for i in range(RECORDS):
        line = f.readline()
        if not line:
            break

        line = line.strip()
        ok = re.match("^date:", line)
        if ok:
            dtStr = line[5:].strip('\n').strip(' ')
            dt = datetime.date.fromisoformat(dtStr)

            if dt == td:
                data = today
            elif dt > td:
                data = later
            else:
                data = older
        else:
            ok = re.match("^\- todo:", line)
            if ok is not None:
                data['todo'].append(line)

            ok = re.match("^\- done:", line)
            if ok is not None:
                data['done'].append(line)

    return today, older, later

def show(today, older, later):
    todo = len(today['todo'])
    done = len(today['done'])
    total = todo + done

    overdue = len(older['todo'])

    res = "{}/{}".format(done, total)

    if overdue > 0:
      res += " overdue({})".format(overdue)

    if total == 0:
        return res

    res += ": "
    res += putInto1Line(today['todo'])
    res += putInto1Line(today['done'], True)

    return res

def putInto1Line(data, strike = False):
    res = ""
    id = 1

    for content in data:
        content = content[7:].strip('\n').strip(' ')

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
