#!/usr/bin/python
# -*- coding: utf-8 -*-
# make_unicode_fold_data.py
# Copyright (c) 2016-2019  K.Kosako

import sys
import re

SOURCE_FILE = 'CaseFolding.txt'
GPERF_UNFOLD_KEY_FILE = 'unicode_unfold_key.gperf'
GPERF_FOLD_KEY_FILES  = ['unicode_fold1_key.gperf', 'unicode_fold2_key.gperf', 'unicode_fold3_key.gperf']


DataName = 'OnigUnicodeFolds'

ENCODING = 'utf-8'

LINE_REG = re.compile("([0-9A-F]{1,6}); (.); ([0-9A-F]{1,6})(?: ([0-9A-F]{1,6}))?(?: ([0-9A-F]{1,6}))?;(?:\s*#\s*)(.*)")
VERSION_REG  = re.compile("#.*-(\d+)\.(\d+)\.(\d+)\.txt")

VERSION_INFO = [-1, -1, -1]

FOLDS = {}
TURKISH_FOLDS = {}
LOCALE_FOLDS  = {}

UNFOLDS = {}
TURKISH_UNFOLDS = {}
LOCALE_UNFOLDS  = {}

class Entry:
    def __init__(self, fold):
        self.fold = fold
        self.unfolds = []
        self.fold_len = len(fold)
        self.index = -1
        self.comment = None

def fold_key(fold):
    sfold = map(lambda i: "%06x" % i, fold)
    return ':'.join(sfold)

def form16(x, size):
    form = "0x%06x" if x > 0xffff else "0x%04x"
    s = form % x
    rem = size - len(s)
    if rem > 0:
        s = ' ' * rem + s

    return s

def form3bytes(x):
    x0 = x & 0xff
    x1 = (x>>8)  & 0xff
    x2 = (x>>16) & 0xff
    return "\\x%02x\\x%02x\\x%02x" % (x2, x1, x0)

def check_version_info(s):
  m = VERSION_REG.match(s)
  if m is not None:
    VERSION_INFO[0] = int(m.group(1))
    VERSION_INFO[1] = int(m.group(2))
    VERSION_INFO[2] = int(m.group(3))

def parse_line(s):
    if len(s) == 0:
      return False
    if s[0] == '#':
      if VERSION_INFO[0] < 0:
        check_version_info(s)
      return False

    m = LINE_REG.match(s)
    if m is None:
        print >> sys.stderr, s.encode(ENCODING)
        sys.exit(-1)

    s_unfold = m.group(1)
    s_type   = m.group(2)
    s_fold   = m.group(3)
    comment  = m.group(6)

    if s_type == 'S':
        return False;

    unfold = int(s_unfold, 16)
    f1     = int(s_fold, 16)
    fold = [f1]
    if m.group(4) is not None:
        f2 = int(m.group(4), 16)
        fold.append(f2)
        if m.group(5) is not None:
            f3 = int(m.group(5), 16)
            fold.append(f3)

    if s_type == 'T':
        dic   = TURKISH_FOLDS
        undic = TURKISH_UNFOLDS
    else:
        dic   = FOLDS
        undic = UNFOLDS

    key = fold_key(fold)
    e = dic.get(key, None)
    if e is None:
        e = Entry(fold)
        e.comment = comment
        dic[key] = e

    e.unfolds.append(unfold)

    if undic.get(unfold, None) is not None:
        print >> sys.stderr, ("unfold dup: 0x%04x %s\n" % (unfold, s_type))
    undic[unfold] = e

    return True

def parse_file(f):
    line = f.readline()
    while line:
        s = line.strip()
        parse_line(s)
        line = f.readline()

def make_locale():
    for unfold, te in TURKISH_UNFOLDS.items():
        e = UNFOLDS.get(unfold, None)
        if e is None:
            continue

        fkey = fold_key(e.fold)
        if len(e.unfolds) == 1:
            del FOLDS[fkey]
        else:
            e.unfolds.remove(unfold)
            e = Entry(e.fold)
            e.unfolds.append(unfold)

        LOCALE_FOLDS[fkey] = e
        LOCALE_UNFOLDS[unfold] = e
        del UNFOLDS[unfold]

def output_typedef(f):
    s = """\
typedef unsigned long OnigCodePoint;
"""
    print >> f, s

def divide_by_fold_len(d):
    l = d.items()
    l1 = filter(lambda (k,e):e.fold_len == 1, l)
    l2 = filter(lambda (k,e):e.fold_len == 2, l)
    l3 = filter(lambda (k,e):e.fold_len == 3, l)
    sl1 = sorted(l1, key=lambda (k,e):k)
    sl2 = sorted(l2, key=lambda (k,e):k)
    sl3 = sorted(l3, key=lambda (k,e):k)
    return (sl1, sl2, sl3)

def output_comment(f, s):
    f.write(" /* %s */" % s)

def output_data_n1(f, n, fn, c, out_comment):
    for k, e in fn:
        e.index = c
        if out_comment and n > 1 and e.comment is not None:
            output_comment(f, e.comment)
            print >> f, ''

        f.write(' ')
        f.write("/*%4d*/ " % c)
        for i in range(0, n):
            s = form16(e.fold[i], 8)
            f.write(" %s," % s)

        usize = len(e.unfolds)
        f.write("  %d," % usize)
        for u in e.unfolds:
            s = form16(u, 8)
            f.write(" %s," % s)

        if out_comment and n == 1 and e.comment is not None:
            if len(e.comment) < 35:
                s = e.comment
            else:
                s = e.comment[0:33] + '..'

            output_comment(f, s)

        f.write("\n")
        c += n + 1 + usize

    return c

def output_data_n(f, name, n, fn, lfn, out_comment):
    print >> f, "OnigCodePoint %s%d[] = {" % (name, n)
    c = 0
    c = output_data_n1(f, n,  fn, c, out_comment)
    print >> f, "#define FOLDS%d_NORMAL_END_INDEX   %d" % (n, c)
    print >> f, " /* ----- LOCALE ----- */"
    c = output_data_n1(f, n, lfn, c, out_comment)
    print >> f, "#define FOLDS%d_END_INDEX   %d" % (n, c)
    print >> f, "};"

def output_fold_data(f, name, out_comment):
    f1, f2, f3 = divide_by_fold_len(FOLDS)
    lf1, lf2, lf3 = divide_by_fold_len(LOCALE_FOLDS)

    output_data_n(f, name, 1, f1, lf1, out_comment)
    print >> f, ''
    output_data_n(f, name, 2, f2, lf2, out_comment)
    print >> f, ''
    output_data_n(f, name, 3, f3, lf3, out_comment)
    print >> f, ''

def output_macros(f, name):
    print >> f, "#define FOLDS1_FOLD(i)         (%s1 + (i))" % name
    print >> f, "#define FOLDS2_FOLD(i)         (%s2 + (i))" % name
    print >> f, "#define FOLDS3_FOLD(i)         (%s3 + (i))" % name

    print >> f, "#define FOLDS1_UNFOLDS_NUM(i)  %s1[(i)+1]" % name
    print >> f, "#define FOLDS2_UNFOLDS_NUM(i)  %s2[(i)+2]" % name
    print >> f, "#define FOLDS3_UNFOLDS_NUM(i)  %s3[(i)+3]" % name

    print >> f, "#define FOLDS1_UNFOLDS(i)      (%s1 + (i) + 2)" % name
    print >> f, "#define FOLDS2_UNFOLDS(i)      (%s2 + (i) + 3)" % name
    print >> f, "#define FOLDS3_UNFOLDS(i)      (%s3 + (i) + 4)" % name

    print >> f, "#define FOLDS1_NEXT_INDEX(i)   ((i) + 2 + %s1[(i)+1])" % name
    print >> f, "#define FOLDS2_NEXT_INDEX(i)   ((i) + 3 + %s1[(i)+2])" % name
    print >> f, "#define FOLDS3_NEXT_INDEX(i)   ((i) + 4 + %s1[(i)+3])" % name

def output_fold_source(f, out_comment):
    print >> f, "/* This file was generated by make_unicode_fold_data.py. */"
    print >> f, '#include "regenc.h"'
    print >> f, ''
    if VERSION_INFO[0] < 0:
      raise RuntimeError("Version is not found")

    print "#define UNICODE_CASEFOLD_VERSION  %02d%02d%02d" % (VERSION_INFO[0], VERSION_INFO[1], VERSION_INFO[2])
    print ''
    #output_macros(f, DataName)
    print >> f, ''
    #output_typedef(f)
    output_fold_data(f, DataName, out_comment)


HEAD = '''
%{
/* This gperf source file was generated by make_unicode_fold_data.py */

/*-
 * Copyright (c) 2017-2019  K.Kosako
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <string.h>
#include "regenc.h"
%}
'''.strip()

def output_gperf_unfold_key(f):
    head = HEAD + """\

struct ByUnfoldKey {
  OnigCodePoint code;
  short int   index;
  short int   fold_len;
};
%%
"""
    f.write(head)
    UNFOLDS.update(LOCALE_UNFOLDS)
    l = UNFOLDS.items()
    sl = sorted(l, key=lambda (k,e):(e.fold_len, e.index))
    for k, e in sl:
        f.write('"%s", /*0x%04x*/ %4d, %d\n' %
                (form3bytes(k), k, e.index, e.fold_len))

    print >> f, '%%'

def output_gperf_fold_key(f, key_len):
    head = HEAD + """\

short int
%%
"""
    f.write(head)
    l = FOLDS.items()
    l = filter(lambda (k,e):e.fold_len == key_len, l)
    sl = sorted(l, key=lambda (k,e):e.index)
    for k, e in sl:
        skey = ''.join(map(lambda i: form3bytes(i), e.fold))
        f.write('"%s", %4d\n' % (skey, e.index))

    print >> f, '%%'

def output_gperf_source():
   with open(GPERF_UNFOLD_KEY_FILE, 'w') as f:
       output_gperf_unfold_key(f)

   FOLDS.update(LOCALE_FOLDS)

   for i in range(1, 4):
       with open(GPERF_FOLD_KEY_FILES[i-1], 'w') as f:
           output_gperf_fold_key(f, i)


## main ##
with open(SOURCE_FILE, 'r') as f:
    parse_file(f)

make_locale()

out_comment = True
output_fold_source(sys.stdout, out_comment)

output_gperf_source()
